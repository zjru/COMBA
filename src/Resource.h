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


#ifndef RESOURCE_H_
#define RESOURCE_H_

#include "test.h"

void inst_map_resource(Instruction *inst, bool &shareable_unit,std::map<Instruction*,std::string> &inst_unit);
void get_unit_resource(Instruction *inst, std::map<Instruction*,std::string> &inst_unit, std::map<std::string, unsigned> &resource_usage,std::vector<std::pair<std::string,unsigned> > *function_resource);
void compute_resource(Instruction *inst,std::map<std::string, unsigned> &loop_resource, std::vector<std::pair<std::string,unsigned> > *function_resource, std::map<Instruction*, std::string> &inst_unit, std::map<Instruction*,std::string> &scheduled_inst_unit, std::map<Instruction*, unsigned> &instr_index, std::vector<std::pair<int,float> > *dependence);
bool loop_pipeline_is_set(Loop *L);
Loop *get_pipelined_loop(Loop *L);
void count_related_loops(Loop *L, std::map<Loop*,unsigned> &Loop_counted);
bool all_loop_unrolled(Loop *L);
void compute_loop_resource(LoopInfo* LI, Loop *L,std::vector<std::pair<std::string,unsigned> > *function_resource, std::map<std::string, unsigned> &loop_resource, std::map<Instruction*, unsigned> &load_buff,std::map<Loop*,unsigned> &Loop_counted);
void resource_accumulation(LoopInfo* LI,Function *F,std::vector<std::pair<std::string,unsigned> > *function_resource, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop);
void bram_accumulation(LoopInfo* LI,Function *F,std::vector<std::pair<std::string,unsigned> > *function_resource);


#endif
