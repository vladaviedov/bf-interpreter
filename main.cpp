/**
 * @file main.cpp
 * @author Vladyslav Aviedov (vladaviedov@protonmail.com)
 * @brief Interpreter for the Brainfuck esoteric language written in C++.
 * @version 0.1
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

#define MEM_DEFAULT 256

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
std::stack<uint64_t> goto_stack;

enum state {
	NO_INPUT,
	ARG_INPUT,
	FILE_INPUT
};

void usage(int e);
int execute(std::istream& code, uint64_t mem_size);
int verify(std::istream& code);
int jump_ff(std::istream& code);

int main(int argc, char** argv) {
	uint64_t mem_size = MEM_DEFAULT;
	enum state st = state::NO_INPUT;
	int interactive = 0;
	int newline = 0;
	char* filepath;
	char* arg_input;

	char opt;
	while ((opt = getopt(argc, argv, "hf:m:in")) != -1) {
		switch (opt) {
			case 'h':
				usage(0);
				break;
			case 'f':
				filepath = optarg;
				st = state::FILE_INPUT;
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
		if (st == state::FILE_INPUT) {
			usage(1);
		}
		st = state::ARG_INPUT;
		arg_input = argv[optind];
	} else {
		if (st == state::NO_INPUT) {
			interactive = 1;
		}
	}

	// initialize memory
	memory = (uint8_t*)std::calloc(mem_size, sizeof(uint8_t));
	ptr_loc = 0;

	// run code
	if (st == state::ARG_INPUT) {
		std::istringstream bf_code(arg_input);
		execute(bf_code, mem_size);
		if (newline) std::cout << std::endl;
	}
	if (st == state::FILE_INPUT) {
		std::ifstream bf_code(filepath);
		execute(bf_code, mem_size);
		if (newline) std::cout << std::endl;
	}
	if (interactive) {
		std::cout << "TODO: Interactive" << std::endl;
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
	std::cout << "  " << "-h" << "\t\t\t" << "print usage" << std::endl;
	std::cout << "  " << "-f <filepath>" << "\t\t" << "execute code from a file" << std::endl;
	std::cout << "  " << "-m <size>" << "\t\t" << "specify memory size in bytes (default: 256)" << std::endl;
	std::cout << "  " << "-i" << "\t\t\t" << "interactive shell (todo)" << std::endl;
	std::cout << "  " << "-n" << "\t\t\t" << "print newline at the end the program" << std::endl;
	exit(e);
}

/**
 * @brief Executes a block of brainfuck code.
 * 
 * @param code code stream
 * @param mem_size size of memory in bytes
 * @return 0 on success or -1 if code is invalid
 */
int execute(std::istream& code, uint64_t mem_size) {
	if (verify(code) < 0) {
		std::cout << "Inputted code is invalid" << std::endl;
		return -1;
	}

	code.clear();
	code.seekg(0);

	char cmd;
	while (code >> cmd) {
		uint8_t* cell = memory + ptr_loc;
		// std::cout << cmd;
		switch (cmd) {
			case PTR_INC:
				ptr_loc++;
				if (ptr_loc == mem_size) {
					ptr_loc = 0;
				}
				break;
			case PTR_DEC:
				if (ptr_loc == 0) {
					ptr_loc = mem_size;
				}
				ptr_loc--;
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