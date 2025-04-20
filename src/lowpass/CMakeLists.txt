add_executable(ex01_low_pass_filter main.cpp)

target_link_libraries(ex01_low_pass_filter PRIVATE cpp_engineering_sandbox::cpp_engineering_sandbox_options
                                                   cpp_engineering_sandbox::cpp_engineering_sandbox_warnings)

target_link_system_libraries(
  ex01_low_pass_filter
  PRIVATE
  spdlog::spdlog
  fmt::fmt
  Matplot++::matplot)

target_include_directories(ex01_low_pass_filter PRIVATE $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>)
