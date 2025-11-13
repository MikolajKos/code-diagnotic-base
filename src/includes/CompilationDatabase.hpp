#ifndef COMPILATION_DATABASE_HPP
#define COMPILATION_DATABASE_HPP

#include <filesystem>
#include <sstream>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>
#include <nlohmann\json.hpp>

//#define DEBUG

namespace fs = std::filesystem;
using json = nlohmann::json;

/*
*	This class will need reimplementation because of its unsafe features.
*	
*	Things to reimplement:
*	1.	in get_flags() first flag is being ingored because 
*		in given test case it coresponds to compilers include directory,
*		which is replaced by --target flag in main.cpp, 
*		specifying the environment of diagnostic program
*	
*	2.	in get_full_command() commands are being selected
*		with database[0]["command"] statement which doesnt
*		guarantee reading correct data. Serching mechanism
*		for compile_commands.json must be implemented
*/

class CompilationDb {
	const fs::path path;
	json database; // Json variable for command field storage from compile_commands.json
	std::string command; // Storage for 'command' field from compile_commands.json
public:
	CompilationDb(const fs::path& p) : path(p) {};

	std::vector<std::string> get_commands_list() {
		get_full_command();
		if (command.empty()) {
			std::cerr << "Data was not read\n";
			return {};
		}

		std::vector<std::string> args;
		std::stringstream ss(command);
		char c;
		bool in_quotes = false;
		std::string current_arg;

		ss.get(c);

		while (ss) {
			if (c == '"') {
				current_arg += c;
				in_quotes = !in_quotes;
			}
			else if (c == ' ' && !in_quotes) {
				if (!current_arg.empty()) {
					args.push_back(current_arg);
					current_arg.clear();
				}
			}
			else {
				current_arg += c;
			}

			ss.get(c);
		}

		if (!current_arg.empty()) {
			args.push_back(current_arg);
		}

		return args;
	}

	std::vector<std::string> get_flags() {
		std::vector<std::string> all_flags;
		std::vector<std::string> all_commands = get_commands_list();
		
		for (auto& item : all_commands){
			// DEBUG!!!!!!! If statement for ignoring first element (if (item.rfind("-IC:/toolchains", 0) == 0))
			// This instruction is temporary and unsafe.
			// It removes first flag which in this case
			// is compiler include parameters.
			// But if structure is different, wrong flag might be deleted.
			if (item.rfind("-IC:/toolchains", 0) == 0) {
				continue;
			}
			else if (item.rfind("-I", 0) == 0) {
				all_flags.push_back(item);
			}
			else if (item.rfind("-D", 0) == 0) {
				all_flags.push_back(item);
			}
		}

		return all_flags;
	}

	void get_full_command() {
		if (read_compile_commands() != 0) {
			std::cerr << "Could not read the file\n";
			return;
		}

		// If compile_commands.json structure was changed [0]["command"] must be reimplemented
		try {
			command = database[0]["command"].get<std::string>();
		}
		catch (const json::out_of_range& ex) {
			std::cerr << "Command field was not found\n";
			return;
		}
	}

private:
	int read_compile_commands() {
		std::ifstream in_file(path);
		if (!in_file) {
			std::cerr << "Could not open the file\n";
			return 1;
		}

		// Reading compile_commands.json
		try {
			in_file >> database;
		}
		catch (const json::parse_error& e) {
			std::cerr << "Could not parse file\n";
			in_file.close();
			return 1;
		}
		in_file.close();

		return 0;
	}
};

#endif
