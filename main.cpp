/**
 * @file main.cpp
 * @author Vladyslav Aviedov (vladaviedov@protonmail.com)
 * @brief Interpreter for the Brainfuck esoteric language written in C++.
 * @version 0.2
 * @date 2022-02-11
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
#include <getopt.h>

#define VERSION 0.2
#define MEM_DEFAULT 256

#define SHELL_PS1 "bf> "
#define SHELL_CMD_PREFIX '$'
#define SHELL_WINDOW_SIZE 5

// brainfuck commands
#define PTR_INC '>'
#define PTR_DEC '<'
#define MEM_INC '+'
#define MEM_DEC '-'
#define PUT_CHR '.'
#define GET_CHR ','
#define JMP_FWD '['
#define JMP_BCK ']'

uint8_t* memory;
uint64_t ptr_loc;
uint64_t mem_size;
std::stack<uint64_t> goto_stack;

int newline;

enum state {
	NO_INPUT,
	ARG_INPUT,
	FILE_INPUT
};

#ifdef __GNU_LIBRARY__
struct option long_opts[] = {
	{"help", 	no_argument,		0, 'h'},
	{"version",	no_argument,		0, 'v'},
	{"file",	required_argument,	0, 'f'},
	{"memory",	required_argument,	0, 'm'},
	{"bytes",	required_argument,	0, 'm'},
	{"newline",	no_argument,		0, 'n'},
	{"shell",	no_argument,		0, 'i'}
};
#endif

void usage(int e);
int execute(std::istream& code);
int verify(std::istream& code);
uint64_t ptr_offset(int offset);
int jump_ff(std::istream& code);
void shell();
int shell_cmd(std::string input);

int main(int argc, char** argv) {
	mem_size = MEM_DEFAULT;
	enum state st = ::NO_INPUT;
	int interactive = 0;
	newline = 0;
	char* filepath;
	char* arg_input;

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
				std::cout << "bfi " << VERSION << std::endl;
				exit(0);
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
	ptr_loc = 0;
	memory = (uint8_t*)std::calloc(mem_size, sizeof(uint8_t));
	if (memory == NULL) {
		std::cerr << "Failed to allocate memory" << std::endl;
		exit(1);
	}

	// run code
	if (st == ::ARG_INPUT) {
		std::istringstream bf_code(arg_input);
		execute(bf_code);
		if (newline) std::cout << std::endl;
	}
	if (st == ::FILE_INPUT) {
		std::ifstream bf_code(filepath);
		execute(bf_code);
		if (newline) std::cout << std::endl;
	}
	if (interactive) {
		shell();
	}

	// exit
	std::free(memory);
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
	std::cout << "  " << "-m, --memory, bytes <size>" << "\t" << "specify memory size in bytes (default: 256)" << std::endl;
	std::cout << "  " << "-i, --shell" << "\t\t\t" << "interactive shell" << std::endl;
	std::cout << "  " << "-n, --newline" << "\t\t\t" << "print newline at the end the program & toggle newline for shell" << std::endl;
	#else
	std::cout << "  " << "-h" << "\t\t\t" << "print usage" << std::endl;
	std::cout << "  " << "-f <filepath>" << "\t\t" << "execute code from a file" << std::endl;
	std::cout << "  " << "-m <size>" << "\t\t" << "specify memory size in bytes (default: 256)" << std::endl;
	std::cout << "  " << "-i" << "\t\t\t" << "interactive shell" << std::endl;
	std::cout << "  " << "-n" << "\t\t\t" << "print newline at the end the program & toggle newline for shell" << std::endl;
	#endif
	exit(e);
}

/**
 * @brief Executes a block of brainfuck code.
 * 
 * @param code code stream
 * @return 0 on success or -1 if code is invalid
 */
int execute(std::istream& code) {
	if (verify(code) < 0) {
		std::cerr << "Inputted code is invalid" << std::endl;
		return -1;
	}

	code.clear();
	code.seekg(0);

	char cmd;

	while (code >> cmd) {
		uint8_t* cell = memory + ptr_loc;
		switch (cmd) {
			case PTR_INC:
				ptr_loc = ptr_offset(1);
				break;
			case PTR_DEC:
				ptr_loc = ptr_offset(-1);
				break;
			case MEM_INC:
				(*cell)++;
				break;
			case MEM_DEC:
				(*cell)--;
				break;
			case PUT_CHR:
				putchar(*cell);
				break;
			case GET_CHR:
				*cell = getchar();
				break;
			case JMP_FWD:
				if (*cell != 0) {
					goto_stack.push(code.tellg());
				} else {
					jump_ff(code);
				}
				break;
			case JMP_BCK:
				if (*cell != 0) {
					code.seekg(goto_stack.top());
				} else {
					goto_stack.pop();
				}
				break;
		}
	}

	return 0;
}

