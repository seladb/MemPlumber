cmake_minimum_required(VERSION 3.12)
project(memplumber CXX)

add_library(memplumber memplumber.cpp)

# Build options
option(COLLECT_STATIC_VAR_DATA
       "Collect data also on static variable memory allocation" OFF)

if(COLLECT_STATIC_VAR_DATA)
  add_definitions(-DCOLLECT_STATIC_VAR_DATA)
endif()

target_include_directories(memplumber PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# execinfo for backtraces. This is only required on FreeBSD, as both Mac OS X
# and Linux include backtrace functions in libSystem / libc.
if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
  find_package(Backtrace REQUIRED)

  target_include_directories(memplumber PUBLIC ${Backtrace_INCLUDE_DIRS})
  target_link_libraries(memplumber PUBLIC ${Backtrace_LIBRARIES})
endif()

enable_testing()

add_subdirectory(Tests)

add_subdirectory(Examples)
