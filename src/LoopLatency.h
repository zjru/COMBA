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


#ifndef LOOPLATENCY_H_
#define LOOPLATENCY_H_

#include "test.h"

bool test_loop_is_perfect(Loop* L);
float pipeline_iteration_latency(LoopInfo* LI, Loop *L,int II, std::map<Instruction*, unsigned> &instr_index_loop, std::map<Instruction*, unsigned> &load_order_buff, std::vector<std::pair<int,float> > *dependence_loop);
float perfect_pipeline_iteration_latency(LoopInfo* LI, Loop *L, int II, std::map<Instruction*, unsigned> &instr_index_loop,std::map<Instruction*, unsigned> &load_order_buff, std::vector<std::pair<int,float> > *dependence_loop);
float compute_cycles_in_loop(LoopInfo* LI, Loop* L, std::map<Instruction*, unsigned> &load_order_buff);
void Loop_insert(LoopInfo* LI, Loop *L, unsigned &index_loop, std::map<Instruction*, unsigned> &instr_index_loop, std::map<Instruction*, unsigned> &load_order_buff, std::vector<std::pair<int,float> > *dependence_loop, std::map<Instruction*,std::queue<unsigned> > &inst_map_branch);


#endif
