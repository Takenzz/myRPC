#ifndef RPC_CONNECTION_H
#define RPC_CONNECTION_H
#include "asio.hpp"
#include "register.h"
#include <mutex>

using asio::ip::tcp;

class connection : public std::enable_shared_from_this<connection> {
public:
    connection(asio::io_context &io_, register_center &center_);
    void set_id(uint32_t);
    uint32_t get_id();
    void start();
    void response(int, std::string);
    tcp::socket &socket();
private:
    void read_header();
    void read_body(std::size_t);
    void write();

    uint32_t req_id_;
    register_center &center_;
    asio::io_context &io_;
    tcp::socket socket_;
    char data[8];
    std::deque<rpc_message> msg_queue;
    std::vector<char> body;
};


#endif //RPC_CONNECTION_H
