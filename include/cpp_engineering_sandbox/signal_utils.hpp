#ifndef CPP_ENGINEERING_SANDBOX_SIGNAL_UTILS
#define CPP_ENGINEERING_SANDBOX_SIGNAL_UTILS

#include <algorithm>
#include <concepts>
#include <numbers>
#include <random>
#include <stdexcept>
#include <type_traits>
#include <vector>

// Concepts
template<typename T>
concept Container = requires(T a) {
  typename T::value_type;
  { a.begin() } -> std::input_iterator;
  { a.end() } -> std::sentinel_for<decltype(a.begin())>;
  { a.size() } -> std::convertible_to<std::size_t>;
};

// Randomization and noise
template<typename T> T get_rand_noise_normal(double mean, double stddev, std::mt19937 &gen)
{
  std::normal_distribution<double> dist(mean, stddev);
  return static_cast<T>(dist(gen));
}

template<Container C>
C make_signal_noisy(const C &sig, double mean = 0.0, double stddev = 0.2, unsigned seed = std::random_device{}())
{
  std::mt19937 gen(seed);
  C noisy_sig;
  noisy_sig.reserve(std::size(sig));
  std::transform(std::cbegin(sig), std::cend(sig), std::back_inserter(noisy_sig), [&](const auto &el) {
    return el + get_rand_noise_normal<typename C::value_type>(mean, stddev, gen);
  });
  return noisy_sig;
}

// Some basic signals
template<typename T>
  requires std::is_arithmetic_v<T>
std::vector<T> linspace(T start, T end, std::size_t num)
{
  if (num < 2) { throw std::invalid_argument("linspace: num must be at least 2"); }
  std::vector<T> result;
  result.reserve(num);
  T step = (end - start) / static_cast<T>(num - 1);
  for (std::size_t i = 0; i < num; ++i) { result.push_back(start + step * static_cast<T>(i)); }
  // Ensure last value is exactly 'end'
  result.back() = end;
  return result;
}

// Generate a sinusoidal signal
template<typename T = double>
  requires std::is_arithmetic_v<T>
std::vector<T> sinusoid(T amplitude, T frequency, T phase, std::size_t n_samples, T sample_rate)
{
  std::vector<T> result;
  result.reserve(n_samples);
  for (std::size_t i = 0; i < n_samples; ++i) {
    T t = static_cast<T>(i) / sample_rate;
    result.push_back(amplitude * std::sin(T{ 2 } * std::numbers::pi * frequency * t + phase));
  }
  return result;
}

template<typename T = double>
std::vector<T> sinusoid_from_time(T amplitude, T frequency, T phase, const std::vector<T> &t)
{
  std::vector<T> y;
  y.reserve(t.size());
  for (auto ti : t) { y.push_back(amplitude * std::sin(T{ 2 } * std::numbers::pi * frequency * ti + phase)); }
  return y;
}


// Filters

// Single-pole IIR low-pass filter
template<typename T>
  requires std::is_arithmetic_v<T>
T low_pass_filter(T signal_in, T &filtered_signal_old, T time_const, T cycle_time)
{
  T alpha = cycle_time / (time_const + cycle_time);
  T filtered_signal = alpha * signal_in + (T{ 1 } - alpha) * filtered_signal_old;
  filtered_signal_old = filtered_signal;
  return filtered_signal;
}

//------------------------------------------------------------------------------
// Util function
//------------------------------------------------------------------------------

/** Function maps input from range [input_min, input_max] to range [output_min, output_max]
 *
 * result y = f(x) where f maps x from [x_min, x_max] to [y_min, y_max]
 */
template<typename T>
  requires std::is_arithmetic_v<T>
constexpr T linmap_from_to(T input, T input_min, T input_max, T output_min, T output_max)
{
  if (input_max == input_min) {
    throw std::invalid_argument("linmap_from_to: input_max and input_min must not be equal");
  }
  T slope = (output_max - output_min) / (input_max - input_min);
  return output_min + slope * (input - input_min);
}


#endif /* ifndef CPP_ENGINEERING_SANDBOX_SIGNAL_UTILS */
