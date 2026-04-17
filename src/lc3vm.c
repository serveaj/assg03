/** @file lc3vm.c
 * @brief LC-3 VM Implementation
 *
 * @author Joshua Darwin
 * @note   cwid: 50327671
 * @date   Spring 2026
 * @note   ide:  g++ 8.2.0 / GNU Make 4.2.1
 *
 * Implementation of LC-3 VM/Microarchitecture simulator.  Functions
 * to fetch-decode-execute LC-3 encoded machine instructions.
 * Support functions for the microcode to decode instructions, addresses
 * and simulate registers, datapath and ALU operations.
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lc3vm.h"
#include "lc3vm_dbg.h"

/// running global variable used by trap in simulator to halt machine execution
bool running = true;

/// declarations of memory and registers used in the microprogram simulator
uint16_t mem[UINT16_MAX + 1] = {0};
uint16_t reg[RCNT] = {0};
uint16_t PC_START = 0x3000;

/** @brief memory read, transfer from memory
 *
 * Given a 16 bit address (from 0x0 to UINT_MAX=65535), access
 * the simulated memory and read and return the indicated value.
 * No error checking is done here, though the parameter type is
 * limited to 16 bits so in theory no illegal memory access should
 * be possible from this function.
 *
 * @param address The memory address to read and transfer data from.
 *   This parameter is an unsigned 16 bit integer, all addresses
 *   are constrained to be 16 bits in size in LC-3.
 *
 * @returns uint16_t Retuns a 16 bit result.  The result may be interpreted
 *   later as something other than an unsigned integer, but this function
 *   simply reads and returns the 16 bits stored at the indicated address.
 */
uint16_t mem_read(uint16_t address)
{ return mem[address]; }

/** @brief memory write, transfer to memory
 *
 * Given a 16 bit address and a 16 bit value, store the value in our
 * LC-3 memory at the indicated address.  No error checking is done here,
 * though the parameter type for both the address and value are limited
 * to 16 bits, so in theory no illegal memory access should be possible, nor
 * is it possible to provide too few or too many bits in the requested read
 * operation.
 *
 * @param address A unsigned 16 bit memory address, the target location where the
 *   given value should be written into our simulated memory.
 * @param value A unsigned 16 bit value, this value is not interpreted, it is simply
 *   stored where requested, it could actually be a signed number, or an ascii
 *   character, or some other type of data.
 */
void mem_write(uint16_t address, uint16_t value)
{ mem[address] = value; }

/** @brief sign extend bits
 *
 * Given a 16-bit value and a sign position, perform a twos-complement sign
 * extension on the original 16-bit value.  For example, if the bits given are
 *    0000 0000 0001 1010
 * This number is a 5 bit value in twos-complement (-6 in this case), but we need
 * to extend this result to the full 16 bits before we can add this value to
 * other twos-complement encoded values and get the expected result.  For a
 * sign_position of 5, the result after sign extend should be:
 *    1111 1111 1111 1010 (which is -6 in twos-complement encoded in full 16 bits)
 *
 * NOTE: This method assumes all bits in bit position+1 and higher have already been
 * masked to 0 before being called.
 *
 * @param bits The original bits that encode a twos-complement number but only int
 *    last n significant bits
 * @param sign_position The position of the sign bit that needs to be extended to
 *    all higher bits given bits value.
 *
 * @returns uint16_t Returns the bits with the sign bit extended to all higher
 *    bit positions, thus converting this to a full 16-bit twos-complement sigend
 *    value.
 */
uint16_t sign_extend(uint16_t bits, int size)
{
  // mask to get the sign bit
  uint16_t sign_bit = (bits >> (size - 1)) & 1;

  if (sign_bit == 1)
  {
    // extend with 1s
    bits |= (0xFFFF << size);
  }
  else
  {
    // ensure upper bits are cleared
    bits &= ~(0xFFFF << size);
  }

  return bits;
}

