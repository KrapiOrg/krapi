//
// Created by mythi on 13/10/22.
//

#ifndef KRAPI_MODELS_OVERLOAD_H
#define KRAPI_MODELS_OVERLOAD_H

template<typename ...Ts>
struct Overload : Ts ... {
    using Ts::operator()...;
};
template<typename ...Ts> Overload(Ts...) -> Overload<Ts...>;

#endif //KRAPI_MODELS_OVERLOAD_H
