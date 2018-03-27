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


#ifndef FINDLATENCY_H_
#define FINDLATENCY_H_

#include "test.h"


float find_latency_before_inst(Instruction *inst, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop);
float find_latency_after_inst(Instruction *inst, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop);
float find_latency(Instruction *inst_begin, Instruction *inst_end, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop);
float get_inst_latency(Instruction *inst, std::map<Instruction*, unsigned> &instr_index_loop,std::vector<std::pair<int,float> > *dependence_loop);
void set_inst_latency(Instruction *inst,float latency,std::map<Instruction*, unsigned> &instr_index_loop,std::vector<std::pair<int,float> > *dependence_loop);


#endif
