#include <array>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <fmt/ranges.h>
#include <matplot/matplot.h>
#include <nlopt.hpp>
#include <spdlog/common.h>
#include <spdlog/spdlog.h>


namespace {

struct constraints_data
{
  double a, b;
};

struct objective_additional_data
{
  int count;
};

double objective_function(const std::vector<double> &x, std::vector<double> &grad, void *my_func_data = nullptr)
{
  if (my_func_data != nullptr) {
    auto *additional_data = static_cast<objective_additional_data *>(my_func_data);
    ++additional_data->count;
  }
  if (!grad.empty()) {
    grad[0] = 0.0;
    grad[1] = 0.5 / sqrt(x[1]);
  }
  return sqrt(x[1]);
}

double constraint_functions(const std::vector<double> &x, std::vector<double> &grad, void *data)
{
  const auto *d = static_cast<constraints_data *>(data);
  double a = d->a;
  double b = d->b;
  if (!grad.empty()) {
    grad[0] = 3 * a * (a * x[0] + b) * (a * x[0] + b);
    grad[1] = -1.0;
  }
  return ((a * x[0] + b) * (a * x[0] + b) * (a * x[0] + b) - x[1]);
}


void plot_optimization_problem(double x1_opt, double x2_opt)
{
  using namespace matplot;

  // Grid
  size_t N = 200;
  std::vector<double> x1 = linspace(-0.5, 1.0, N);
  std::vector<double> x2 = linspace(-1.0, 4.0, N);

  auto [X1, X2] = meshgrid(x1, x2);

  // Constraint data
  constraints_data c1_data{ 2.0, 0.0 };
  constraints_data c2_data{ -1.0, 1.0 };

  // Storage
  size_t nrows = X1.size();
  size_t ncols = X1[0].size();
  assert(nrows == x2.size());
  assert(ncols == x1.size());

  std::vector<std::vector<double>> feasible(nrows, std::vector<double>(ncols, 0.0));
  std::vector<std::vector<double>> obj(nrows, std::vector<double>(ncols, 0.0));
  std::vector<double> grad_dummy(2);

  for (size_t i = 0; i < nrows; ++i) {
    for (size_t j = 0; j < ncols; ++j) {
      std::vector<double> x = { X1[i][j], X2[i][j] };
      bool is_feasible = constraint_functions(x, grad_dummy, &c1_data) <= 0.0
                         && constraint_functions(x, grad_dummy, &c2_data) <= 0.0 && x[1] >= 0.0;
      if (is_feasible) {
        feasible[i][j] = 1.0;
        obj[i][j] = objective_function(x, grad_dummy, nullptr);
      }
    }
  }

  // Plot feasible region as filled contour
  // contourf(X1, X2, feasible)->line_width(1);
  // cf->n_levels(N);
  // cf->filled(true);
  hold(on);

  // Plot constraint boundaries
  std::vector<double> c1_boundary(N);
  std::vector<double> c2_boundary(N);
  for (size_t i = 0; i < N; ++i) {
    c1_boundary[i] = std::pow(2.0 * x1[i] + 0.0, 3);
    c2_boundary[i] = std::pow(-1.0 * x1[i] + 1.0, 3);
  }
  plot(x1, c1_boundary, "r-")->line_width(2).display_name("Constraint 1");
  plot(x1, c2_boundary, "b-")->line_width(2).display_name("Constraint 2");

  // Plot objective function contours
  // contour(X1, X2, obj, 10)->line_width(1).display_name("Objective sqrt(x2)");
  // contour(X1, X2, obj, 10)->line_width(1).display_name(" ");
  // contour(X1, X2, obj)->line_width(1);

  // Plot optimum
  plot({ x1_opt }, { x2_opt }, "ko")->marker_size(10).display_name("Optimum");
  // text(x1_opt, x2_opt, "Optimum", "FontSize", 12);

  // Style
  xlabel("x1");
  ylabel("x2");
  title("Feasible Region, Constraints, and Objective Contours");
  legend();
  colormap(palette::parula());

  show();
}
}// namespace

int main()
{
  try {
    spdlog::set_level(spdlog::level::debug);
    spdlog::info("Starting example 5: optimization problem with nlopt");

    nlopt::opt opt(nlopt::algorithm::LD_MMA, 2);// ctor with algorithm and dimensionality

    const std::vector<double> lb{ -HUGE_VAL, 0.0 };
    opt.set_lower_bounds(lb);

    objective_additional_data obj_add_data{};
    opt.set_min_objective(objective_function, &obj_add_data);

    std::array<constraints_data, 2> data = { { { 2.0, 0.0 }, { -1.0, 1.0 } } };// NOLINT
    opt.add_inequality_constraint(constraint_functions, &data[0], 1e-8);// NOLINT
    opt.add_inequality_constraint(constraint_functions, &data[1], 1e-8);// NOLINT

    opt.set_xtol_rel(1e-4);// NOLINT

    std::vector<double> x{ 1.234, 5.678 };// some initial guess //NOLINT
    double minf{};// the minimum objective value, upon return
    nlopt::result result = opt.optimize(x, minf);

    spdlog::info("result: {}", result);
    spdlog::info("found minimum after {} evaluations", obj_add_data.count);
    spdlog::info("found minimum at f({:.3},{:.3}) = {:.3}", x[0], x[1], minf);

    plot_optimization_problem(x[0], x[1]);

    return 0;
  } catch (std::exception &e) {
    spdlog::error("nlopt failed");
    spdlog::error(e.what());
    return 1;
  } catch (...) {
    spdlog::error("something went wrong");
    return 2;
  }
}
