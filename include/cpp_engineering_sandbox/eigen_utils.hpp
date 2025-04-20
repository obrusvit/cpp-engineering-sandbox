#ifndef CPP_ENGINEERING_SANDBOX_EIGEN_UTILS
#define CPP_ENGINEERING_SANDBOX_EIGEN_UTILS

#include <Eigen/Dense>
#include <fmt/ostream.h>

template<> struct fmt::formatter<Eigen::MatrixXd> : ostream_formatter
{
};
template<> struct fmt::formatter<Eigen::VectorXd> : ostream_formatter
{
};

#endif
