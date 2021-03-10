#
#
cmake_minimum_required(VERSION 3.14)

project ("Regex" VERSION 0.1)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(EXEC_NAME "regexpr")
set(SOURCE_CXX_LIST "${EXEC_NAME}.cpp" "../Error/error.cpp" "printFA.cpp")

# Add source to this project's executable.
add_executable(${EXEC_NAME} ${SOURCE_CXX_LIST})

# TODO: Add tests and install targets if needed.
install(TARGETS ${EXEC_NAME} CONFIGURATIONS Debug DESTINATION "Debug")
install(TARGETS ${EXEC_NAME} CONFIGURATIONS Release DESTINATION "Release")
