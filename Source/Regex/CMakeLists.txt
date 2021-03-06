﻿
# Copyright (c) 2021 Vitaly Dikov
# 
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.14)

project("Regex" VERSION 0.1)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

option(REGEX_PRINT_FA_STATE "Output the state of the finite state machines in the stdout" OFF)



configure_file("regexpr_config.hpp.in" "${CMAKE_CURRENT_SOURCE_DIR}/regexpr_config.hpp" @ONLY)

set(EXEC_NAME "regexpr")
set(SOURCE_CXX_LIST "${EXEC_NAME}_main.cpp"
                    "${EXEC_NAME}.cpp"
                    "../Error/error.cpp"
                    )

# Add source to this project's executable.
add_executable(${EXEC_NAME} ${SOURCE_CXX_LIST})

# Add tests and install targets if needed.
install(TARGETS ${EXEC_NAME} CONFIGURATIONS Debug DESTINATION "Debug")
install(TARGETS ${EXEC_NAME} CONFIGURATIONS Release DESTINATION "Release")