/** @brief update condition register flags
 *
 * Given a destination register that was just modified by an LC-3 operation,
 * update the condition codes register pased on the value in the modified
 * register.  If the register is 0 then the FZ condition should be set.  If
 * the register contains a twos-complement negative signed number (e.g. bit 15
 * is a 1), then the FN condition should be set for a negative number.  Otherwise
 * the FP condition flag should be set for a positive number.
 *
 * @param modified_register A (symbolic enumerated type name of) a register that
 *   was just modified by an operation and needs to have the condition code flags
 *   updated as a side effect of the operation just performed.
 */
void update_flags(enum registr r)
{
  uint16_t val = reg[r];

  if (val == 0)
  {
    reg[RCND] = FZ;
  }
  else if ((val >> 15) & 1)
  {
    reg[RCND] = FN;
  }
  else
  {
    reg[RCND] = FP;
  }
}

/** @brief add operation
 *
 * Add two values together and store result in destination register.
 * In one version, when the FIMM flag at bit 5 is 0, there are two
 * source registers to add to get the result.  When the FIMM flag at
 * bit 5 is 1, then the least significant 5 bits are interpreted as an
 * immediate twos-complement signed value that is added to the source
 * register to get the result.  Don't forget that we need to perform
 * sign extension if we have an immediate value in the instruction.
 *
 * Although all values in registers are uint16_t (unsigned 16 bit
 * integers), we can cheat a bit and just use the C + operator to do
 * the addition.  This is because, adding the bits of twos-complement
 * encoded values gives the correct result.  The only issue is that
 * the result may no longer fit into 16-bit twos complement.  If it
 * overflows the results is nonsensical.  Some architectures have
 * additional condition codes that would detect and be set if this
 * type of overflow occurs.
 *
 * @param i The instruction.  The bits of the instruction we are
 *   executing.  We need all of the bits so that we can extract the
 *   destination and source register operands, and to extract the
 *   second source register or the immediate value encoded in the
 *   instruction.
 */
void add(uint16_t i)
{
  uint16_t dr = DR(i);
  uint16_t sr1 = SR1(i);

  uint16_t result;

  if (FIMM(i))
  {
    uint16_t imm = SEXTIMM(i);
    result = reg[sr1] + imm;
  }
  else
  {
    uint16_t sr2 = SR2(i);
    result = reg[sr1] + reg[sr2];
  }

  reg[dr] = result;
  update_flags(dr);
}

/** @brief logical AND operation
 *
 * Compute the logical AND of 2 16 bit values and store the result in
 * the destination register.  In one version, when the FIMM flag at bit
 * 5 is 0, there are two source registers to AND together to get the
 * result.  When the FIMM flag at bit 5 is 1, then the least significant
 * 5 bits are extracted.  For consistency we interpret as a signed number,
 * and perform sign extension to create a 16 bit value from the 5 immediate
 * bits in the instruction.
 *
 * We can use the bitwise and operator '&' to perform this operation for
 * both versions of our operation.
 *
 * @param i The instruction.  The bits of the instruction we are
 *   executing.  We need all of the bits so that we can extract the
 *   destination and source register operands, and to extract the
 *   second source register or the immediate value encoded in the
 *   instruction.
 */
void andlc(uint16_t i)
{
  uint16_t dr = DR(i);
  uint16_t sr1 = SR1(i);

  uint16_t result;

  if (FIMM(i))
  {
    uint16_t imm = SEXTIMM(i);
    result = reg[sr1] & imm;
  }
  else
  {
    uint16_t sr2 = SR2(i);
    result = reg[sr1] & reg[sr2];
  }

  reg[dr] = result;
  update_flags(dr);
}

/** @brief logical NOT operation
 *
 * Perform a logical NOT on the indicated source register and save the
 * result into the destination register.  We will use the C logical not
 * operator '~' to perform the actual operation.
 *
 * @param i The instruction.  The bits of the instruction we are
 *   executing.  We need all of the bits so that we can extract the
 *   destination and source register operands, and to extract the
 *   second source register or the immediate value encoded in the
 *   instruction.
 */
