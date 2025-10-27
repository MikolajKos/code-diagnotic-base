#include <clang-c/Index.h> // G³ówny nag³ówek libclang
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>

namespace fs = std::filesystem;

int main() {
    std::cout << "--- Libclang testing... ---\n";

    CXIndex index = clang_createIndex(0, 0);
    if (index == nullptr) {
        std::cerr << "Nie uda³o siê utworzyæ CXIndex\n";
        return 1;
    }

    const char* source = R"(C:\Users\nikok\projects\clair\src\test_file.cpp)";
    const char* args[] = {
        "-DDEBUG",
        "-I\"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\VC\\Tools\\MSVC\\14.40.33807\\include\""
    };

    CXTranslationUnit tu = clang_parseTranslationUnit(
        index,
        source,
        args, 2,
        nullptr, 0,
        CXTranslationUnit_None
    );

    if (tu == nullptr) {
        std::cerr << "Could not parse translation unit\n";
        clang_disposeIndex(index);
        return 1;
    }

    unsigned numDiags = clang_getNumDiagnostics(tu);
    std::cout << "Errors found: " << numDiags << "\n";

    for (unsigned i = 0; i < numDiags; ++i) {
        CXDiagnostic diag = clang_getDiagnostic(tu, i);
        CXString msg = clang_getDiagnosticSpelling(diag);

        std::string msg_str = clang_getCString(msg);
        std::cout << "ERROR MESSAGE: " << msg_str << "\n";

        clang_disposeString(msg);
        clang_disposeDiagnostic(diag);
    }

    clang_disposeTranslationUnit(tu);
    clang_disposeIndex(index);

    return 0;
}