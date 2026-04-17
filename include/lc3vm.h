/** @file lc3vm.h
 * @brief LC-3 VM API
 *
 * @author Joshua Darwin
 * @note   cwid: 50327671
 * @date   Spring 2026
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
#define OPC(i) ((i) >> 12)
#define DR(i) (((i) >> 9) & 0x7)
#define SR1(i) (((i) >> 6) & 0x7)
#define SR2(i) ((i) & 0x7)

#define SEXTIMM(i) sign_extend((i) & 0x1F, 5)
#define OFF6(i) sign_extend((i) & 0x3F, 6)
#define PCOFF9(i) sign_extend((i) & 0x1FF, 9)
#define PCOFF11(i) sign_extend((i) & 0x7FF, 11)

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
uint16_t mem_read(uint16_t address);
void mem_write(uint16_t address, uint16_t value);
uint16_t sign_extend(uint16_t bits, int sign_position);
void update_flags(enum registr r);
void add(uint16_t i);
void andlc(uint16_t i);
void notlc(uint16_t i);
void ld(uint16_t i);
void ldi(uint16_t i);
void ldr(uint16_t i);
void lea(uint16_t i);
void st(uint16_t i);
void sti(uint16_t i);
void str(uint16_t i);
void jmp(uint16_t i);
void br(uint16_t i);
void jsr(uint16_t i);
void start(uint16_t offset);

#ifdef TEST
} // end extern C for C++ test runner
#endif

#endif // LC3VM_H
