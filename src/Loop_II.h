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


#ifndef LOOP_II_H_
#define LOOP_II_H_

#include "test.h"

int compute_II(LoopInfo* LI, Loop* L, std::map<Instruction*, unsigned> &instr_index_loop, std::map<Instruction*, unsigned> &load_order_buff, std::vector<std::pair<int,float> > *dependence_loop);
int find_array_offset(LoopInfo* LI, Instruction *inst);
int find_distance(LoopInfo* LI, Instruction *inst_l, Instruction *inst_s);
int find_max_rec_II(LoopInfo* LI, Loop *L, Instruction *inst, PHINode *phi, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop);
int find_ResMII(LoopInfo* LI, Loop *L, std::map<Instruction*, unsigned> &load_order_buff);


#endif
