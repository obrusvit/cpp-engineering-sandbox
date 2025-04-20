add_executable(ex02_system_linear_equations main.cpp)

target_link_libraries(ex02_system_linear_equations PRIVATE cpp_engineering_sandbox::cpp_engineering_sandbox_options
                                                           cpp_engineering_sandbox::cpp_engineering_sandbox_warnings)

target_link_system_libraries(
  ex02_system_linear_equations
  PRIVATE
  spdlog::spdlog
  Eigen3::Eigen
  fmt::fmt
  Matplot++::matplot)

target_include_directories(ex02_system_linear_equations PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
