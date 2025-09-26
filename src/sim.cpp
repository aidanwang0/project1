#include "sim.h"

using namespace std;

// RV64I without csr, environment, or fence instructions

//           31          25 24 20 19 15 14    12 11          7 6      0
// R  type: | funct7       | rs2 | rs1 | funct3 | rd          | opcode |
// I  type: | imm[11:0]          | rs1 | funct3 | rd          | opcode |
// S  type: | imm[11:5]    | rs2 | rs1 | funct3 | imm[4:0]    | opcode |
// SB type: | imm[12|10:5] | rs2 | rs1 | funct3 | imm[4:1|11] | opcode |
// U  type: | imm[31:12]                        | rd          | opcode |
// UJ type: | imm[20|10:1|11|19:12]             | rd          | opcode |

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
                inst.funct3 == FUNCT3_XOR || inst.funct3== FUNCT3_RSHIFT || inst.funct3 == FUNCT3_LSHIFT)  {
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
            if (inst.funt3== FUNCT3_ARITH){
                inst.doesArithLogic = true;
                inst.writesRd = false;
                inst.writesRs1 = false;
                inst.writeRs2 = false;
            }
            else {
                inst.isLegal = false;
            }
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

        case OP_R_64BIT:
            if (inst.funct3 == FUNCT3_ARITH || inst.funct3 == FUNCT3_AND || inst.funct3 == FUNCT3_OR ||
                 inst.funct3 == FUNCT3_XOR  || inst.funct3 == FUNCT3_RSHIFT || inst.funct3 == FUNCT3_LSHIFT) {
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

int32_t 

// Resolve next PC whether +4 or branch/jump target
Instruction simNextPCResolution(Instruction inst) {
    inst.opcode = inst.instruction & 0b1111111
    switch(inst.opcode){
        case OP_SB_BRANCH: {
            uint64_t 
            if(inst.op1Val == inst.op2Val) {
                
                inst.nextPC = inst.PC + 
            }
        }
        break;

        inst.nextPC = inst.PC + 4; // default
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
            else if (inst.funct3 == FUNCT3_RSHIFT){
                uint64_t shamt = (inst.instruction >> 20) & 0b111111; //shift amount lower 6bits
                int64_t val = (int64_t)inst.op1Val;              
                inst.arithResult = val >> shamt; 
            }

            //slli (shift left, fill with 0) (not tested)
            else if (inst.funct3 == FUNCT3_LSHIFT){
                uint64_t shamt = (inst.instruction >> 20) & 0b111111; //shift amount lower 6bits
                uint64_t val = (uint64_t)inst.op1Val;            // unsigned
                uint64_t result64 = val << shamt;
                inst.arithResult = (int64_t) result64;
            }

            break;
        }   

        //auipic
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

            //slliw (shift left immediate, fill with 0, 32 bit) (not tested) (write test case)
            else if (inst.funct3 == FUNCT3_LSHIFT && inst.funct7==FUNCT7_ADD){
                uint64_t shamt = (inst.instruction >> 20) & 0b11111;    // shift amount lower 5 bits
                uint32_t val32 = (uint32_t)(inst.op1Val);  //truncate to 32 bits
                uint32_t result32 = val32 << shamt;                 
                inst.arithResult = (int64_t)(int32_t)result32;   //sign extend to 64 bits
            }
            
            break;
        }

        case OP_R_64BIT: {

            //and
            if (inst.funct3 == FUNCT3_AND && inst.funct7 == FUNCT7_ADD){
                inst.arithResult = inst.op1Val & inst.op2Val;
            }

            //or
            else if (inst.funct3 == FUNCT3_OR && inst.funct7 == FUNCT7_ADD){
                inst.arithResult = inst.op1Val | inst.op2Val;
            }

            //xor
            else if (inst.funct3 == FUNCT3_XOR && inst.funct7 == FUNCT7_ADD) {
                inst.arithResult = inst.op1Val ^ inst.op2Val;
            }

            //add
            else if (inst.funct7 == FUNCT7_ADD && inst.funct3 == FUNCT3_ARITH){
                inst.arithResult = inst.op1Val + inst.op2Val;
            }

            //sub
            else if (inst.funct7 == FUNCT7_SUBSHIFT && inst.funct3 == FUNCT3_ARITH){
                inst.arithResult = inst.op1Val - inst.op2Val;
            }

            //srl (shift right, fill leftmost bits with 0) (not tested)
            else if (inst.funct7 == FUNCT7_ADD && inst.funct3==FUNCT3_RSHIFT){
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
            else if (inst.funct7 == FUNCT7_ADD && inst.funct3==FUNCT3_LSHIFT){
                uint64_t shamt = inst.op2Val & 0b111111; //shift amount lower 6 bits
                uint64_t val = inst.op1Val;            // unsigned
                inst.arithResult = val << shamt;    
            }

            break;
        }

        case OP_R_32BIT:{
            
            //addw (what if we have overflow....)
            if (inst.funct7== FUNCT7_ADD && inst.funct3==FUNCT3_ARITH){
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
            else if (inst.funct7 == FUNCT7_ADD && inst.funct3 == FUNCT3_RSHIFT) {
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
            else if (inst.funct7==FUNCT7_ADD && inst.funct3 ==FUNCT3_LSHIFT){
                uint64_t shamt = inst.op2Val & 0b11111;                     // shift amountlower 5 bits
                uint32_t val32  = (uint32_t)(inst.op1Val); // truncate to 32 bits
                uint32_t result32 = val32 << shamt;                      
                inst.arithResult = (int64_t)(int32_t)result32; 
            }

            //slliw (shift left fill with 0, 32 bit) (not tested)
            else if (inst.funct7 ==)
            
        }
    }
    return inst;
    
}

// Generate memory address for load/store instructions
Instruction simAddrGen(Instruction inst) {
    return inst;
}

// Perform memory access for load/store instructions
Instruction simMemAccess(Instruction inst, MemoryStore *myMem) {
    return inst;
}

// Write back results to registers
Instruction simCommit(Instruction inst, REGS &regData) {

    // regData here is passed by reference, so changes will be reflected in original
    regData.registers[inst.rd] = inst.arithResult;

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
            fprintf(stderr, "Illegal instruction encountered at PC: 0x%lx\n", inst.PC);
            err = true;
        }
    }

    // dump and exit with error
    dump(myMem);
    exit(127);
    return -1;
}



