/*
 *  COMBA
 *  Copyright (C) 2017  RCSL, HKUST
 *  
 *  ONLY FOR ACADEMIC USE, NOT FOR COMMERCIAL USE.
 *  
 *  Please use our tool at academic institutions and non-profit 
 *  research organizations for research use. 
 *  
 *  
 */


#ifndef MEMOPNUM_H_
#define MEMOPNUM_H_

#include "test.h"

int compute_BB_load_num(BasicBlock *itb, Instruction *inst);
int compute_BB_store_num(BasicBlock *itb, Instruction *inst);
int partition_mem_op_num(LoopInfo *LI, BasicBlock *bb, Instruction *inst);
int compute_total_LS_num(LoopInfo* LI, Instruction *inst);


#endif
