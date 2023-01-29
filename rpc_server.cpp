#include "rpc_server.h"

rpc_server::rpc_server(unsigned short port) : acceptor(io), signal(io),req_id(0), socket(io) {
    signal.add(SIGTERM);
    signal.add(SIGINT);
    #if defined(SIGQUIT)
    signal.add(SIGQUIT);
    #endif // defined(SIGQUIT)
    do_stop();
    acceptor.open(tcp::v4());
    acceptor.bind(tcp::endpoint{tcp::v4(),port});
    acceptor.set_option(tcp::acceptor::reuse_address(true));
    acceptor.listen();
    do_accept();
}


void rpc_server::do_accept() {
    conn.reset(new connection(io,center));
    acceptor.async_accept(conn->socket(),
                          [this](asio::error_code ec){
        if(ec) {
            printf("%s",ec.message().c_str());
        }else {
            {
                std::lock_guard lk(lock);
                conn->set_id(req_id);
                connections.emplace(req_id++, conn);
            }
            conn->start();
        }
        do_accept();
    });

}

void rpc_server::do_stop() {
    signal.async_wait(
            [this](const asio::error_code &, int) {
                acceptor.close();
            });
}

void rpc_server::run() {
    io.run();
}
