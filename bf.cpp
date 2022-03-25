/**
 * @file bf.cpp
 * @author Vladyslav Aviedov <vladaviedov@protonmail.com>
 * @brief The brainfuck interpreter itself.
 * @date 2022-03-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "bf.h"

#include <iostream>
#include <istream>
#include <stack>
#include <cstring>
#include <unistd.h>

// brainfuck commands
#define PTR_INC '>'
#define PTR_DEC '<'
#define MEM_INC '+'
#define MEM_DEC '-'
#define PUT_CHR '.'
#define GET_CHR ','
#define JMP_FWD '['
#define JMP_BCK ']'

// brainfuck memory
uint8_t* mem;
uint64_t memsize;
uint64_t ptr;
std::stack<uint64_t> goto_stack;

int verify(std::istream& code);
int jump_ff(std::istream& code);

uint64_t bf_ptr() {
	return ptr;
}

uint64_t bf_memsize() {
	return memsize;
}

uint8_t bf_value() {
	return mem[ptr];
}

uint8_t bf_value(uint64_t location) {
	return mem[location];
}

int bf_execute(std::istream& code) {
	if (!verify(code)) {
		std::cerr << "Inputted code is invalid" << std::endl;
		return -1;
	}

	code.clear();
	code.seekg(0);

	char cmd;

	while (code >> cmd) {
		uint8_t* cell = mem + ptr;
		switch (cmd) {
			case PTR_INC:
				ptr = bf_ptroffset(1);
				break;
			case PTR_DEC:
				ptr = bf_ptroffset(-1);
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

uint64_t bf_ptroffset(int offset) {
	if (offset == 0) return ptr;

	uint64_t offset_abs = (offset < 0) ? (-offset) : offset;

	if (offset > 0) {
		uint64_t diff = memsize - ptr;
		if (diff > offset_abs) return ptr + offset_abs;
		return 0 + (offset_abs - diff);
	}

	uint64_t diff = ptr;
	if (diff >= offset_abs) return ptr - offset_abs;
	return memsize - (offset_abs - diff);
}

int bf_malloc(uint64_t size) {
	if (mem) std::free(mem);

	mem = (uint8_t*)std::calloc(size, sizeof(uint8_t));
	if (mem == NULL) {
		return -1;
	}

	memsize = size;
	return 0;
}

void bf_free() {
	if (mem) std::free(mem);
}

void bf_reset() {
	std::memset(mem, 0, memsize);
	ptr = 0;
}

/** Internal Functions **/

/**
 * @brief Verifies that the code stream does not have any open brackets.
 * 
 * @param code code stream
 * @return 1 if code is valid, 0 if invalid
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

	return (brackets_open == 0);
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
