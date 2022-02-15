#ifndef JFLECT_JFLECT_HPP_
#define JFLECT_JFLECT_HPP_
#include <algorithm>
#include <cassert>
#include <charconv>
#include <concepts>
#include <iterator>
#include <ranges>
#include <string>
#include <string_view>
#include <optional>
#include <functional>

#include "concepts.hpp"
#include "helper.hpp"
#include "meta.hpp"
#include "parser.hpp"
#include "traits.hpp"

#include "fast_float/fast_float.h"

#define CONSTEXPR_20_STRING

namespace jflect {

template<class T>
  requires(std::is_same_v<T, bool>)
constexpr void write_to(std::output_iterator<const char&> auto out, T value) {
  const auto sv = std::string_view(value ? "true" : "false");
  std::copy(std::begin(sv), std::end(sv), out); // [INFO] c++20 ranges
}

template<class T>
  requires(std::is_same_v<T, bool>)
constexpr auto read_to(std::string_view sv, T& value) -> decltype(std::begin(sv)) {
  parser::trim(sv);

  if (sv.starts_with("true")) {
    value = true;
    return std::begin(sv) + 4;
  } else if (sv.starts_with("false")) {
    value = false;
    return std::begin(sv) + 5;
  } else {
    assert(false && "str is neither true or false");
    return {};
  }
}

template<std::integral T>
  requires(!std::same_as<T, bool>)
/*[TODO] constexpr to_chars*/ void write_to(std::output_iterator<const char&> auto out, T value) {
  std::array<char, std::numeric_limits<decltype(value)>::digits10 + 1> str;
  const auto [ptr, ec] = std::to_chars(std::begin(str), std::end(str), value);
  assert(ec != std::errc::value_too_large);
  std::copy(std::data(str), ptr, out);
}

template<std::integral T>
  requires(!std::same_as<T, bool>)
/*[TODO] constexpr from_chars*/ auto read_to(std::string_view sv, T& value) -> decltype(std::begin(sv)) {
  parser::trim(sv);

  const auto [ptr, ec] = std::from_chars(std::data(sv), std::data(sv) + std::size(sv), value);
  assert(ec == std::errc());
  return ptr;
}

template<std::floating_point T>
/*[TODO] constexpr to_chars*/ void write_to(std::output_iterator<const char&> auto out, T value) {
#if __has_cpp_attribute(__cpp_lib_to_chars)
  std::array<char, std::numeric_limits<T>::max_digits10 + 1> str;
  const auto [ptr, ec] = std::to_chars(std::begin(str), std::end(str), value);
  assert(ec == std::errc());
  buffer.append(std::data(str), ptr)
#else
  const auto output = std::to_string(value);
  std::copy(std::begin(output), std::end(output), out); // [INFO] c++20 ranges
#endif
}

template<std::floating_point T>
/*[TODO] constexpr from_chars*/ auto read_to(std::string_view sv, T& value) -> decltype(std::begin(sv)) {
  parser::trim(sv);

#if __has_cpp_attribute(__cpp_lib_to_chars)
  const auto [ptr, ec] = std::from_chars(std::data(sv), std::data(sv) + std::size(sv), value);
#else
  const auto [ptr, ec] = fast_float::from_chars(std::data(sv), std::data(sv) + std::size(sv), value);
#endif
  assert(ec == std::errc());

  return ptr;
}

template<cpt::enumeration T>
constexpr void write_to(std::output_iterator<const char&> auto out, T value) {
  const auto output = meta::enumerator_helper<T>::toString(value);
  out = '\"';
  std::copy(std::begin(output), std::end(output), out); // [INFO] c++20 ranges
  out = '\"';
}

template<cpt::enumeration T>
constexpr auto read_to(std::string_view sv, T& value) -> decltype(std::begin(sv)) {
  parser::trim_read(sv, '"');

  const auto end = std::find_if_not(std::begin(sv), std::end(sv), [](auto c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || c == '_';
  });

  assert(end != std::end(sv) && *end == '"' && "string is unterminated");

  const auto identifier = std::string_view(std::data(sv), &*end);

  const auto result = meta::enumerator_helper<T>::fromString(identifier);
  assert(static_cast<bool>(result));
  if (result) {
    value = *result;
  }

  return std::next(end);
}

template<cpt::string_like T>
constexpr void write_to(std::output_iterator<const char&> auto out, const T& value) {
  out = '\"';
  std::copy(std::begin(value), std::end(value), out);
  out = '\"';
}

// [INFO] T HAS TO BE OWNING
template<cpt::string_like T>
/* [TODO] constexpr*/ auto read_to(std::string_view sv, T& value)
    -> decltype(std::begin(sv)) requires(std::constructible_from<T, const char*, const char*>) {
  static_assert(!std::same_as<std::remove_cvref<T>, std::string_view> && "std::string_view is non owning!");
  static_assert(!detail::is_span_v<std::remove_cvref<T>> && "std::span is non owning!");

  parser::trim_read(sv, '"');

  const auto [str, newsv] = parser::parse_string(sv);

  value = T(std::begin(str), std::end(str));

  return std::begin(newsv);
}

constexpr void write_to(std::output_iterator<const char&> auto out, const char* value) {
  write_to(out, std::string_view(value));
}

template<std::size_t N>
constexpr void write_to(std::output_iterator<const char&> auto out, const char value[N]) {
  write_to(out, std::string_view(value));
}

constexpr auto read_to(std::string_view sv, const char* value) = delete;

template<std::size_t N>
constexpr auto read_to(std::string_view sv, const char value[N]) = delete;

/*--------------------------- Forward Declarations ---------------------------*/
template<cpt::range_like T>
constexpr void write_to(std::output_iterator<const char&> auto out, T&& value);

template<cpt::range_like T>
  requires(cpt::reconstructible_range<T>)
constexpr auto read_to(std::string_view sv, T& value) -> decltype(std::begin(sv));

template<cpt::map_like T>
constexpr void write_to(std::output_iterator<const char&> auto out, T&& value);

template<cpt::map_like T>
  requires(cpt::reconstructible_range<T>)
constexpr auto read_to(std::string_view sv, T& value) -> decltype(std::begin(sv));

template<cpt::public_struct T>
  requires(!std::ranges::range<T> &&                                              // no ranges
           !cpt::tuple_like<T> &&                                                 // no tuple
           !detail::is_specialization_of_v<std::remove_cvref_t<T>, std::optional> // no optional
  )
constexpr void write_to(std::output_iterator<const char&> auto out, T&& value);

template<cpt::public_struct T>
  requires(!std::ranges::range<T> &&                                              // no ranges
           !cpt::tuple_like<T> &&                                                 // no tuple
           !detail::is_specialization_of_v<std::remove_cvref_t<T>, std::optional> // no optional
  )
constexpr auto read_to(std::string_view sv, T& value) -> decltype(std::begin(sv));

template<cpt::tuple_like T>
constexpr void write_to(std::output_iterator<const char&> auto out, T&& value);

template<cpt::tuple_like T>
constexpr auto read_to(std::string_view sv, T& value) -> decltype(std::begin(sv));

template<class T>
constexpr void write_to(std::output_iterator<const char&> auto out, const std::optional<T>& value);

template<class T>
constexpr auto read_to(std::string_view sv, std::optional<T>& value) -> decltype(std::begin(sv));

/*----------------------------------------------------------------------------*/

namespace detail {

struct read_sentinel {};

template<class R>
struct read_range_iterator {
  using iterator_category = std::input_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = std::remove_cvref_t<std::ranges::range_reference_t<R>>;

