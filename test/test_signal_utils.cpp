#include <catch2/catch_test_macros.hpp>

#include <cmath>
#include <vector>

#include <cpp_engineering_sandbox/signal_utils.hpp>

TEST_CASE("linspace generates correct number of points", "[linspace]")
{
  auto v = linspace(0.0, 1.0, 5);
  REQUIRE(v.size() == 5);
}

TEST_CASE("linspace includes endpoints", "[linspace]")
{
  auto v = linspace(2, 10, 5);
  REQUIRE(v.front() == 2);
  REQUIRE(v.back() == 10);
}

TEST_CASE("linspace computes correct step", "[linspace]")
{
  auto v = linspace(0.0, 1.0, 5);
  double expected_step = 0.25;
  for (std::size_t i = 0; i < v.size(); ++i) {
    REQUIRE(std::abs(v[i] - (0.0 + expected_step * static_cast<double>(i))) < 1e-12);
  }
}

TEST_CASE("linspace throws on num < 2", "[linspace]")
{
  REQUIRE_THROWS_AS(linspace(0.0, 1.0, 1), std::invalid_argument);
}

TEST_CASE("generate_sinusoid_from_time produces correct amplitude and phase", "[sinusoid_from_time]")
{
  auto t = linspace(0.0, 1.0, 5);// 0, 0.25, 0.5, 0.75, 1.0
  auto y = sinusoid_from_time(2.0, 1.0, 0.0, t);
  REQUIRE(y.size() == t.size());
  REQUIRE(std::abs(y[0] - 0.0) < 1e-12);
  REQUIRE(std::abs(y[1] - 2.0) < 1e-12);
  REQUIRE(std::abs(y[2] - 0.0) < 1e-12);
  REQUIRE(std::abs(y[3] + 2.0) < 1e-12);
  REQUIRE(std::abs(y[4] - 0.0) < 1e-12);
}

TEST_CASE("low_pass_filter smooths a step input", "[low_pass_filter]")
{
  double prev = 0.0;
  double time_const = 0.1;
  double cycle_time = 0.01;
  double out = 0.0;
  // Step input: first 0, then 1
  for (int i = 0; i < 10; ++i) out = low_pass_filter(0.0, prev, time_const, cycle_time);
  for (int i = 0; i < 10; ++i) out = low_pass_filter(1.0, prev, time_const, cycle_time);
  REQUIRE(out > 0.0);
  REQUIRE(out < 1.0);
}

TEST_CASE("make_signal_noisy output has same size and is different", "[make_signal_noisy]")
{
  std::vector<double> v(100, 1.0);
  auto noisy = make_signal_noisy(v, 0.0, 0.5, 42);
  REQUIRE(noisy.size() == v.size());
  // At least one value should differ
  bool any_diff = false;
  for (std::size_t i = 0; i < v.size(); ++i)
    if (std::abs(noisy[i] - v[i]) > 1e-6) any_diff = true;
  REQUIRE(any_diff);
}

TEST_CASE("linmap_from_to maps correctly", "[linmap_from_to]")
{
  REQUIRE(linmap_from_to(5.0, 0.0, 10.0, 0.0, 100.0) == 50.0);
  REQUIRE(linmap_from_to(0.0, 0.0, 10.0, -1.0, 1.0) == -1.0);
  REQUIRE(linmap_from_to(10.0, 0.0, 10.0, -1.0, 1.0) == 1.0);
}
