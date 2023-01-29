#ifndef RPC_CODEC_H
#define RPC_CODEC_H
#include "msgpack.hpp"
#include <functional>

struct rpc_message{
    uint32_t req_id;
    uint32_t size;
    std::shared_ptr<std::string> content;
};


struct rpc_header {
    uint32_t body_len;
    uint32_t req_id;
};

enum class result_code : std::int16_t {
    OK = 0,
    FAIL = 1,
};

template<typename T> struct function_traits;

template <typename Func, typename Arg, typename... Args>
struct function_traits<Func(Arg,Args...)> {

    using args_tuple = std::tuple<std::string, Arg, std::remove_const_t<std::remove_reference_t<Args>>...>;
    using args_tuple_2nd = std::tuple<std::string, std::remove_const_t<std::remove_reference_t<Args>>...>;
}; //提取函数参数模板

template<typename Func>
struct function_traits<Func()> {
    using args_tuple = std::tuple<std::string>;
    using args_tuple_2nd = std::tuple<std::string>;
}; //无参函数

template<typename Func, typename... Args> // 模板偏特化，函数指针
struct function_traits<Func (*)(Args...)> : function_traits<Func(Args...)> {};

template<typename Func, typename... Args> //模板偏特化，std::function
struct function_traits<std::function<Func(Args...)>> : function_traits<Func(Args...)> {};

template<typename ReturnType, typename ClassType, typename... Args> //模板偏特化，成员函数
struct function_traits<ReturnType (ClassType::*)(Args...)> : function_traits<ReturnType(Args...)> {};

template<typename ReturnType, typename ClassType, typename... Args> //模板偏特化，const成员函数
struct function_traits<ReturnType (ClassType::*)(Args...) const> : function_traits<ReturnType(Args...)> {};

template<typename ReturnType, typename ClassType, typename... Args>
struct function_traits<ReturnType (ClassType::* const&)(Args...)> : function_traits<ReturnType(Args...)> {};


class msgpack_codec {
public:
    const static size_t init_size = 2 * 1024;

    template<typename... Args>
    static msgpack::sbuffer pack_args(Args &&...args) {
        msgpack::sbuffer buffer(init_size);
        msgpack::pack(buffer, std::forward_as_tuple(std::forward<Args>(args)...));
        return buffer;
    }

    template<typename Arg, typename... Args,
            typename = typename std::enable_if<std::is_enum<Arg>::value>::type>
    static std::string pack_args_str(Arg arg, Args &&... args) {
        msgpack::sbuffer buffer(init_size);
        msgpack::pack(buffer, std::forward_as_tuple((int)arg, std::forward<Args>(args)...));
        return std::string(buffer.data(),buffer.size());
    }

    template<typename T>
    static msgpack::sbuffer pack(T &&t) {
        msgpack::sbuffer buffer(init_size);
        msgpack::pack(buffer,std::forward<T>(t));
        return buffer;
    }

    template<typename T>
    T unpack(char const *data, size_t length) {
        msgpack::unpack(msg, data, length);
        return msg.get().template as<T>();
    }

private:
    msgpack::unpacked msg;

};


#endif //RPC_CODEC_H
