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


#ifndef LOOPUNROLLFACTOR_H_
#define LOOPUNROLLFACTOR_H_

#include "test.h"

int total_unroll_factor(Loop *L);
bool unroll_loop_relation(Loop *L1, Loop *L2);
int total_unroll_factor_ls(LoopInfo *LI, Loop *L, Instruction *inst);
int total_unroll_factor_op(LoopInfo *LI, Loop *L, Instruction *inst);

#endif
