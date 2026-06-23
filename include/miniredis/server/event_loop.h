#pragma once

#include "connection.h"
#include "miniredis/server/session.h"
#include "miniredis/command/command.h"
#include "miniredis/command/registry.h"
#include "miniredis/storage/database.h"
#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <sys/epoll.h>
#include <unordered_map>

namespace miniredis {

class Server;

// epoll-based event loop
class EventLoop {
public:
    EventLoop();
    ~EventLoop();

    // Non-copyable
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(const EventLoop&) = delete;

    // Add a file descriptor to epoll
    bool AddFd(int fd, uint32_t events, void* user_data);

    // Modify events for a file descriptor
    bool ModFd(int fd, uint32_t events, void* user_data);

    // Remove a file descriptor from epoll
    bool RemoveFd(int fd);

    // Run the event loop. Calls the handler for each event.
    // Returns when stop_ is set or error.
    void Run(int listen_fd, std::atomic<bool>& stop,
             std::function<void(int listen_fd)> on_accept,
             std::function<void(Connection& conn)> on_readable,
             std::function<void(Connection& conn)> on_writable,
             std::function<void(int fd)> on_close);

    // Stop the event loop
    void Stop();

private:
    int epoll_fd_;
    static constexpr int kMaxEvents = 1024;
    struct epoll_event events_[kMaxEvents];
};

}  // namespace miniredis