void notlc(uint16_t i)
{
  uint16_t dr = DR(i);
  uint16_t sr = SR1(i);

  reg[dr] = ~reg[sr];
  update_flags(dr);
}

/** @brief load RPC + offset
 *
 * Load a value from memory calculated as some offset from the current
 * RPC program counter location.  The low 9 bits of the instruction are
 * interpreted as a signed twos-complement number (and need to be sign
 * extended).  This offset is added to the RPC to calculate an address in
 * memory.  Because we use 9 signed bits, the offset from the current
 * RPC ranges from +256 to -257.  The value in the calculated address is
 * transfered from memory into the destination register specified in
 * the instruction.
 *
 * @param i The instruction.  The bits of the instruction we are
 *   executing.  We need all of the bits so that we can extract the
 *   destination and source register operands, and to extract the
 *   second source register or the immediate value encoded in the
 */
void ld(uint16_t i)
{
  uint16_t dr = DR(i);
  uint16_t offset = PCOFF9(i);

  uint16_t address = reg[RPC] + offset;
  reg[dr] = mem_read(address);

  update_flags(dr);
}

/** @brief load indirect
 *
 * Load a value from memory using indirect addressing.  The same
 * initial address is calculated from the PCOffset9 low order bits of
 * this instruction, giving a first target address that is +256 or
 * -257 distance from the current RPC.  However the value in that
 * location is then interpreted as another memory address, which will
 * be fetched and then loaded into the destination register.
 * Thus two memory access reads are performed by this operation.
 *
 * @param i The instruction.  The bits of the instruction we are
 *   executing.  We need all of the bits so that we can extract the
 *   destination and source register operands, and to extract the
 *   second source register or the immediate value encoded in the
 */
void ldi(uint16_t i)
{
  uint16_t dr = DR(i);
  uint16_t offset = PCOFF9(i);

  uint16_t address = reg[RPC] + offset;
  uint16_t indirect = mem_read(address);

  reg[dr] = mem_read(indirect);

  update_flags(dr);
}

/** @brief load base + relative offset
 *
 * This instruction uses SR1 as a base address.  The value in this register is
 * treated as and address and the low 6 bits are treated as an offset which is
 * a twos-complement signed number.  So offset values can range from
 * +32 to -32 from the base address in the base register.  This offset is added
 * to the base address and the value from this memory location is fetched
 * and stored in the destination register.
 *
 * @param i The instruction.  The bits of the instruction we are
 *   executing.  We need all of the bits so that we can extract the
 *   destination and source register operands, and to extract the
 *   second source register or the immediate value encoded in the
 */
void ldr(uint16_t i)
{
  uint16_t dr = DR(i);
  uint16_t base = SR1(i);
  uint16_t offset = OFF6(i);

  uint16_t address = reg[base] + offset;
  reg[dr] = mem_read(address);

  update_flags(dr);
}

/** @brief load effective address
 *
 * Despite this functions name, a memory access is not performed.
 * This function calculates an effective address that may be useful to
 * load further values.  The low order 9 bits are treated as a
 * twos-complement signed value and added to the current RPC.  This
 * calculated address is simply saved into the indicated destination
 * register (e.g. we do not perform a memory read here, only
 * calculate an address).
 *
 * @param i The instruction.  The bits of the instruction we are
 *   executing.  We need all of the bits so that we can extract the
 *   destination and source register operands, and to extract the
 *   second source register or the immediate value encoded in the
 */
void lea(uint16_t i)
{
  uint16_t dr = DR(i);
  uint16_t offset = PCOFF9(i);

  reg[dr] = reg[RPC] + offset;
}

