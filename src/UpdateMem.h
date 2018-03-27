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


#ifndef UPDATEMEM_H_
#define UPDATEMEM_H_

#include "test.h"

void complete_load_update(Loop *L1, Loop *L2, float latency,float latency_delta, unsigned index1,unsigned index2, Instruction* inst,std::vector<std::pair<int,float> > *dependence_loop);
void update_load(LoopInfo* LI, Instruction *inst, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop);
void replace_write_latency(int store_num1, int store_num2, unsigned index, Instruction *inst,std::vector<std::pair<int,float> > *dependence_loop);
void complete_store_update(Loop *L1, Loop *L2, int store_num1, int store_num2, unsigned index, Instruction *inst, std::vector<std::pair<int,float> > *dependence_loop);
void update_store(LoopInfo* LI, Instruction *inst, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop);


#endif
