#
#
cmake_minimum_required(VERSION 3.14)

project ("RegexTest" VERSION 0.1)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(EXEC_NAME "regexpr-test")
set(SOURCE_CXX_LIST "${EXEC_NAME}.cpp")

# Add source to this project's executable.
add_executable(${EXEC_NAME} ${SOURCE_CXX_LIST})
target_link_libraries(${EXEC_NAME} gtest_main)

# Add tests and install targets if needed.
add_test(NAME "RegexTest" COMMAND ${EXEC_NAME})
