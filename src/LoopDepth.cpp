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


#include "LoopDepth.h"

int nested_loop_depth_for_loop(Loop* L)
{
	int counter_depth=0;
	std::vector<Loop*> subLoops=L->getSubLoops();
	int subLoop_num = subLoops.size();
	if(subLoops.empty()){
		counter_depth += 1;
	}
	else{
		int counter=0;
		for(int j=0;j<subLoop_num;j++)
		{
			int counter_tmp=0;
			Loop* sub_loop=subLoops.at(j);
			counter_tmp = nested_loop_depth_for_loop(sub_loop);
			counter=std::max(counter,counter_tmp);
		}
		counter_depth = counter + 1;
	}
	return counter_depth;
}
