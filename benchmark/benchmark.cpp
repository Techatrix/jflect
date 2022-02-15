#include "benchmark/benchmark.h"

#include "jflect/jflect.hpp"

using T = std::tuple<int, double, std::array<int, 3>>;

constexpr auto input = std::string_view("[42,33.2,[1,2,3]]");
// constexpr auto output = T{42, 33.2, std::array{1, 2, 3}};

static void BM_jflect_read_int(benchmark::State& state) {
  for (auto _ : state) {
    T result = jflect::read<T>(input);
    benchmark::DoNotOptimize(result);
  }
}

/*
static void BM_native_read_int(benchmark::State& state) {
  T result;
  for (auto _ : state) {
    auto iter = std::begin(input);
    const auto end = std::end(input);

    iter = std::find_if_not(iter, end, [](auto c) { return c == ' '; }) + 1;

    int r0;
    const auto [ptr0, ec0] = std::from_chars(&*iter, &*end, r0);
    iter = ptr0;

    iter = std::find(iter, end, ',') + 1;
    iter = std::find_if_not(iter, end, [](auto c) { return c == ' '; });

    float r1;
    const auto [ptr1, ec1] = fast_float::from_chars(&*iter, &*end, r1);
    iter = ptr1;

    iter = std::find_if_not(iter, end, [](auto c) { return c == ' '; });
    iter = std::find(iter, end, ',') + 1;

    const auto strStart = std::next(std::find_if_not(iter, end, [](auto c) { return c == ' '; }));
    const auto strEnd = std::find(strStart, end, '"');

    const std::string r2 = std::string(&*strStart, &*strEnd);

    iter = std::next(strEnd);

    iter = std::find(iter, end, ']');
    benchmark::DoNotOptimize(iter);

    std::get<0>(result) = r0;
    std::get<1>(result) = r1;
    std::get<2>(result) = r2;

    benchmark::DoNotOptimize(result);
  }
  // assert(std::get<0>(result) == std::get<0>(output));
  // assert(std::abs(std::get<1>(result) - std::get<1>(output)) < 0.1);
  // assert(std::get<2>(result) == std::get<2>(output));
}
*/

BENCHMARK(BM_jflect_read_int);
// BENCHMARK(BM_native_read_int);

BENCHMARK_MAIN();
