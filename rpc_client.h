#ifndef RPC_RPC_CLIENT_H
#define RPC_RPC_CLIENT_H
#include <future>
#include <unordered_map>
#include <thread>
#include "codec.h"
#include "asio.hpp"

using Work = asio::executor_work_guard<asio::any_io_executor> ;

class rpc_result {
public:
    rpc_result() = default;
    rpc_result(std::string_view a) : data(a.data(),a.length()) {}

    template<typename T>
    T get_result() {
        msgpack_codec codec;
        auto p = codec.template unpack<std::tuple<int,T>>(data.data(),data.size());
        return std::get<1>(p);
    };

private:
    std::string data;
};

using asio::ip::tcp;
class rpc_client {
public:

    rpc_client(): socket_(io_), work(std::make_shared<Work>(io_.get_executor())) {
        thd_run = std::make_shared<std::thread>([this](){ io_.run(); });
    }

    ~rpc_client();
    void connect(const std::string &host, unsigned short port);

    template<typename... Args>
    std::future<rpc_result> rpc_future(const std::string &name, Args... arg) {
        auto p = std::make_shared<std::promise<rpc_result>>();
        std::future<rpc_result> future = p->get_future();
        uint32_t id = rpc_id++;
        rpc_map.template emplace(id,std::move(p));
        msgpack_codec codec;
        auto ret = codec.pack_args(name,std::forward<Args>(arg)...);

        write(id,std::move(ret));
        return std::move(future);
    }

    template<typename T, typename... Args>
    typename std::enable_if<!std::is_void<T>::value, T>::type
    call(const std::string &name,Args &&...arg) {
        std::future<rpc_result> future_result = rpc_future(name, std::forward<Args>(arg)...);

        return future_result.get().template get_result<T>();
    }

    template<typename T,typename... Args>
    typename std::enable_if<std::is_void<T>::value>::type
    call(const std::string &name, Args &&... arg) {
        std::future<rpc_result> future_result = rpc_future(name, std::forward<Args>(arg)...);
        return ;
    }


private:
    void write(uint32_t,msgpack::sbuffer &&);
    void write();
    void read(uint32_t, uint32_t);
    void do_read();
    asio::io_context io_;
    tcp::socket socket_;
    std::string host_;
    unsigned short port_;
    uint32_t rpc_id = 0;
    std::unordered_map<uint32_t ,std::shared_ptr<std::promise<rpc_result>>> rpc_map;
    std::deque<rpc_message> msg_queue;
    char header_[8];
    std::vector<char> body;
    std::shared_ptr<std::thread> thd_run = nullptr;
    std::shared_ptr<Work> work;
};


#endif //RPC_RPC_CLIENT_H