  std::reference_wrapper<const char*> m_iter;
  const char* m_end;

  constexpr read_range_iterator(const char*& t_iter, const char* t_end) noexcept : m_iter(t_iter), m_end(t_end) {
    assert(m_iter != m_end && *m_iter == '[');
    ++m_iter;
  }

  constexpr value_type operator*() const {
    value_type result;

    m_iter.get() = read_to(std::string_view(m_iter.get(), m_end), result);

    return result;
  }

  constexpr read_range_iterator& operator++() noexcept {
    m_iter.get() = std::find_if(m_iter.get(), m_end, [](auto c) { return c == ',' || c == ']'; });
    assert(m_iter != m_end);
    if (*m_iter != ']') {
      ++m_iter;
    }
    return *this;
  }

  constexpr read_range_iterator operator++(int) noexcept {
    auto tmp = *this;
    ++(*this);
    return tmp;
  }

  constexpr bool operator==(const read_sentinel&) const noexcept {
    assert(m_iter != m_end);
    return *m_iter == ']';
  };
};

template<class R>
struct read_map_iterator {
  using iterator_category = std::input_iterator_tag;
  using difference_type = std::ptrdiff_t;
  using value_type = std::remove_cvref_t<std::ranges::range_value_t<R>>;

  std::reference_wrapper<std::string_view> m_sv;

