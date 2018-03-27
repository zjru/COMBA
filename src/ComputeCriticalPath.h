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


#ifndef COMPUTECRITICALPATH_H_
#define COMPUTECRITICALPATH_H_

#include "test.h"

void loop_compute_critical_path_gep(Instruction *inst, float latency, unsigned &index_loop, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop, std::map<Instruction*,std::queue<unsigned> > &inst_map_branch);
void loop_compute_critical_path(Instruction * inst, float latency, unsigned &index_loop, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop, std::map<Instruction*,std::queue<unsigned> > &inst_map_branch);
void loop_compute_critical_path_call(Instruction *inst, unsigned &index_loop, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop, std::map<Instruction*,std::queue<unsigned> > &inst_map_branch);
void loop_compute_critical_path_branch(LoopInfo* LI, Instruction *inst, unsigned &index_loop, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop, std::map<Instruction*,std::queue<unsigned> > &inst_map_branch);
void loop_compute_critical_path_switch(LoopInfo* LI, Instruction *inst, unsigned &index_loop, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop, std::map<Instruction*,std::queue<unsigned> > &inst_map_branch);
void update(LoopInfo* LI, Instruction *inst, unsigned &index_loop, std::map<Instruction*, unsigned> &instr_index_loop, std::map<Instruction*, unsigned> &load_order_buff, std::vector<std::pair<int,float> > *dependence_loop, std::map<Instruction*,std::queue<unsigned> > &inst_map_branch);
float loop_solveCP(std::vector<std::pair<int,float> > *dependence, std::map<unsigned, float> &loop_inst_till_latency);
void dependence_set(LoopInfo* LI, Loop *L, std::map<Instruction*, unsigned> &instr_index_loop, std::map<Instruction*, unsigned> &load_order_buff, std::vector<std::pair<int,float> > *dependence_loop, std::map<Instruction*,std::queue<unsigned> > &inst_map_branch);
float loopCP(LoopInfo* LI, Loop* L, std::map<Instruction*, unsigned> &load_order_buff);
float dependent_loopCP(LoopInfo* LI, Loop *L, std::map<Instruction*, unsigned> &load_order_buff);
float update_loopCP(LoopInfo* LI, Loop *L, std::map<Instruction*, unsigned> &load_order_buff);


#endif
