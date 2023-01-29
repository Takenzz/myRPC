#include <iostream>
#include "rpc_client.h"
#include "rpc_server.h"


int main() {
    rpc_client client;
    client.connect("192.168.174.1",4444);
    auto p = client.call<std::string>("hello","hello rpc");
    std::cout << p << std::endl;
    auto q = client.call<int>("add", 1, 2);
    std::cout << q << std::endl;
    return 0;
}
