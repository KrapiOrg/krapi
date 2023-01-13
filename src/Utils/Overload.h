#pragma once

template<typename... Ts>
struct Overload : Ts... {
  using Ts::operator()...;
};
template<typename... Ts>
Overload(Ts...) -> Overload<Ts...>;
