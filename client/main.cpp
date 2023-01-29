#include <iostream>
#include "rpc_server.h"


struct dummy {
    int add(rpc_conn conn, int a, int b) {
        std::cout << a << "+" << b << std::endl;
        return a+b;
    }
};

std::string hello(rpc_conn conn, const std::string &str) {
    std::cout << str << std::endl;
    return str;
}

int main() {
    rpc_server server(4444);
    dummy d;
    server.register_handler("hello",hello);
    server.register_handler("add",&dummy::add,&d);
    server.run();
    return 0;
}