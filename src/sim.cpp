#include "sim.h"
#include "inttypes.h"

#include <iostream>
#include <bitset>




using namespace std;

// RV64I without csr, environment, or fence instructions

//           31          25 24 20 19 15 14    12 11          7 6      0
// R  type: | funct7       | rs2 | rs1 | funct3 | rd          | opcode |
// I  type: | imm[11:0]          | rs1 | funct3 | rd          | opcode |
// S  type: | imm[11:5]    | rs2 | rs1 | funct3 | imm[4:0]    | opcode |
// SB type: | imm[12|10:5] | rs2 | rs1 | funct3 | imm[4:1|11] | opcode |
// U  type: | imm[31:12]                        | rd          | opcode |
// UJ type: | imm[20|10:1|11|19:12]             | rd          | opcode |

void printInstructionBinary(const Instruction &inst) {
    // Use std::bitset<32> to print the lower 32 bits in binary
    std::bitset<32> bits(inst.instruction & 0xFFFFFFFF);
    std::cout << "PC = 0x" << std::hex << inst.PC
              << " Instruction = 0b" << bits
              << " (0x" << std::hex << (inst.instruction & 0xFFFFFFFF) << ")"
              << std::dec << std::endl;
}

// initialize memory with program binary
bool initMemory(char *programFile, MemoryStore *myMem) {
    // open instruction file
    ifstream infile;
    infile.open(programFile, ios::binary | ios::in);

    if (!infile.is_open()) {
        fprintf(stderr, "\tError open input file\n");
        return false;
    }

    // get length of the file and read instruction file into a buffer
    infile.seekg(0, ios::end);
    int length = infile.tellg();
    infile.seekg (0, ios::beg);

    char *buf = new char[length];
    infile.read(buf, length);
    infile.close();

    int memLength = length / sizeof(buf[0]);
    int i;
    for (i = 0; i < memLength; i++) {
        myMem->setMemValue(i * BYTE_SIZE, buf[i], BYTE_SIZE);
    }

    return true;
}

// dump registers and memory
void dump(MemoryStore *myMem) {

    dumpRegisterState(regData.reg);
    dumpMemoryState(myMem);
}

// TODO All functions below (except main) are incomplete.
// Only ADDI is implemented. Your task is to complete these functions.

// Get raw instruction bits from memory
Instruction simFetch(uint64_t PC, MemoryStore *myMem) {
    // fetch current instruction
    uint64_t instruction;
    myMem->getMemValue(PC, instruction, WORD_SIZE);
    instruction = (uint32_t)instruction;

    Instruction inst;
    inst.PC = PC;
    inst.instruction = instruction;
    return inst;
}

