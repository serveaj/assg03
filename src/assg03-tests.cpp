/** @file assg03-tests.cpp
 * @brief Unit tests for LC-3 microarchitecture/VM simulator.
 *
 * @author Student Name
 * @note   cwid: 123456
 * @date   Spring 2024
 * @note   ide:  VS Code Editor / IDE ; g++ 8.2.0 / GNU Make 4.2.1
 *
 * Unit tests for assignment 03, implementaiton of LC-3
 * microarchitecture/VM simulator.
 */
#include "catch.hpp"

#define TEST
#include "lc3vm.h"
#include "lc3vm_dbg.h"

#define task1
#define task2
#define task3
#define task4
#define task5
#define task6
#define task7
#undef task8
#undef task9
#undef task10

/**
 * @brief Task 1: Test memory access functions to read and write into
 *   the simulated 64k 16-bit addressable memory of the LC-3
 */
#ifdef task1
TEST_CASE("Task 1: memory access functions", "[task1]")
{
  // all of memory should initially be 0, test this
  for (int addr = 0; addr < UINT16_MAX + 1; addr++)
  {
    CHECK(mem_read(addr) == 0x0);
  }

  // test writing and reading back at a few locations
  mem_write(PC_START, 0x1234);
  CHECK(mem_read(PC_START) == 0x1234);
  mem_write(PC_START + 1, 0xabcd);
  CHECK(mem_read(PC_START + 1) == 0xabcd);
  mem_write(PC_START + 2, 0xbeef);
  CHECK(mem_read(PC_START + 2) == 0xbeef);

  // begin and end of legal memory
  mem_write(0x0, 0x42);
  CHECK(mem_read(0x0) == 0x42);
  mem_write(UINT16_MAX, 0x24);
  CHECK(mem_read(UINT16_MAX) == 0x24);

  // clear out memory again before proceeding
  for (int addr = 0; addr < UINT16_MAX + 1; addr++)
  {
    mem_write(addr, 0x0);
    CHECK(mem_read(addr) == 0x0);
  }
}
#endif // task1

// Some example values used to test sign extension
uint16_t imm5_pos = 0xF003;  // 1111 0000 0000 0011
uint16_t imm5_neg = 0xF014;  // 1111 0000 0001 0100
uint16_t off6_pos = 0x0712;  // 0000 0111 0001 0010
uint16_t off6_neg = 0x0732;  // 0000 0111 0011 0010
uint16_t off9_pos = 0x0099;  // 0000 0000 1001 1001
uint16_t off9_neg = 0x0199;  // 0000 0001 1001 1001
uint16_t off11_pos = 0xA2AA; // 1010 0010 1010 1010
uint16_t off11_neg = 0xA6AA; // 1010 0110 1010 1010

/**
 * @brief Task 2: Test sign extension function
 */
#ifdef task2
TEST_CASE("Task 2: sign_extend function implementation", "[task2]")
{
  CHECK(sign_extend(imm5_pos, 5) == 0x0003);
  CHECK(sign_extend(imm5_neg, 5) == 0xFFF4);
  CHECK(sign_extend(off6_pos, 6) == 0x0012);
  CHECK(sign_extend(off6_neg, 6) == 0xFFF2);
  CHECK(sign_extend(off9_pos, 9) == 0x0099);
  CHECK(sign_extend(off9_neg, 9) == 0xFF99);
  CHECK(sign_extend(off11_pos, 11) == 0x02AA);
  CHECK(sign_extend(off11_neg, 11) == 0xFEAA);
}
#endif // task2

// some example instructions to test, used in task 3 tests
uint16_t trap_ins = 0xF026; // 1111 0000 0010 0110 TRAP trap_in_u16
uint16_t add_ins = 0x1223;  // 0001 0010 0010 0011 ADD R1, R0, #x3
uint16_t br_ins = 0x0A08;   // 0000 1010 0000 1000 BRnp 0x8
uint16_t ld_ins = 0x270F;   // 0010 0111 0000 1111 LD R3, -#xF1
uint16_t not_ins = 0x98BF;  // 1001 1000 1011 1111 NOT R4, R2
uint16_t str_ins = 0x7F5F;  // 0111 1111 0101 1111 STR R7, R5, #x1F
uint16_t lea_ins = 0xE610;  // 1110 0110 0001 0000 LEA, R3, #x10
uint16_t and_ins = 0x5D05;  // 0101 1101 0000 0101 AND R6, R4, R5
uint16_t add2_ins = 0x1043; // 0001 0000 0100 0011 ADD R0, R1, R3
uint16_t jsr_ins = 0x4C55;  // 0100 1100 0101 0101 JSR -#x3AB

