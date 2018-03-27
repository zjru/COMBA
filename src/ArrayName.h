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

#ifndef ARRAYNAME_H_
#define ARRAYNAME_H_

#include "test.h"

Value *check_PHI_operand(PHINode *phi);
Value *get_array_name(Instruction *inst);

#endif
