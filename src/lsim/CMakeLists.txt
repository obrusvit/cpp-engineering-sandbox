add_executable(ex04_lsim main.cpp)

target_link_libraries(ex04_lsim PRIVATE cpp_engineering_sandbox::cpp_engineering_sandbox_options
                                        cpp_engineering_sandbox::cpp_engineering_sandbox_warnings)

target_include_directories(ex04_lsim PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)

target_link_system_libraries(
  ex04_lsim
  PRIVATE
  spdlog::spdlog
  Eigen3::Eigen
  fmt::fmt
  Matplot++::matplot
  ftxui::screen
  ftxui::dom
  ftxui::component)
