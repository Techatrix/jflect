#ifndef JFLECT_META_HPP_
#define JFLECT_META_HPP_
#include <array>
#include <string_view>

#include <experimental/reflect>

namespace jflect {
    namespace meta {
namespace refl = std::experimental::reflect;
template<cpt::enumeration E>
class enumerator_helper {
public:
  template<class... Args>
  struct helper {
  public:
    static consteval std::array<std::pair<std::string_view, E>, sizeof...(Args)> create_map() noexcept {
      return {{{refl::get_name_v<Args>, refl::get_constant_v<Args>}...}};
    }

    static consteval std::array<std::underlying_type_t<E>, sizeof...(Args)> constantsAsUnderlying() noexcept {
      return {detail::to_underlying(refl::get_constant_v<Args>)...};
    }
    static consteval std::array<std::string_view, sizeof...(Args)> names() noexcept {
      return {refl::get_name_v<Args>...};
    }
  };

  using unpacked_helper = refl::unpack_sequence_t<helper, refl::get_enumerators_t<reflexpr(E)>>;

public:
  constexpr static std::string_view toString(E value) noexcept {
    constexpr auto constants = unpacked_helper::constantsAsUnderlying();
    constexpr auto size = std::size(constants);
    if constexpr (size <= 0) {
      return {};
    } else if constexpr (detail::isContiguous(constants)) {
      constexpr auto names = unpacked_helper::names();
      constexpr auto offset = *std::begin(constants);
      const auto index = static_cast<std::size_t>(detail::to_underlying(value)) + offset;
      if (index < size)
        return names[index];
      else [[unlikely]]
        return {};
    } else {
      constexpr auto map = unpacked_helper::create_map();
      for (const auto& [name, constant] : map) {
        if (constant == value)
          return name;
      }
      return {};
    }
  }

  constexpr static std::optional<E> fromString(std::string_view sv) noexcept {
    constexpr auto map = unpacked_helper::create_map();
    for (const auto& [name, constant] : map) {
      if (name == sv)
        return {constant};
    }
    return std::nullopt;
  }
};

template<class... Args>
struct sat_helper_unnamed {
  template<class T>
  constexpr static auto create(T&& value) noexcept {
    return std::tie(value.*refl::get_pointer_v<Args>...);
  }
};

template<class... Args>
struct sat_helper_named {
  template<class T>
  constexpr static auto create(T&& value) noexcept {
    return std::make_tuple(std::pair<std::string_view, decltype(value.*refl::get_pointer_v<Args>)>(
        std::string_view(refl::get_name_v<Args>), value.*refl::get_pointer_v<Args>)...);
  }
};

template<class... Args>
struct sat_helper_names {
  template<class T>
  consteval static auto create() noexcept {
    return std::array{std::string_view(refl::get_name_v<Args>)...};
  }
};

template<class... Args>
struct sat_helper_ptr_to_mem {
  template<class T>
  consteval static auto create() noexcept {
    return std::make_tuple(refl::get_pointer_v<Args>...);
  }
};

template<template<class...> class Tpl, class T>
using unpack_members_t = refl::unpack_sequence_t<Tpl, refl::get_data_members_t<reflexpr(T)>>;

template<class T>
inline constexpr auto memberCount = refl::get_size_v<refl::get_data_members_t<reflexpr(T)>>;

template<class T>
using struct_types = refl::unpack_sequence_t<std::tuple, refl::get_data_members_t<reflexpr(T)>>;

// struct T { int alpha; float berta;} -> tuple<int, float>
template<class T>
constexpr auto structAsTuple(T&& value) noexcept {
  return unpack_members_t<sat_helper_unnamed, std::remove_reference_t<T>>::create(std::forward<T>(value));
}

// struct T { int alpha; float berta;} -> tuple<pair<string_view, int>, pair<string_view, float>>
template<class T>
constexpr auto structAsNamedTuple(T&& value) noexcept {
  return unpack_members_t<sat_helper_named, std::remove_reference_t<T>>::create(std::forward<T>(value));
}

template<class T>
consteval auto structMemberNames() noexcept {
  return unpack_members_t<sat_helper_names, std::remove_reference_t<T>>::template create<T>();
}

template<class T>
consteval auto structAsPtrToMem() noexcept {
  return unpack_members_t<sat_helper_ptr_to_mem, std::remove_reference_t<T>>::template create<T>();
}

template<class T, class F, std::size_t... Is>
constexpr void map_tuple_elements(T&& tuple, F&& f, std::index_sequence<Is...>) {
  (std::invoke(f, std::get<Is>(std::forward<T>(tuple))), ...);
}

template<class T, class F>
constexpr void map_tuple_elements(T&& tuple, F&& f) {
  constexpr auto size = std::tuple_size_v<std::decay_t<T>>;
  map_tuple_elements(std::forward<T>(tuple), std::forward<F>(f), std::make_index_sequence<size>{});
}
} // namespace meta
}
#endif // JFLECT_META_HPP_
