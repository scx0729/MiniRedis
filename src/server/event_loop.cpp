// TODO(Task 6): implement the epoll event loop.
#include "miniredis/server/event_loop.h"
#include <unistd.h>

namespace miniredis {

EventLoop::EventLoop() : epoll_fd_(-1) {}

EventLoop::~EventLoop() {
    Stop();
}

bool EventLoop::AddFd([[maybe_unused]] int fd, [[maybe_unused]] uint32_t events, [[maybe_unused]] void* user_data) {
    // TODO(Task 6): epoll_ctl(EPOLL_CTL_ADD).
    return false;
}

bool EventLoop::ModFd([[maybe_unused]] int fd, [[maybe_unused]] uint32_t events, [[maybe_unused]] void* user_data) {
    // TODO(Task 6): epoll_ctl(EPOLL_CTL_MOD).
    return false;
}

bool EventLoop::RemoveFd([[maybe_unused]] int fd) {
    // TODO(Task 6): epoll_ctl(EPOLL_CTL_DEL).
    return false;
}

void EventLoop::Run([[maybe_unused]] int listen_fd, std::atomic<bool>& stop,
                    [[maybe_unused]] std::function<void(int)> on_accept,
                    [[maybe_unused]] std::function<void(Connection&)> on_readable,
                    [[maybe_unused]] std::function<void(Connection&)> on_writable,
                    [[maybe_unused]] std::function<void(int)> on_close,
                    [[maybe_unused]] std::function<void()> on_tick) {
    // TODO(Task 6): wait for epoll events and dispatch callbacks.
    stop.store(true);
}

void EventLoop::Stop() {
    if (epoll_fd_ >= 0) {
        ::close(epoll_fd_);
        epoll_fd_ = -1;
    }
}

}  // namespace miniredis
