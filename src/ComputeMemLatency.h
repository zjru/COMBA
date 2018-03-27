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


#ifndef COMPUTEMEMLATENCY_H_
#define COMPUTEMEMLATENCY_H_

#include "test.h"

int partition_load_num(LoopInfo *LI, BasicBlock *bb, Instruction *inst, std::map<Instruction*, unsigned> &instr_index_loop, std::map<Instruction*, unsigned> &load_order_buff);
int partition_store_num(LoopInfo* LI, BasicBlock *bb, Instruction *inst, unsigned &index_loop, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop);
int compute_asap_load_num(BasicBlock *bb, Instruction *inst, std::map<Instruction*, unsigned> &instr_index_loop, std::map<Instruction*, unsigned> &load_order_buff);
int compute_asap_store_num(LoopInfo* LI, BasicBlock *bb, Instruction *inst, unsigned &index_loop, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop);
float mux_latency(LoopInfo* LI, BasicBlock *bb, Instruction *inst);
float get_load_latency(LoopInfo* LI, Instruction *inst, std::map<Instruction*, unsigned> &instr_index_loop, std::map<Instruction*, unsigned> &load_order_buff);
bool load_before_store_partitioned(LoopInfo* LI, Instruction *inst, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop);
float get_store_latency(LoopInfo* LI, Instruction *inst, unsigned &index_loop, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop);
PHINode *get_previous_phi(Instruction *inst);
bool loop_unroll_effective(Loop *L);
bool loop_carry_load_store_dependency(LoopInfo *LI, Instruction *load, Instruction *store);
void reset_sameArray_store_latency(LoopInfo *LI, BasicBlock *bb, Instruction *inst, int unroll_factor, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop);
void update_after_store(LoopInfo* LI, Instruction *inst, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop);
void update_after_load(Instruction *inst, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop);

#endif
