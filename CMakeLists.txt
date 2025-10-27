cmake_minimum_required(VERSION 3.20)
project(code_diagnostic LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(LLVM REQUIRED clang)

find_library(
    LIBCLANG_LIBRARY     # Nazwa zmiennej, w której zapiszemy œcie¿kê
    NAMES libclang       # Nazwa biblioteki (CMake doda .lib na Windows)
    HINTS ${LLVM_LIBRARY_DIRS} # Przeszukaj katalogi bibliotek, które znalaz³ LLVM
    REQUIRED             # Zakoñcz b³êdem, jeœli nie znajdzie
)

message(STATUS "Znaleziono bibliotekê libclang w: ${LIBCLANG_LIBRARY}")

add_executable(codiag src/main.cpp)

target_include_directories(codiag PRIVATE ${LLVM_INCLUDE_DIRS})

target_compile_definitions(codiag PRIVATE ${LLVM_DEFINITIONS})

target_link_libraries(codiag PRIVATE ${LIBCLANG_LIBRARY})