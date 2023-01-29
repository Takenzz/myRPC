# myRPC

### 基于C++与ASIO完成的tinyRPC通信
- 简单易用，能快速进行RPC通信
- 使用msgpack进行二进制序列化，比json更快，更小
- 采用unordered_map对server端的调用方法进行组织，能更快的找到相应的方法
- 使用模板和tuple对函数的参数萃取函数参数类型，用正确的函数参数调用函数
- 对每个连接单独进行处理，并用unordered_map组织到来的连接
- client端使用C++ future-promise模型，对RPC的结果进行异步调用。

client



