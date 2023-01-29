#include "rpc_client.h"


void rpc_client::write(uint32_t id, msgpack::sbuffer &&msg) {
    uint32_t size = msg.size();
    rpc_message m{id,size,std::make_shared<std::string>(msg.release())};
    msg_queue.emplace_back(std::move(m));

    write();
}

void rpc_client::write() {
    auto &msg = msg_queue.front();
    std::vector<asio::const_buffer> buffers;
    buffers.emplace_back(&msg.size, sizeof(uint32_t));
    buffers.emplace_back(&msg.req_id, sizeof(uint32_t));
    buffers.emplace_back((char *)msg.content->data(), msg.size);

    asio::async_write(socket_,buffers,
                      [this](const asio::error_code ec, const size_t length){
        if(ec) {

        } else {
            msg_queue.pop_front();
            if (!msg_queue.empty()) write();
        }
    });
}

void rpc_client::connect(const std::string &host, unsigned short port) {
    auto addr = asio::ip::address::from_string(host);
    socket_.async_connect(tcp::endpoint{addr,port},
                          [this](const asio::error_code ){
        printf("yes");
        do_read();
    });

}

void rpc_client::do_read() {
    asio::async_read(socket_, asio::buffer(header_,8),
                     [this](const asio::error_code ec, const size_t length){
        if(ec) {
        } else {
            rpc_header *header = (rpc_header *)header_;
            body.resize(header->body_len);
            read(header->req_id,header->body_len);
        }
    });
}

void rpc_client::read(uint32_t id, uint32_t length) {
    asio::async_read(socket_, asio::buffer(body.data(), length),
                     [this,id](asio::error_code ec, std::size_t length){
        auto &f = rpc_map[id];
        std::string_view view{body.data(),body.size()};
        f->set_value(rpc_result{view});
        rpc_map.erase(id);
        do_read();
    });
}

rpc_client::~rpc_client() {
    work->reset();
    if(thd_run->joinable()) {
        thd_run->join();
    }
}

