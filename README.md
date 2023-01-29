# myRPC

### 基于ASIO的tinyRPC通信
- 简单易用，能快速进行RPC通信
- 使用msgpack进行二进制序列化，比json更快，更小
- 采用unordered_map对server端的调用方法进行组织，能更快的找到相应的方法
- 使用模板和tuple对函数的参数萃取函数参数类型，用正确的函数参数调用函数
- 对每个连接单独进行处理，并用unordered_map组织到来的连接
- client端使用C++ future-promise模型，对RPC的结果进行异步调用。

sever.cpp  
```
struct dummy {
    int add(rpc_conn conn, int a, int b) {
        std::cout << a << "+" << b << std::endl;
        return a+b;
    }
};

std::string hello(rpc_conn conn, const std::string &str) {
    std::cout << str << std::endl;
    return str;
} //创建调用方法

int main() {
    ....
    server.register_handler("hello",hello); // 注册
    server.register_handler("add",&dummy::add,&d);//注册
    server.run();
    return 0;
}
```

client.cpp  
```
auto p = client.call<std::string>("hello","hello rpc");       //远程调用
std::cout << p << std::endl;
auto q = client.call<int>("add", 1, 2);
std::cout << q << std::endl;
```

clietn端输出为
```
hello rpc
3
```


