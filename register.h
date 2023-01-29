#ifndef RPC_REGISTER_H
#define RPC_REGISTER_H
#include <string>
#include <unordered_map>
#include <functional>
#include <memory>
#include "codec.h"
#include <iostream>

class connection;

class register_center {
public:
    template<typename Function>
    void register_handler(std::string const &name, Function f) {
        return register_nonmember(name, std::move(f));
    }

    template<typename Function, typename Self>
    void register_handler(std::string const &name, const Function &f, Self *self)  {
        return register_member(name, f, self);
    }

    template<typename T>
    void handler(const char *data, std::size_t size, std::weak_ptr<T> conn) {
        msgpack_codec codec;
        std::string result;
        auto conn_ptr = conn.lock();
        auto p = codec.unpack<std::tuple<std::string>>(data,size);
        auto &func_name = std::get<0>(p);
        auto it = map_invokers.find(func_name);
        auto id = conn_ptr->get_id();
        it->second(conn, data, size, result);

        conn_ptr->response(id,std::move(result));
    }



    void remove_handle(std::string const &name);

private:

    template<typename F, size_t... I, typename Arg, typename... Args>
    static typename std::invoke_result<F,std::weak_ptr<connection>, Args...>::type
    call_helper(const F &f, const std::index_sequence<I...> &, std::tuple<Arg, Args...> tup,
                std::weak_ptr<connection> ptr) {
        return f(ptr,std::move(std::get<I + 1>(tup))...);
    }

    template<typename F, typename Self, size_t... I, typename Arg, typename... Args>
    static auto
    call_helper_member(const F &f, Self *self, const std::index_sequence<I...> &,
                        std::tuple<Arg, Args...> tup, std::weak_ptr<connection> ptr)
    -> decltype((*self.*f)(ptr,std::get<I + 1>(tup)...))                    {
       return (*self.*f)(ptr,std::move(std::get<I + 1>(tup))...);
    }

    template <typename F, typename Arg, typename... Args>
    static typename std::enable_if<std::is_void<typename std::invoke_result<F,std::weak_ptr<connection>, Args...>::type>::value>::type
    call(const F &f, std::weak_ptr<connection> ptr, std::string &result, std::tuple<Arg, Args...> tp) {
        call_helper(f, std::make_index_sequence<sizeof...(Args)>{}, std::move(tp), ptr);
        result = msgpack_codec::pack_args_str(result_code::OK);
    }


    template <typename F, typename Arg, typename... Args>
    static typename std::enable_if<!std::is_void<typename std::invoke_result<F,std::weak_ptr<connection>, Args...>::type>::value>::type
    call(const F &f, std::weak_ptr<connection> ptr, std::string &result, std::tuple<Arg, Args...> tp) {
        auto r = call_helper(f, std::make_index_sequence<sizeof...(Args)>{}, std::move(tp), ptr);
        msgpack_codec codec;
        result = msgpack_codec::pack_args_str(result_code::OK, r);
    }

    template <typename F, typename Self, typename Arg, typename... Args>
    static typename std::enable_if<std::is_void<typename std::result_of<F(Self, std::weak_ptr<connection>, Args...)>::type>::value>::type
    call_member(const F &f, Self *self, std::weak_ptr<connection> ptr, std::string &result, std::tuple<Arg, Args...> tp) {
        call_helper_member(f,self, std::make_index_sequence<sizeof...(Args)>{}, std::move(tp),ptr);
        result = msgpack_codec::pack_args_str(result_code::OK);
    }

    template <typename F, typename Self, typename Arg, typename... Args>
    static auto
    call_member(const F &f, Self *self, std::weak_ptr<connection> ptr, std::string &result, std::tuple<Arg, Args...> tp)
    ->typename std::enable_if<!std::is_void<typename std::result_of<F(Self, std::weak_ptr<connection>, Args...)>::type>::value>::type {
        auto  r = call_helper_member(f,self, std::make_index_sequence<sizeof...(Args)>{}, std::move(tp),ptr);
        result = msgpack_codec::pack_args_str(result_code::OK, r);
    }
    template<typename Function>
    struct invoker {
    static inline void apply(const Function &func, std::weak_ptr<connection> conn, const char *data,
                   size_t size, std::string &result) {
            using args_tuple = typename function_traits<Function>::args_tuple_2nd ;
            msgpack_codec codec;
            auto tp = codec.unpack<args_tuple>(data, size);
            call(func,conn, result,std::move(tp));
    }

    template<typename Self>
    static void apply_member(const Function &func, Self *self, std::weak_ptr<connection> conn, const char *data,
                          size_t size, std::string &result) {
            using args_tuple = typename function_traits<Function>::args_tuple_2nd;
            msgpack_codec codec;
            auto tp = codec.template unpack<args_tuple>(data, size);
            call_member(func, self, conn, result, std::move(tp));
        }
    };


    template<typename Function>
    void register_nonmember(std::string const &name, Function f) {
        this->map_invokers[name] = {std::bind(&invoker<Function>::apply, std::move(f), std::placeholders::_1,
                                       std::placeholders::_2, std::placeholders::_3,
                                       std::placeholders::_4)};
    }

    template<typename Function, typename  Self>
    void register_member(std::string const &name,const Function &f, Self *self) {
        map_invokers[name] = {std::bind(&invoker<Function>::template apply_member<Self>, f, self,
                                        std::placeholders::_1, std::placeholders::_2, std::placeholders::_3,
                                        std::placeholders::_4)};
    }



    std::unordered_map<std::string, std::function<void(std::weak_ptr<connection>, const char *,
                                                                                   size_t,std::string &)> >
        map_invokers;
};

#endif //RPC_REGISTER_H