/**
 * @brief Task 3: Test instruction extraction macros
 */
#ifdef task3
TEST_CASE("Task 3: OPC instruction extraction operations", "[task3]")
{
  CHECK(OPC(trap_ins) == 0xF);
  CHECK(OPC(add_ins) == 0x1);
  CHECK(OPC(br_ins) == 0x0);
  CHECK(OPC(ld_ins) == 0x2);
  CHECK(OPC(not_ins) == 0x9);
  CHECK(OPC(str_ins) == 0x7);
  CHECK(OPC(lea_ins) == 0xE);
  CHECK(OPC(and_ins) == 0x5);
  CHECK(OPC(add2_ins) == 0x1);
}

TEST_CASE("Task 3: SR2 source register 2 extraction operations", "[task3]")
{
  CHECK(SR2(and_ins) == 0x5);
  CHECK(SR2(add2_ins) == 0x3);
}

TEST_CASE("Task 3: SR1 source register 1 extraction operations", "[task3]")
{
  CHECK(SR1(add_ins) == 0x0);
  CHECK(SR1(not_ins) == 0x2);
  CHECK(SR1(str_ins) == 0x5);
  CHECK(SR1(and_ins) == 0x4);
  CHECK(SR1(add2_ins) == 0x1);
}

TEST_CASE("Task 3: DR destination register extraction operations", "[task3]")
{
  CHECK(DR(add_ins) == 0x1);
  CHECK(DR(ld_ins) == 0x3);
  CHECK(DR(not_ins) == 0x4);
  CHECK(DR(str_ins) == 0x7);
  CHECK(DR(lea_ins) == 0x3);
  CHECK(DR(and_ins) == 0x6);
  CHECK(DR(add2_ins) == 0x0);
}

TEST_CASE("Task 3: SEXTIMM immediate operand extraction and sign extension", "[task3]")
{
  CHECK(SEXTIMM(add_ins) == 0x0003);
}

TEST_CASE("Task 3: OFF6 immediate operand extraction and sign extension", "[task3]")
{
  CHECK(OFF6(str_ins) == 0x001F);
}

TEST_CASE("Task 3: PCOFF9 immediate operand extraction and sign extension", "[task3]")
{
  CHECK(PCOFF9(ld_ins) == 0xFF0F);
}

TEST_CASE("Task 3: PCOFF11 immediate operand extraction and sign extension", "[task3]")
{
  CHECK(PCOFF11(jsr_ins) == 0xFC55);
}
#endif // task3

/**
 * @brief Task 4: Test condition code flags update function
 */
#ifdef task4
TEST_CASE("Task 4: condition code flags update function", "[task4]")
{
  // Test only 0 is set
  reg[R1] = 0x0000;
  update_flags(R1);
  CHECK(reg[RCND] == FZ);

  // Test a negative results
  reg[R2] = 0xFFFF; // -x1
  update_flags(R2);
  CHECK(reg[RCND] == FN);

  reg[R3] = 0x8000; // -x8000
  update_flags(R3);
  CHECK(reg[RCND] == FN);

  // Test positive results
  reg[R4] = 0x0001; // +x1
  update_flags(R4);
  CHECK(reg[RCND] == FP);

  reg[R5] = 0x7FFF; // +x7FFF
  update_flags(R5);
  CHECK(reg[RCND] == FP);
}
#endif // task4

/**
 * @brief Task 5: Implement LC-3 arithmetic / logic Microcode Operations
 */
