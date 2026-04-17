/** @file lc3vm.h
 * @brief LC-3 VM API
 *
 * @author Student Name
 * @note   cwid: 123456
 * @date   Spring 2024
 * @note   ide:  g++ 8.2.0 / GNU Make 4.2.1
 *
 * Header include file for LC-3 simulator API/functions.
 */
#include <stdbool.h>
#include <stdint.h>

#ifndef LC3VM_H
#define LC3VM_H

// total number of opcodes in the LC-3 architecture.
#define NUMOPS (16)

// Need to #define all of your bit manipulation macros like DR, SR1, etc. here.
#define FIMM(i) ((i >> 5) & 0x1)
#define FCND(i) (((i) >> 9) & 0x7)
#define BR(i) (((i) >> 6) & 0x7)
#define FL(i) (((i) >> 11) & 1)
#define TRP(i) ((i) & 0xFF)

#define OPC(i) ((i) >> 12)           // opcode is in bits 15-12
#define SR2(i) ((i) & 0x0007)        // source register 2 is in bits 2-0
#define SR1(i) (((i) >> 6) & 0x0007) // source register 1 is in bits 8-6
#define DR(i) (((i) >> 9) & 0x0007)  // destination register is in bits 11-9
#define SEXTIMM(i) (sign_extend((i), 5))  // 5-bit immediate operand is in bits 8-0
#define OFF6(i) (sign_extend((i), 6))     // 6-bit offset operand is in bits 5-0
#define PCOFF9(i) (sign_extend((i), 9))   // 9-bit PC-relative offset operand is in bits 8-0
#define PCOFF11(i) (sign_extend((i), 11)) // 11-bit PC-relative offset operand is in bits 10-0

typedef void (*op_ex_f)(uint16_t i);
typedef void (*trp_ex_f)();

enum
{
  trp_offset = 0x20
};

enum registr
{
  R0 = 0,
  R1,
  R2,
  R3,
  R4,
  R5,
  R6,
  R7,
  RPC,
  RCND,
  RCNT
};

enum flags
{
  FP = 1 << 0,
  FZ = 1 << 1,
  FN = 1 << 2
};

// If we are creating tests, make all declarations extern C so can
// work with catch2 C++ framework
#ifdef TEST
extern "C" {
#endif

extern bool running;
extern uint16_t mem[];
extern uint16_t reg[];
extern uint16_t PC_START;

// your task functions should go here
uint16_t mem_read(uint16_t address);              // read memory at address
void mem_write(uint16_t address, uint16_t value); // write value to memory address
uint16_t sign_extend(uint16_t bits, int size);    // sign extend bits to 16 bits based on size of bits
void update_flags(enum registr r);                // update condition code flags based on value in register r

void add(uint16_t i);   // add operation
void andlc(uint16_t i); // logical AND operation
void notlc(uint16_t i); // logical NOT operation

void ld(uint16_t i);  // load memory access operation
void ldi(uint16_t i); // load indirect memory access operation
void ldr(uint16_t i); // load base + offset memory access operation
void lea(uint16_t i); // load effective address operation

void st(uint16_t i);  // store memory access operation
void sti(uint16_t i); // store indirect memory access operation
void str(uint16_t i); // store base and offset memory access operation

void jmp(uint16_t i); // jump operation
void br(uint16_t i);  // conditional branch operation
void jsr(uint16_t i); // jump to subroutine operation

void start(uint16_t i); // start the LC-3 VM execution at the given offset

void rti(uint16_t i);
void res(uint16_t i);
void tgetc();
void tout();
void tputs();
void tin();
void thalt();
void tinu16();
void toutu16();
void trap(uint16_t i);
void ld_img(char* fname, uint16_t offset);

#ifdef TEST
} // end extern C for C++ test runner
#endif

#endif // LC3VM_H
