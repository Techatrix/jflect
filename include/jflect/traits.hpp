#ifndef JFLECT_TRAITS_HPP_
#define JFLECT_TRAITS_HPP_
#include <span>

namespace jflect::detail {

template<class T>
struct is_span : std::false_type {};

template<class T, std::size_t N>
struct is_span<std::span<T, N>> : std::true_type {};

template<class T>
inline constexpr bool is_span_v = is_span<T>::value;

template<typename T, template<typename...> class Primary>
struct is_specialization_of : std::false_type {};

template<template<typename...> class Primary, typename... Args>
struct is_specialization_of<Primary<Args...>, Primary> : std::true_type {};

template<typename T, template<typename...> class Primary>
inline constexpr bool is_specialization_of_v = is_specialization_of<T, Primary>::value;
} // namespace jflect::detail
#endif // JFLECT_TRAITS_HPP_
