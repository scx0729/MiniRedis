#include "miniredis/server/server.h"
#include "miniredis/persistence/aof_replay.h"
#include "miniredis/resp/encoder.h"
#include "miniredis/resp/value.h"
#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>

namespace miniredis {

std::atomic<bool> Server::signal_received_{false};

void Server::SignalHandler(int /*sig*/) {
    signal_received_.store(true);
}

Server::Server(const Config& config) : config_(config) {}

Server::~Server() {
    CloseAllConnections();
    if (listen_fd_ >= 0) {
        close(listen_fd_);
    }
}

bool Server::Start() {
    // Set up signal handlers
    struct sigaction sa{};
    sa.sa_handler = SignalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);
    // Ignore SIGPIPE
    signal(SIGPIPE, SIG_IGN);

    // Create database
    expiration_ = std::make_unique<Expiration>(clock_);
    db_ = std::make_unique<Database>(clock_, expiration_.get());
    registry_ = std::make_unique<CommandRegistry>();

    // Create AOF writer if enabled
    if (config_.appendonly) {
        aof_ = std::make_unique<AofWriter>(config_.appendfilename, config_.fsync_always);

        // Replay AOF if file exists
        auto replay_result = AofReplay::Replay(config_.appendfilename, *db_, *registry_);
        if (replay_result.ok() && replay_result.value() > 0) {
            // Log replay info
        }
    }

    cmd_ctx_ = CommandContext{db_.get(), aof_.get()};

    // Create listen socket
    if (!CreateListenSocket()) {
        return false;
    }

    return true;
}

bool Server::CreateListenSocket() {
    listen_fd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
    if (listen_fd_ < 0) {
        return false;
    }

    // SO_REUSEADDR
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

    // Register listen socket with epoll
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
        [this](int /*fd*/) { AcceptConnection(); },
        [this](Connection& conn) { OnReadable(conn.Fd()); },
        [this](Connection& conn) { OnWritable(conn.Fd()); },
        [this](int fd) { CloseConnection(fd); }
    );
}

void Server::Shutdown() {
    stop_.store(true);
    event_loop_.Stop();
}

void Server::AcceptConnection() {
    while (true) {
        int fd = accept4(listen_fd_, nullptr, nullptr,
                         SOCK_NONBLOCK | SOCK_CLOEXEC);
        if (fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            if (errno == EINTR) continue;
            break;
        }

        // Set TCP_NODELAY
        int opt = 1;
        setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt));

        auto conn = std::make_unique<Connection>(fd);
        auto session = std::make_unique<Session>(*conn, *db_, *registry_, cmd_ctx_);

        // Register with epoll
        event_loop_.AddFd(fd, EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP, conn.get());

        connections_[fd] = {std::move(conn), std::move(session)};
    }
}

void Server::OnReadable(int fd) {
    auto it = connections_.find(fd);
    if (it == connections_.end()) return;

    auto& state = it->second;
    if (!state.conn->Read()) {
        CloseConnection(fd);
        return;
    }

    if (!state.session->ProcessInput()) {
        // Connection should be closed (protocol error or QUIT)
        // But first, try to send any queued output
        if (state.conn->HasOutput()) {
            // Register for write events to flush remaining output
            event_loop_.ModFd(fd, EPOLLOUT | EPOLLRDHUP | EPOLLERR | EPOLLHUP,
                              state.conn.get());
            state.conn->MarkClosing();
        } else {
            CloseConnection(fd);
        }
        return;
    }

    // If we have output to send, register for write events
    if (state.conn->HasOutput()) {
        event_loop_.ModFd(fd, EPOLLIN | EPOLLOUT | EPOLLRDHUP | EPOLLERR | EPOLLHUP,
                          state.conn.get());
    }
}

void Server::OnWritable(int fd) {
    auto it = connections_.find(fd);
    if (it == connections_.end()) return;

    auto& state = it->second;
    if (!state.conn->Write()) {
        CloseConnection(fd);
        return;
    }

    // If all output sent
    if (!state.conn->HasOutput()) {
        if (state.conn->IsClosing()) {
            CloseConnection(fd);
            return;
        }
        // Unregister write events
        event_loop_.ModFd(fd, EPOLLIN | EPOLLRDHUP | EPOLLERR | EPOLLHUP,
                          state.conn.get());
    }
}

void Server::CloseConnection(int fd) {
    auto it = connections_.find(fd);
    if (it == connections_.end()) return;

    event_loop_.RemoveFd(fd);
    connections_.erase(it);
}

void Server::CloseAllConnections() {
    for (auto& [fd, state] : connections_) {
        event_loop_.RemoveFd(fd);
    }
    connections_.clear();
}

void Server::PerformActiveExpire() {
    if (expiration_) {
        db_->ExpireSample(20);
    }
}

}  // namespace miniredis
