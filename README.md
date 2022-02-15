# JFlect

JFlect is a header-only json library which uses static reflection

Example:

```c++
#include "jflect/jflect.hpp"

enum struct gender { male, female };
struct Person {std::string name;  int age; gender gender; };

int main() {
	const auto data = std::array{Person{"elizabeth", 24, gender::female}, Person{"victor", 63, gender::male}};
	std::cout << jflect::write(data) << '\n'; // prints [{"name":"elizabeth","age":24,"gender":"female"},{"name":"victor","age":63,"gender":"male"}]
}
```

## Concepts

Any type in jflect must satisfy one of these concepts:
- ``` std::is_integral ``` -> *bool, char, int, unsigned long*
- ``` std::is_floating_point ``` -> *float, double
- ``` cpt::string_like ``` -> *std::string, std::string_view*
- ``` std::enumerator ```
- ``` std::ranges::range ``` -> *std::vector, std::array, std::span, std::set*
- ``` cpt::map_like ``` -> *std::map, std::unordered_map*
- ``` cpt::tuple_like ``` -> *std::pair, std::tuple*
- ``` cpt::public_struct ``` -> a struct with only public members with no user defined (de)constructors
- ``` std::optional ```

Type requirements expand recursively.

## Examples

### Serialization

```c++
#include "jflect/jflect.hpp"

enum struct gender { male, female };
struct Person {std::string name;  int age; gender gender; };

int main() {
	std::cout << jflect::write("hello jflect") << '\n'; // "hello jflect"
	std::cout << jflect::write(gender::male) << '\n'; // "male"
	std::cout << jflect::write(14) << '\n'; // 14
	std::cout << jflect::write(std::array{"first", "second", "third"}) << '\n'; // ["first", "second", "third"]
	std::cout << jflect::write(std::map<std::string, int>{{"2+2", 4}, {"2-2", 0}}) << '\n'; // {"2+2":4,"2-2",0}
	std::cout << jflect::write(Person{"techatrix", -1, gender::male}) << '\n'; // {"name":"techatrix","age":-1,"gender":"male"}
}
```

### Deserialization

```c++
#include "jflect/jflect.hpp"

enum struct gender { male, female };
struct Person {std::string name;  int age; gender gender; };

int main() {
	const auto str = jflect::read<std::string>("hello jflect");
	const auto gender = jflect::read<gender>("male");
	const auto age = jflect::read<int>("14");
	const auto array = jflect::read<array<int, 3>>("[\"first\", \"second\", \"third\"]");
	const auto map = jflect::read<std::map<std::string, int>>>("{\"2+2\":4,\"2-2\",0}");
	const auto person = jflect::read("{\"name\":\"techatrix\",\"age\":-1,\"gender\":\"male\"}");
}
```


## Requirements

`JFlect`'s struct serialization utilizes static reflection and thefore requires the following:

- a c++20 compliant compiler with an implementation of the reflection-ts like https://github.com/matus-chochlik/llvm-project
- CMake
