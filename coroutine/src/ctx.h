#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif
typedef struct context
{
/**
 * General-perpose registers
 * Special uses:
 * RAX:Accumulator for operands and results data
 * RCX:Counter for string and loop operations
 * RDX:I/O pointer
 * RBX:Pointer to data in the DS segment
 * RBP:Pointer to data on the stack(in the SS segment)
 * RSI:Pointer to data in the segment pointed to by the DS register;
 *     source pointer for string operations
 * RDI:Pointer to data(or destination)in the segment pointed by the ES register;
 *     destination pointer for string operations
 * 
 */
    uint32_t mxcsr;

#define mxcsr_IM (1 << 7)
#define mxcsr_DM (1 << 8)
#define mxcsr_ZM (1 << 9)
#define mxcsr_OM (1 << 10)
#define mxcsr_UM (1 << 11)
#define mxcsr_PM (1 << 12)
#define mxcsr_MASK (mxcsr_IM | mxcsr_DM | mxcsr_ZM | mxcsr_OM | mxcsr_UM | mxcsr_PM)
    
    uint32_t x87_cw;
    
#define x87_cw_mask 0x37f
// 	uint64_t RAX;
// 	uint64_t RCX;
// 	uint64_t RDX;
	uint64_t RBX;
	uint64_t RBP;
// 	uint64_t RSI;
//  uint64_t RDI;
//New general-perpose registers(GPRs)
// 	uint64_t R8;
// 	uint64_t R9;
// 	uint64_t R10;
// 	uint64_t R11;
	uint64_t R12;
	uint64_t R13;
	uint64_t R14;
	uint64_t R15;

    uint64_t RIP;
    
}context_t;
#ifdef __cplusplus
}
#endif

