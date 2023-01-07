#include <concepts>
#include <iostream>
#include <source_location>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <type_traits>

// A counter for every test case.
inline auto test_case_count  = 0;
inline auto test_case_passed = 0;

template <typename T, typename V>
void Equals(T const& actual, V const& expected, std::source_location const& location = std::source_location::current())
  requires(std::convertible_to<T, V> or std::convertible_to<V, T>) and std::equality_comparable_with<T, V>
{
  ++test_case_count;
  auto current = std::source_location::current();
  if (actual != expected) {
    std::cerr << location.file_name() << ":" << location.line() << ":" << location.column() << ": "
              << "Failed: Expected: " << expected << ", Actual: " << actual << std::endl;
    return;
  }
  ++test_case_passed;
}

template <typename T>
void Equals(std::span<T> const& actual, std::string_view const& expected,
            std::source_location const& location = std::source_location::current()) {
  ++test_case_count;
  auto current = std::source_location::current();
  if (actual.size() != expected.size()) {
    std::cerr << location.file_name() << ":" << location.line() << ":" << location.column() << ": "
              << current.function_name() << " Expected: " << expected.size() << ", Actual: " << actual.size()
              << std::endl;
    return;
  }
  for (auto i = 0; i < actual.size(); ++i) {
    if (actual[i] != expected[i]) {
      std::cerr << location.file_name() << ":" << location.line() << ":" << location.column() << ": "
                << current.function_name() << " Expected: " << expected[i] << ", Actual: " << actual[i] << std::endl;
      return;
    }
  }
  ++test_case_passed;
}

inline auto GetTestInfo() { return std::make_tuple(test_case_passed, test_case_count); }