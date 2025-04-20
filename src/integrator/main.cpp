#include <algorithm>
#include <iomanip>
#include <iostream>
#include <matplot/freestanding/plot.h>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <units.h>
#include <vector>

using namespace units;
using namespace units::literals;
using namespace units::time;
using namespace units::mass;
using namespace units::length;
using namespace units::force;
using namespace units::velocity;
using namespace units::acceleration;

namespace units {
UNIT_ADD(pendulum,
  kilogram_per_second,
  kilogram_per_second,
  kgps,
  compound_unit<units::mass::kilograms, inverse<units::time::seconds>>)
UNIT_ADD(pendulum,
  kilogram_per_second_squared,
  kilogram_per_second_squared,
  kgps2,
  compound_unit<units::mass::kilograms, inverse<squared<units::time::seconds>>>)
}// namespace units
using namespace units::pendulum;

struct Pendulum
{

  struct State
  {
    meter_t pos;
    meters_per_second_t vel;
  };

  struct DState
  {
    meters_per_second_t dpos;
    meters_per_second_squared_t dvel;
  };

  struct Params
  {
    kilogram_t mass;
    kilogram_per_second_t damping;
    kilogram_per_second_squared_t spring_constant;
  };

  Pendulum(State s, Params p) : state{ s }, params{ p } {}
  Pendulum(meter_t x0, meters_per_second_t v0, kilogram_t m, kilogram_per_second_t b, kilogram_per_second_squared_t k)
    : state{ .pos = x0, .vel = v0 }, params{ .mass = m, .damping = b, .spring_constant = k }
  {}

  [[nodiscard]] Params &getParams() { return params; }
  [[nodiscard]] State &getState() { return state; }
  [[nodiscard]] const Params &getParams() const { return params; }
  [[nodiscard]] const State &getState() const { return state; }

  /***
   * RK4 integrator
   */
  void integrate_RK4(millisecond_t dt)
  {
    DState a = evaluate(state, 0_ms, DState{});
    DState b = evaluate(state, 0.5 * dt, a);
    DState c = evaluate(state, 0.5 * dt, b);
    DState d = evaluate(state, dt, c);

    meters_per_second_t dxdt = (1.0 / 6.0) * (a.dpos + 2.0 * (b.dpos + c.dpos) + d.dpos);
    meters_per_second_squared_t dvdt = (1.0 / 6.0) * (a.dvel + 2.0 * (b.dvel + c.dvel) + d.dvel);
    state.pos = state.pos + dxdt * dt;
    state.vel = state.vel + dvdt * dt;
  }

  /** Semi-implicit euler.
   * computing higher derivative first.
   */
  void integrate_euler(millisecond_t dt)
  {
    state.vel += acc() * dt;
    state.pos += state.vel * dt;
  }

private:
  State state;
  Params params;

  [[nodiscard]] DState evaluate(const State &st, millisecond_t dt, const DState &dst) const
  {
    State s{};
    s.vel = st.vel + dst.dvel * dt;
    s.pos = st.pos + dst.dpos * dt;

    DState out{};
    out.dpos = s.vel;
    out.dvel = acc();
    return out;
  }

  [[nodiscard]] meters_per_second_squared_t acc() const
  {
    return (-1 / params.mass) * (params.damping * state.vel + params.spring_constant * state.pos);
  }
};

template<typename TimeUnit, typename YAxisUnit>
void plot_units(const std::vector<TimeUnit> &t_vec, const std::vector<YAxisUnit> &x_vec)
{
  using namespace matplot;
  if (t_vec.size() != x_vec.size()) {
    throw std::invalid_argument("plot_units function, time and data vectors do not have same size.");
  }

  auto cast_to_double = [](auto el) { return unit_cast<double>(el); };
  std::vector<double> tt_vec(x_vec.size());
  std::vector<double> xx_vec(x_vec.size());
  std::transform(std::begin(t_vec), std::end(t_vec), std::begin(tt_vec), cast_to_double);
  std::transform(std::begin(x_vec), std::end(x_vec), std::begin(xx_vec), cast_to_double);

  plot(tt_vec, xx_vec);
  hold(on);
  show();
}

template<typename TimeUnit, typename XAxisUnit, typename YAxisUnit>
void plot_units(const std::vector<TimeUnit> &t_vec,
  const std::vector<XAxisUnit> &x1_vec,
  const std::vector<YAxisUnit> &x2_vec)
{
  using namespace matplot;
  if (t_vec.size() != x1_vec.size()) {
    throw std::invalid_argument("plot_units function, time and data vectors do not have same size.");
  }

  auto cast_to_double = [](auto el) { return unit_cast<double>(el); };
  std::vector<double> tt_vec(x1_vec.size());
  std::vector<double> xx1_vec(x1_vec.size());
  std::vector<double> xx2_vec(x1_vec.size());
  std::transform(std::begin(t_vec), std::end(t_vec), std::begin(tt_vec), cast_to_double);
  std::transform(std::begin(x1_vec), std::end(x1_vec), std::begin(xx1_vec), cast_to_double);
  std::transform(std::begin(x2_vec), std::end(x2_vec), std::begin(xx2_vec), cast_to_double);

  plot(tt_vec, xx1_vec);
  hold(on);
  plot(tt_vec, xx2_vec);
  title("Integrator");
  legend({ "RK4", "Euler" });
  ylabel("Amplitude [m]");
  xlabel("Time [s]");
  show();
}

void sim_pendulum()
{
  Pendulum::State init_state{ .pos = 1_m, .vel = 0_mps };
  Pendulum::Params params{ .mass = 1_kg, .damping = 0.2_kgps, .spring_constant = 15_kgps2 };
  Pendulum sys1{ init_state, params };
  Pendulum sys2{ init_state, params };

  const auto &state1 = sys1.getState();
  const auto &state2 = sys2.getState();

  second_t t = 0_s;
  millisecond_t dt = 10_ms;
  std::vector<second_t> t_vec{ t };
  std::vector<meter_t> x1_vec{ init_state.pos };
  std::vector<meter_t> x2_vec{ init_state.pos };

  spdlog::info("Starting:");
  spdlog::info("pos: {:.2f}, vel: {:.2f}, mass: {:.2f}, force: {}",
    state1.pos.value(),
    state1.vel.value(),
    params.mass.value(),
    to_string(0_N));
  while (t <= 90_s) {
    sys1.integrate_RK4(dt);
    sys2.integrate_euler(dt);
    t += dt;
    t_vec.push_back(t);
    x1_vec.push_back(state1.pos);
    x2_vec.push_back(state2.pos);
    spdlog::info("time: {:.2f}, pos: {:.6f}, vel: {:.6f}", t.value(), state1.pos.value(), state1.vel.value());
  }
  plot_units(t_vec, x1_vec, x2_vec);
  /* plot_units(t_vec, x1_vec); */
}

int main()
{
  try {
    sim_pendulum();
    return 0;
  } catch (std::exception &e) {
    std::cerr << e.what() << "\n";
    return 1;
  } catch (...) {
    std::cerr << "something went wrong\n";
    return 2;
  }
}
