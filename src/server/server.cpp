// TODO(Task 6, Task 7, Task 8): implement the TCP server lifecycle.
#include "miniredis/server/server.h"
#include <unistd.h>

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
    // TODO(Task 6): initialize signal handlers, database, registry, and listen socket.
    // TODO(Task 8): create AOF writer and replay append-only file on startup.
    return false;
}

bool Server::CreateListenSocket() {
    // TODO(Task 6): socket(), bind(), listen(), and register listen_fd_ with EventLoop.
    return false;
}

void Server::Run() {
    // TODO(Task 6): call event_loop_.Run() with server callbacks.
}

void Server::Shutdown() {
    stop_.store(true);
    event_loop_.Stop();
}

void Server::AcceptConnection() {
    // TODO(Task 6): accept4() clients, create Connection + Session, register fd.
}

void Server::OnReadable(int) {
    // TODO(Task 6): read, process session input, and enable write notifications.
}

void Server::OnWritable(int) {
    // TODO(Task 6): flush output and close when needed.
}

void Server::CloseConnection(int) {
    // TODO(Task 6): remove fd from epoll and erase connection state.
}

void Server::CloseAllConnections() {
    // TODO(Task 6): close all active connections.
    connections_.clear();
}

void Server::PerformActiveExpire() {
    // TODO(Task 7): run a bounded active expiration sample.
}

void Server::OnLoopTick() {
    // TODO(Task 6): handle signals. TODO(Task 7): drive active expiration periodically.
}

}  // namespace miniredis
