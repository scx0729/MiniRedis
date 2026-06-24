#pragma once

#include "event_loop.h"
#include "connection.h"
#include "session.h"
#include "miniredis/command/command.h"
#include "miniredis/command/registry.h"
#include "miniredis/config/config.h"
#include "miniredis/persistence/aof.h"
#include "miniredis/storage/database.h"
#include "miniredis/storage/expiration.h"
#include "miniredis/base/clock.h"
#include <atomic>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>

namespace miniredis {

// Top-level server class
class Server {
public:
    explicit Server(const Config& config);
    ~Server();

    // Non-copyable
    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    // Initialize and start the server
    bool Start();

    // Run the event loop (blocks until shutdown signal)
    void Run();

    // Request shutdown
    void Shutdown();

    // Check if shutdown has been requested
    bool IsShutdown() const { return stop_.load(); }

private:
    bool CreateListenSocket();
    void AcceptConnection();
    void OnReadable(int fd);
    void OnWritable(int fd);
    void CloseConnection(int fd);
    void CloseAllConnections();
    void PerformActiveExpire();
    void OnLoopTick();

    Config config_;
    SystemClock clock_;
    std::unique_ptr<Database> db_;
    std::unique_ptr<Expiration> expiration_;
    std::unique_ptr<CommandRegistry> registry_;
    std::unique_ptr<AofWriter> aof_;
    CommandContext cmd_ctx_{nullptr, nullptr};

    int listen_fd_ = -1;
    EventLoop event_loop_;
    std::atomic<bool> stop_{false};
    int64_t last_active_expire_ms_ = 0;

    // Connection management
    struct ConnState {
        std::unique_ptr<Connection> conn;
        std::unique_ptr<Session> session;
    };
    std::unordered_map<int, ConnState> connections_;

    // Signal handling
    static std::atomic<bool> signal_received_;
    static void SignalHandler(int sig);
};

}  // namespace miniredis
