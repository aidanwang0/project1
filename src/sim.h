#include <stdio.h>
#include <assert.h>
#include <fstream>
#include <iostream>
#include <string>

#include "MemoryStore.h"
#include "RegisterInfo.h"

// --------------------------------------------------------------------------
// Reg data structure
// --------------------------------------------------------------------------

union REGS {
    RegisterInfo reg;
    uint64_t registers[REG_SIZE] {0};
};

union REGS regData;

uint64_t PC;

// --------------------------------------------------------------------------
// Decode constants
// --------------------------------------------------------------------------

// TODO Complete these enums, add more if necessary

enum OPCODES {
    // I-type opcodes
    OP_INTIMM  = 0b0010011, // Integer ALU immediate instructions addi, slli, slti, sltiu, xori, srli, srai, ori, andi
    OP_WINTIMM = 0b0011011, // Integer ALU (word) immediate instructions addiw, srliw, sraiw, slliw
    // R-type opcodes
    OP_R_64BIT = 0b0110011, //64 bit ALU instructions add, sub, sll, slt, sltu, xor, srl, sra, or, and
    OP_R_32BIT = 0b0111011, //32 bit ALU instructions adsdw, subw, sllw, srlw, sraw
    // U-type opcode
    OP_U_AUIPC = 0b0010111, // add upper immediate to PC
    OP_SB_BRANCH = 0b1100011, // branch instructions

};

enum FUNCT3 {
    // For integer ALU instructions
    FUNCT3_ARITH  = 0b000, // add, sub, addi, addiw, beq
    FUNCT3_AND = 0b111, // and, andi
    FUNCT3_OR = 0b110, // or, ori
    FUNCT3_XOR = 0b100, // xor
    FUNCT3_RSHIFT = 0b101, //sra, sraw, srai
    FUNCT3_LSHIFT = 0b001, //sll, sllw, slli,slliw
    // ...
};

enum RI_FUNCT7 {
    // for R type add/sub instruction
    FUNCT7_ADDSHIFT     = 0b0000000, // add, and, or, xor, slliw
    FUNCT7_SUBSHIFT     = 0b0100000, //sub, sra, sraiw, srai
    // ...
};

// --------------------------------------------------------------------------
// Bit-level manipulation helpers
// --------------------------------------------------------------------------

// TODO You may wish to declare some helper functions for bit extractions
// and sign extensions

// --------------------------------------------------------------------------
// Utilities
// --------------------------------------------------------------------------

// initialize memory with program binary
bool initMemory(char *programFile, MemoryStore *myMem);

// dump registers and memory
void dump(MemoryStore *myMem);

// --------------------------------------------------------------------------
// Simulation functions
// --------------------------------------------------------------------------

// The simulator maintains the following struct as it simulates 
// RISC-V instructions. Feel free to add more fields if needed.
struct Instruction {
    uint64_t PC = 0;
    uint64_t instruction = 0; // raw instruction binary

    bool     isHalt = false;
    bool     isLegal = false;
    bool     isNop = false;

    bool     readsMem = false;
    bool     writesMem = false;
    bool     doesArithLogic = false;
    bool     writesRd = false;
    bool     readsRs1 = false;
    bool     readsRs2 = false;

    uint64_t opcode = 0;
    uint64_t funct3 = 0;
    uint64_t funct7 = 0;
    uint64_t rd = 0;
    uint64_t rs1 = 0;
    uint64_t rs2 = 0;

    uint64_t nextPC = 0;

    uint64_t op1Val = 0;
    uint64_t op2Val = 0;

    uint64_t arithResult = 0;
    uint64_t memAddress = 0;
    uint64_t memResult = 0;
};

// The following functions are the core of the simulator. Your task is to
// complete these functions in sim.cpp. Do not modify their signatures.
// However, feel free to declare more functions if needed.

// There is no strict rule on what each function should do, but the
// following comments give suggestions.

// Get raw instruction bits from memory
Instruction simFetch(uint64_t PC, MemoryStore *myMem);

// Determine instruction opcode, funct, reg names, and what resources to use
Instruction simDecode(Instruction inst);

// Collect reg operands for arith or addr gen
Instruction simOperandCollection(Instruction inst, REGS regData);

// Resolve next PC whether +4 or branch/jump target
Instruction simNextPCResolution(Instruction inst);

// Perform arithmetic/logic operations
Instruction simArithLogic(Instruction inst);

// Generate memory address for load/store instructions
Instruction simAddrGen(Instruction inst);

// Perform memory access for load/store instructions
Instruction simMemAccess(Instruction inst, MemoryStore *myMem);

// Write back results to registers
Instruction simCommit(Instruction inst, REGS &regData);

// Simulate the whole instruction using functions above
Instruction simInstruction(uint64_t &PC, MemoryStore *myMem, REGS &regData);