// Determine instruction opcode, funct, reg names, and what resources to use
Instruction simDecode(Instruction inst) {
    inst.opcode = inst.instruction & 0b1111111;
    inst.funct3 = inst.instruction >> 12 & 0b111;
    inst.funct7 = inst.instruction >> 25 & 0b1111111;
    inst.rd = inst.instruction >> 7 & 0b11111;
    inst.rs1 = inst.instruction >> 15 & 0b11111;
    inst.rs2 = inst.instruction >> 20 & 0b11111;
    // printf("opcode = %llu\n", (unsigned long long)inst.opcode);
    if (inst.instruction == 0xfeedfeed) {
        inst.isHalt = true;
        return inst; // halt instruction
    }
    if (inst.instruction == 0x00000013) {
        inst.isNop = true;
        return inst; // NOP instruction
    }
    inst.isLegal = true; // assume legal unless proven otherwise

    switch (inst.opcode) {
        case OP_INTIMM:
            if (inst.funct3 == FUNCT3_ARITH || inst.funct3 ==FUNCT3_AND || inst.funct3 == FUNCT3_OR || 
                    inst.funct3 == FUNCT3_XOR || inst.funct3== FUNCT3_RSHIFT || inst.funct3 == FUNCT3_LSHIFT ||
                    inst.funct3 == FUNCT3_LOAD)  {
                inst.doesArithLogic = true;
                inst.writesRd = true;
                inst.readsRs1 = true;
                inst.readsRs2 = false;
            } 
            else {
                inst.isLegal = false;
            }
            break;

        case OP_SB_BRANCH:
            if (inst.funct3== FUNCT3_ARITH || inst.funct3 == FUNCT3_LSHIFT || inst.funct3 == FUNCT3_XOR || inst.funct3 == FUNCT3_RSHIFT || 
                inst.funct3 == FUNCT3_OR || inst.funct3 == FUNCT3_AND){
                inst.doesArithLogic = false;
                inst.writesRd = false;
                inst.readsRs1 = true;
                inst.readsRs2 = true;
            }
            else {
                inst.isLegal = false;
            }
            break;

        case OP_LUI:
            inst.doesArithLogic = false;
            inst.writesRd = true;
            inst.readsRs1 = false;
            inst.readsRs2 = false;
            break;

        case OP_U_AUIPC:
            inst.doesArithLogic = true;
            inst.writesRd = true;
            inst.readsRs1 = false;
            inst.readsRs2 = false;
            break;
            
        case OP_WINTIMM:
            if (inst.funct3==FUNCT3_ARITH || inst.funct3 == FUNCT3_RSHIFT || inst.funct3 == FUNCT3_LSHIFT){
                inst.doesArithLogic = true;
                inst.writesRd = true;
                inst.readsRs1 = true;
                inst.readsRs2 = false;
            }
            else {
                inst.isLegal = false;
            }
            break;
                    


        case OP_LOAD:
            if (inst.funct3 == FUNCT3_LOAD || inst.funct3 == FUNCT3_DOUBLELOADSTORE || inst.funct3 == FUNCT3_LSHIFT ||
                inst.funct3 == FUNCT3_ARITH || inst.funct3 == FUNCT3_OR){
  
                inst.doesArithLogic=false;
                inst.writesRd=true;
                inst.readsRs1= true;
                inst.readsRs2=false;
            }
            else{
                inst.isLegal = false;
            }
            break;

        case OP_R_64BIT:
            if (inst.funct3 == FUNCT3_ARITH || inst.funct3 == FUNCT3_AND || inst.funct3 == FUNCT3_OR ||
                    inst.funct3 == FUNCT3_XOR  || inst.funct3 == FUNCT3_RSHIFT || inst.funct3 == FUNCT3_LSHIFT ||
                    inst.funct3 == FUNCT3_LOAD) {
                inst.doesArithLogic = true;
                inst.writesRd = true;
                inst.readsRs1 = true;
                inst.readsRs2 = true;
            }
            else { 
                inst.isLegal = false;
            }
            break;
        
        case OP_R_32BIT:
            if (inst.funct3 == FUNCT3_ARITH || inst.funct3 == FUNCT3_RSHIFT || inst.funct3 == FUNCT3_LSHIFT){
                inst.doesArithLogic = true;
                inst.writesRd = true;
                inst.readsRs1 = true;
                inst.readsRs2 = true;
            }
    
            else { 
                inst.isLegal = false;
            }
            break;
        
        case OP_UJ_JAL:
            inst.doesArithLogic = false;
            inst.writesRd = true;
            inst.readsRs1 = false;
            inst.readsRs2 = false;
            break;

        case OP_S_STORE:
            if (inst.funct3 == FUNCT3_DOUBLELOADSTORE || inst.funct3 == FUNCT3_ARITH || inst.funct3 == FUNCT3_LSHIFT || inst.funct3 == FUNCT3_LOAD) {
                inst.doesArithLogic = false;
                inst.writesRd = false;
                inst.readsRs1 = true;
                inst.readsRs2 = true;
                inst.writesMem = true;
            }
            else{
                inst.isLegal = false;
            }
            break;

        case OP_JALR:
            if(inst.funct3 == FUNCT3_ARITH) {
                inst.doesArithLogic = false;
                inst.writesRd = true;
                inst.readsRs1 = true;
                inst.readsRs2= false;
            }
            else{
                inst.isLegal = false;
            }
            break;

        default:
            inst.isLegal = false;
    }
    
    return inst;
}

// Collect reg operands for arith or addr gen
Instruction simOperandCollection(Instruction inst, REGS regData) {
    
    inst.op1Val = regData.registers[inst.rs1];
    inst.op2Val = regData.registers[inst.rs2];

    return inst;
}


void print_binary(uint32_t value) {
    for (int i = 31; i >= 0; i--) {
        putchar((value & (1u << i)) ? '1' : '0');
        if (i % 4 == 0) putchar(' '); // optional: add a space every 4 bits
    }
    putchar('\n');
}

