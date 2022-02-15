#include "jflect/jflect.hpp"

#include <vector>
#include <iostream>
#include <fstream>

std::vector<char> readFile(const std::filesystem::path& path) {
  assert(std::filesystem::exists(path) && "file not found");
  assert(std::filesystem::is_regular_file(path) && "path is not a file");

  auto fileSize = std::filesystem::file_size(path);

  std::ifstream file(path, std::ios::in | std::ios::binary);

  assert(file.is_open() && "failed to open file");

  std::vector<char> buffer(static_cast<std::size_t>(fileSize));

  file.read(std::data(buffer), fileSize);

  return buffer;
}

enum struct gender { male, female };

struct Person {
  std::string name;
  int age;
  gender gender;
};

struct Data {
  int count;
  std::vector<Person> people;
};

int main() {
  std::cout << jflect::write(Person{"techatrix", -1, gender::male}) << '\n';
  /*
  const auto data = readFile("data.json");

  const auto [count, people] = jflect::read<Data>(std::string_view(std::data(data), std::data(data) + std::size(data)));

  std::cout << "count: " << count << '\n';
  for(auto&& [name, age, gender] : people) {
    std::cout << "[name: " << name << ", age: " << age << ", gender: " << (gender == gender::male ? "male" : "female")
  << "]\n";
  }
  */
}
