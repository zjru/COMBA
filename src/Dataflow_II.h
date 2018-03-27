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


#ifndef DATAFLOW_II_H_
#define DATAFLOW_II_H_

#include "test.h"

bool check_dataflow(Function *F, LoopInfo* LI);
int compute_dataflow_II(Function *F, LoopInfo* LI);


#endif
