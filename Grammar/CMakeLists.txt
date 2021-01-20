# CMakeList.txt : CMake project for Template, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

project ("Grammar")

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/..)

set (FILE_NAME grammar)
set (SOURCE_CXX_LIST ${FILE_NAME}.cpp ../Error/error.cpp)

# Add source to this project's executable.
add_executable (${FILE_NAME} ${SOURCE_CXX_LIST})

# TODO: Add tests and install targets if needed.
install (TARGETS ${FILE_NAME} CONFIGURATIONS Debug DESTINATION Build/Debug)
install (TARGETS ${FILE_NAME} CONFIGURATIONS Release DESTINATION Build)
install (FILES "OriginalGrammar.txt" CONFIGURATIONS Debug DESTINATION Build/Debug)
install (FILES "OriginalGrammar.txt" CONFIGURATIONS Release DESTINATION Build)
