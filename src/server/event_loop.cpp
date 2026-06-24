#include "miniredis/server/event_loop.h"
#include <cerrno>
#include <cstring>
#include <unistd.h>

namespace miniredis {

EventLoop::EventLoop() {
    epoll_fd_ = epoll_create1(0);
    if (epoll_fd_ < 0) {
        // Fatal error
        return;
    }
}

EventLoop::~EventLoop() {
    if (epoll_fd_ >= 0) {
        close(epoll_fd_);
    }
}

bool EventLoop::AddFd(int fd, uint32_t events, void* user_data) {
    struct epoll_event ev{};
    ev.events = events;
    ev.data.ptr = user_data;
    return epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == 0;
}

bool EventLoop::ModFd(int fd, uint32_t events, void* user_data) {
    struct epoll_event ev{};
    ev.events = events;
    ev.data.ptr = user_data;
    return epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) == 0;
}

bool EventLoop::RemoveFd(int fd) {
    return epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr) == 0;
}

void EventLoop::Run(int listen_fd, std::atomic<bool>& stop,
                     std::function<void(int listen_fd)> on_accept,
                     std::function<void(Connection& conn)> on_readable,
                     std::function<void(Connection& conn)> on_writable,
                     std::function<void(int fd)> on_close,
                     std::function<void()> on_tick) {
    while (!stop.load()) {
        int nfds = epoll_wait(epoll_fd_, events_, kMaxEvents, 100);
        if (nfds < 0) {
            if (errno == EINTR) {
                if (on_tick) on_tick();
                continue;
            }
            break;
        }

        if (on_tick) on_tick();
        if (stop.load()) break;

        for (int i = 0; i < nfds; ++i) {
            auto& ev = events_[i];
            auto* ptr = ev.data.ptr;

            if (ptr == nullptr) {
                on_accept(listen_fd);
                continue;
            }

            auto* conn = static_cast<Connection*>(ptr);

            if (ev.events & EPOLLIN) {
                on_readable(*conn);
                continue;
            }

            if (ev.events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
                on_close(conn->Fd());
                continue;
            }

            if (ev.events & EPOLLOUT) {
                on_writable(*conn);
            }
        }
    }

}

void EventLoop::Stop() {
    // epoll_wait will return due to timeout (100ms)
}

}  // namespace miniredis