/** @brief store to PC + offset
 *
 * Store a value into memory from a source register.  The location where the
 * value is stored is calculated from an offset relative to the current RPC
 * value.  The low 9 bits of the instruction are treated as an signed
 * twos-complement number for the offset, thus values from +0xFF to -0X100
 * can be stored relative to the current RPC location.
 *
 * @param i The instruction.  The bits of the instruction we are
 *   executing.  We need all of the bits so that we can extract the
 *   destination and source register operands, and to extract the
 *   second source register or the immediate value encoded in the
 */
void st(uint16_t i)
{
  uint16_t sr = DR(i); // actually source
  uint16_t offset = PCOFF9(i);

  uint16_t address = reg[RPC] + offset;
  mem_write(address, reg[sr]);
}

/** @brief store indirect
 *
 * Similar to the basic store, but with an extra level of indirection.
 * The PCOFF9 low order bits provide an offset in the range of -0x100
 * to 0xFF relative to the PC.  This address is first read to obtain
 * the indirect address.  This indirect fetched address is the
 * ultimate location where the source register for this instruction is
 * stored.
 *
 * @param i The instruction.  The bits of the instruction we are
 *   executing.  We need all of the bits so that we can extract the
 *   destination and source register operands, and to extract the
 *   second source register or the immediate value encoded in the
 */
void sti(uint16_t i)
{
  uint16_t sr = DR(i);
  uint16_t offset = PCOFF9(i);

  uint16_t address = reg[RPC] + offset;
  uint16_t indirect = mem_read(address);

  mem_write(indirect, reg[sr]);
}

/** @brief store offset relative to base address
 *
 * This instruction has a register with a base address, and the low 6
 * bits are interpreted as a twos-complement signed number that is
 * added to the base address to get the actuall address for storing
 * the value.  The source register of this instruction contains the
 * value that will be stored to the calculated base + offset address.
 *
 * @param i The instruction.  The bits of the instruction we are
 *   executing.  We need all of the bits so that we can extract the
 *   destination and source register operands, and to extract the
 *   second source register or the immediate value encoded in the
 */
void str(uint16_t i)
{
  uint16_t sr = DR(i);
  uint16_t base = SR1(i);
  uint16_t offset = OFF6(i);

  uint16_t address = reg[base] + offset;
  mem_write(address, reg[sr]);
}

/** @brief jump unconditionally
 *
 * Jump unconditionally to a 16 bit address.  The jump
 * destination is held in the indicated base
 * register (in the SR1 bits) of the given instruction.
 *
 * @param i The instruction.  The bits of the instruction we are
 *   executing.  We need all of the bits so that we can extract the
 *   destination and source register operands, and to extract the
 *   second source register or the immediate value encoded in the
 */
void jmp(uint16_t i)
{ reg[RPC] = reg[SR1(i)]; }

/** @brief conditional branch
 *
 * Perform a conditional branch.  First we extract the
 * N,Z,P bits from positions 11-9 of the instruction.  These
 * indicate which conditions (if any) we should branch on,
 * N negative, Z zero, P positive.  These conditions need to
 * be checked agains the current condition flags in the RCND
 * register.  If the condition to branch is met, we use the low
 * 9 bits to perform a PC relative branch to the new location
 * in the code.
 *
 * @param i The instruction.  The bits of the instruction we are
 *   executing.  We need all of the bits so that we can extract the
 *   destination and source register operands, and to extract the
 *   second source register or the immediate value encoded in the
 */
void br(uint16_t i)
{
  uint16_t cond = DR(i);
  uint16_t offset = PCOFF9(i);

  if (cond & reg[RCND])
  {
    reg[RPC] += offset;
  }
}
/** @brief jump to/from subtroutine
 *
 * This microcode handles both jump into a subroutine and return
 * from subroutine, which may appear as different opcodes jsr and
 * jsrr respectively in the assembly.
 *
 * @param i The instruction.  The bits of the instruction we are
 *   executing.  We need all of the bits so that we can extract the
 *   destination and source register operands, and to extract the
 *   second source register or the immediate value encoded in the
 */
