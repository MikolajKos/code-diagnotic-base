#include <clang-c/Index.h> // G³ówny nag³ówek libclang
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <filesystem>
#include <nlohmann\json.hpp>
#include <CompilationDatabase.hpp>

namespace fs = std::filesystem;

enum CXChildVisitResult my_visitor(CXCursor cursor, CXCursor parent, void* client_data) {
	CXCursorKind kind = clang_getCursorKind(cursor);

	// Get cursor name
	CXString spelling_str = clang_getCursorSpelling(cursor);
	const char* spelling = clang_getCString(spelling_str);

	// Get cursor location
	CXSourceLocation loc = clang_getCursorLocation(cursor);

	CXFile file;
	unsigned int line, col;
	clang_getSpellingLocation(loc, &file, &line, &col, NULL);

	std::vector<std::string>* found_funcs =
		static_cast<std::vector<std::string>*>(client_data);

	if (kind == CXCursor_FunctionDecl) {
		std::string info = "I found function: ";
		info += spelling;
		found_funcs->push_back(info);
	}

	return CXChildVisit_Recurse;
}

CXCursor findContainingFunc(CXCursor cursor) {
	CXCursor current = cursor;

	while (!clang_Cursor_isNull(current)) {
		CXCursorKind kind = clang_getCursorKind(current);

		if (kind == CXCursor_FunctionDecl) {
			return current;
		}

		current = clang_getCursorSemanticParent(current);
	}

	return clang_getNullCursor();
}

std::string getFileContents(std::string filename) {
	std::ifstream file(filename);
	if (!file) {
		std::cerr << "Cannot open the file\n";
		return "	";
	}

	std::stringstream buffer;
	buffer << file.rdbuf();
	return buffer.str();
}

std::string getSourceText(CXSourceRange range) {
	CXSourceLocation start_loc = clang_getRangeStart(range);
	CXSourceLocation end_loc = clang_getRangeEnd(range);

	unsigned int start_offset, end_offset;
	CXFile file;

	clang_getSpellingLocation(start_loc, &file, NULL, NULL, &start_offset);
	clang_getSpellingLocation(end_loc, &file, NULL, NULL, &end_offset);

	if (file == NULL) {
		return "No file was found\n";
	}

	// Getting filename
	CXString file_str = clang_getFileName(file);
	std::string filename = clang_getCString(file_str);
	clang_disposeString(file_str);

	if (filename.empty()) {
		return "Filename is empty\n";
	}

	// Reading whole file
	std::string file_contents = getFileContents(filename);
	if (file_contents.empty()) {
		return "Cannot open the file " + filename + "\n";
	}

	// Reading data by offset
	unsigned int length = end_offset - start_offset;
	if (end_offset <= start_offset || (start_offset + length) > file_contents.length()) {
		return "Error: incorrect range\n";
	}

	return file_contents.substr(start_offset, length);
}

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

	// DIAGNOSTIC PART

	unsigned num_diag = clang_getNumDiagnostics(tu);

	// Null cursor to which error func will be asigned
	CXCursor error_cursor = clang_getNullCursor();
	// Error text message
	std::string error_msg;

	for (unsigned i = 0; i < num_diag; ++i) {
		CXDiagnostic diag = clang_getDiagnostic(tu, i);

		// Ignoring warnings and everything less than error
		if (clang_getDiagnosticSeverity(diag) < CXDiagnostic_Error) {
			continue;
		}

		// Getting error message
		CXString msg_str = clang_getDiagnosticSpelling(diag);
		error_msg = clang_getCString(msg_str);
		clang_disposeString(msg_str);

		// Getting error location
		CXSourceLocation loc = clang_getDiagnosticLocation(diag);
		if (clang_equalLocations(loc, clang_getNullLocation())) {
			clang_disposeDiagnostic(diag);
			continue;
		}

		// Getting cursor where error occurred
		error_cursor = clang_getCursor(tu, loc);
		clang_disposeDiagnostic(diag);
		break;
	}

	// Generating Context
	std::string func_code;
	if (clang_Cursor_isNull(error_cursor)) {
		std::cout << "No errors were found\n";
	}
	else {
		std::cout << "Error was found: " << error_msg << "\n";
		CXCursor func_cursor = findContainingFunc(error_cursor);

		if (!clang_Cursor_isNull(func_cursor)) {
			// Get the range of function declaration
			CXSourceRange range = clang_getCursorExtent(func_cursor);

			func_code = getSourceText(range);
		}
		else {
			std::cout << "Error occurred outside the function, for example globally.\n";
		}
	}

	std::cout << "--- GENERATED CONTEXT ---\n";
	std::cout << func_code << "\n";

	return 0;
}

// VISITOR IMPLEMENTATION
/*	CXCursor cursor = clang_getTranslationUnitCursor(tu);

	std::vector<std::string> findings;

	unsigned int num_errors = clang_visitChildren(
		cursor,
		my_visitor,
		&findings
	);

	for (const auto& item : findings) {
		std::cout << item << "\n";
	}

	clang_disposeTranslationUnit(tu);
	clang_disposeIndex(index);

	return 0;
*/


/*
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
*/
