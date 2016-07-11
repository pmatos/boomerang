/**
 * \file
 * \brief Some often-used basic type definitions.
 */

#ifndef TYPES_H
#define TYPES_H

#include <stdint.h>

// Machine types
typedef uint8_t  Byte;
typedef uint16_t SWord;
typedef uint32_t DWord;
typedef uint64_t QWord;

typedef DWord dword;

typedef unsigned int ADDRESS;       /* 32-bit unsigned */


#define STD_SIZE    32              // Standard size
#define NO_ADDRESS ((ADDRESS)-1)    // For invalid ADDRESSes

#endif
