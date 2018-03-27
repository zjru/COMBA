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


#include "LoopInterface.h"
#include "ArrayName.h"


void get_loop_input(Loop *L, std::vector<Value*> &loop_read)
{
	for(Loop::block_iterator BI=L->block_begin(), BE=L->block_end(); BI !=BE; ++BI)
	{
		BasicBlock *bb=*BI;
		for(auto iti=bb->begin();iti!=bb->end();++iti)
		{
			if(isa<LoadInst>(iti)){
				Value *array=get_array_name(iti);
				std::vector<Value*>::iterator i;
				if(loop_read.empty()){
					loop_read.push_back(array);
				}
				else{
					for(i=loop_read.begin();i!=loop_read.end();++i)
					{
						Value *array1=*i;
						if(array1==array){
							break;
						}
					}

					if(i==loop_read.end()){
						i--;
						Value *array2=*i;
						if(array != array2){
							loop_read.push_back(array);
						}
					}
				}
			}
		}
	}
}

void get_loop_output(Loop *L, std::vector<Value*> &loop_write)
{
	for(Loop::block_iterator BI=L->block_begin(), BE=L->block_end(); BI !=BE; ++BI)
	{
		BasicBlock *bb=*BI;
		for(auto iti=bb->begin();iti!=bb->end();++iti)
		{
			if(isa<StoreInst>(iti)){
				Value *array=get_array_name(iti);
				std::vector<Value*>::iterator i;
				if(loop_write.empty()){
					loop_write.push_back(array);
				}
				else{
					for(i=loop_write.begin();i!=loop_write.end();++i)
					{
						Value *array1=*i;
						if(array1==array){
							break;
						}
					}
					if(i==loop_write.end()){
						i--;
						Value *array2=*i;
						if(array != array2){
							loop_write.push_back(array);
						}
					}
				}
			}
		}
	}
}
