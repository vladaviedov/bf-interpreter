/**
 * @file bf.hpp
 * @author Vladyslav Aviedov <vladaviedov@protonmail.com>
 * @brief The brainfuck interpreter itself.
 * @date 2022-03-25
 * 
 * @copyright Copyright (c) 2022
 * 
 */
#ifndef BFI_BF_HPP
#define BFI_BF_HPP

#include <istream>

/**
 * @brief Get current pointer location.
 * 
 * @return pointer address
 */
uint64_t bf_ptr();

/**
 * @brief Get memory size.
 * 
 * @return memory size
 */
uint64_t bf_memsize();

/**
 * @brief Get memory value at current location.
 * 
 * @return value
 */
uint8_t bf_value();

/**
 * @brief Get memory value at location.
 * 
 * @param location address
 * @return value
 */
uint8_t bf_value(uint64_t location);

/**
 * @brief Get pointer offset from current pointer.
 * 
 * @param offset
 * @return new pointer location
 */
uint64_t bf_ptroffset(int offset);

/**
 * @brief Allocate memory for brainfuck program.
 * 
 * @param size memory size
 * @return 0 - success; -1 - error
 */
int bf_malloc(uint64_t size);

/**
 * @brief Free brainfuck memory.
 * 
 */
void bf_free();

/**
 * @brief Reset brainfuck memory.
 * 
 */
void bf_reset();

/**
 * @brief Execute brainfuck code.
 * 
 * @param code code stream
 * @return 0 - sucess; -1 - error
 */
int bf_execute(std::istream &code);

#endif // BFI_BF_HPP
