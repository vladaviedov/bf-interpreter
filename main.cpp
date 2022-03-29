/**
 * @file main.cpp
 * @author Vladyslav Aviedov (vladaviedov@protonmail.com)
 * @brief Brainfuck interpreter and shell written in C++.
 * @version 0.6.0
 * @date 2022-03-29
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include <iostream>
#include <cstdlib>
#include <stack>
#include <string>
#include <istream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <getopt.h>
#include <unistd.h>

#include "bf.hpp"
#include "shell.hpp"

#define VERSION "0.6.0"
#define MEM_DEFAULT 30000

static int newline = 0;
static int interactive = 0;

enum state {
	NO_INPUT,
	ARG_INPUT,
	FILE_INPUT,
	PIPED_INPUT
};

#ifdef __GNU_LIBRARY__
struct option long_opts[] = {
	{"help", 	no_argument,		0, 'h'},
	{"version",	no_argument,		0, 'v'},
	{"file",	required_argument,	0, 'f'},
	{"memory",	required_argument,	0, 'm'},
	{"bytes",	required_argument,	0, 'm'},
	{"newline",	no_argument,		0, 'n'},
	{"shell",	no_argument,		0, 'i'},
	{"repl",	no_argument,		0, 'i'}
};
#endif

void usage(int e);
void version();

int main(int argc, char **argv) {
	uint64_t mem_size = MEM_DEFAULT;
	enum state st = ::NO_INPUT;

	char *filepath;
	char *arg_input;

	char opt;
	#ifdef __GNU_LIBRARY__
	while ((opt = getopt_long(argc, argv, "hvf:m:in", long_opts, NULL)) != -1) {
	#else
	while ((opt = getopt(argc, argv, "hvf:m:in")) != -1) {
	#endif
		switch (opt) {
			case 'h':
				usage(0);
				break;
			case 'v':
				version();
				break;
			case 'f':
				filepath = optarg;
				st = ::FILE_INPUT;
				break;
			case 'm':
				mem_size = atoi(optarg);
				break;
			case 'i':
				interactive = 1;
				break;
			case 'n':
				newline = 1;
				break;
			case '?':
				usage(1);
				break;
		}
	}

	if (!isatty(fileno(stdin))) {
		st = ::PIPED_INPUT;
	}

	if (optind < argc) {
		if (st == ::FILE_INPUT) {
			std::cerr << "warning: -f flag is set, ignoring CLI argument" << std::endl;
		} else {
			st = ::ARG_INPUT;
			arg_input = argv[optind];
		}
	} else {
		if (st == ::NO_INPUT) {
			interactive = 1;
		}
	}

	// initialize memory
	if (bf_malloc(mem_size) < 0) {
		std::cerr << "Failed to allocate memory" << std::endl;
		exit(1);
	}

	// run code
	if (st == ::ARG_INPUT) {
		std::istringstream bf_code(arg_input);
		bf_execute(bf_code);
	}
	if (st == ::FILE_INPUT) {
		std::ifstream bf_code(filepath);
		bf_execute(bf_code);
	}
	if (st == ::PIPED_INPUT) {
		// Read piped input to string stream
		std::istreambuf_iterator<char> eos;
		std::string s(std::istreambuf_iterator<char>(std::cin), eos);
		std::istringstream bf_code(s);

		// Redirect stdin
		freopen("/dev/tty", "r", stdin);

		bf_execute(bf_code);
	}

	if (newline) {
		std::cout << std::endl;
	}
	if (interactive) {
		shell(newline);
	}

	// exit
	bf_free();
	return 0;
}

/**
 * @brief Print program usage and exit.
 * 
 * @param e exit code
 */
void usage(int e) {
	std::cout << "Usage:" << std::endl;
	std::cout << "   bfi [options]" << std::endl;
	std::cout << "   bfi [options] <code>" << "\t" << "(if -f is not set)" << std::endl;
	std::cout << "Options:" << std::endl;
	#ifdef __GNU_LIBRARY__
	std::cout << "  " << "-h, --help" << "\t\t\t" << "print usage" << std::endl;
	std::cout << "  " << "-f, --file <filepath>" << "\t\t" << "execute code from a file" << std::endl;
	std::cout << "  " << "-m, --memory, bytes <size>" << "\t" << "specify memory size in bytes (default: 30000)" << std::endl;
	std::cout << "  " << "-i, --shell, repl" << "\t\t" << "interactive REPL shell" << std::endl;
	std::cout << "  " << "-n, --newline" << "\t\t\t" << "print newline at the end the program & toggle newline for shell" << std::endl;
	#else
	std::cout << "  " << "-h" << "\t\t\t" << "print usage" << std::endl;
	std::cout << "  " << "-f <filepath>" << "\t\t" << "execute code from a file" << std::endl;
	std::cout << "  " << "-m <size>" << "\t\t" << "specify memory size in bytes (default: 30000)" << std::endl;
	std::cout << "  " << "-i" << "\t\t\t" << "interactive REPL shell" << std::endl;
	std::cout << "  " << "-n" << "\t\t\t" << "print newline at the end the program & toggle newline for shell" << std::endl;
	#endif
	exit(e);
}

/**
 * @brief Print program version and exit.
 * 
 */
void version() {
	std::cout << "bfi " << VERSION << std::endl;
	exit(0);
}
