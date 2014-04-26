/*
 * types.h: some often used basic type definitions
 */
#ifndef TYPES_H
#define TYPES_H

// Machine types
typedef unsigned char		Byte;		/* 8 bits */
typedef unsigned short		SWord;		/* 16 bits */
typedef unsigned int		DWord;		/* 32 bits */
typedef unsigned int		dword;		/* 32 bits */
typedef unsigned int		Word;		/* 32 bits */
typedef unsigned int		ADDRESS;	/* 32-bit unsigned */


#define STD_SIZE	32					// Standard size
#define NO_ADDRESS ((ADDRESS)-1)		// For invalid ADDRESSes

typedef long unsigned long QWord;		// 64 bits

#endif
