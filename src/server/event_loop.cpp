// TODO(Task 6): implement the epoll event loop.
#include "miniredis/server/event_loop.h"
#include "miniredis/server/connection.h"
#include <cerrno>
#include <cstddef>
#include <sys/epoll.h>
#include <unistd.h>

namespace miniredis {

EventLoop::EventLoop() {
    epoll_fd_ = epoll_create1(0);
}

EventLoop::~EventLoop() {
    Stop();
}

bool EventLoop::AddFd([[maybe_unused]] int fd, [[maybe_unused]] uint32_t events, [[maybe_unused]] void* user_data) {
    struct epoll_event ev{};
    ev.events = events;
    ev.data.ptr = user_data;
    return epoll_ctl(epoll_fd_, EPOLL_CTL_ADD, fd, &ev) == 0;
}

bool EventLoop::ModFd([[maybe_unused]] int fd, [[maybe_unused]] uint32_t events, [[maybe_unused]] void* user_data) {
    struct epoll_event ev{};
    ev.events = events;
    ev.data.ptr = user_data;
    return epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd, &ev) == 0;
}

bool EventLoop::RemoveFd([[maybe_unused]] int fd) {
    return epoll_ctl(epoll_fd_, EPOLL_CTL_DEL, fd, nullptr) == 0;
}

void EventLoop::Run([[maybe_unused]] int listen_fd, std::atomic<bool>& stop,
                    [[maybe_unused]] std::function<void(int)> on_accept,
                    [[maybe_unused]] std::function<void(Connection&)> on_readable,
                    [[maybe_unused]] std::function<void(Connection&)> on_writable,
                    [[maybe_unused]] std::function<void(int)> on_close,
                    [[maybe_unused]] std::function<void()> on_tick) {
    while(!stop.load()){
        auto nfds = epoll_wait(epoll_fd_, events_, kMaxEvents, 100);
        if(nfds < 0){
            if(errno != EINTR) { break; }
        }
        on_tick();
        if(stop.load()) { break; }
        for(int i = 0;i < nfds;i++){
            if(events_[i].data.ptr == nullptr){
                on_accept(listen_fd);
            } else {
                auto conn = static_cast<Connection*>(events_[i].data.ptr);
                if(events_[i].events & EPOLLIN){
                    on_readable(*conn);
                } else if(events_[i].events & (EPOLLERR | EPOLLHUP)){
                    on_close(conn->Fd());
                } else if(events_[i].events & EPOLLOUT){
                    on_writable(*conn);
                } else if(events_[i].events & EPOLLRDHUP){
                    on_close(conn->Fd());
                }
            }
        }
    }
}

void EventLoop::Stop() {
    if (epoll_fd_ >= 0) {
        ::close(epoll_fd_);
        epoll_fd_ = -1;
    }
}

}  // namespace miniredis