void jsr(uint16_t i)
{
  reg[R7] = reg[RPC];

  if (FL(i))
  {
    reg[RPC] = reg[RPC] + PCOFF11(i);
  }
  else
  {
    reg[RPC] = reg[SR1(i)];
  }
}

/** @brief return from interrupt
 *
 * We do not implement interrupts (yet), this
 * instruction causes a return from an iterrupt routine
 * that was generated by an interrupt.
 *
 * @param i The instruction.  The bits of the instruction we are
 *   executing.  We need all of the bits so that we can extract the
 *   destination and source register operands, and to extract the
 *   second source register or the immediate value encoded in the
 */
void rti(uint16_t i) {} // unused

/** @brief reserved
 *
 * Reserved/unused opcode (1101).  We do nothing if it somehow
 * get invoked in simulation, so this is also effectively a
 * NOOP instruction currently.
 *
 * @param i The instruction.  The bits of the instruction we are
 *   executing.  We need all of the bits so that we can extract the
 *   destination and source register operands, and to extract the
 *   second source register or the immediate value encoded in the
 */
void res(uint16_t i) {} // unused

/** @brief read character OS service routine
 *
 * Read a single character and save as 16 bits into R0
 * register.  We hook into C standard library to get the
 * character and store it.
 */
void tgetc()
{ reg[R0] = getchar(); }

/** @brief write character OS service routine
 *
 * Write a single character found in register R0 to
 * standard output (usually connected to a console
 * or terminal).
 */
void tout()
{ fprintf(stdout, "%c", (char)reg[R0]); }

/** @brief write string OS service routine
 *
 * Write a string of characters to the standard output
 * console.  We probably normally would reuse the
 * tout() in a real implementation and call repeatedly.
 * Here we hook into C standard library to output a string
 * of characters.
 *
 * Note: this method assumes a null character 0x0000 terminates
 * the string to be written (which is came as C standard library).
 *
 * Note: Our simulated memory holds 16 bit values, and this
 * routine iterates through them 1 by 1, displaying them.  But
 * it casts the 16 bits to an 8 bit (ascii) character and outputs
 * that.  High 8 bits are not used here for characters / strings.
 */
void tputs()
{
  uint16_t* p = mem + reg[R0];
  while (*p)
  {
    fprintf(stdout, "%c", (char)*p);
    p++;
  }
}

/** @brief read character and display OS service routine
 *
 * Reads a character and puts it into R0, same as tgetc().
 * But character is echoed to console after being typed, so
 * can see text as you type it if use this routine.
 */
void tin()
{
  reg[R0] = getchar();
  fprintf(stdout, "%c", reg[R0]);
}

/** @brief store 2 characters / 16bit word
 *
 * LC-3 uses 16 bit words, but normal ASCII encoding of characters
 * only needs 8 bits.  So in theory we could implement service routines
 * that get 8-bit characters and packs 2 of them in each word of the
 * LC-3 memory.  And correspondingly here have routines that expect
 * 2 ASCII characters per word and displays them accordingly.
 */
void tputsp()
{ /* Not Implemented */
}

/** @brief halt system service routine
 *
 * Really a simulation service routine, causes immediate halt of
 * the LC-3 simulation.  This is done simply enough by using a flag
 * that is checked at start of each fetch-decode-execute cycle to
 * determine if we should halt execution.
 */
void thalt()
{ running = false; }

/** @brief read unsigned int OS service routine
 *
 * Read digits from the standard input keyboard and interpret them
 * as an unsigned 16 bit integer.  If the value fits (in range from
 * 0 to 2^16) then it will be correctly encoded as an unsigned 16
 * bit value into R0.  It is undefined what happens if the value
 * cannot be correctly parsed into an unsigned 16 bit value here.
 */
void tinu16()
{ fscanf(stdin, "%hu", &reg[R0]); }