//constructs the immediate for sb types
int32_t getSBImmediate(Instruction inst) {
    printInstructionBinary(inst);
    uint32_t bit12 = (inst.instruction >> 31) & 0b1;
    uint32_t bits10_5 = (inst.instruction >> 25) & 0b111111;
    uint32_t bit11 = (inst.instruction >> 7) & 0b1;
    uint32_t bits4_1 = (inst.instruction >> 8) & 0b1111;

    uint32_t immediate = (bit12 << 12) | (bit11 << 11) | (bits10_5 << 5) | (bits4_1 << 1);
    print_binary(immediate);
    return immediate;
}

//constructs immediate for uj
uint32_t getUJImmediate(Instruction inst) {
    // extract bits
    printInstructionBinary(inst);
    uint32_t bit20 = (inst.instruction >> 31) & 0b1;      
    uint32_t bits10_1 = (inst.instruction >> 21) & 0b1111111111;     
    uint32_t bit11 = (inst.instruction >> 20) & 0b1;       
    uint32_t bits19_12 = (inst.instruction >> 12) & 0b11111111;      

    uint32_t immediate = (bit20) << 20 | bits19_12 << 12 | (bit11) << 11 | (bits10_1 << 1);
    print_binary(immediate);
    return immediate;
}

// Resolve next PC whether +4 or branch/jump target
Instruction simNextPCResolution(Instruction inst) {
    inst.opcode = inst.instruction & 0b1111111;

    // default case
    inst.nextPC = inst.PC + 4;

    switch(inst.opcode){
        case OP_SB_BRANCH: {
            uint64_t imm12 = getSBImmediate(inst);
            uint64_t sext_imm12 = (imm12 & 0x800) ? (imm12 | 0xFFFFFFFFFFFFF000) : imm12;
            // beq
            if(inst.funct3 == FUNCT3_ARITH && inst.op1Val == inst.op2Val) {
                inst.nextPC = inst.PC + sext_imm12;
            }
            // bne
            else if(inst.funct3 == FUNCT3_LSHIFT && inst.op1Val != inst.op2Val) {
                inst.nextPC = inst.PC + sext_imm12;
            }
            // bge r1 >= r2
            else if(inst.funct3 == FUNCT3_RSHIFT){
                uint64_t diff = inst.op2Val - inst.op1Val; // if rs2-rs1 is negative, or they are equal, then rs1 must be >=
                if (diff & 0x8000000000000000 || diff == 0x0){
                    inst.nextPC = inst.PC + sext_imm12;
                }
            } 
            // bgeu
            else if(inst.funct3 == FUNCT3_AND && inst.op1Val >= inst.op2Val) {
                inst.nextPC = inst.PC + sext_imm12;
            }
            // blt
            else if(inst.funct3 == FUNCT3_XOR && inst.op1Val < inst.op2Val) {
                inst.nextPC = inst.PC + sext_imm12;
            }
            // bltu
            else if(inst.funct3 == FUNCT3_OR && inst.op1Val < inst.op2Val) {
                inst.nextPC = inst.PC + sext_imm12;
            }
            break;
        }

        case OP_UJ_JAL: {
            inst.arithResult = inst.PC + 4; // R[rd] = PC+4
            uint64_t imm20 = getUJImmediate(inst);
            uint64_t sext_imm20 = (imm20 & 0x80000) ? (imm20 | 0xFFFFFFFFFFF00000) : imm20;
            inst.nextPC = inst.PC + sext_imm20;
            break;
        }

        case OP_JALR: {
            inst.arithResult = inst.PC + 4;
            uint64_t imm12  = inst.instruction >> 20 & 0b111111111111; //12 bit
            uint64_t sext_imm12 = (imm12 & 0x800) ? (imm12 | 0xFFFFFFFFFFFFF000) : imm12; //sign extend 12 bit

            inst.nextPC = inst.op1Val + sext_imm12;
            break;
        }
    }

    return inst;
}

