#include <Eigen/Dense>
#include <Eigen/src/Core/Matrix.h>
#include <Eigen/src/Core/util/Constants.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <ftxui/dom/deprecated.hpp>
#include <ftxui/screen/color.hpp>
#include <iostream>
#include <matplot/freestanding/axes_functions.h>
#include <matplot/matplot.h>
#include <span>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>
#include <string>
#include <vector>

#include "cpp_engineering_sandbox/eigen_utils.hpp"
#include "cpp_engineering_sandbox/signal_utils.hpp"

struct System
{
  Eigen::MatrixXd A;
  Eigen::MatrixXd B;
  Eigen::MatrixXd C;
  Eigen::MatrixXd D;

  System() = default;
  System(Eigen::MatrixXd matA, Eigen::MatrixXd matB, Eigen::MatrixXd matC, Eigen::MatrixXd matD)
    : A{ std::move(matA) }, B{ std::move(matB) }, C{ std::move(matC) }, D{ std::move(matD) }
  {}

  System(Eigen::MatrixXd matA, Eigen::MatrixXd matB, Eigen::MatrixXd matC)
    : A{ std::move(matA) }, B{ std::move(matB) }, C{ std::move(matC) }, D{ Eigen::MatrixXd::Zero(C.rows(), B.cols()) }
  {}
};

std::ostream &operator<<(std::ostream &os, const System &sys)
{
  os << "System:\nA:\n" << sys.A;
  os << "\nB:\n" << sys.B;
  os << "\nC:\n" << sys.C;
  os << "\nD:\n" << sys.D;
  return os;
}
template<> struct fmt::formatter<System> : ostream_formatter
{
};

System get_system_random()
{
  std::random_device rd;
  const auto seed = rd();
  spdlog::debug("seed: {}", seed);
  std::mt19937 gen(seed);

  std::uniform_int_distribution<int> uni(1, 4);// Guaranteed unbiased
  long xN = uni(gen);
  long uN = uni(gen);
  long yN = uni(gen);

  std::uniform_real_distribution<double> dis(-1.0, 1.0);
  const auto rand_num = [&gen, &dis]() -> double { return dis(gen); };
  Eigen::MatrixXd A = Eigen::MatrixXd::NullaryExpr(xN, xN, rand_num);
  Eigen::MatrixXd B = Eigen::MatrixXd::NullaryExpr(xN, uN, rand_num);
  Eigen::MatrixXd C = Eigen::MatrixXd::Identity(yN, xN);
  Eigen::MatrixXd D = Eigen::MatrixXd::Zero(yN, uN);
  return System{ A, B, C, D };
}

System get_system_cartpole_01()
{
  // discretization with Ts=0.1
  // continuous system has matrices:
  // A, B, C of values
  // A = [0.0, 1.0, 0.0, 0.0; 0.0, -0.5, 0.981, 0.05; 0.0, 0.0, 0.0, 1.0; 0.0, 0.25, -5.3955, -0.275]
  // B = [0.0; 1.0; 0.0; -0.5]
  // C = [1.0, 0.0, 0.0, 0.0; 0.0, 1.0, 0.0, 0.0; 0.0, 0.0, 1.0, 0.0; 0.0, 0.0, 0.0, 1.0]
  Eigen::Matrix<double, 4, 4> A;
  // clang-format off
    A << 1.0,  0.09754415800251533,     0.0047586713511242615,  0.00040248000449906516,//NOLINT
         0.0,  0.9513285409998672,      0.09351923813619285,    0.009525197250012787,//NOLINT
         0.0,  0.0012127093147615345,   0.9734274622149495,     0.09775593660729356,//NOLINT
         0.0,  0.02383262949444262,    -0.5262524881268714,     0.9466052151136823;//NOLINT
    Eigen::Matrix<double, 4, 1> B;
    B << 0.00491168399496933,//NOLINT
         0.0973429180002658,//NOLINT
        -0.0024254186295230695,//NOLINT
        -0.04766525898888525;//NOLINT
  // clang-format on
  Eigen::Matrix<double, 4, 4> C = Eigen::Matrix<double, 4, 4>::Identity();
  return System{ A, B, C };
}

