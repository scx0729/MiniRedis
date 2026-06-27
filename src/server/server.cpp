// TODO(Task 6, Task 7, Task 8): implement the TCP server lifecycle.
#include "miniredis/server/server.h"
#include "miniredis/command/command.h"
#include "miniredis/command/registry.h"
#include "miniredis/persistence/aof_replay.h"  // AOF 启动恢复
#include "miniredis/resp/encoder.h"
#include "miniredis/resp/value.h"
#include "miniredis/storage/database.h"
#include "miniredis/storage/expiration.h"
#include <arpa/inet.h>      // inet_addr
#include <cerrno>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <memory>
#include <netinet/in.h>     // sockaddr_in
#include <netinet/tcp.h>    // TCP_NODELAY
#include <signal.h>         // sigaction
#include <sys/epoll.h>
#include <sys/socket.h>     // socket, bind, listen, accept4
#include <unistd.h>         // close

namespace miniredis {

std::atomic<bool> Server::signal_received_{false};

void Server::SignalHandler(int) {
    signal_received_.store(true);
}

Server::Server(const Config& config) : config_(config) {}

Server::~Server() {
    CloseAllConnections();
    if (listen_fd_ >= 0) {
        ::close(listen_fd_);
        listen_fd_ = -1;
    }
}

bool Server::Start() {
    stop_ = false;
    signal_received_.store(false);
    struct sigaction sa{};
    sa.sa_handler = SignalHandler;
    // 清空信号掩码，表示处理信号期间不阻塞其他信号
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
    signal(SIGPIPE, SIG_IGN);
    // 创建存储层
    expiration_ = std::make_unique<Expiration>(clock_);
    db_ = std::make_unique<Database>(clock_, expiration_.get());
    registry_ = std::make_unique<CommandRegistry>();
    cmd_ctx_ = CommandContext{db_.get(), aof_.get()};
    // TODO(Task 8): create AOF writer and replay append-only file on startup.
    if(!CreateListenSocket()){
        return false;
    }
    return true;
}

bool Server::CreateListenSocket() {
    listen_fd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (listen_fd_ < 0) {
        return false;
    }

    int opt = 1;
    setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(config_.port);
    addr.sin_addr.s_addr = inet_addr(config_.bind_address.c_str());

    if (bind(listen_fd_, reinterpret_cast<struct sockaddr*>(&addr), sizeof(addr)) < 0) {
        close(listen_fd_);
        listen_fd_ = -1;
        return false;
    }

    if (listen(listen_fd_, SOMAXCONN) < 0) {
        close(listen_fd_);
        listen_fd_ = -1;
        return false;
    }

    if (!event_loop_.AddFd(listen_fd_, EPOLLIN, nullptr)) {
        close(listen_fd_);
        listen_fd_ = -1;
        return false;
    }

    return true;
}

void Server::Run() {
    event_loop_.Run(
        listen_fd_, stop_,
        [this](int)         { AcceptConnection(); },
        [this](Connection& conn) { OnReadable(conn.Fd()); }, 
        [this](Connection& conn) { OnWritable(conn.Fd()); }, 
        [this](int fd) { CloseConnection(fd); },
        [this]() { OnLoopTick(); });
}

void Server::Shutdown() {
    stop_.store(true);
    event_loop_.Stop();
}

void Server::AcceptConnection() {
    while (true) {
        int fd = accept4(listen_fd_, nullptr, nullptr, SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break; // 非阻塞，队列空
            if (errno == EINTR) continue;                       // 被其他信号中断
            break;
        }
        // 禁用 Nagle 算法，减少延迟
        int opt = 1;
        setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));
        auto conn = std::make_unique<Connection>(fd, config_.max_client_input_buffer, config_.max_client_output_buffer);
        ParserLimits parser_limits;
        parser_limits.max_bulk_length = config_.max_bulk_length;
        auto session = std::make_unique<Session>(*conn, *db_, *registry_, cmd_ctx_, parser_limits);
        if (!event_loop_.AddFd(fd, EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP, conn.get())) {
            continue;
        }
        connections_[fd] = {std::move(conn), std::move(session)};
    }
}

void Server::OnReadable(int fd) {
    auto it = connections_.find(fd);
    if (it == connections_.end()) return;
    auto& state = it->second;
    // 从 socket 读数据到输入缓冲区
    if (!state.conn->Read()) {
        // 连接关闭或错误
        if (state.conn->HasOutput()) {
            // 还有数据没发完，标记 closing，切到只监听写事件
            state.conn->MarkClosing();
            event_loop_.ModFd(fd, EPOLLOUT | EPOLLRDHUP | EPOLLERR | EPOLLHUP, state.conn.get());
        } else {
            CloseConnection(fd);
        }
        return;
    }
    // 解析命令、执行、编码响应到输出缓冲区
    if (!state.session->ProcessInput()) {
        // 协议错误或 QUIT
        if (state.conn->HasOutput()) {
            state.conn->MarkClosing();
            event_loop_.ModFd(fd, EPOLLOUT | EPOLLRDHUP | EPOLLERR | EPOLLHUP, state.conn.get());
        } else {
            CloseConnection(fd);
        }
        return;
    }
    // 有输出要发，注册 EPOLLOUT
    if (state.conn->HasOutput()) {
        event_loop_.ModFd(fd, EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLERR | EPOLLHUP, state.conn.get());
    } else if (state.conn->IsClosing()) {
        CloseConnection(fd);
    }
}

void Server::OnWritable([[maybe_unused]] int fd) {
    auto it = connections_.find(fd);
    if(it == connections_.end()) return;
    // 发送输出缓冲区中的数据
    auto& state = it->second;
    if(!state.conn->Write()){
        // 写失败，关闭连接
        CloseConnection(fd);
        return;
    }
    // 所有数据发完
    if(!state.conn->HasOutput()){
        // 发完的数据是QUIT或者读错误后的残留数据，关闭连接
        if(state.conn->IsClosing()){
            CloseConnection(fd);
            return;
        }
        // 没有更多输出，取消EPOLLOUT，避免空转
        event_loop_.ModFd(fd, EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP, state.conn.get());
    }
}

void Server::CloseConnection([[maybe_unused]] int fd) {
    auto it = connections_.find(fd);
    if(it == connections_.end()) { return; }
    event_loop_.RemoveFd(fd);
    connections_.erase(it);
}

void Server::CloseAllConnections() {
    for(auto &[fd, state] : connections_){
        event_loop_.RemoveFd(fd);
    }
    connections_.clear();
}

void Server::PerformActiveExpire() {
    // TODO(Task 7): run a bounded active expiration sample.
}

void Server::OnLoopTick() {
    // TODO(Task 7): drive active expiration periodically.
    if(signal_received_.load()){
        Shutdown();
        return;
    }
}

}  // namespace miniredis
