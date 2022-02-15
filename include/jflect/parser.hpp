#ifndef JFLECT_PARSER_HPP_
#define JFLECT_PARSER_HPP_
#include <string_view>

#include "helper.hpp"

namespace jflect::parser {

template<class CharT, class Traits>
constexpr void trim(std::basic_string_view<CharT, Traits>& sv) noexcept {
  sv.remove_prefix(std::min(sv.find_first_not_of(" \n\r\t"), std::size(sv)));
}

template<class CharT, class Traits>
constexpr void read(std::basic_string_view<CharT, Traits>& sv, [[maybe_unused]] CharT c) noexcept {
  assert(sv.starts_with(c));
  sv.remove_prefix(1);
}

template<class CharT, class Traits>
constexpr void trim_read(std::basic_string_view<CharT, Traits>& sv, [[maybe_unused]] CharT c) noexcept {
  assert(c != ' ' && c != '\n' && c != '\r' && c != '\t' && "illogical operation");
  trim(sv);
  read(sv, c);
}

template<class CharT, class Traits>
constexpr void trim_read_trim(std::basic_string_view<CharT, Traits>& sv, [[maybe_unused]] CharT c) noexcept {
  assert(c != ' ' && c != '\n' && c != '\r' && c != '\t' && "illogical operation");
  trim(sv);
  read(sv, c);
  trim(sv);
}

template<class CharT, class Traits>
constexpr bool optional_read(std::basic_string_view<CharT, Traits>& sv, CharT c) noexcept {
  const auto result = sv.starts_with(c);
  if (result)
    sv.remove_prefix(1);
  return result;
}

template<class CharT, class Traits, class UnaryPredicate>
constexpr bool optional_read_if(std::basic_string_view<CharT, Traits>& sv, UnaryPredicate p) noexcept {
  if (sv.empty())
    return false;

  const auto result = std::invoke(p, sv[0]);
  if (result)
    sv.remove_prefix(1);
  return result;
}

/*----------------------------------------------------------------------------*/

template<class CharT, class Traits>
constexpr std::basic_string_view<CharT, Traits> read_value(std::basic_string_view<CharT, Traits> sv);

template<class CharT, class Traits>
constexpr std::basic_string_view<CharT, Traits> read_number(std::basic_string_view<CharT, Traits> sv) noexcept {
  const auto isDigit = [](char c) { return '0' <= c && c <= '9'; };
  const auto isOneNine = [](char c) { return '1' <= c && c <= '9'; };

  optional_read(sv, '-');

  bool allowZero = false;
  while (optional_read_if(sv, allowZero ? isDigit : isOneNine)) {
    allowZero = true;
  }

  if (optional_read(sv, '.')) {
    while (optional_read_if(sv, isDigit)) {
    }
  }

  if (sv.starts_with('e') || sv.starts_with('E')) {
    sv.remove_prefix(1);
    if (sv.starts_with('+') || sv.starts_with('-'))
      sv.remove_prefix(1);
  }

  while (optional_read_if(sv, isDigit)) {
  }

  return sv;
}

template<class CharT, class Traits>
constexpr std::basic_string_view<CharT, Traits> read_other(std::basic_string_view<CharT, Traits> sv) {
  trim(sv);
  if (sv.starts_with("true"))
    sv.remove_prefix(4);
  else if (sv.starts_with("false"))
    sv.remove_prefix(5);
  else if (sv.starts_with("null"))
    sv.remove_prefix(4);
  else
    return read_number(sv);
  return sv;
}

namespace detail {
template<class CharT, class Traits>
constexpr std::basic_string_view<CharT, Traits> read_string_impl(std::basic_string_view<CharT, Traits> sv) {
  [[maybe_unused]] const auto isHex = [](auto c) {
    return ('0' <= c && c <= '9') || ('A' <= c && c <= 'F') || ('a' <= c && c <= 'f');
  };

  while (!sv.empty()) {
    switch (sv.front()) {
      case '\x00':
      case '\x01':
      case '\x02':
      case '\x03':
      case '\x04':
      case '\x05':
      case '\x06':
      case '\x07':
      case '\x08':
      case '\x09':
      case '\x0A':
      case '\x0B':
      case '\x0C':
      case '\x0D':
      case '\x0E':
      case '\x0F':
      case '\x10':
      case '\x11':
      case '\x12':
      case '\x13':
      case '\x14':
      case '\x15':
      case '\x16':
      case '\x17':
      case '\x18':
      case '\x19':
      case '\x1A':
      case '\x1B':
      case '\x1C':
      case '\x1D':
      case '\x1E':
      case '\x1F':
        assert(false && "illegal character");
        break;
      case '\"':
        return sv;
      case '\\':
        sv.remove_prefix(1);
        assert(!sv.empty());

        switch (sv.front()) {
          case '\"':
          case '\\':
          case '/':
          case 'b':
          case 'f':
          case 'n':
          case 'r':
          case 't':
            break;
          case 'u':
            assert(std::size(sv) >= 4);
            for (int i = 0; i < 4; ++i) {
              assert(isHex(sv[1 + i]));
            }
            sv.remove_prefix(4);
            break;
          default:
            assert(false && "illegal character");
            break;
        }
        [[fallthrough]];
      default:
        sv.remove_prefix(1);
        break;
    }
  }
  assert(false && "missing \" character");
  return sv;
}
} // namespace detail

template<class CharT, class Traits>
constexpr std::basic_string_view<CharT, Traits> read_string(std::basic_string_view<CharT, Traits> sv) {
  trim_read(sv, '"');
  sv = detail::read_string_impl(sv);
  read(sv, '"');
  return sv;
}

template<class CharT, class Traits>
constexpr std::basic_string_view<CharT, Traits> read_array(std::basic_string_view<CharT, Traits> sv) {
  trim_read_trim(sv, '[');

  if (sv.starts_with(']')) { // empty array
    sv.remove_prefix(1);
    return sv;
  }

  for (;;) {
    sv = read_value(sv);

    trim(sv);
    if (!optional_read(sv, ','))
      break;
    ;
  }

  trim_read(sv, ']');
  return sv;
}

template<class CharT, class Traits>
constexpr std::basic_string_view<CharT, Traits> read_object(std::basic_string_view<CharT, Traits> sv) {
  trim_read_trim(sv, '{');

  if (sv.starts_with('}')) // empty object
  {
    sv.remove_prefix(1);
    return sv;
  }

  for (;;) {
    sv = read_string(sv);
    trim_read(sv, ':');
    sv = read_value(sv);
    trim(sv);
    if (!optional_read(sv, ','))
      break;
  }

  trim_read(sv, '}');
  return sv;
}

/**
 * @brief reads a json-value
 *
 * @param sv a view from the begining of a json-value to its end (or beyond)
 * @return a std::string_view past the value
 */
template<class CharT, class Traits>
constexpr std::basic_string_view<CharT, Traits> read_value(std::basic_string_view<CharT, Traits> sv) {
  // clang-format off
  trim(sv);
  assert(!sv.empty());
  if (sv.starts_with('{')) return read_object(sv);
  if (sv.starts_with('[')) return read_array(sv);
  if (sv.starts_with('"')) return read_string(sv);
  return read_other(sv);
  // clang-format on
}

template<class CharT, class Traits>
std::pair<std::basic_string<CharT, Traits>, std::basic_string_view<CharT, Traits>> parse_string(
    std::basic_string_view<CharT, Traits> sv) {
  std::basic_string<CharT, Traits> result;

  auto begin = std::begin(sv);
  const auto end = std::end(sv);

  const auto readUnicode = [&]() {
    int32_t codepoint = 0;

    for (const auto factor : {12u, 8u, 4u, 0u}) {
      ++begin;
      assert(begin != end);

      if ('0' <= *begin && *begin <= '9') {
        codepoint += static_cast<int32_t>((static_cast<uint32_t>(*begin) - 0x30u) << factor);
      } else if ('A' <= *begin && *begin <= 'F') {
        codepoint += static_cast<int32_t>((static_cast<uint32_t>(*begin) - 0x37u) << factor);
      } else if ('a' <= *begin && *begin <= 'f') {
        codepoint += static_cast<int32_t>((static_cast<uint32_t>(*begin) - 0x57u) << factor);
      } else {
        assert(false);
      }
    }

    // translate codepoint into bytes
    if (codepoint < 0x80) {
      // 1-byte characters: 0xxxxxxx (ASCII)
      result.push_back(static_cast<char>(codepoint));
    } else if (codepoint <= 0x7FF) {
      // 2-byte characters: 110xxxxx 10xxxxxx
      result.push_back(static_cast<char>(0xC0u | (static_cast<uint32_t>(codepoint) >> 6u)));
      result.push_back(static_cast<char>(0x80u | (static_cast<uint32_t>(codepoint) & 0x3Fu)));
    } else if (codepoint <= 0xFFFF) {
      // 3-byte characters: 1110xxxx 10xxxxxx 10xxxxxx
      result.push_back(static_cast<char>(0xE0u | (static_cast<uint32_t>(codepoint) >> 12u)));
      result.push_back(static_cast<char>(0x80u | ((static_cast<uint32_t>(codepoint) >> 6u) & 0x3Fu)));
      result.push_back(static_cast<char>(0x80u | (static_cast<uint32_t>(codepoint) & 0x3Fu)));
    } else {
      // 4-byte characters: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
      result.push_back(static_cast<char>(0xF0u | (static_cast<uint32_t>(codepoint) >> 18u)));
      result.push_back(static_cast<char>(0x80u | ((static_cast<uint32_t>(codepoint) >> 12u) & 0x3Fu)));
      result.push_back(static_cast<char>(0x80u | ((static_cast<uint32_t>(codepoint) >> 6u) & 0x3Fu)));
      result.push_back(static_cast<char>(0x80u | (static_cast<uint32_t>(codepoint) & 0x3Fu)));
    }
  };

  for (; begin != end; ++begin) {
    switch (*begin) {
      case '\x00':
      case '\x01':
      case '\x02':
      case '\x03':
      case '\x04':
      case '\x05':
      case '\x06':
      case '\x07':
      case '\x08':
      case '\x09':
      case '\x0A':
      case '\x0B':
      case '\x0C':
      case '\x0D':
      case '\x0E':
      case '\x0F':
      case '\x10':
      case '\x11':
      case '\x12':
      case '\x13':
      case '\x14':
      case '\x15':
      case '\x16':
      case '\x17':
      case '\x18':
      case '\x19':
      case '\x1A':
      case '\x1B':
      case '\x1C':
      case '\x1D':
      case '\x1E':
      case '\x1F':
        assert(false && "illegal character");
        break;
      case '\"':
        return {result, {++begin, end}};
      case '\\':
        ++begin;
        assert(begin != end);
        switch (*begin) {
          case '\"':
            result.push_back('\"');
            break;
          case '\\':
            result.push_back('\\');
            break;
          case '/':
            result.push_back('/');
            break;
          case 'b':
            result.push_back('\b');
            break;
          case 'f':
            result.push_back('\f');
            break;
          case 'n':
            result.push_back('\n');
            break;
          case 'r':
            result.push_back('\r');
            break;
          case 't':
            result.push_back('\t');
            break;
          case 'u':
            readUnicode();
            break;
          default:
            assert(false && "Unexspected character");
            break;
        }
        break;
      default:
        result.push_back(*begin);
        break;
    }
  }
  assert(false && "missing \" character");
  return {{}, {}};
}

} // namespace jflect::parser
#endif // JFLECT_PARSER_HPP_