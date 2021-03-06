/*! \file
 *
 * This file contains the definitions for the instruction decoder for the
 * branching processor.  The instruction decoder extracts bits from the current
 * instruction and turns them into separate fields that go to various functional
 * units in the processor.
 */

 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "branching_decode.h"
#include "register_file.h"
#include "instruction.h"


/*
 * Branching Instruction Decoder
 *
 *  The Instruction Decoder extracts bits from the instruction and turns
 *  them into separate fields that go to various functional units in the
 *  processor.
 */


/*!
 * This function dynamically allocates and initializes the state for a new 
 * branching instruction decoder instance.  If allocation fails, the program is
 * terminated.
 */
Decode * build_decode() {
    Decode *d = malloc(sizeof(Decode));
    if (!d) {
        fprintf(stderr, "Out of memory building an instruction decoder!\n");
        exit(11);
    }
    memset(d, 0, sizeof(Decode));
    return d;
}


/*!
 * This function frees the dynamically allocated instruction-decoder instance.
 */
void free_decode(Decode *d) {
    free(d);
}


/*!
 * This function decodes the instruction on the input pin, and writes all of the
 * various components to output pins.  Other components can then read their
 * respective parts of the instruction.
 *
 * NOTE:  the busdata_t type is defined in bus.h, and is simply
 *        an unsigned long.
 */
void fetch_and_decode(InstructionStore *is, Decode *d, ProgramCounter *pc) {
    /* This is the current instruction byte we are decoding. */ 
    unsigned char instr_byte;

    /* The CPU operation the instruction represents.  This will be one of the
     * OP_XXXX values from instruction.h.
     */
    busdata_t operation;
    
    /* Source-register values, including defaults for src1-related values. */
    busdata_t src1_addr = 0, src1_const = 0, src1_isreg = 1;
    busdata_t src2_addr = 0;

    /* Destination register.  For both single-argument and two-argument
     * instructions, dst == src2.
     */
    busdata_t dst_addr = 0;
    
    /* Flag controlling whether the destination register is written to.
     * Default value is to *not* write to the destination register.
     */
    busdata_t dst_write = NOWRITE_REG;

    /* The branching address loaded from a branch instruction. */
    busdata_t branch_addr = 0;

    /* All instructions have at least one byte, so read the first byte. */
    ifetch(is);   /* Cause InstructionStore to push out the instruction byte */
    instr_byte = pin_read(d->input);
    
    
    /* Get the opcode from the first instruction byte. */
    operation = instr_byte >> 4;
    unsigned char reg1, reg2;
    /* Then decide what to do next. */
    switch(operation) {
        /* If opcode is DONE, we don't need to set anything. */
        case OP_DONE:
            break;
        
        /* One-argument instructions occupy one byte.
         * This byte takes the form [op3 op2 op1 op0 x src2 src1 src0]
         * src2 src1 src0 specify the register to perform the operation on,
         * as well as where to store the result.
         * All six operations write back to the register.
         */
        case OP_DEC:
        case OP_NEG:
        case OP_INV:
        case OP_SHL:
        case OP_SHR:
            src1_addr = src2_addr = dst_addr = instr_byte & 0x07;
            dst_write = WRITE_REG;
            break;
        
        /* Two-argument instructions occupy two bytes.
         * The first four bits of the first byte is the opcode.
         * The fifth bit of the first byte specifies whether the first
         * argument is a constant or a register. (= src1_isreg)
         * The last three bytes of the first byte specify the second register
         * argument. (= src2_addr as well as dst_addr)
         * Then we increment the program counter and fetch the next instruction
         * byte.
         * Depending on whether src1_isreg, we specify the first register
         * argument (= src1_addr) or the first argument constant (= src1_const)
         * All six operations write to the destination register.
         */
        case OP_MOV:
        case OP_ADD:
        case OP_SUB:
        case OP_AND:
        case OP_OR:
        case OP_XOR:
            src2_addr = dst_addr = instr_byte & 0x07;
            src1_isreg = (instr_byte & 0x08) >> 3;
            incrPC(pc);
            ifetch(is);
            instr_byte = pin_read(d->input);
            if (src1_isreg) {
                src1_addr = instr_byte & 0x07;
            }
            else {
                src1_const = instr_byte;
            }
            dst_write = WRITE_REG;
            break;
        
        /* Branching instructions occupy one byte.
         * The first four bits are the opcode, and the last four bits
         * are the branching address. (= branch_addr)
         */
        case OP_BRA:
        case OP_BRZ:
        case OP_BNZ:
            branch_addr = instr_byte & 0x0f;
            break;
        
        default:
            fprintf(stderr, "Opcode %d not recognized!\n", operation);
            break;
    }

    /* All decoded!  Write out the decoded values. */

    pin_set(d->cpuop,       operation);

    pin_set(d->src1_addr,   src1_addr);
    pin_set(d->src1_const,  src1_const);
    pin_set(d->src1_isreg,  src1_isreg);

    pin_set(d->src2_addr,   src2_addr);

    /* For this processor, like IA32, dst is always src2. */
    pin_set(d->dst_addr,    src2_addr);
    pin_set(d->dst_write,   dst_write);

    pin_set(d->branch_addr, branch_addr);
}
