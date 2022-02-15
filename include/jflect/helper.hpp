#ifndef JFLECT_HELPER_HPP_
#define JFLECT_HELPER_HPP_
#include <string_view>

#include "concepts.hpp"

namespace jflect::detail {

template<cpt::enumeration E>
[[nodiscard("pure function")]] constexpr std::underlying_type_t<E> to_underlying(E e) noexcept {
  return static_cast<std::underlying_type_t<E>>(e);
}

template<std::ranges::range R>
  requires(std::is_integral_v<std::ranges::range_value_t<R>>) // reduce to std::totally_ordered
[[nodiscard("pure function")]] constexpr bool isContiguous(R&& values) {
  /* [INFO] c++23
   return std::ranges::all_of(values | std::views::adjacent<2> | std::views::transform([](auto&& p) {
    return p.first <= p.second;
   }));
  */

  auto&& begin = std::begin(values);
  auto&& end = std::end(values);

  if (begin == end)
    return true;
  auto previous = *begin;

  for (; begin != end; ++begin) {
    if ((*begin - previous) > 1)
      return false;
    previous = *begin;
  }

  return true;
}
} // namespace jflect::detail
#endif // JFLECT_HELPER_HPP_
