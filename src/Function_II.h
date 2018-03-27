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


#ifndef FUNCTION_II_H_
#define FUNCTION_II_H_

#include "test.h"

int compute_Fn_II(Function *F, LoopInfo* LI,std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop,std::vector<std::pair<unsigned,int> > *function_arg_II);
int find_Fn_ResMII(Function *F, LoopInfo* LI, std::vector<std::pair<unsigned,int> > *function_arg_II);
bool is_call_argument(CallInst *call, Value *arg);
unsigned get_arg_number(CallInst *call, Value *arg);
int find_Fns_ResMII(Function *F, LoopInfo* LI, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop,std::vector<std::pair<unsigned,int> > *function_arg_II);



#endif
