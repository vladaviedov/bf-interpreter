/**
 * @file shell.cpp
 * @author Vladyslav Aviedov <vladaviedov@protonmail.com>
 * @brief REPL shell for brainfuck.
 * @date 2022-03-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#include "shell.hpp"

#include <iostream>
#include <sstream>
#include <string>

#include "bf.hpp"

// shell settings
#define SHELL_PS1 "bf> "
#define SHELL_CMD_PREFIX '$'
#define SHELL_WINDOW_SIZE 5

static int newlines = 0;

int shell_cmd(std::string input);
void shell_help();

void shell(int nl) {
	newlines = nl;

	// Enter REPL loop
	std::string input;
	while (1) {
		std::cout << SHELL_PS1;
		std::cin >> input;

		if (input[0] == SHELL_CMD_PREFIX) {
			if (shell_cmd(input.substr(1)) == 1) return;
			continue;
		}

		std::istringstream bf_code(input);
		if (bf_execute(bf_code) < 0) continue;
		if (newlines) std::cout << std::endl;
	}
}

/** Internal Functions **/

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
				shell_help();
				break;
			case 'l':
				std::cout << bf_ptr() << std::endl;
				break;
			case 'x':
				printf("0x%.2x\n", bf_value());
				break;
			case 'd':
				printf("%d\n", bf_value());
				break;
			case 'w':
				std::cout << "val: \t";
				for (int i = 0; i < 5; i++) {
					int offset = i - SHELL_WINDOW_SIZE / 2;
					printf(" 0x%.2x ", bf_value(bf_ptroffset(offset)));
				}
				std::cout << std::endl;
				std::cout << "ptr: \t";
				for (int i = 0; i < 5; i++) {
					int offset = i - SHELL_WINDOW_SIZE / 2;
					printf(" %-4lu ", bf_ptroffset(offset) % 10000);
				}
				std::cout << std::endl;
				break;
			case 'n':
				if (newlines) {
					std::cout << "Newlines: off" << std::endl;
					newlines = 0;
				} else {
					std::cout << "Newlines: on" << std::endl;
					newlines = 1;
				}
				break;
			case 'r':
				bf_reset();
				std::cout << "Memory zeroed" << std::endl;
				break;
			default:
				std::cout << "Unknown command: " << cmd << std::endl;
		}
	}
	
	return 0;
}

/**
 * @brief Print shell help information.
 * 
 */
void shell_help() {
	std::cout << "Interactive/REPL shell:" << std::endl;
	std::cout << "  Evaluates brainfuck code" << std::endl;
	std::cout << "  Start input with '$' to input non-brainfuck commands" << std::endl << std::endl;
	std::cout << "Commands:" << std::endl;
	std::cout << "  h" << "\t" << "Help (this message)" << std::endl;
	std::cout << "  q" << "\t" << "Exit" << std::endl;
	std::cout << "  l" << "\t" << "Print pointer location" << std::endl;
	std::cout << "  x" << "\t" << "Print current cell value in hex" << std::endl;
	std::cout << "  d" << "\t" << "Print current cell value in decimal" << std::endl;
	std::cout << "  w" << "\t" << "Print window" << std::endl;
	std::cout << "  n" << "\t" << "Toggle newlines (after code is executed)" << std::endl;
	std::cout << "  r" << "\t" << "Reset (zero) memory and return pointer to 0" << std::endl;
}