System get_system_cartpole_001()
{
  // discretization with Ts=0.01
  Eigen::Matrix<double, 4, 4> A;
  // clang-format off
    A << 1.0,  0.009975043793209074,    4.892128114437166e-5,  2.656618610414282e-6, //NOLINT
         0.0,  0.995013142258048,       0.009771184175425615,  0.0005469429006869616,//NOLINT
         0.0,  1.2467197029656384e-5,   0.9997305250056948,    0.009985366784911109, //NOLINT
         0.0,  0.0024901080977129493,  -0.053863816167701806,  0.9969851724996955;   //NOLINT
    Eigen::Matrix<double, 4, 1> B;
    B << 4.9912413581847744e-5,//NOLINT
         0.00997371548390387,//NOLINT
        -2.493439405931277e-5,//NOLINT
        -0.004980216195425899;//NOLINT
  // clang-format on
  Eigen::Matrix<double, 4, 4> C = Eigen::Matrix<double, 4, 4>::Identity();
  return System{ A, B, C };
}

Eigen::MatrixXd
  lsim(const System &sys, const Eigen::MatrixXd &u, const std::vector<double> &t, const Eigen::VectorXd &x0)
{
  spdlog::info("lsim simulating the following system:\n{}\ntimespan from {:.2f} to {:.2f}\ninputs:\n{}",
    sys,
    t.front(),
    t.back(),
    u);
  Eigen::MatrixXd resX = Eigen::MatrixXd(sys.A.rows(), t.size());
  Eigen::MatrixXd resY = Eigen::MatrixXd(sys.C.rows(), t.size());
  Eigen::VectorXd x = x0;
  Eigen::VectorXd y{};
  for (int i = 0; i < static_cast<int>(t.size()); ++i) {
    y = sys.C * x + sys.D * u.col(i);
    resY.col(i) = y;
    resX.col(i) = x;
    x = sys.A * x + sys.B * u.col(i);
    spdlog::trace("index: {}, t = {:.2f}, y = {}",
      i,
      t.at(static_cast<size_t>(i)),
      std::span{ y.data(), static_cast<size_t>(y.size()) });
  }
  spdlog::info("result:\n{}\n", resY);
  return resY;
}

void plot_lsim_result(const std::vector<double> &time, const Eigen::MatrixXd &y)
{
  using namespace matplot;
  auto f = figure(true);
  auto ax = f->current_axes();
  std::vector<std::string> legend_labels{};
  for (int i = 0; i < y.rows(); ++i) {
    Eigen::VectorXd state = y.row(i);
    const std::vector stateVec(state.data(), state.data() + state.size());// NOLINT
    legend_labels.emplace_back(fmt::format("y{}", i));
    ax->plot(time, stateVec)->line_width(2);
    hold(on);
  }
  hold(off);
  ax->legend(legend_labels);
  ax->title("lsim results");
  ax->xlabel("Time [s]");
  ax->ylabel("Output values [-]");
  ax->grid(on);
  f->show();// draw only once and pause console
}

struct bool_wrap
{
  bool_wrap() = default;
  explicit(false) bool_wrap(bool b) : val{ b } {}// NOLINT
  explicit(false) operator bool() const { return val; }// NOLINT

  bool *operator&() { return &val; }

private:
  bool val{};
};


int main()
{
  try {
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Starting example 4: linear system simulation");
    const auto sys = get_system_cartpole_01();
    const auto time = linspace(0.0, 5.0, 100);
    Eigen::MatrixXd u = Eigen::MatrixXd::Zero(sys.B.cols(), static_cast<long>(time.size()));
    u(0, 1) = 1.0;
    u(0, 2) = 1.0;
    u(0, 3) = 1.0;
    u(0, 4) = 1.0;
    u(0, 5) = 1.0;
    const Eigen::VectorXd x0 = Eigen::VectorXd::Zero(sys.A.rows());
    const Eigen::MatrixXd y = lsim(sys, u, time, x0);
    plot_lsim_result(time, y);

    return 0;
  } catch (std::exception &e) {
    spdlog::error(e.what());
    return 1;
  } catch (...) {
    spdlog::error("something went wrong");
    return 2;
  }
}
