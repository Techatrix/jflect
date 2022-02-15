#ifndef JFLECT_CONCEPTS_HPP_
#define JFLECT_CONCEPTS_HPP_
#include <ranges>
#include <tuple>
#include <type_traits>

namespace jflect::cpt {

namespace detail {
template<class T, std::size_t N>
concept is_tuple_element = requires(T t) {
  typename std::tuple_element_t<N, T>;
  { std::get<N>(t) } -> std::convertible_to<std::tuple_element_t<N, T>&>;
};

template<class T, std::size_t... I>
consteval bool is_tuple_like_elements(std::index_sequence<I...>) noexcept {
  return (is_tuple_element<T, I> && ...);
}

// clang-format off
template <class T>
concept tuple_like = !std::is_reference_v<T> && requires {
  typename std::tuple_size<T>::type;
  requires std::same_as<std::remove_cvref_t<decltype(std::tuple_size_v<T>)>, size_t>;
} && is_tuple_like_elements<T>(std::make_index_sequence<std::tuple_size_v<T>>{});

template<class T>
concept pair_like = tuple_like<T> && std::tuple_size_v<T> == 2;
// clang-format on
} // namespace detail

template<class T>
concept string_like = (std::ranges::contiguous_range<T> && std::ranges::sized_range<T> &&
                       std::same_as<std::ranges::range_value_t<T>, char>);

template<class T>
concept enumeration = std::is_enum_v<T>;

template<class T>
concept public_struct = std::is_class_v<std::remove_reference_t<T>> /*&&
(std::experimental::reflect::get_size_v<std::experimental::reflect::get_public_data_members_t<reflexpr(T)>> ==
 std::experimental::reflect::get_size_v<std::experimental::reflect::get_data_members_t<reflexpr(T)>>)*/
    ;

template<class T>
concept tuple_like = detail::tuple_like<std::remove_reference_t<T>>;

template<class T>
concept pair_like = detail::pair_like<std::remove_reference_t<T>>;

template<class T>
concept range_like = std::ranges::range<T> && !cpt::tuple_like<T> && !cpt::string_like<T>;

template<class T, class R = std::remove_cvref_t<T>>
concept map_like = range_like<T> &&                                            // T is range like
    pair_like<std::ranges::range_value_t<T>> &&                                // Value Type is a Pair
    std::is_const_v<std::tuple_element_t<0, std::ranges::range_value_t<T>>> && // Key Type is const
    string_like<std::tuple_element_t<0, std::ranges::range_value_t<T>>>        // Key is string_like
    && requires {
  typename R::key_type;
  typename R::mapped_type;
  typename R::value_type;
};

template<class R,
         class It = std::ranges::iterator_t<std::remove_reference_t<R>>,
         class Sen = std::ranges::sentinel_t<std::remove_reference_t<R>>>
concept reconstructible_range = std::ranges::range<R> && std::constructible_from<R, It, Sen>;

} // namespace jflect::cpt
#endif // JFLECT_CONCEPTS_HPP_
