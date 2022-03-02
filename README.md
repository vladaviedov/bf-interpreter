# Brainfuck Interpreter

## Build & Install
Requires CMake 3.22+ (Older versions not tested). \
Install requires root.
```
mkdir build && cd build
cmake .. && make
make install
```

## Running
```
bfi [options]
bfi [options] <code>
```
See `bfi -h` for more info.

## Shell
Running `bfi` without file or code input, activates the interactive shell. \
If `-i` is provided, program will enter the shell after the code is executed, conserving memory state. \
The shell can execute brainfuck code normally, but also provides commands for viewing memory. \
Input `$h`, while in the shell, for more info.


## Samples
* **hello_world.bf** - Prints "Hello World!".
	+ From: [Wikipedia](https://en.wikipedia.org/wiki/Brainfuck#Hello_World!)
* **bitwidth.bf** - Prints "Hello World!" and what kind of cell the interpreter has (255 for mine). 
	+ Author: [rdebath](https://github.com/rdebath/Brainfuck)
* **calculator.bf** - Calculator with basic arithmetic operations.
	+ Author: [Douglas Liu](https://github.com/DouglasLiuGamer/Brainfuck-Calculator)
