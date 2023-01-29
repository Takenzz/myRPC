#include "connection.h"
#include <iostream>

connection::connection(asio::io_context &io, register_center &center):
                            socket_(io),io_(io),center_(center), req_id_(0) {

}

tcp::socket &connection::socket() {
    return socket_;
}

uint32_t connection::get_id() {
    return (uint32_t)req_id_;
}

void connection::set_id(uint32_t id) {
    req_id_ = id;
}

void connection::start() {
    read_header();
}

void connection::read_header() {
    asio::async_read(socket_,asio::buffer(data,8),
                     [this](asio::error_code ec, std::size_t length){
    if(ec) {

    }else {
        rpc_header *header = (rpc_header *) (data);
        req_id_ = (uint32_t) header->req_id;
        const uint32_t len = header->body_len;
        body.resize(len);
        read_body(len);
    }
    });

}

void connection::read_body(std::size_t len) {
    auto self(this->shared_from_this());
    asio::async_read(socket_,asio::buffer(body.data(),len),
                     [this,self](asio::error_code ec, std::size_t length){
    if(ec) {
    } else {
        read_header();
        center_.handler<connection>(body.data(), body.size(), this->shared_from_this());
    }
    });

}

void connection::response(int id, std::string result) {
    auto length = result.size();
    msg_queue.push_back(rpc_message{(uint32_t)id,(uint32_t)length,std::make_shared<std::string>(std::move(result))});
    write();
}

void connection::write() {
    auto &msg = msg_queue.front();
    std::vector<asio::const_buffer> buffers;
    buffers.emplace_back(asio::buffer(&msg.size, sizeof(uint32_t)));
    buffers.emplace_back(asio::buffer(&msg.req_id,sizeof(uint32_t)));
    buffers.emplace_back(asio::buffer((char *)msg.content->data(),msg.size));

    asio::async_write(socket_,buffers,
                      [this](asio::error_code ec, std::size_t length){
        if(ec){
        } else {
            msg_queue.pop_front();
            if(!msg_queue.empty()) {
                 write();
            }
        }
    });
}

