add_executable(ex05_optimization main.cpp)

target_link_libraries(ex05_optimization PRIVATE cpp_engineering_sandbox::cpp_engineering_sandbox_options
                                                cpp_engineering_sandbox::cpp_engineering_sandbox_warnings)

target_link_system_libraries(
  ex05_optimization
  PRIVATE
  spdlog::spdlog
  fmt::fmt
  Matplot++::matplot
  nlopt)

# FIXME: Without including these files manually, the compiler treats `nlopt.hpp` with `-Werror`. Not sure why the above does not work
target_include_directories(
  ex05_optimization SYSTEM PRIVATE /home/obrusvit/repos/cpp-engineering-sandbox/build/_deps/nlopt-src/src/api
                                   /home/obrusvit/repos/cpp-engineering-sandbox/build/_deps/nlopt-build)

target_include_directories(ex05_optimization PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
