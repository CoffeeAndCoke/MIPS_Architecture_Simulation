/****************************
 * Edgar Jaimes
 * CS 3339 Spring 2016
 ****************************/
#include "CPU.h"

const string CPU::regNames[] = {"$zero","$at","$v0","$v1","$a0","$a1","$a2","$a3",
                                "$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7",
                                "$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",
                                "$t8","$t9","$k0","$k1","$gp","$sp","$fp","$ra"};

CPU::CPU(uint32_t pc, Memory &iMem, Memory &dMem) : pc(pc), iMem(iMem), dMem(dMem) {
  for(int i = 0; i < NREGS; i++) {
    regFile[i] = 0;
  }
  hi = 0;
  lo = 0;
  regFile[28] = 0x10008000; // gp
  regFile[29] = 0x10000000 + dMem.getSize(); // sp

  instructions = 0;
  stop = false;
}

void CPU::run() {
  while(!stop) {
    instructions++;

    fetch();
    decode();
    execute();

    D(printRegFile());
  }
}

void CPU::fetch() {
  instr = iMem.loadWord(pc);
  pc = pc + 4;
}

/////////////////////////////////////////
// ALL YOUR CHANGES GO IN THIS FUNCTION 
/////////////////////////////////////////
void CPU::decode() {
  uint32_t opcode = instr >> 26;
  uint32_t rs = (instr >> 21) & 0x1f;
  uint32_t rt = (instr >> 16) & 0x1f;
  uint32_t rd = (instr >> 11) & 0x1f;
  uint32_t shamt = (instr >> 6) & 0x1f;
  uint32_t funct = instr & 0x3f;
  uint32_t uimm = instr & 0xffff;
  int32_t  simm = ((signed)uimm << 16) >> 16;
  uint32_t addr = instr & 0x3ffffff;

  opIsLoad = false;
  opIsStore = false;
  opIsMultDiv = false;
  writeDest = false; 

  D(cout << "  " << hex << setw(8) << pc - 4 << ": ");
  switch(opcode) {
      
    case 0x00:
      switch(funct) {
        
        case 0x00: D(cout << "sll " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt); //correct
                    aluOp = SHF_L; aluSrc1 = regFile[rs]; aluSrc2 = shamt; 
                    writeDest = true; destReg = rd;
                    break;
        case 0x03: D(cout << "sra " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt); //correct
                   aluOp = SHF_R; aluSrc1 = regFile[rs]; aluSrc2 = shamt;
                   writeDest = true; destReg = rd;
                   break;
        case 0x08: D(cout << "jr " << regNames[rs]); 
                   //aluOp = OUT_S1; 
                   pc = regFile[rs];
                   break;
        case 0x10: D(cout << "mfhi " << regNames[rd]); 
                   aluOp = ADD; aluSrc1 = hi; aluSrc2 = 0; 
                   writeDest = true; destReg = rd; 
                   break;
        case 0x12: D(cout << "mflo " << regNames[rd]); 
                   aluOp = ADD; aluSrc1 = lo; aluSrc2 = 0; 
                   writeDest = true; destReg = rd; 
                   break;
        case 0x18: D(cout << "mult " << regNames[rs] << ", " << regNames[rt]); 
                   opIsMultDiv = true; 
                   aluOp = MUL; aluSrc1 = regFile[rs]; aluSrc2 = regFile[rt]; 
                   //writeDest = true; destReg = REG_HILO; CORRECTION
                   break;
        case 0x1a: D(cout << "div " << regNames[rs] << ", " << regNames[rt]); // correct
                   opIsMultDiv = true;
                   aluOp = DIV; aluSrc1 = regFile[rs]; aluSrc2 = regFile[rt]; 
                   //writeDest = true; destReg = REG_HILO; CORRECTION
                   break;
        case 0x21: D(cout << "addu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]); //correct
                    aluOp = ADD; aluSrc1 = regFile[rs]; aluSrc2 = regFile[rt]; 
                    writeDest = true; destReg = rd; 
                   break;
        case 0x23: D(cout << "subu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]); //correct
                    aluOp = ADD;  aluSrc1 = regFile[rs]; aluSrc2 = ~regFile[rt] + 1;
                    writeDest = true; destReg = rd;
                    break;
        case 0x2a: D(cout << "slt " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]); //correct
                    aluOp = CMP_LT; aluSrc1 = regFile[rs]; aluSrc2 = regFile[rt];
                    writeDest = true; destReg = rd;
                    break;
        default: cerr << "unimplemented instruction: pc = 0x" << hex << pc - 4 << endl;
      }
      break;
    case 0x02: D(cout << "j " << hex << ((pc & 0xf0000000) | addr << 2)); // Re-Check
               //aluOp = OUT_S1; //pass src1 stright through ALU
               pc = ((pc & 0xf0000000) | addr << 2);
               break;
    case 0x03: D(cout << "jal " << hex << ((pc & 0xf0000000) | addr << 2)); // Correct
               aluOp = OUT_S1; // pass src1 straight through ALU
               writeDest = true; destReg = REG_RA; // writes PC+4 to $ra
               aluSrc1 = pc;
               pc = ((pc & 0xf0000000) | addr << 2);
               break;
    case 0x04: D(cout << "beq " << regNames[rs] << ", " << regNames[rt] << ", " << pc + (simm << 2));
               //aluOp = OUT_S1; 
               if(regFile[rs] == regFile[rt]) {                
                   pc = pc + (simm << 2);
               } 
                
               break;
    case 0x05: D(cout << "bne " << regNames[rs] << ", " << regNames[rt] << ", " << pc + (simm << 2));
               //aluOp = OUT_S1; 
               if(regFile[rs] != regFile[rt]){
                  pc = (pc + (simm << 2));
               }
               break;
    case 0x08: D(cout << "addi " << regNames[rt] << ", " << regNames[rs] << ", " << dec << simm); //correct
               aluOp = ADD; aluSrc1 = regFile[rs]; aluSrc2 = simm; 
               writeDest = true; destReg = rt; 
               break;
    case 0x09: D(cout << "addiu " << regNames[rt] << ", " << regNames[rs] << ", " << dec << simm); //correct
                aluOp = ADD; aluSrc1 = regFile[rs]; aluSrc2 = simm; 
                writeDest = true; destReg = rt; 
                break;
    case 0x0c: D(cout << "andi " << regNames[rt] << ", " << regNames[rs] << ", " << dec << uimm); //correct
               aluOp = AND; aluSrc1 = regFile[rs]; aluSrc2 = uimm;
               writeDest = true; destReg = rt;
               break;
    case 0x0f: D(cout << "lui " << regNames[rt] << ", " << dec << simm); 
               aluOp = SHF_L; aluSrc1 = simm; aluSrc2 = 16; 
               writeDest = true; destReg = rt; 
               break;
    case 0x1a: D(cout << "trap " << hex << addr);
               aluOp = OUT_S1; // don't need the ALU
               switch(addr & 0xf) {
                 case 0x0: cout << endl; break;
                 case 0x1: cout << " " << (signed)regFile[rs]; break;
                 case 0x5: cout << endl << "? "; cin >> regFile[rt]; break;
                 case 0xa: stop = true; break;
                 default: cerr << "unimplemented trap: pc = 0x" << hex << pc - 4 << endl;
                          stop = true;
               }
              break;
    case 0x23: D(cout << "lw " << regNames[rt] << ", " << dec << simm << "(" << regNames[rs] << ")");
                 opIsLoad = true; 
                 aluOp = ADD; aluSrc1 = regFile[rs]; aluSrc2 = simm; 
                 writeDest = true; destReg = rt; 
                 break;
    case 0x2b: D(cout << "sw " << regNames[rt] << ", " << dec << simm << "(" << regNames[rs] << ")");
                 opIsStore = true; 
                 storeData = regFile[rt]; 
                 aluOp = ADD; aluSrc1 = regFile[rs]; aluSrc2 = simm; 
                 break;
    default: cerr << "unimplemented instruction: pc = 0x" << hex << pc - 4 << endl;
  }
  D(cout << endl);
}

void CPU::execute() {
  uint32_t aluOut = alu.op(aluOp, aluSrc1, aluSrc2);

  if(opIsLoad)
    aluOut = dMem.loadWord(aluOut);
  else if(opIsStore)
    dMem.storeWord(storeData, aluOut);
  else if(opIsMultDiv) {
    hi = alu.getUpper();
    lo = alu.getLower();
  }

  // Regfile update (but never write to register 0)
  if(writeDest && destReg > 0)
    regFile[destReg] = aluOut;
}

void CPU::printRegFile() {
  cout << hex;
  for(int i = 0; i < NREGS; i++) {
    cout << "    " << regNames[i];
    if(i > 0) cout << "  ";
    cout << ": " << setfill('0') << setw(8) << regFile[i];
    if( i == (NREGS - 1) || (i + 1) % 4 == 0 )
      cout << endl;
  }
  cout << "    hi   : " << setfill('0') << setw(8) << hi;
  cout << "    lo   : " << setfill('0') << setw(8) << lo;
  cout << dec << endl;
}

void CPU::printFinalStats() {
  cout << "Program finished at pc = 0x" << hex << pc << "  ("
       << dec << instructions << " instructions executed)" << endl;
}
