#ifndef AFINA_NETWORK_ST_COROUTINE_CONNECTION_H
#define AFINA_NETWORK_ST_COROUTINE_CONNECTION_H

#include <afina/execute/Command.h>
#include <cstring>

#include <afina/Storage.h>
#include <afina/coroutine/Engine.h>
#include <protocol/Parser.h>
#include <spdlog/logger.h>
#include <sys/epoll.h>

namespace Afina {
    namespace Network {
        namespace STcoroutine {

            class Connection {
            public:
                Connection(int s, std::shared_ptr<Afina::Storage> ps, std::shared_ptr<spdlog::logger> lg)
                        : _socket(s), pStorage(ps), _logger(lg), _is_alive(true) {
                    std::memset(&_event, 0, sizeof(struct epoll_event));
                    _event.data.ptr = this;
                }

                inline bool isAlive() const { return _is_alive; }

                void Start();

            protected:
                void OnError();

                void OnClose();

                void DoRead();

                void DoWrite();

                void main_proc(Afina::Coroutine::Engine &engine);

            private:
                friend class ServerImpl;

                int _socket;
                struct epoll_event _event;

                bool _is_alive;

                std::shared_ptr<spdlog::logger> _logger;
                std::shared_ptr<Afina::Storage> pStorage;

                Protocol::Parser parser;
                std::unique_ptr<Execute::Command> command_to_execute;
                std::size_t arg_remains;
                std::string argument_for_command;

                int _pos;
                int cur_pos;
                std::vector<std::string> bufer;

                Afina::Coroutine::Engine::context *_ctx;

                char client_buffer[4096];
            };
        } // namespace STcoroutine
    } // namespace Network
} // namespace Afina

#endif // AFINA_NETWORK_ST_COROUTINE_CONNECTION_H