
# Copyright (c) 2021 Vitaly Dikov
# 
# Distributed under the Boost Software License, Version 1.0.
# (See accompanying file LICENSE or copy at https://www.boost.org/LICENSE_1_0.txt)

cmake_minimum_required(VERSION 3.14)

project("Grammar" VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

set(EXEC_NAME "grammar")
set(SOURCE_CXX_LIST "${EXEC_NAME}.cpp"
                    "../Error/error.cpp"
                    )

# Add source to this project's executable.
add_executable(${EXEC_NAME} ${SOURCE_CXX_LIST})

# Add tests and install targets if needed.
install(TARGETS ${EXEC_NAME} CONFIGURATIONS Debug DESTINATION "Debug")
install(TARGETS ${EXEC_NAME} CONFIGURATIONS Release DESTINATION "Release")
install(FILES "OriginalGrammar.txt" CONFIGURATIONS Debug DESTINATION "Debug")
install(FILES "OriginalGrammar.txt" CONFIGURATIONS Release DESTINATION "Release")
