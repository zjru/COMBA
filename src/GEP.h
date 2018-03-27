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


#ifndef GEP_H_
#define GEP_H_

#include "test.h"

GetElementPtrInst *get_GEP(Instruction *inst);
int compute_gep_operand(Value *op, bool second_iteration_flag, int compute_second);
int get_phi_second(Loop *L, PHINode *PHI);
PHINode *get_phi(Instruction *inst, PHINode *PHI);
int get_gep_phi(GetElementPtrInst *gep, PHINode *PHI);


#endif
