#ifndef AFINA_NETWORK_MT_NONBLOCKING_CONNECTION_H
#define AFINA_NETWORK_MT_NONBLOCKING_CONNECTION_H

#include <cstring>
#include <atomic>
#include <memory>
#include <stdexcept>


#include <afina/execute/Command.h>
#include <afina/network/Server.h>
#include "protocol/Parser.h"
#include <afina/logging/Service.h>

#include <sys/epoll.h>

namespace spdlog {
    class logger;
}

namespace Afina {
namespace Network {
namespace MTnonblock {

class Connection {
public:
    Connection(int s, std::shared_ptr<Afina::Storage> ps, std::shared_ptr<spdlog::logger> pl) :
                    _socket(s), pStorage(ps), _logger(pl) {
        std::memset(&_event, 0, sizeof(struct epoll_event));
        _event.data.ptr = this;
        cur_pos = 0;
        now_pos = 0;
    }

    inline bool isAlive() const { return _is_alive.load(); }

    void Start();

protected:
    void OnError();
    void OnClose();
    void DoRead();
    void DoWrite();

private:
    friend class Worker;
    friend class ServerImpl;

    int _socket;
    struct epoll_event _event;
    std::shared_ptr<spdlog::logger> _logger;
    std::shared_ptr<Afina::Storage> pStorage;

    std::atomic<bool> _is_alive;
    std::unique_ptr<Execute::Command> command_to_execute;
    char client_buffer[4096];
    std::vector<std::string> buffer;
    int cur_pos;
    int now_pos;
};

} // namespace MTnonblock
} // namespace Network
} // namespace Afina

#endif // AFINA_NETWORK_MT_NONBLOCKING_CONNECTION_H