// Perform arithmetic/logic operations
Instruction simArithLogic(Instruction inst) {
    inst.opcode = inst.instruction & 0b1111111;
    switch (inst.opcode) {
        case OP_INTIMM: {
            uint64_t imm12  = inst.instruction >> 20 & 0b111111111111; //12 bit
            uint64_t sext_imm12 = (imm12 & 0x800) ? (imm12 | 0xFFFFFFFFFFFFF000) : imm12; //sign extend 12 bit
            //addi
            if (inst.funct3 == FUNCT3_ARITH){
                inst.arithResult = inst.op1Val + sext_imm12;
            }

            //andi
            else if (inst.funct3 == FUNCT3_AND){
                inst.arithResult = inst.op1Val & sext_imm12;
            }

            //ori
            else if (inst.funct3 == FUNCT3_OR){
                inst.arithResult = inst.op1Val | sext_imm12;
            }

            //xori
            else if(inst.funct3 == FUNCT3_XOR){
                inst.arithResult = inst.op1Val ^ sext_imm12;
            }

            //srai (shift right, fill with sign bit) (not tested)
            else if (inst.funct3 == FUNCT3_RSHIFT && inst.funct7==FUNCT7_SUBSHIFT){
                uint64_t shamt = (inst.instruction >> 20) & 0b111111; //shift amount lower 6bits
                int64_t val = (int64_t)inst.op1Val;              
                inst.arithResult = val >> shamt; 
            }

            //srli (shift right, fill with 0) not tested
            else if (inst.funct3==FUNCT3_RSHIFT && inst.funct7==FUNCT7_ADDSHIFT){
                uint64_t shamt = (inst.instruction >> 20) & 0b111111; //shift amount lower 6bits
                uint64_t val = (uint64_t)inst.op1Val;            // unsigned
                uint64_t result64 = val >> shamt;
                inst.arithResult = (int64_t) result64;
            }

            //slli (shift left, fill with 0) (not tested)
            else if (inst.funct3 == FUNCT3_LSHIFT && inst.funct7 == FUNCT7_ADDSHIFT){
                uint64_t shamt = (inst.instruction >> 20) & 0b111111; //shift amount lower 6bits
                uint64_t val = (uint64_t)inst.op1Val;            // unsigned
                uint64_t result64 = val << shamt;
                inst.arithResult = (int64_t) result64;
            }

            //slti (set less than imm rd = 1 if rs1 < imm)
            else if (inst.funct3==FUNCT3_LOAD){
                if ((int64_t)inst.op1Val < sext_imm12) {
                    inst.arithResult = 1; 
                } else {
                    inst.arithResult = 0;  
                }
            }

            //sltiu (set less than imm)
            else if (inst.funct3==FUNCT3_DOUBLELOADSTORE){
                if ((uint64_t)inst.op1Val < imm12) {
                    inst.arithResult = 1; 
                } else {
                    inst.arithResult = 0;  
                }
            }
            break;
        }   

        //auipc
        case OP_U_AUIPC: {
            uint64_t imm12  = inst.instruction >> 12 & 0b11111111111111111111;
            inst.arithResult = inst.PC + imm12;
            break;
        }

        case OP_WINTIMM: {
            
            uint64_t imm12  = inst.instruction >> 20 & 0b111111111111;
            int32_t imm12_sext = (imm12 & 0x800) ? (imm12 | 0xFFFFF000) : imm12;

            //addiw (does this make sense??)
            if (inst.funct3 == FUNCT3_ARITH){
                int32_t val32 = (int32_t)(inst.op1Val); //truncate to 32 bits                
                int32_t result32 = val32 + imm12_sext;
                inst.arithResult = (int64_t)result32; //sign extend to 64 bits
            }

            //sraiw (shift right immediate, fill with sign bit, 32 bit) (not tested)
            else if (inst.funct3 == FUNCT3_RSHIFT && inst.funct7==FUNCT7_SUBSHIFT){
                uint64_t shamt = (inst.instruction >> 20) & 0b11111;    // shift amount lower 5 bits
                int32_t val32 = (int32_t)(inst.op1Val);  //truncate to 32 bits
                int32_t result32 = val32 >> shamt;                 
                inst.arithResult = (int64_t)result32;   //sign extend to 64 bits
            }
            
            //srliw (shift rigfht immediate, fill with 0, 32 bit) (not tested) (write test case)
            else if (inst.funct3 == FUNCT3_RSHIFT && inst.funct7==FUNCT7_ADDSHIFT){
                uint64_t shamt = (inst.instruction >> 20) & 0b11111;    // shift amount lower 5 bits
                uint32_t val32 = (uint32_t)(inst.op1Val);  //truncate to 32 bits
                uint32_t result32 = val32 >> shamt;                 
                inst.arithResult = (int64_t)(int32_t)result32;   //sign extend to 64 bits
            }

            //slliw (shift left immediate, fill with 0, 32 bit) (not tested) (write test case)
            else if (inst.funct3 == FUNCT3_LSHIFT && inst.funct7==FUNCT7_ADDSHIFT){
                uint64_t shamt = (inst.instruction >> 20) & 0b11111;    // shift amount lower 5 bits
                uint32_t val32 = (uint32_t)(inst.op1Val);  //truncate to 32 bits
                uint32_t result32 = val32 << shamt;                 
                inst.arithResult = (int64_t)(int32_t)result32;   //sign extend to 64 bits
            }
            
            break;
        }

        case OP_R_64BIT: {

            //and
            if (inst.funct3 == FUNCT3_AND && inst.funct7 == FUNCT7_ADDSHIFT){
                inst.arithResult = inst.op1Val & inst.op2Val;
            }

            //or
            else if (inst.funct3 == FUNCT3_OR && inst.funct7 == FUNCT7_ADDSHIFT){
                inst.arithResult = inst.op1Val | inst.op2Val;
            }

            //xor
            else if (inst.funct3 == FUNCT3_XOR && inst.funct7 == FUNCT7_ADDSHIFT) {
                inst.arithResult = inst.op1Val ^ inst.op2Val;
            }

            //add
            else if (inst.funct7 == FUNCT7_ADDSHIFT && inst.funct3 == FUNCT3_ARITH){
                inst.arithResult = inst.op1Val + inst.op2Val;
            }

            //sub
            else if (inst.funct7 == FUNCT7_SUBSHIFT && inst.funct3 == FUNCT3_ARITH){
                inst.arithResult = inst.op1Val - inst.op2Val;
            }

            //srl (shift right, fill leftmost bits with 0) (not tested)
            else if (inst.funct7 == FUNCT7_ADDSHIFT && inst.funct3==FUNCT3_RSHIFT){
                uint64_t shamt = inst.op2Val & 0b111111;  //shift amount lower 6 bits 
                uint64_t val = inst.op1Val;            // unsigned
                inst.arithResult = val >> shamt;     
            }

            //sra (shift right, fill left bits with sign bit)
            else if (inst.funct7 == FUNCT7_SUBSHIFT && inst.funct3 == FUNCT3_RSHIFT){
                uint64_t shamt = inst.op2Val & 0b111111; // shift amount lower 6 bits
                int64_t val = (int64_t)inst.op1Val;              
                inst.arithResult = val >> shamt; 
            }           
            
            //sll (shift left, fill with 0) (not tested)
            else if (inst.funct7 == FUNCT7_ADDSHIFT && inst.funct3==FUNCT3_LSHIFT){
                uint64_t shamt = inst.op2Val & 0b111111; //shift amount lower 6 bits
                uint64_t val = inst.op1Val;            // unsigned
                inst.arithResult = val << shamt;    
            }

            //slt (set less than, rd=1 if r1 <r2)
            else if (inst.funct7 == FUNCT7_ADDSHIFT && inst.funct3 == FUNCT3_LOAD){
                if ((int64_t)inst.op1Val < (int64_t)inst.op2Val) {
                    inst.arithResult = 1; 
                } else {
                    inst.arithResult = 0;  
                }
            }

            //sltu (set less than unsigned)
            else if (inst.funct7 == FUNCT7_ADDSHIFT && inst.funct3 == FUNCT3_DOUBLELOADSTORE){
                if ((uint64_t)inst.op1Val < (uint64_t)inst.op2Val) {
                    inst.arithResult = 1; 
                } else {
                    inst.arithResult = 0;  
                }
            }

            break;
        }

        case OP_R_32BIT:{
            
            //addw (what if we have overflow....)
            if (inst.funct7== FUNCT7_ADDSHIFT && inst.funct3==FUNCT3_ARITH){
                uint32_t a32= (uint32_t)(inst.op1Val);
                uint32_t b32 = (uint32_t)(inst.op2Val);
                int32_t sum32 = (int32_t)(a32 + b32); // truncate to 32 bits
                inst.arithResult = (int64_t)sum32; //sign extend to 64 bits
            }

            //subw
            else if (inst.funct7 == FUNCT7_SUBSHIFT && inst.funct3 == FUNCT3_ARITH) {
                uint32_t a32= (uint32_t)(inst.op1Val);
                uint32_t b32 = (uint32_t)(inst.op2Val);
                int32_t sum32 = (int32_t)(a32 - b32); // truncate to 32 bits
                inst.arithResult = (int64_t)sum32; //sign extend to 64 bits
            }

            //srlw (shift right fill with 0 for 32 bit) (not tested)
            else if (inst.funct7 == FUNCT7_ADDSHIFT && inst.funct3 == FUNCT3_RSHIFT) {
                uint64_t shamt = inst.op2Val & 0b11111;                     // shift amount lower 5 bits
                uint32_t val32  = (uint32_t)(inst.op1Val); // truncate to 32 bits (unsigned)
                uint32_t result32 = val32 >> shamt;                      
                inst.arithResult = (int64_t)(int32_t)result32; 
            }

            //sraw (shift right fill with sign bit, 32 bit) (may need to test)
            else if (inst.funct7 == FUNCT7_SUBSHIFT && inst.funct3 == FUNCT3_RSHIFT) {
                uint64_t shamt = inst.op2Val & 0b11111;                  // shift amount lower 5 bits
                int32_t val32 = (int32_t)(inst.op1Val);  //truncate to 32 bits
                int32_t result32 = val32 >> shamt;                 
                inst.arithResult = (int64_t)result32; //sign extend to 64 bits
            }

            //sllw (shift left fill with 0 , 32bit) (write test case)
            else if (inst.funct7==FUNCT7_ADDSHIFT && inst.funct3 ==FUNCT3_LSHIFT){
                uint64_t shamt = inst.op2Val & 0b11111;                     // shift amountlower 5 bits
                uint32_t val32  = (uint32_t)(inst.op1Val); // truncate to 32 bits
                uint32_t result32 = val32 << shamt;                      
                inst.arithResult = (int64_t)(int32_t)result32; 
            }
            
        }
    }
    return inst;
    
}


