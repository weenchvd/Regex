#
#
cmake_minimum_required(VERSION 3.14)

project ("RegexTest" VERSION 0.1)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(EXEC_NAME "regexpr_test")
set(SOURCE_CXX_LIST "${EXEC_NAME}.cpp"
					"${CMAKE_SOURCE_DIR}/Source/Regex/regexpr.cpp"
					"${CMAKE_SOURCE_DIR}/Source/Regex/printFA.cpp"
					"${CMAKE_SOURCE_DIR}/Source/Error/error.cpp"
					)

# Add source to this project's executable.
add_executable(${EXEC_NAME} ${SOURCE_CXX_LIST})
target_link_libraries(${EXEC_NAME} gtest_main)
target_include_directories(${EXEC_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/Source")

# Add tests and install targets if needed.
add_test(NAME ${EXEC_NAME} COMMAND ${EXEC_NAME})
install(FILES "ValidRegexes.txt" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
install(FILES "InvalidRegexes.txt" DESTINATION "${CMAKE_CURRENT_BINARY_DIR}")