#ifdef task5
TEST_CASE("Task 5: implement add arithmetic / logic operator", "[task5]")
{
  // test two source register mode
  reg[R1] = 0x0001;
  reg[R2] = 0x0001;
  uint16_t inst = 0x1042; // 0001 000 001 000 010 ADD R0, R1, R2
  add(inst);
  CHECK(reg[R0] == 0x0002);
  CHECK(reg[RCND] == FP);

  reg[R4] = 0xFFFF; // -1
  reg[R5] = 0xFFFF; // -1
  inst = 0x1705;    // 0001 011 100 000 101 ADD R3, R4, R5
  add(inst);
  CHECK(reg[R3] == 0xFFFE); // -2
  CHECK(reg[RCND] == FN);

  reg[R6] = 0xFFFF; // -1
  reg[R7] = 0x0001; // +1
  inst = 0x1187;    // 0001 000 110 000 111 ADD R0, R6, R7
  add(inst);
  CHECK(reg[R0] == 0x0000);
  CHECK(reg[RCND] == FZ);

  // test source plus immediate value mode
  reg[R1] = 0x0002;
  inst = 0x1063; // 0001 000 001 1 00011 ADD R0, R1, #x3
  add(inst);
  CHECK(reg[R0] == 0x0005);
  CHECK(reg[RCND] == FP);

  reg[R4] = 0xFFFE; // -2
  inst = 0x173D;    // 0001 011 100 1 11101 ADD R3, R4, -#x3
  add(inst);
  CHECK(reg[R3] == 0xFFFB); // -5
  CHECK(reg[RCND] == FN);

  reg[R6] = 0xFFFA; // -6
  inst = 0x1BA6;    // 0001 101 110 1 00110 ADD R5, R6, +#x6
  add(inst);
  CHECK(reg[R5] == 0x0000);
  CHECK(reg[RCND] == FZ);
}

TEST_CASE("Task 5: implement andlc arithmetic / logic operator", "[task5]")
{
  // test two source register mode
  reg[R1] = 0xF1F1;
  reg[R2] = 0xAFAF;
  uint16_t inst = 0x5042; // 0101 000 001 000 010 AND R0, R1, R2
  andlc(inst);
  CHECK(reg[R0] == 0xA1A1);
  CHECK(reg[RCND] == FN);

  reg[R4] = 0x1111;
  reg[R5] = 0x0033;
  inst = 0x5705; // 0101 011 100 000 101 AND R3, R4, R5
  andlc(inst);
  CHECK(reg[R3] == 0x0011);
  CHECK(reg[RCND] == FP);

  reg[R6] = 0x3333;
  reg[R7] = 0xCCCC;
  inst = 0x5187; // 0101 000 110 000 111 AND R0, R6, R7
  andlc(inst);
  CHECK(reg[R0] == 0x0000);
  CHECK(reg[RCND] == FZ);

  // test source plus immediate value mode
  reg[R1] = 0xA16F;
  inst = 0x5069; // 0101 000 001 1 01001 AND R0, R1, #x9
  andlc(inst);
  CHECK(reg[R0] == 0x0009);
  CHECK(reg[RCND] == FP);

  reg[R4] = 0xFFFF;
  inst = 0x573D; // 0101 011 100 1 11101 AND R3, R4, -#x3
  andlc(inst);
  CHECK(reg[R3] == 0xFFFD);
  CHECK(reg[RCND] == FN);

  reg[R6] = 0xFFF0;
  inst = 0x5BA6; // 0101 101 110 1 00110 AND R5, R6, +#x6
  andlc(inst);
  CHECK(reg[R5] == 0x0000);
  CHECK(reg[RCND] == FZ);
}

TEST_CASE("Task 5: implement notlc  arithmetic / logic operator", "[task5]")
{
  // test not of value in register, flags will be set according to sign bit
  reg[R1] = 0xF1F1;
  uint16_t inst = 0x907F; // 1001 000 001 111111 NOT R0, R1
  notlc(inst);
  CHECK(reg[R0] == 0x0E0E);
  CHECK(reg[RCND] == FP);

  reg[R3] = 0x1234;
  inst = 0x94FF; // 1001 010 011 111111 NOT R2, R3
  notlc(inst);
  CHECK(reg[R2] == 0xEDCB);
  CHECK(reg[RCND] == FN);

  reg[R5] = 0xFFFF;
  inst = 0x997F; // 1001 100 101 111111 NOT R4, R5
  notlc(inst);
  CHECK(reg[R4] == 0x000);
  CHECK(reg[RCND] == FZ);
}
#endif // task5