  constexpr read_map_iterator(std::string_view& sv) noexcept : m_sv(sv) { parser::trim_read(m_sv.get(), '{'); }

  constexpr value_type operator*() const {
    using key_type = std::remove_const_t<std::tuple_element_t<0, value_type>>;
    using mapped_type = std::tuple_element_t<1, value_type>;

    key_type key;

    m_sv.get() = std::string_view(read_to(m_sv.get(), key), std::end(m_sv.get()));

    parser::trim_read(m_sv.get(), ':');

    mapped_type mapped;

    m_sv.get() = std::string_view(read_to(m_sv.get(), mapped), std::end(m_sv.get()));

    return {key, mapped};
  }

  constexpr read_map_iterator& operator++() noexcept {
    parser::trim(m_sv.get());
    if (m_sv.get().starts_with(',')) {
      m_sv.get().remove_prefix(1);
    }
    return *this;
  }

  constexpr read_map_iterator operator++(int) noexcept {
    auto tmp = *this;
    ++(*this);
    return tmp;
  }

  constexpr bool operator==(const read_sentinel&) const noexcept { return m_sv.get().starts_with('}'); };
};
} // namespace detail

template<cpt::range_like T>
constexpr void write_to(std::output_iterator<const char&> auto out, T&& value) {
  out = '[';

  bool first = true;
  for (auto&& element : value) {
    if (!first) {
      out = ',';
    }

    write_to(out, element);

    first = false;
  }
  out = ']';
}

// [WARN] T HAS TO BE OWNING
template<cpt::range_like T>
  requires(cpt::reconstructible_range<T>)
constexpr auto read_to(std::string_view sv, T& value) -> decltype(std::begin(sv)) {
  static_assert(!detail::is_span_v<std::remove_cvref<T>> && "std::span is non owning!");

  parser::trim(sv);
  auto iter = std::data(sv);

  const auto begin = detail::read_range_iterator<T>(iter, &*std::end(sv));
  const auto end = detail::read_sentinel{};

  if constexpr (std::constructible_from<detail::read_range_iterator<T>, detail::read_sentinel>) {
    value = T(begin, end);
  } else {
    using common_iterator = std::common_iterator<detail::read_range_iterator<T>, detail::read_sentinel>;
    value = T(common_iterator(begin), common_iterator(end));
  }

  return std::next(iter);
}

template<cpt::map_like T>
constexpr void write_to(std::output_iterator<const char&> auto out, T&& value) {
  out = '{';

  bool first = true;
  for (const auto& [key, mapped] : value) {
    if (!first) {
      out = ',';
    }

    write_to(out, key);
    out = ':';
    write_to(out, mapped);

    first = false;
  }

  out = '}';
}

template<cpt::map_like T>
  requires(cpt::reconstructible_range<T>)
constexpr auto read_to(std::string_view sv, T& value) -> decltype(std::begin(sv)) {
  const auto begin = detail::read_map_iterator<T>(sv);
  const auto end = detail::read_sentinel{};

  if constexpr (std::constructible_from<detail::read_map_iterator<T>, detail::read_sentinel>) {
    value = T(begin, end);
  } else {
    using common_iterator = std::common_iterator<detail::read_map_iterator<T>, detail::read_sentinel>;
    value = T(common_iterator(begin), common_iterator(end));
  }

  parser::trim_read(sv, '}');

  return std::begin(sv);
}

template<cpt::public_struct T>
  requires(!std::ranges::range<T> &&                                              // no ranges
           !cpt::tuple_like<T> &&                                                 // no tuple
           !detail::is_specialization_of_v<std::remove_cvref_t<T>, std::optional> // no optional
  )
constexpr void write_to(std::output_iterator<const char&> auto out, T&& value) {
  out = '{';

  bool isFirst = true;

  meta::map_tuple_elements(meta::structAsNamedTuple(value), [&](auto&& t) {
    const auto& [name, member] = t;

    if (!isFirst) {
      out = ',';
    }
    out = '\"';
    std::copy(std::begin(name), std::end(name), out); // [INFO] c++20 ranges
    out = '\"';
    out = ':';

    write_to(out, member);

    isFirst = false;
  });

  out = '}';
}

namespace struct_helper {

template<class T>
struct MapValue {
private:
  using fn = const char* (*)(std::string_view sv, T& value);

public:
  std::string_view key;
  fn read;
  bool default_constructible;
};

template<class T>
struct Reader {
  static constexpr auto memberNames = meta::structMemberNames<T>();
  static constexpr auto ptrToMembers = meta::structAsPtrToMem<T>();

