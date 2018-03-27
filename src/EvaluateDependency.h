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


#ifndef EVALUATEDEPENDENCY_H_
#define EVALUATEDEPENDENCY_H_

#include "test.h"

bool load_store_dependency(Instruction *inst_l, Instruction *inst_s);
bool store_store_dependency(Instruction *inst_s1, Instruction *inst_s2);
bool test_loop_carried_dependence(LoopInfo* LI, Loop *L);


#endif
