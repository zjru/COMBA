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


#ifndef CAST_H_
#define CAST_H_

#include "test.h"

bool inst_is_cast(Instruction *inst);
Instruction *get_cast_inst(Instruction *inst);

#endif
