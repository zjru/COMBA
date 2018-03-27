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


#ifndef SETPARAMETER_H_
#define SETPARAMETER_H_

#include "test.h"


void set_loop_map(Loop *L,Loop *parent,Loop *loop,std::map<Loop*,int> loop_index_tmp,std::vector<int> *loop_index);
void set_loop_index(Function *F, LoopInfo* LI, Loop *L,std::map<Loop*,int> &loop_index_tmp,std::vector<int> *loop_index);
void set_array_index(Module &M);
void set_array_map(Module &M);
void store_function_IO(Module &M);


#endif
