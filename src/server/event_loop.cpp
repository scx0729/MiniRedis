// TODO(Task 6): implement the epoll event loop.
#include "miniredis/server/event_loop.h"
#include <unistd.h>

namespace miniredis {

EventLoop::EventLoop() : epoll_fd_(-1) {}

EventLoop::~EventLoop() {
    Stop();
}

bool EventLoop::AddFd(int, uint32_t, void*) {
    // TODO(Task 6): epoll_ctl(EPOLL_CTL_ADD).
    return false;
}

bool EventLoop::ModFd(int, uint32_t, void*) {
    // TODO(Task 6): epoll_ctl(EPOLL_CTL_MOD).
    return false;
}

bool EventLoop::RemoveFd(int) {
    // TODO(Task 6): epoll_ctl(EPOLL_CTL_DEL).
    return false;
}

void EventLoop::Run(int, std::atomic<bool>& stop,
                    std::function<void(int)>,
                    std::function<void(Connection&)>,
                    std::function<void(Connection&)>,
                    std::function<void(int)>,
                    std::function<void()>) {
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