/**
 * @brief Verifies that the code stream does not have any open brackets.
 * 
 * @param code code stream
 * @return 0 if code is valid or -1 if invalid
 */
int verify(std::istream& code) {
	char cmd;
	int brackets_open = 0;

	while (code >> cmd) {
		switch (cmd) {
			case JMP_FWD:
				brackets_open++;
				break;
			case JMP_BCK:
				brackets_open--;
				break;
		}
	}

	return (brackets_open == 0) ? 0 : -1;
}

/**
 * @brief Calculates the pointer value at ptr_loc + offset.
 * 
 * @param offset offset to calculate
 * @return New pointer value
 */
uint64_t ptr_offset(int offset) {
	if (offset == 0) return ptr_loc;

	if (offset > 0) {
		uint64_t diff = mem_size - ptr_loc;
		if (diff > offset) return ptr_loc + offset;
		return 0 + (offset - diff);
	}

	uint64_t diff = ptr_loc;
	if (diff >= (-offset)) return ptr_loc + offset;
	return mem_size - ((-offset) - diff);
}

/**
 * @brief Fast forward code stream to the matching closed bracket.
 * 
 * @param code code stream
 * @return 0 on success or -1 on EOF
 */
int jump_ff(std::istream& code) {
	char cmd;
	int skip = 0;

	while (code >> cmd) {
		if (cmd == JMP_FWD) skip++;
		if (cmd == JMP_BCK) {
			if (skip > 0) skip--;
			else return 0;
		}
	}

	return -1;
}

/**
 * @brief Start Interactive Shell.
 * 
 */
void shell() {
	std::string input;
	while (1) {
		std::cout << SHELL_PS1;
		std::cin >> input;

		if (input[0] == SHELL_CMD_PREFIX) {
			if (shell_cmd(input.substr(1)) == 1) return;
			continue;
		}

		std::istringstream bf_code(input);
		execute(bf_code);
		if (newline) std::cout << std::endl;
	}
}

/**
 * @brief Execute shell commands.
 * 
 * @param input string of commands
 * @return 1 if quit is inputted or 0 otherwise
 */
int shell_cmd(std::string input) {
	std::istringstream in_str(input);
	char cmd;

	while (in_str >> cmd) {
		switch (cmd) {
			case 'q':
				return 1;
			case 'h':
				std::cout << "Interactive shell:" << std::endl;
				std::cout << "  Executes inputted code" << std::endl;
				std::cout << "  Start with '$' to input command" << std::endl << std::endl;
				std::cout << "Commands:" << std::endl;
				std::cout << "  h" << "\t" << "Help (this message)" << std::endl;
				std::cout << "  q" << "\t" << "Exit" << std::endl;
				std::cout << "  l" << "\t" << "Print pointer location" << std::endl;
				std::cout << "  x" << "\t" << "Print current cell value in hex" << std::endl;
				std::cout << "  d" << "\t" << "Print current cell value in decimal" << std::endl;
				std::cout << "  w" << "\t" << "Print window" << std::endl;
				std::cout << "  n" << "\t" << "Toggle newlines (after code is executed)" << std::endl;
				break;
			case 'l':
				std::cout << ptr_loc << std::endl;
				break;
			case 'x':
				printf("0x%.2x\n", *(memory + ptr_loc));
				break;
			case 'd':
				printf("%d\n", *(memory + ptr_loc));
				break;
			case 'w':
				std::cout << "val: \t";
				for (int i = 0; i < 5; i++) {
					int offset = i - SHELL_WINDOW_SIZE / 2;
					printf(" 0x%.2x ", *(memory + ptr_offset(offset)));
				}
				std::cout << std::endl;
				std::cout << "ptr: \t";
				for (int i = 0; i < 5; i++) {
					int offset = i - SHELL_WINDOW_SIZE / 2;
					printf(" %-4ld ", ptr_offset(offset) % 10000);
				}
				std::cout << std::endl;
				break;
			case 'n':
				if (newline) {
					std::cout << "Newlines: off" << std::endl;
					newline = 0;
				} else {
					std::cout << "Newlines: on" << std::endl;
					newline = 1;
				}
				break;
			default:
				std::cout << "Unknown command: " << cmd << std::endl;
		}
	}
	
	return 0;
}