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


#ifndef RESCHEDULE_H_
#define RESCHEDULE_H_

#include "test.h"

void reschedule_dependence(LoopInfo* LI, int freq, std::map<Instruction*, unsigned> &instr_index_loop,std::vector<std::pair<int,float> > *dependence_loop);
void reschedule_dependence_100(Instruction *inst, std::map<Instruction*,unsigned>::iterator it, std::vector<std::pair<int,float> >::iterator i, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop);
void reschedule_dependence_125(Instruction *inst, std::map<Instruction*,unsigned>::iterator it, std::vector<std::pair<int,float> >::iterator i, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop);
void reschedule_dependence_150(Instruction *inst, std::map<Instruction*,unsigned>::iterator it, std::vector<std::pair<int,float> >::iterator i, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop);
void reschedule_dependence_200(Instruction *inst, std::map<Instruction*,unsigned>::iterator it, std::vector<std::pair<int,float> >::iterator i, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop);
void reschedule_dependence_250(Instruction *inst, std::map<Instruction*,unsigned>::iterator it, std::vector<std::pair<int,float> >::iterator i, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop);
bool num_is_int(float number);
bool latency_is_large(Instruction *inst,std::map<Instruction*, unsigned> &instr_index_loop,std::vector<std::pair<int,float> > *dependence_loop);
bool latency_is_large125(Instruction *inst,std::map<Instruction*, unsigned> &instr_index_loop,std::vector<std::pair<int,float> > *dependence_loop);
void reschedule_load_freq125(bool large_latency_flag,Instruction *inst,Instruction *inst1,std::map<Instruction*, unsigned> &instr_index_loop,std::vector<std::pair<int,float> > *dependence_loop);
bool load_shift_store(Instruction*inst,std::map<Instruction*, unsigned> &instr_index_loop,std::vector<std::pair<int,float> > *dependence_loop);
bool before_inst_large(Instruction *inst,std::map<Instruction*, unsigned> &instr_index_loop,std::vector<std::pair<int,float> > *dependence_loop);
bool after_inst_large(Instruction *inst,std::map<Instruction*, unsigned> &instr_index_loop,std::vector<std::pair<int,float> > *dependence_loop);


#endif
