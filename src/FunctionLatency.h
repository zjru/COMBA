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


#ifndef FUNCTIONLATENCY_H_
#define FUNCTIONLATENCY_H_

#include "test.h"

int compute_cycles_in_Fn(Function *F,LoopInfo* LI,std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop);
void reorder_load(Function *F, std::map<Instruction*,unsigned> &load_order_buff);
void store_complete_subloop(Loop *L, std::map<Loop*, unsigned> &Complete_Loop_Index);
void store_complete_loop(LoopInfo* LI, std::map<Loop*, unsigned> &Complete_Loop_Index);
Loop *get_top_loop(Loop *L);
bool all_is_small(BasicBlock *bb, Instruction *inst);

#endif
