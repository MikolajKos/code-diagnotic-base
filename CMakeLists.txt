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

# --- Include FetchContent Module ---
include(FetchContent)

# --- Declare nlohmann/json dependency ---
FetchContent_Declare(
  nlohmann_json
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3 # Or specify another release tag or branch
)

# --- Make the dependency available ---
# This will download the source (if needed) and add its targets
FetchContent_MakeAvailable(nlohmann_json)

add_executable(codiag src/main.cpp "src/CompilationDatabase.cpp")

target_include_directories(codiag PRIVATE ${LLVM_INCLUDE_DIRS})

target_compile_definitions(codiag PRIVATE ${LLVM_DEFINITIONS})

target_include_directories(codiag PUBLIC src/includes)

# --- Link Libraries ---
target_link_libraries(codiag PRIVATE ${LIBCLANG_LIBRARY})

target_link_libraries(codiag PRIVATE
    ${LIBCLANG_LIBRARY}             # Link against libclang
    nlohmann_json::nlohmann_json    # Link against the fetched nlohmann::json target
)