#ifndef AFINA_NETWORK_ST_COROUTINE_CONNECTION_H
#define AFINA_NETWORK_ST_COROUTINE_CONNECTION_H

#include <cstring>
#include <string>
#include <vector>

#include <spdlog/logger.h>
#include <protocol/Parser.h>
#include <afina/coroutine/Engine.h>

#include <sys/epoll.h>
#include <afina/Storage.h>

namespace Afina {
    namespace Network {
        namespace STcoroutine {

            class Connection {
            public:
    Connection(int s) : _socket(s) {
        std::memset(&_event, 0, sizeof(struct epoll_event));
        _event.data.ptr = this;
    }

                inline bool isAlive() const { return is_alive; }

                void Start();

            protected:
                void OnError();

                void OnClose();

                void DoRead(Afina::Coroutine::Engine &pe);

                void DoWrite(Afina::Coroutine::Engine &pe);

            private:
                friend class ServerImpl;

                int _socket;
                struct epoll_event _event;

                bool is_alive;
                int now_pos_bytes = 0;
                char client_buffer[4096];
                std::size_t arg_remains;
                Protocol::Parser parser;
                std::string argument_for_command;
                std::unique_ptr<Execute::Command> command_to_execute;
                std::shared_ptr<Afina::Storage> pStorage;
                std::shared_ptr<spdlog::logger> _logger;
                std::vector<std::string> bufer;
                int pos = 0;
            };

} // namespace STcoroutine
} // namespace Network
} // namespace Afina

#endif // AFINA_NETWORK_ST_COROUTINE_CONNECTION_H