/**
 * @brief Task 6: Implement load memory access operators
 */
#ifdef task6
TEST_CASE("Task 6: ld implement load memory access operators", "[task6]")
{
  // put some values into memory and set RPC to 0x3000 for tests of load operators
  reg[RPC] = PC_START;
  mem[PC_START] = 0x1234;
  mem[PC_START - 0x1] = 0x0000;
  mem[PC_START + 0xFF] = 0xbeef;
  mem[PC_START - 0x100] = 0xdead;

  // load offset 0 from pc into R1
  uint16_t inst = 0x2200; // 0010 001 000000000 LD R1, #x0
  ld(inst);
  CHECK(reg[R1] == 0x1234);
  CHECK(reg[RCND] == FP);

  // load offset +0xFF from pc into R2
  inst = 0x24FF; // 0010 010 011111111 LD R2, #xFF
  ld(inst);
  CHECK(reg[R2] == 0xbeef);
  CHECK(reg[RCND] == FN);

  // load offset -0x100 from pc into R3
  inst = 0x2700; // 0010 011 1000000 LD R3, -#x100
  ld(inst);
  CHECK(reg[R3] == 0xdead);
  CHECK(reg[RCND] == FN);

  // load offset -0x1 from pc into R4
  inst = 0x29FF; // 0010 100 111111111 LD R4, -#x100
  ld(inst);
  CHECK(reg[R4] == 0x0000);
  CHECK(reg[RCND] == FZ);
}

TEST_CASE("Task 6: ldi implement load memory access operators", "[task6]")
{
  // put some values into memory and set RPC to 0x3000 for tests of load operators
  reg[RPC] = PC_START;
  mem[PC_START] = 0x1234;
  mem[PC_START - 0x1] = 0x0000;
  mem[PC_START + 0xFF] = 0xbeef;
  mem[PC_START - 0x100] = 0xdead;
  mem[0x1234] = 0x8989;
  mem[0xdead] = 0x4242;
  mem[0xbeef] = 0x2121;
  mem[0x0000] = 0x0000;

  // load indirect mem[PC+0] == 0x1234, mem[0x1234] == 0x8989
  uint16_t inst = 0xA200; // 1010 001 000000000 LDI R1, #x0
  ldi(inst);
  CHECK(reg[R1] == 0x8989);
  CHECK(reg[RCND] == FN);

  // load offset +0xFF from pc indirectly into R2
  inst = 0xA4FF; // 1010 010 011111111 LDI R2, #xFF
  ldi(inst);
  CHECK(reg[R2] == 0x2121);
  CHECK(reg[RCND] == FP);

  // load offset -0x100 from pc into R3
  inst = 0xA700; // 1010 011 1000000 LD R3, -#x100
  ldi(inst);
  CHECK(reg[R3] == 0x4242);
  CHECK(reg[RCND] == FP);

  // load offset -0x1 from pc into R4
  inst = 0xA9FF; // 1010 100 111111111 LD R4, -#x100
  ldi(inst);
  CHECK(reg[R4] == 0x0000);
  CHECK(reg[RCND] == FZ);
}

