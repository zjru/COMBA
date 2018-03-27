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


#ifndef COMPUTEFIXEDPATH_H_
#define COMPUTEFIXEDPATH_H_

#include "test.h"

void loop_computeCP_gep_fixed_latency(Loop *L, Instruction *inst, float latency, unsigned &index_loop, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop);
void loop_computeCP_fixed_latency(Loop *L, Instruction * inst, float latency, unsigned &index_loop, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop);
void loop_computeCP_call_fixed_latency(Loop *L, Instruction *inst, float latency, unsigned &index_loop, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop);
void loop_computeCP_phi_fixed_latency(Loop *L, Instruction *inst, float latency, unsigned &index_loop, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop);
void update_fix_latency(Loop *L, Instruction *inst, float latency, unsigned &index_loop, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop);


#endif
