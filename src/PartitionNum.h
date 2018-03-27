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


#ifndef PARTITIONNUM_H_
#define PARTITIONNUM_H_

#include "test.h"

int get_bank_number(Instruction *inst);
int compute_array_element_offset(Instruction *inst, BasicBlock *bb, int base_variable);
int get_unroll_SameBank_num(BasicBlock *bb, Instruction *inst, Instruction *inst1, std::vector<int> &array_element_counted);

#endif
