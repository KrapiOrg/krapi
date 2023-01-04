//
// Created by mythi on 13/10/22.
//
#pragma once

template<typename... Ts>
struct Overload : Ts... {
  using Ts::operator()...;
};
template<typename... Ts>
Overload(Ts...) -> Overload<Ts...>;
