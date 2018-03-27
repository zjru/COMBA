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


#include "Dataflow_II.h"
#include "LoopInterface.h"
#include "ArrayName.h"


bool check_dataflow(Function *F, LoopInfo* LI)
{
	std::map<Value*, int> Dataflow_read_num;
	std::map<Value*, int> Dataflow_write_num;
	//Dataflow_read_num.clear();
	//Dataflow_write_num.clear();
	bool bb_in_loop=false;
	//std::queue<Value*> rw_queue;
	int sub_element_num=0;
	for(auto itb=F->begin();itb!=F->end();++itb)
	{
		for(LoopInfo::iterator i=LI->begin(),e=LI->end();i!=e;++i)
		{
			Loop *L=*i;
			if(L->contains(itb)){
				bb_in_loop=true;
				break;
			}
		}
		if(bb_in_loop==false){
			for(auto iti=itb->begin();iti!=itb->end();++iti){
				if(CallInst *call=dyn_cast<CallInst>(iti)){
					Function *callee=call->getCalledFunction();
					unsigned id=callee->getIntrinsicID();
					if(id==0){
						unsigned f_index=Function_index[callee];
						for(std::vector<unsigned>::iterator i=Function_read_arg[f_index].begin(); i!=Function_read_arg[f_index].end(); ++i)
						{
							unsigned argno=*i;
							Value *call_read=call->getArgOperand(argno);
							if(Dataflow_read_num.count(call_read)){
								Dataflow_read_num[call_read]++;
							}
							else{
								Dataflow_read_num[call_read]=1;
							}
						}
						for(std::vector<unsigned>::iterator i=Function_write_arg[f_index].begin(); i!=Function_write_arg[f_index].end(); ++i)
						{
							unsigned argno=*i;
							Value *call_write=call->getArgOperand(argno);
							if(Dataflow_write_num.count(call_write)){
								Dataflow_write_num[call_write]++;
							}
							else{
								Dataflow_write_num[call_write]=1;
							}
						}
						sub_element_num++;
					}
				}
			}
		}
	}
	for(LoopInfo::iterator i=LI->begin(),e=LI->end();i!=e;++i)
	{
		Loop *L=*i;
		std::vector<Value*> loop_read;
		std::vector<Value*> loop_write;
		get_loop_input(L,loop_read);
		get_loop_output(L,loop_write);
		for(std::vector<Value*>::iterator i=loop_read.begin();i!=loop_read.end();++i)
		{
			Value *read=*i;
			if(Dataflow_read_num.count(read)){
				Dataflow_read_num[read]++;
			}
			else{
				Dataflow_read_num[read]=1;
			}
		}
		for(std::vector<Value*>::iterator i=loop_write.begin();i!=loop_write.end();++i)
		{
			Value *write=*i;
			if(Dataflow_write_num.count(write)){
				Dataflow_write_num[write]++;
			}
			else{
				Dataflow_write_num[write]=1;
			}
		}
		sub_element_num++;
	}
	//No need to dataflow if only one sub-element
	if(sub_element_num<=1){
		return false;
	}
	for(std::map<Value*, int>::iterator d=Dataflow_read_num.begin();d!=Dataflow_read_num.end();++d)
	{
		int num=d->second;
		if(num>1){
			Value *array_read=d->first;
			if(Dataflow_write_num.count(array_read)){
				return false;
			}
			//errs()<<"No dataflow: Array is read by more than one functions.\n";
		}
	}
	for(std::map<Value*, int>::iterator d=Dataflow_write_num.begin();d!=Dataflow_write_num.end();++d)
	{
		int num=d->second;
		if(num>1){
			//errs()<<"No dataflow: Array is written by more than one functions.\n";
			return false;
		}
	}
	//if load a and store a, should be load a->store in the same loop, or store a and then load it next, otherwise false...
	int inst_index=0;
	for(inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I){
		Instruction *inst=&*I;
		if(isa<StoreInst>(inst)){
			Value *array_store=get_array_name(inst);
			int inst1_index=0;
			for(inst_iterator II = inst_begin(F), IE = inst_end(F); II != IE; ++II){
				Instruction *inst1=&*II;
				if(isa<LoadInst>(inst1)){
					Value *array_load=get_array_name(inst1);
					if(array_store==array_load){
						if(inst1_index<inst_index){
							BasicBlock *bb_load=inst1->getParent();
							BasicBlock *bb_store=inst->getParent();
							if(bb_load!=bb_store){
								Loop *l_load;
								Loop *l_store;
								for(LoopInfo::iterator i=LI->begin(),e=LI->end();i!=e;++i)
								{
									Loop *L=*i;
									if(L->contains(bb_load)){
										l_load=L;
									}
									if(L->contains(bb_store)){
										l_store=L;
									}
								}
								if(l_load!=l_store){
									return false;
								}
							}
						}
					}
				}
				inst1_index++;
			}
		}
		inst_index++;
	}

	return true;
}

int compute_dataflow_II(Function *F, LoopInfo* LI)
{
	int II=1;
	bool bb_in_loop=false;
	for(auto itb=F->begin();itb!=F->end();++itb)
	{
		for(LoopInfo::iterator i=LI->begin(),e=LI->end();i!=e;++i)
		{
			Loop *L=*i;
			if(L->contains(itb)){
				bb_in_loop=true;
				break;
			}
		}
		if(bb_in_loop==false){
			for(auto iti=itb->begin();iti!=itb->end();++iti){
				if(CallInst *call=dyn_cast<CallInst>(iti)){
					Function *callee=call->getCalledFunction();
					unsigned FnID=callee->getIntrinsicID();
					if(FnID==0){
						int m=0;
						if(Function_pipeline[F]==1){
							m=function_II[callee];
						}
						else{
							m=function_cycles[callee];
						}
						II=std::max(m,II);
					}
				}
			}
		}
	}
	for(LoopInfo::reverse_iterator i=LI->rbegin(),e=LI->rend();i!=e;++i)
	{
		Loop *L=*i;
		int n=0;
		n=(int)loop_cycles[L];
		II=std::max(n,II);
	}
	return II;
}
