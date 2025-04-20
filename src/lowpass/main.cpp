#include <algorithm>
#include <cmath>
#include <iostream>
#include <matplot/freestanding/plot.h>
#include <matplot/matplot.h>
#include <vector>

#include <cpp_engineering_sandbox/signal_utils.hpp>


int main()
{
  try {
    double amplitude = 1.0;
    double frequency = 2.0;// 2 Hz
    double phase = 0.0;
    std::size_t n_samples = 200;
    double duration = 2.0;// seconds
    double sample_rate = static_cast<double>(n_samples) / duration;// 100 Hz

    // Generate a sinusoidal signal
    auto time = linspace(0.0, duration, n_samples);
    auto signal_clean = sinusoid_from_time(amplitude, frequency, phase, time);
    auto signal_noisy = make_signal_noisy(signal_clean, 0.0, 0.3);

    // Filtering
    std::vector<double> signal_filtered;
    signal_filtered.reserve(signal_noisy.size());
    double time_const = 0.1;
    double cycle_time = 2.0 / sample_rate;
    for (auto prev = 0.0; auto x : signal_noisy) {
      signal_filtered.push_back(low_pass_filter(x, prev, time_const, cycle_time));
    }

    // Plotting
    using namespace matplot;
    plot(signal_clean, "--r");
    hold(on);
    plot(signal_noisy);

    plot(signal_filtered);
    title("Low-pass filter");
    xlabel("Samples [-]");
    ylabel("Amplitude [-]");
    legend({"clean signal", "noisy signal", "filtered signal"});
    show();

    return 0;
  } catch (std::exception &e) {
    std::cerr << e.what() << "\n";
    return 1;
  } catch (...) {
    std::cerr << "something went wrong\n";
    return 2;
  }
}
