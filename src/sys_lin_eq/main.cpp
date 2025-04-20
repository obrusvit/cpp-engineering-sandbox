#include <Eigen/Dense>
#include <fmt/ostream.h>
#include <matplot/core/figure_registry.h>
#include <matplot/matplot.h>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>

#include <cpp_engineering_sandbox/eigen_utils.hpp>

using namespace Eigen;
using namespace std;

void plot_surface(auto A, auto b)
{
  using namespace matplot;
  auto [X, Y] = meshgrid(linspace(-5, +5, 40), linspace(-5, +5, 40));

  auto a0 = A.row(0);
  auto a1 = A.row(1);
  auto a2 = A.row(2);
  auto Z0 = transform(X, Y, [&a0, &b](double x, double y) { return -a0(0) * x - a0(1) * y - b(0); });
  auto Z1 = transform(X, Y, [&a1, &b](double x, double y) { return -a1(0) * x - a1(1) * y - b(0); });
  auto Z2 = transform(X, Y, [&a2, &b](double x, double y) { return -a2(0) * x - a2(1) * y - b(0); });

  auto f = figure(true);

  auto ax = f->current_axes();
  auto p1 = ax->surf(X, Y, Z0)->face_alpha(0.5);
  /* p1->color({0, 1, 0, 0.5}); */
  colormap(palette::parula(5));

  hold(on);
  auto p2 = ax->surf(X, Y, Z1)->face_alpha(0.5);
  auto p3 = ax->surf(X, Y, Z2)->face_alpha(0.5);
  hold(off);
  /* p->color("red").line_width(2); // does not draw */
  f->show();// draw only once and pause console
}

void solve_system_of_linear_equations(const MatrixXd &A, const VectorXd &b)
{
  spdlog::info("Solving a system of linear equations");
  spdlog::info("Matrix A:\n{}", A);
  spdlog::info("Vector b:\n{}", b);
  VectorXd solSvd = A.bdcSvd(ComputeThinU | ComputeThinV).solve(b);
  VectorXd solQr = A.colPivHouseholderQr().solve(b);
  VectorXd solNorm = (A.transpose() * A).ldlt().solve(A.transpose() * b);
  VectorXd solLu = A.lu().solve(b);
  spdlog::info("The least-squares solution is:\n{}", solSvd);
  spdlog::info("The solution using the QR decomposition is:\n{}", solQr);
  spdlog::info("The solution using normal equations is:\n{}", solNorm);
  spdlog::info("The solution using the Lu decomposition is:\n{}", solLu);
  plot_surface(A, b);
}

int main()
{
  try {
    MatrixXd A = MatrixXd::Random(3, 3);
    VectorXd b = VectorXd::Random(3);
    solve_system_of_linear_equations(A, b);

    // clang-format off
        A << 1.0, 1.0, 3.0, 
             0.0, 1.0, 2.0, 
             0.0, 0.0, 1.0;
        b << 1, 1, 3;
    // clang-format on
    solve_system_of_linear_equations(A, b);

    return 0;
  } catch (std::exception &e) {
    spdlog::error(e.what());
    return 1;
  } catch (...) {
    spdlog::error("something went wrong");
    return 2;
  }
}
