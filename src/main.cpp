#include <clang-c/Index.h> // G³ówny nag³ówek libclang
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>
#include <nlohmann\json.hpp>
#include <CompilationDatabase.hpp>

namespace fs = std::filesystem;

int main() {
    fs::path dir_path = "C:\\Users\\nikok\\projects\\clair_target\\build";
    fs::path full_path = dir_path / "compile_commands.json";

    CompilationDb db(full_path);

    std::cout << "--- Libclang testing... ---\n";

    CXIndex index = clang_createIndex(0, 0);
    if (index == nullptr) {
        std::cerr << "Nie uda³o siê utworzyæ CXIndex\n";
        return 1;
    }

    const char* source = "C:\\Users\\nikok\\projects\\clair_target\\src\\main.cpp";

    std::vector<const char*> flags_c;
    std::vector<std::string> flags_str = db.get_flags();

    // Set the target architecture, OS, and environment (MSVC)
    // Crucial for libclang to correctly interpret MSVC/Windows system headers.
    flags_c.push_back("--target=x86_64-pc-windows-msvc");

    // Parse flags from vector<std::string> to vector<const char *>
    for (const auto& item : flags_str) {
        flags_c.push_back(item.c_str());
    }

    CXTranslationUnit tu = clang_parseTranslationUnit(
        index,
        source,
        flags_c.data(),
        (int)flags_c.size(),
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