// Generate memory address for load/store instructions
Instruction simAddrGen(Instruction inst) {

    //address = rs1 + imm
    if (inst.opcode == OP_LOAD) {
        int32_t imm12 = (inst.instruction >> 20) & 0b111111111111;
        int32_t sext_imm12 = (imm12 & 0x800) ? (imm12 | 0xFFFFF000) : imm12;
        inst.memAddress = inst.op1Val + sext_imm12;
    }
    else if (inst.opcode == OP_S_STORE) {
        int32_t bits11_5 = (inst.instruction >> 25) & 0b1111111;
        int32_t bits4_0 = (inst.instruction >> 7) & 0b11111;

        int32_t imm12 = (bits11_5 << 5) | (bits4_0);
        int32_t sext_imm12 = (imm12 & 0x800) ? (imm12 | 0xFFFFF000) : imm12;

        inst.memAddress = inst.op1Val + sext_imm12;
    }
    return inst;
}


// Perform memory access for load/store instructions
Instruction simMemAccess(Instruction inst, MemoryStore *myMem) {

    if (inst.opcode == OP_LOAD) {

        //lw: 32 bit
        if (inst.funct3== FUNCT3_LOAD){
            uint64_t val;
            myMem->getMemValue(inst.memAddress, val, WORD_SIZE);
            // sign extend the 32 bit value to 64
            // check if 32 bit is 1
            uint64_t sext_val = (val & 0x80000000) ? (val |  0xFFFFFFFF00000000) : val;
            inst.arithResult = sext_val; 
        }

        //lwu: 32 bit fill with 0
        else if (inst.funct3== FUNCT3_OR){
            uint64_t val;
            myMem->getMemValue(inst.memAddress, val, WORD_SIZE);
            // 0 extend the 32 bit value to 64
            uint64_t ext_val = (val & 0x00000000FFFFFFFF);
            inst.arithResult = ext_val; 
        }        
        
        //ld: 64 bit
        else if (inst.funct3==FUNCT3_DOUBLELOADSTORE){
            uint64_t val;
            myMem->getMemValue(inst.memAddress, val, DOUBLE_SIZE);
            inst.arithResult = val; 
        }

        //lh: 16 bit
        else if (inst.funct3== FUNCT3_LSHIFT){
            uint64_t val=0;
            myMem->getMemValue(inst.memAddress, val, HALF_SIZE);
            // sign extend the 16 bit value to 64
            // check if 16 bit is 1
            uint64_t sext_val = (val & 0x8000) ? (val |  0xFFFFFFFFFFFF0000) : val;
            inst.arithResult = sext_val; 
        }

        //lhu: 16 bit fill with 0
        else if (inst.funct3== FUNCT3_RSHIFT){
            uint64_t val=0;
            myMem->getMemValue(inst.memAddress, val, HALF_SIZE);
            uint64_t ext_val = (val & 0x000000000000FFFF);
            inst.arithResult = ext_val;
        }

        //lb: 8bit
        else if (inst.funct3== FUNCT3_ARITH){
            uint64_t val;
            myMem->getMemValue(inst.memAddress, val, BYTE_SIZE);
            uint64_t sext_val = (val & 0x80) ? (val |  0xFFFFFFFFFFFFFF00) : val;
            inst.arithResult = sext_val; 
        }

        //lbu: 8 bit fill with 0
        else if (inst.funct3== FUNCT3_XOR){
            uint64_t val;
            myMem->getMemValue(inst.memAddress, val, BYTE_SIZE);
            uint64_t ext_val = (val & 0x00000000000000FF);
            inst.arithResult = ext_val;
        }
  
    }
    else if (inst.opcode == OP_S_STORE) {
        //sw
        if (inst.funct3 == FUNCT3_LOAD) {
            // printf("op2val = %" PRIu64 "\n", inst.op2Val);
            myMem->setMemValue(inst.memAddress, inst.op2Val, WORD_SIZE);
            myMem->printMemory(inst.memAddress, inst.memAddress + WORD_SIZE);
        }
        //sd
        if (inst.funct3 == FUNCT3_DOUBLELOADSTORE) {
            myMem->setMemValue(inst.memAddress, inst.op2Val, DOUBLE_SIZE);
        }
        //sb
        else if(inst.funct3 == FUNCT3_ARITH) {
            myMem->setMemValue(inst.memAddress, inst.op2Val, BYTE_SIZE);
        }
        //sh
        else if(inst.funct3 == FUNCT3_LSHIFT) {
            myMem->setMemValue(inst.memAddress, inst.op2Val, HALF_SIZE);
        }
    }
    return inst;
}

