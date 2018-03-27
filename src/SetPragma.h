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


#ifndef SETPRAGMA_H_
#define SETPRAGMA_H_

#include "test.h"

void set_dataflow(Module &M);
bool function_is_inline(Function *F);
bool function_is_noinline(Function *F);
bool has_subFn(Function *F);
bool Fn_pipeline_modify(Function *F);
void set_function_pipeline(Module &M);
bool set_subFn_pipeline(Loop *L);
void set_loop_counter_ul_pipeline(LoopInfo* LI, Function *F);
void set_array_partition(Module &M);
void keep_array_consistent(Function *callee, CallInst *call);

#endif
