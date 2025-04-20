add_executable(ex03_integrator main.cpp)

target_link_libraries(ex03_integrator PRIVATE cpp_engineering_sandbox::cpp_engineering_sandbox_options
                                              cpp_engineering_sandbox::cpp_engineering_sandbox_warnings)

target_link_system_libraries(
  ex03_integrator
  PRIVATE
  spdlog::spdlog
  fmt::fmt
  units::units
  Matplot++::matplot)

target_include_directories(ex03_integrator PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