TEST_CASE("Task 6: ldr implement load memory access operators", "[task6]")
{
  // put some values into memory, put some base addresses into R6 and R7
  reg[R6] = 0xdead;
  mem[0xdead] = 0x1234;
  mem[0xdead + 0x1] = 0x8ABC;
  mem[0xdead + 0x1F] = 0x0000;
  mem[0xdead - 0x20] = 0x4567;

  // load 0 offset from base address in R6 into R1
  uint16_t inst = 0x6380; // 0110 001 110 000000 LDR R1, R6, #0x0
  ldr(inst);
  CHECK(reg[R1] == 0x1234);
  CHECK(reg[RCND] == FP);

  // load +0x1 offset from base address in R6 into R2
  inst = 0x6581; // 0110 010 110 000001 LDR R2, R6, #0x1
  ldr(inst);
  CHECK(reg[R2] == 0x8ABC);
  CHECK(reg[RCND] == FN);

  // load +0x1F offset from base address in R6 into R3
  inst = 0x679F; // 0110 011 110 011111 LDR R3, R6, #0x1F
  ldr(inst);
  CHECK(reg[R3] == 0x0000);
  CHECK(reg[RCND] == FZ);

  // load -0x20 offset from base address in R6 into R4
  inst = 0x69A0; // 0110 100 110 100000 LDR R4, R6, -#0x20
  ldr(inst);
  CHECK(reg[R4] == 0x4567);
  CHECK(reg[RCND] == FP);
}

TEST_CASE("Task 6: lea implement load memory access operators", "[task6]")
{
  // put some values into PC for calculating effective address
  reg[RPC] = PC_START;

  // calculate effective address 0 offset from RPC, put address in R1
  // NOTE: lea doesn't effect status flags, so we are not checking flags here
  uint16_t inst = 0xE200; // 1110 001 000000000 LEA R1, #x0
  lea(inst);
  CHECK(reg[R1] == PC_START);

  // calculate effective address +0xFF from RPC, put address in R2
  inst = 0xE4FF; // 1110 010 0111111 LEA R2, #xFF
  lea(inst);
  CHECK(reg[R2] == PC_START + 0xFF);

  // calculate effective address -0x100 from RPC, put address in R3
  inst = 0xE700; // 1110 011 100000000 LEA R#, -#x100
  lea(inst);
  CHECK(reg[R3] == PC_START - 0x100);
}
#endif // task6

/**
 * @brief Task 7: Implement store memory access operators
 */
#ifdef task7
TEST_CASE("Task 7: st implement store memory access operators", "[task7]")
{
  // put some values into memory and set RPC to 0x3000 for tests of store operators
  reg[RPC] = PC_START;
  reg[R1] = 0xdead;
  reg[R2] = 0xbeef;
  reg[R3] = 0x4242;
  reg[R4] = 0x1234;

  // store value inR1 at 0 offset from current PC
  uint16_t inst = 0x3200; // 0011 001 000000000 ST R1, #x0
  st(inst);
  CHECK(mem[PC_START] == 0xdead);

  // store value in R2 at +0xFF offset from current PC
  inst = 0x34FF; // 0011 010 011111111 ST R2, #xFF
  st(inst);
  CHECK(mem[PC_START + 0xFF] == 0xbeef);

  // store value in R3 at -0x100 offset from current PC
  inst = 0x3700; // 0011 011 1000000 ST R3, -#x100
  st(inst);
  CHECK(mem[PC_START - 0x100] == 0x4242);

  // store value in R4 at -0x01 offset from current PC
  inst = 0x39FF; // 0011 100 111111111 ST R4, -#x1
  st(inst);
  CHECK(mem[PC_START - 0x1] == 0x1234);
}

TEST_CASE("Task 7: sti implement store memory access operators", "[task7]")
{
  // put some values into memory and set RPC to 0x3000 for tests of
  // store indirect operators
  reg[RPC] = PC_START;
  reg[R1] = 0xdead;
  reg[R2] = 0xbeef;
  reg[R3] = 0x4242;
  reg[R4] = 0x1234;
  mem[PC_START] = 0x4444;
  mem[PC_START + 0xFF] = 0x5555;
  mem[PC_START - 0x100] = 0x6666;

  // store value in R1 at address in RPC+0
  uint16_t inst = 0xB200; // 1011 001 000000000 STI R1, #x0
  sti(inst);
  CHECK(mem[0x4444] == 0xdead);

  // store value in R2 at address in RPC+0xFF
  inst = 0xB4FF; // 1011 010 0111111 STI R2, #xFF
  sti(inst);
  CHECK(mem[0x5555] == 0xbeef);

  // store value in R3 at address in RPC-0x100
  inst = 0xB700; // 1011 011 1000000 STI R3, -#x100
  sti(inst);
  CHECK(mem[0x6666] == 0x4242);
}