/** @brief write unsigned int OS service routine
 *
 * Write value in R0, interpreted as a 16 bit unsigned integer, to the
 * standard output console.
 */
void toutu16()
{ fprintf(stdout, "%hu\n", reg[R0]); }

/**
 * Trap service routine function pointer array.  Routines are indexed from 0
 * to 7 currently for the 8 service routines.
 */
trp_ex_f trp_ex[8] = {tgetc, tout, tputs, tin, tputsp, thalt, tinu16, toutu16};

/** @brief trap instruction
 *
 * The trap service vector is in low 8 bits 7-0 of the instruction.
 * Service routines trap vectors start at 0x20 currently in the
 * specification.  We calculate index into the trap service routine
 * pointer array and invoke the specified trap service routine here.
 *
 * @param i The instruction.  The bits of the instruction we are
 *   executing.  We need all of the bits so that we can extract the
 *   destination and source register operands, and to extract the
 *   second source register or the immediate value encoded in the
 */
void trap(uint16_t i)
{ trp_ex[TRP(i) - trp_offset](); }

/**
 * LC-3 instruction microcode store / lookup table.  Need to define array
 * of function pointers with all (microcode) functions inserted in
 * correct order so that index corresponds with the instruction opcode.
 * Execution happens by extracting opcode from instruction after fetch, and
 * looking up and invoking the (microcode) instruction function from this
 * lookup table.
 */
op_ex_f op_table[NUMOPS];


/** @brief start/run LC-3 simulator
 *
 * Implement the main fetch-decode-execute loop.  The next
 * instruction is fetched based on the RPC.  We extract the
 * opcode using the OPC macro, and use the opcode execution
 * function lookup table to decode the instruction parameters and
 * complete the execution of the opcode.  This simulation
 * keeps fetching and executing instructions until a halt
 * trap service routine is invoked, which halts the simulation
 * from running.
 *
 * @param offset Normal starting RPC is 0x3000, but we can specify
 *   a 16-bit (signed) offset from this location and start there instead
 *   in this routine.
 */
void start(uint16_t offset)
{
  reg[RPC] = PC_START + offset;
  op_table[0x0] = br;
  op_table[0x1] = add;
  op_table[0x2] = ld;
  op_table[0x3] = st;
  op_table[0x4] = jsr;
  op_table[0x5] = andlc;
  op_table[0x6] = ldr;
  op_table[0x7] = str;
  op_table[0x8] = rti;
  op_table[0x9] = notlc;
  op_table[0xA] = ldi;
  op_table[0xB] = sti;
  op_table[0xC] = jmp;
  op_table[0xD] = res;
  op_table[0xE] = lea;
  op_table[0xF] = trap;
  while (running)
  {
    uint16_t instr = mem_read(reg[RPC]);

    reg[RPC]++;

    uint16_t opcode = OPC(instr);

    op_table[opcode](instr);
  }
}

/** @brief load an LC-3 machine instruction image
 *
 * This functions loads a LC-3 machine language file (binary)
 * from a file into memory.  The program is loaded into
 * 0x3000 by default, though a 16-bit offset can be specified to
 * load the machine instructions at some offset from the normal
 * starting location.
 *
 * @param fname The name of the file to open and read the LC-3
 *   machine instructions from.  This is expected to be a binary file
 *   which reads 16 bit values and places them consecutively into
 *   the simulated memory.
 * @param offset Normal location to load programs is to 0x3000, but
 *   the load location can be offset by a signed 16-bit offset value here.
 *   If this value is 0, programs are loaded to 0x3000 by default.
 */
void ld_img(char* fname, uint16_t offset)
{
  FILE* in = fopen(fname, "rb");
  if (NULL == in)
  {
    fprintf(stderr, "Cannot open file %s.\n", fname);
    exit(1);
  }
  uint16_t* p = mem + PC_START + offset;
  fread(p, sizeof(uint16_t), (UINT16_MAX - PC_START), in);
  fclose(in);
}