// Write back results to registers
Instruction simCommit(Instruction inst, REGS &regData) {

    // regData here is passed by reference, so changes will be reflected in original
    if (inst.opcode == OP_LUI){
        //load upper immedaite
        uint64_t imm20  = inst.instruction >> 12 & 0b11111111111111111111; //upper 20 bits
        inst.arithResult = imm20 << 12;
    }
                
    if (inst.writesRd && inst.rd != 0) {
        regData.registers[inst.rd] = inst.arithResult;
    }

    return inst;
}

// Simulate the whole instruction using functions above
Instruction simInstruction(uint64_t &PC, MemoryStore *myMem, REGS &regData) {
    Instruction inst = simFetch(PC, myMem);
    inst = simDecode(inst);
    if (!inst.isLegal || inst.isHalt) return inst;
    inst = simOperandCollection(inst, regData);
    inst = simNextPCResolution(inst);
    inst = simArithLogic(inst);
    inst = simAddrGen(inst);
    inst = simMemAccess(inst, myMem);
    inst = simCommit(inst, regData);
    PC = inst.nextPC;
    return inst;
}

int main(int argc, char** argv) {

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <instruction_file>\n", argv[0]);
        return -1;
    }

    // initialize memory store with buffer contents
    MemoryStore *myMem = createMemoryStore();
    if (!initMemory(argv[1], myMem)) {
        fprintf(stderr, "Failed to initialize memory with program binary.\n");
        return -1;
    }

    // initialize registers and program counter
    regData.reg = {};
    PC = 0;
    bool err = false;
    
    // start simulation
    while (!err) {
        Instruction inst = simInstruction(PC, myMem, regData);
        if (inst.isHalt) {
            // Normal dump and exit
            dump(myMem);
            return 0;
        }
        if (!inst.isLegal) {
            printf("opcode = %" PRIu64 "\n", inst.opcode);
            printInstructionBinary(inst);
            fprintf(stderr, "Illegal instruction encountered at PC: 0x%lx\n", inst.PC);
            err = true;
        }
    }

    // dump and exit with error
    dump(myMem);
    exit(127);
    return -1;
}