TEST_CASE("Task 7: str implement store memory access operators", "[task7]")
{
  // put some values into registers, use R1 and R2 as base registers and R3
  // and R4 as source registers.
  reg[RPC] = PC_START;
  reg[R1] = 0xdead;
  reg[R2] = 0xbeef;
  reg[R3] = 0x4242;
  reg[R4] = 0x1234;
  mem[PC_START] = 0x4444;
  mem[PC_START + 0xFF] = 0x5555;
  mem[PC_START - 0x100] = 0x6666;

  // store value in R3 into base R1+0
  uint16_t inst = 0x7640; // 0111 011 001 000000 STR R3, R1, #x0
  str(inst);
  CHECK(mem[0xdead] == 0x4242);

  // store value in R4 into base R2+0xFF
  inst = 0x789F; // 0111 100 010 011111 STR R4, R2, #x1F
  str(inst);
  CHECK(mem[0xbeef + 0x1F] == 0x1234);

  // store value in R3 into base R1-0x20
  inst = 0x7660; // 0111 011 001 100000 STR R3, R1, -#x20
  str(inst);
  CHECK(mem[0xdead - 0x20] == 0x4242);

  // store value in R4 into base R2-0x1
  inst = 0x78BF; // 0111 100 010 111111 STR R4, R2, -#x1
  str(inst);
  CHECK(mem[0xbeef - 0x1] == 0x1234);
}
#endif // task7

/**
 * @brief Task 8: Implement control flow operators
 */
#ifdef task8
TEST_CASE("Task 8: jmp implement control flow operators", "[task8]")
{
  // set current RPC so can detect if jump occurs as asked for, and
  // give some addresses to jump to in registers
  reg[RPC] = PC_START;
  reg[R1] = 0xabcd;
  reg[R2] = 0x1234;
  reg[R3] = 0xffff;
  reg[R4] = PC_START;

  // absolute jump to address specified in R1
  uint16_t inst = 0xC040; // 1100 000 001 000000 JMP R1
  jmp(inst);
  CHECK(reg[RPC] == 0xabcd);

  // absolute jump to address specified in R2
  inst = 0xC080; // 1100 000 010 000000 JMP R2
  jmp(inst);
  CHECK(reg[RPC] == 0x1234);

  // absolute jump to address specified in R3
  inst = 0xC0C0; // 1100 000 011 000000 JMP R3
  jmp(inst);
  CHECK(reg[RPC] == 0xffff);

  // absolute jump to address specified in R4, back to where we started
  inst = 0xC100; // 1100 000 100 000000 JMP R4
  jmp(inst);
  CHECK(reg[RPC] == PC_START);
}

TEST_CASE("Task 8: br implement control flow operators", "[task8]")
{
  // branch on negative conditionally
  uint16_t inst = 0x0880; // 0000 100 010000000 BRn #x80
  reg[RPC] = PC_START;
  reg[RCND] = FN;
  br(inst);
  CHECK(reg[RPC] == PC_START + 0x80);

  // should not branch on zero given current condition flags
  inst = 0x0480; // 0000 010 010000000 BRz #x80
  br(inst);
  CHECK(reg[RPC] == PC_START + 0x80);

  // should not branch on positive given current condition flags
  inst = 0x0280; // 0000 001 010000000 BRp #x80
  br(inst);
  CHECK(reg[RPC] == PC_START + 0x80);

  // test branch on zero back to start
  inst = 0x0580; // 0000 010 110000000 BRz -#x80
  reg[RCND] = FZ;
  br(inst);
  CHECK(reg[RPC] == PC_START);

  // should not branch on negative given current condition flags
  inst = 0x0980; // 0000 100 110000000 BRz -#x80
  br(inst);
  CHECK(reg[RPC] == PC_START);

  // should not branch on positive given current condition flags
  inst = 0x0380; // 0000 001 110000000 BRz -#x80
  br(inst);
  CHECK(reg[RPC] == PC_START);

  // test branch on positive
  inst = 0x03FD; // 0000 001 111111101 BRp -#x3
  reg[RCND] = FP;
  br(inst);
  CHECK(reg[RPC] == PC_START - 0x3);

  // should not branch on negative given current condition flags
  inst = 0x09FD; // 0000 100 111111101 BRp -#x3
  br(inst);
  CHECK(reg[RPC] == PC_START - 0x3);

  // should not branch on zero given current condition flags
  inst = 0x05FD; // 0000 010 111111101 BRp -#x3
  br(inst);
  CHECK(reg[RPC] == PC_START - 0x3);

  // should always branch if all NZP set
  inst = 0x0E03; // 0000 111 000000011 BRp #x3
  reg[RCND] = FN;
  br(inst);
  CHECK(reg[RPC] == PC_START);

  // shold always branch if all NZP set
  inst = 0x0E03; // 0000 111 000000011 BRp #x3
  reg[RCND] = FZ;
  br(inst);
  CHECK(reg[RPC] == PC_START + 0x3);

  // shold always branch if all NZP set
  inst = 0x0E03; // 0000 111 000000011 BRp #x3
  reg[RCND] = FP;
  br(inst);
  CHECK(reg[RPC] == PC_START + 0x6);
}

