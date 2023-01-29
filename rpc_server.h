#ifndef RPC_RPC_SERVER_H
#define RPC_RPC_SERVER_H
#include "register.h"
#include "connection.h"
#include <mutex>
using rpc_conn = std::weak_ptr<connection>;
using asio::ip::tcp;

class rpc_server {
public:
    rpc_server(unsigned short port);
    template <typename Function>
    void register_handler(std::string const &name, const Function &f) {
        center.register_handler(name,f);
    }

    template<class Function, typename Self>
    void register_handler(std::string const &name, const Function &f, Self *self) {
        center.register_handler(name,f,self);
    }

    void run();
private:
    void do_accept();
    void do_stop();

    asio::io_context io;
    tcp::socket socket;
    register_center center;
    int req_id;
    tcp::acceptor acceptor;
    std::unordered_map<int, std::shared_ptr<connection>> connections;
    std::shared_ptr<connection> conn;
    asio::signal_set signal;
    std::mutex lock;
};

#endif //RPC_RPC_SERVER_H
