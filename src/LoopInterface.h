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


#ifndef LOOPINTERFACE_H_
#define LOOPINTERFACE_H_

#include "test.h"

void get_loop_input(Loop *L, std::vector<Value*> &loop_read);
void get_loop_output(Loop *L, std::vector<Value*> &loop_write);


#endif