TEST_CASE("Task 8: jsr / jsrr implement control flow operators", "[task8]")
{
  // set current RPC so can detect we jump to subroutine
  reg[RPC] = PC_START;
  reg[R7] = 0;
  reg[R6] = 0;

  // jump to subroutine at +0x0200 relative to PC
  // we expect current RPC to be in R7 after jump into routine
  uint16_t inst = 0x4A00; // 0100 1 010 0000 0000 JSR 0x200
  jsr(inst);
  CHECK(reg[RPC] == PC_START + 0x200);
  CHECK(reg[R7] == PC_START);

  // move return to R6 and return back to PC_START
  // after return, R7 has the location we returned from...
  inst = 0x1D87; // 0001 110 110 000 111 ADD R6, R6, R7
  add(inst);
  CHECK(reg[R6] == reg[R7]);
  inst = 0x4180; // 0100 0 00 110 000000 JSRR R6
  jsr(inst);
  CHECK(reg[RPC] == PC_START);
  CHECK(reg[R7] == PC_START + 0x200);

  // test a bigger negative jump to subroutine
  inst = 0x4C0C; // 0100 1 10000001100 JSR -0x3F4
  jsr(inst);
  CHECK(reg[RPC] == PC_START - 0x3F4);
  CHECK(reg[R7] == PC_START);

  // move return to R6 and return back to PC_START
  // after return, R7 has the location we returned from...
  inst = 0x5DA0; // 0101 110 110 1 00000 AND R6, R6, #0
  andlc(inst);
  CHECK(reg[R6] == 0x0);
  inst = 0x1D87; // 0001 110 110 000 111 ADD R6, R6, R7
  add(inst);
  CHECK(reg[R6] == reg[R7]);
  inst = 0x4180; // 0100 0 00 110 000000 JSRR R6
  jsr(inst);
  CHECK(reg[RPC] == PC_START);
  CHECK(reg[R7] == PC_START - 0x3F4);
}
#endif // task8

/**
 * @brief Task 9: Implement trap service routines
 */
#ifdef task9
TEST_CASE("Task 9: implement trap instruction service routines", "[task9]")
{
  // TODO: not testing these currently.  We could redirect standard
  // input and standard output to files or string buffers and test
  // the I/O routines in that manner here.
}
#endif // task9

/**
 * @brief Task 10: Implement main fetch-decode-execute simulation loop
 */
#ifdef task10
TEST_CASE("Task 10: implement main fetch-decode-execute simulation loop", "[task10]")
{
  // try the simple sum program
  char fname[] = "./progs/sum.obj";
  ld_img(fname, 0x0000);
  start(0x0000);

  // on finish, R0 = R1 = 0x02, result of last add was positive FP
  CHECK(reg[R0] == 0x02);
  CHECK(reg[R1] == 0x02);
  CHECK(reg[RPC] == 0x3008);
  CHECK(reg[RCND] == FP);
}
#endif // task10