  template<std::size_t... Is>
  static constexpr auto create_map_impl(std::index_sequence<Is...>) noexcept {
    return std::array{MapValue<T>{
        .key = memberNames[Is],
        .read = [](std::string_view sv, T& value) -> const char* {
          return read_to(sv, value.*std::get<Is>(ptrToMembers));
        },
        .default_constructible = std::is_default_constructible_v<std::tuple_element_t<Is, meta::struct_types<T>>>,
    }...};
  }

  static constexpr auto create_map() noexcept {
    return create_map_impl(std::make_index_sequence<meta::memberCount<T>>{});
  }
};

} // namespace struct_helper

// [TODO] use hash-map (or similar) if there are many struct members instead of a linear search
template<cpt::public_struct T>
  requires(!std::ranges::range<T> &&                                              // no ranges
           !cpt::tuple_like<T> &&                                                 // no tuple
           !detail::is_specialization_of_v<std::remove_cvref_t<T>, std::optional> // no optional
  )
constexpr auto read_to(std::string_view sv, T& value) -> decltype(std::begin(sv)) {
  using namespace struct_helper;
  constexpr auto map = Reader<T>::create_map();

  std::array<bool, meta::memberCount<T>> is_initialized{};

  parser::trim_read(sv, '{');

  for (;;) {
    parser::trim_read(sv, '"');

    const auto endQuotePos = sv.find('"');
    assert(endQuotePos != std::string_view::npos);

    const auto key = sv.substr(0, endQuotePos);

    sv.remove_prefix(endQuotePos + 1);

    parser::trim_read(sv, ':');

    const auto search = std::find_if(std::begin(map), std::end(map), [&key](const auto& p) {
      return p.key == key;
    }); // [INFO] c++20 ranges with projection
    // const auto search = std::ranges::find(map, key, MapValue::key);

    if (search != std::end(map)) {
      sv = std::string_view(search->read(sv, value), std::end(sv));
      const auto index = search - std::begin(map);
      is_initialized[index] = true;
    } else {
      sv = parser::read_value(sv);
    }

    parser::trim(sv);

    if (parser::optional_read(sv, ','))
      continue;
    break;
  }

  parser::read(sv, '}');

  // [INFO] rangify
  // have all members been initalized?
  for (std::size_t i = 0; i < meta::memberCount<T>; ++i) {
    assert(is_initialized[i] || map[i].default_constructible);
  }

  /*
  for (std::size_t index = 0; index < std::size(map); ++index) {
    parser::trim_read(sv, '"');

    const auto endQuotePos = sv.find('"');
    assert(endQuotePos != std::string_view::npos);

    const auto key = sv.substr(0, endQuotePos);

    sv.remove_prefix(endQuotePos + 1);

    parser::trim_read(sv, ':');

    const auto search = std::find_if(std::begin(map), std::end(map), [&key](const auto& p) {
      return p.key == key;
    }); // [INFO] c++20 ranges with projection

    // const auto search = std::ranges::find(map, key, MapValue::key);

    if (search != std::end(map)) {
      sv = std::string_view(search->read(sv, value), std::end(sv));
    } else {
      assert(search->default_constructible);
      sv = parser::read_value(sv);
    }

    if (const auto isLast = index == std::size(map) - 1; !isLast) {
      parser::trim_read(sv, ',');
    }
  }

  parser::trim_read(sv, '}');
  */

  return std::begin(sv);
}

template<cpt::tuple_like T>
constexpr void write_to(std::output_iterator<const char&> auto out, T&& value) {
  out = '[';

  bool isFirst = true;

  meta::map_tuple_elements(value, [&out, &isFirst](auto&& element) {
    if (!isFirst) {
      out = ',';
    }

    write_to(out, element);

    isFirst = false;
  });

  out = ']';
}

namespace tuple_like {
template<class Inner>
constexpr Inner read_inner(std::string_view& sv, bool isLast) {
  Inner result;

  sv = std::string_view(read_to(sv, result), std::end(sv));

  if (!isLast) {
    parser::trim_read(sv, ',');
  }

  return result;
}

template<class T, std::size_t... I>
constexpr T read(std::string_view& sv, std::index_sequence<I...>) {
  return {read_inner<std::tuple_element_t<I, T>>(sv, I == (std::tuple_size_v<T> - 1))...};
}

} // namespace tuple_like

template<cpt::tuple_like T>
constexpr auto read_to(std::string_view sv, T& value) -> decltype(std::begin(sv)) {
  parser::trim_read(sv, '[');

  value = tuple_like::read<T>(sv, std::make_index_sequence<std::tuple_size_v<T>>{});

  parser::trim_read(sv, ']');

  return std::begin(sv);
}

/*------------------------------ standard types ------------------------------*/
template<class T>
constexpr void write_to(std::output_iterator<const char&> auto out, const std::optional<T>& value) {
  if (value.has_value()) {
    write_to(out, value.value());
  } else {
    const auto null = std::string_view("null");
    std::copy(std::begin(null), std::end(null), out);
  }
}

template<class T>
constexpr auto read_to(std::string_view sv, std::optional<T>& value) -> decltype(std::begin(sv)) {
  parser::trim(sv);
  if (sv.starts_with("null")) {
    value.reset();
    return std::begin(sv) + 4;
  }
  T inner;
  const auto iter = read_to(sv, inner);
  value = std::move(inner);
  return iter;
}

// [TODO] basic std::variant support
// std::fundamental || range || tuple_like || non colliding struct

/*----------------------------------------------------------------------------*/

template<class T>
CONSTEXPR_20_STRING std::string write(T&& value) {
  std::string str;
  write_to(std::back_inserter(str), std::forward<T>(value));
  return str;
}

template<class T>
  requires(std::is_default_constructible_v<T>)
constexpr T read(std::string_view sv) {
  T result;
  read_to(sv, result);
  return result;
}

} // namespace jflect
#endif // JFLECT_JFLECT_HPP_
