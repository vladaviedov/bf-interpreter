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
	ARG_INPUT,
	FILE_INPUT,
	INTERACTIVE
};

void usage(int e);
int run_file(char* filepath);
int execute(std::istream& code, uint64_t mem_size);
int verify(std::istream& code);
int jump_ff(std::istream& code);

int main(int argc, char** argv) {
	uint64_t mem_size = MEM_DEFAULT;
	enum state st = state::ARG_INPUT;
	char* filepath;
	char* arg_input;

	char opt;
	while ((opt = getopt(argc, argv, "hf:m:i")) != -1) {
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
				st = state::INTERACTIVE;
				break;
			case '?':
				usage(1);
				break;
		}
	}

	if (optind < argc) {
		arg_input = argv[optind];
	} else {
		if (st != state::FILE_INPUT) {
			st = state::INTERACTIVE;
		}
	}

	// initialize memory
	memory = (uint8_t*)std::calloc(mem_size, sizeof(uint8_t));
	ptr_loc = 0;

	// run code
	if (st == state::ARG_INPUT) {
		// std::cout << arg_input << std::endl;
		std::istringstream code(arg_input);
		execute(code, mem_size);
	}

	// exit
	std::cout << std::endl;
	std::free(memory);
	return 0;
}

void usage(int e) {
	std::cout << "usage: " << std::endl;
	exit(e);
}

int run_file(char* filepath) {
	return 0;
}

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
				*cell++;
				break;
			case MEM_DEC:
				*cell--;
				break;
			case PUT_CHR:
				putchar(*cell);
				break;
			case GET_CHR:
				*cell = getchar();
				break;
			case JMP_FWD:
				if (*cell != 0) {
					goto_stack.push((uint64_t)code.tellg() + 1);
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