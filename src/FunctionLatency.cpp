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


#include "FunctionLatency.h"
#include "Library.h"
#include "UpdateMem.h"
#include "ComputeCriticalPath.h"
#include "LoopLatency.h"
#include "InstLibrary.h"
#include "ArrayName.h"
#include "SetParameter.h"
#include "Reschedule.h"

int compute_cycles_in_Fn(Function *F,LoopInfo* LI,std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop)
{
	std::map<Instruction*, unsigned> load_order_buff;
	std::map<unsigned, float> loop_inst_till_latency;
	std::map<Instruction*,std::queue<unsigned> > inst_map_branch;
	std::map<Loop*,int> updated_loop;
	unsigned index_loop=1;
	int critical_path_function=0;
	loop_critical_path.clear();
	bb_trip_counter.clear();

	reorder_load(F,load_order_buff);

	for(auto itb=F->begin();itb!=F->end();++itb)
	{
		if(!LI->empty()){
			for(LoopInfo::iterator i=LI->begin(),e=LI->end();i!=e;++i)
			{
				Loop *L=*i;
				if(L->contains(itb))
				{
					Loop *loop_parent=LI->getLoopFor(itb);
					bb_trip_counter[itb]=Loops_unroll[loop_parent];
					break;
				}
				else
				{
					bb_trip_counter[itb]=1;
				}
			}
		}
		else{
			bb_trip_counter[itb]=1;
		}
	}
	for(auto itb=F->begin();itb!=F->end();++itb)
	{
		if(LI->empty()){
			for(auto iti=itb->begin();iti!=itb->end();++iti){
				update(LI,iti,index_loop,instr_index_loop,load_order_buff,dependence_loop,inst_map_branch);
				if(isa<LoadInst>(iti)){
					update_load(LI,iti,instr_index_loop,dependence_loop);
				}
				else if(isa<StoreInst>(iti)){
					update_store(LI,iti,instr_index_loop,dependence_loop);
				}
			}
		}
		else{
			bool bb_in_loop_flag=false;
			Loop *loop=NULL;
			for(LoopInfo::reverse_iterator i=LI->rbegin(),e=LI->rend();i!=e;++i)
			{
				Loop *L=*i;
				if(L->contains(itb)){
					bb_in_loop_flag=true;
					loop=L;
					break;
				}
			}
			if(bb_in_loop_flag==false){
				for(auto iti=itb->begin();iti!=itb->end();++iti){
					update(LI,iti,index_loop,instr_index_loop,load_order_buff,dependence_loop,inst_map_branch);
					if(isa<LoadInst>(iti)){
						update_load(LI,iti,instr_index_loop,dependence_loop);
					}
					else if(isa<StoreInst>(iti)){
						update_store(LI,iti,instr_index_loop,dependence_loop);
					}
				}
			}
			else{
				if(updated_loop.count(loop)){
					++updated_loop[loop];
				}
				else{
					updated_loop[loop]=1;
					Loop_insert(LI,loop,index_loop,instr_index_loop,load_order_buff,dependence_loop,inst_map_branch);
				}
			}
		}
		std::map<Loop*, unsigned> Complete_Loop_Index;
		store_complete_loop(LI, Complete_Loop_Index);
	}
	int order=1;
	unsigned F_index=Function_index[F];
	std::map<Function*,int> function_counted;
	if(LI->empty()){
		for(inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I){
			Instruction *inst=&*I;
			if(CallInst *call=dyn_cast<CallInst>(inst)){
				Function *callee = call->getCalledFunction();
				if(callee!=F){
					unsigned FnID=callee->getIntrinsicID();
					if(FnID==0){
						if(!function_counted.count(callee)){
							if(!optimized_functions.count(callee)){
								function_subelement_cycles[F_index].push_back(std::make_pair(order,function_cycles[callee]));
							}
							order++;
							function_counted[callee]=1;
						}
						else{
							function_counted[callee]++;
						}
					}
				}
			}
		}
	}
	else{
		std::map<Loop*,int> top_loop_counted;
		for(inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I){
			Instruction *inst=&*I;
			BasicBlock *bb=inst->getParent();
			Loop *bb_loop=LI->getLoopFor(bb);
			if(bb_loop==NULL){
				if(CallInst *call=dyn_cast<CallInst>(inst)){
					Function *callee = call->getCalledFunction();
					if(callee!=F){
						unsigned FnID=callee->getIntrinsicID();
						if(FnID==0){
							if(!function_counted.count(callee)){
								if(!optimized_functions.count(callee)){
									function_subelement_cycles[F_index].push_back(std::make_pair(order,function_cycles[callee]));
								}
								order++;
								function_counted[callee]=1;
							}
							else{
								function_counted[callee]++;
							}
						}
					}
				}
			}
			else{
				Loop *top_loop=get_top_loop(bb_loop);
				std::vector<int> loop_index[l_num];
				std::map<Loop*,int> loop_index_tmp;
				set_loop_index(F,LI,top_loop,loop_index_tmp,loop_index);
				int top_loop_index=loop_index_tmp[top_loop];
				if(top_loop_counted.count(top_loop)){
					top_loop_counted[top_loop]++;
				}
				else{
					int loop_latency=0;
					if(loop_cycles.count(top_loop)){
						loop_latency=(int)loop_cycles[top_loop];
					}
					if(!optimized_loops.count(top_loop_index)){
						function_subelement_cycles[F_index].push_back(std::make_pair(order,loop_latency));
					}
					order++;
					top_loop_counted[top_loop]=1;
				}
			}
		}
	}
	reschedule_dependence(LI,frequency,instr_index_loop,dependence_loop);
	critical_path_function=(int)round(loop_solveCP(dependence_loop,loop_inst_till_latency));
	return critical_path_function;
}


void reorder_load(Function *F, std::map<Instruction*,unsigned> &load_order_buff)
{
	std::map<Instruction*, unsigned> load_original_order;
	std::map<Instruction*, Instruction*> inst_load_map0;
	std::map<Instruction*, Instruction*> inst_load_map1;
	unsigned original_index=1;
	unsigned reorder_index=1;
	for(auto itb=F->begin();itb!=F->end();++itb)
	{
		for(auto iti=itb->begin();iti!=itb->end();++iti){
			Instruction *inst=iti;
			if(isa<LoadInst>(inst)){
				if(!load_original_order.count(inst)){
					load_original_order[inst]=original_index;
					original_index++;
				}
			}
			else{
				for(unsigned op=0; op!=inst->getNumOperands(); ++op)
				{
					Value *OP = inst->getOperand(op);
					if(Instruction *inst1=dyn_cast<Instruction>(OP)){
						if(isa<LoadInst>(inst1)){
							BasicBlock *bb=inst1->getParent();
							if(itb!=*bb){
								if(!load_order_buff.count(inst1)){
									load_order_buff[inst1]=reorder_index;
									reorder_index++;
								}
							}
							else{
								float inst_latency=inst_latency_in_library(inst);
								if(inst_latency>3.0){
									if(!load_order_buff.count(inst1)){
										load_order_buff[inst1]=reorder_index;
										reorder_index++;
									}
								}
								else{
									int count=0;
									bool close_inst=false;
									if(isa<StoreInst>(inst)){
										if(!load_order_buff.count(inst1)){
											load_order_buff[inst1]=reorder_index;
											reorder_index++;
										}
									}
									else{
										for(auto it=iti;it!=itb->end();++it){
											Instruction *inst_tmp=it;
											for(unsigned op_tmp=0;op_tmp!=inst_tmp->getNumOperands();++op_tmp){
												Value *OP_tmp=inst_tmp->getOperand(op_tmp);
												if(Instruction *inst2=dyn_cast<Instruction>(OP_tmp)){
													if(inst2==inst){
														if(!load_order_buff.count(inst1)){
															load_order_buff[inst1]=reorder_index;
															reorder_index++;
														}
														close_inst=true;
														break;
													}
												}
											}
											if(close_inst==true){
												break;
											}
											else{
												count++;
												if(count>3){
												    break;
											    }
											}
										}
										if(close_inst==false){//both op are loads...consider cast situation(not consider now)
											if(!inst_load_map0.count(inst)){
												inst_load_map0[inst]=inst1;
											}
											else if(!inst_load_map1.count(inst)){
												inst_load_map1[inst]=inst1;
											}
										}
									}
								}
							}
						}
						else{
							if(inst_load_map0.count(inst1)){
								Instruction *load_inst0=inst_load_map0[inst1];
								if(!load_order_buff.count(load_inst0)){
									load_order_buff[load_inst0]=reorder_index;
									reorder_index++;
								}
								if(inst_load_map1.count(inst1)){
									Instruction *load_inst1=inst_load_map1[inst1];
									if(!load_order_buff.count(load_inst1)){
										load_order_buff[load_inst1]=reorder_index;
										reorder_index++;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	//test whether reorder is right
	if(original_index!=reorder_index){
		errs()<<original_index<<"\n";
		errs()<<reorder_index<<"\n";
		errs()<<"Wrong: please check load reorder buffer.\n";
	}

	for(std::map<Instruction*,unsigned>::iterator it=load_order_buff.begin();it!=load_order_buff.end();++it){
		Instruction *inst=it->first;
		if(load_order_buff[inst]>load_original_order[inst]){
			Value *array=get_array_name(inst);
			BasicBlock *itb=inst->getParent();
			for(auto iti=itb->begin();iti!=itb->end();++iti){
				Instruction *inst1=iti;
				for(unsigned op=0; op!=inst1->getNumOperands(); ++op)
				{
					Value *OP = inst1->getOperand(op);
					if( Instruction *tmp_inst = dyn_cast<Instruction>(OP) )
					{
						if(inst==tmp_inst){
							float inst1_latency=inst_latency_in_library(inst1);
							if(inst1_latency>INT_MULT*2){
								for(std::map<Instruction*,unsigned>::iterator ii=load_order_buff.begin();ii!=load_order_buff.end();++ii){
									Instruction *inst2=ii->first;
									BasicBlock *bb2=inst2->getParent();
									if(itb==bb2){
										Value *array2=get_array_name(inst2);
										if(array==array2){
											load_order_buff[inst2]=load_original_order[inst2];
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	for(std::map<Instruction*,unsigned>::iterator it=load_order_buff.begin();it!=load_order_buff.end();++it){
		Instruction *inst=it->first;
		BasicBlock *bb=inst->getParent();
		bool small=all_is_small(bb,inst);
		if(small==true){
			Value *array=get_array_name(inst);
			for(std::map<Instruction*,unsigned>::iterator ii=load_order_buff.begin();ii!=load_order_buff.end();++ii){
				Instruction *inst2=ii->first;
				BasicBlock *bb2=inst2->getParent();
				if(bb==bb2){
					Value *array2=get_array_name(inst2);
					if(array==array2){
						load_order_buff[inst2]=load_original_order[inst2];
					}
				}
			}
		}
	}

}

void store_complete_subloop(Loop *L, std::map<Loop*, unsigned> &Complete_Loop_Index)
{
	unsigned complete_loop_index=1;
	std::vector<Loop*> subLoops = L->getSubLoops();
	int subLoop_num = subLoops.size();
	for(int j=0; j<subLoop_num; j++){
		Loop* sub_loop = subLoops.at(j);
		if(Loops_unroll[sub_loop]>=Loops_counter[sub_loop]){
			Complete_Loop_Index[sub_loop]=complete_loop_index;
			complete_loop_index++;
		}
	}
}

void store_complete_loop(LoopInfo* LI, std::map<Loop*, unsigned> &Complete_Loop_Index)
{
	unsigned complete_loop_index=1;
	for(LoopInfo::reverse_iterator i=LI->rbegin(),e=LI->rend();i!=e;++i){
		Loop *L=*i;
		if(Loops_unroll[L]>=Loops_counter[L]){
			Complete_Loop_Index[L]=complete_loop_index;
			complete_loop_index++;
		}
	}
}

Loop *get_top_loop(Loop *L)
{
	Loop *top_loop=NULL;
	if(L!=NULL){
		Loop *parent=L->getParentLoop();
		if(parent==NULL){
			top_loop=L;
		}
		else{
			top_loop=get_top_loop(parent);
		}
	}
	return top_loop;
}

bool all_is_small(BasicBlock *bb, Instruction *inst)
{
	bool small=true;
	bool break_flag=false;
	Value *array=get_array_name(inst);
	for(auto ii=bb->begin();ii!=bb->end();++ii){
		Instruction *inst_ii=ii;
		if(isa<LoadInst>(inst_ii)){
			Value *array_load=get_array_name(ii);
			if(array_load==array){
				for(auto iti=bb->begin();iti!=bb->end();++iti){
					Instruction *inst1=iti;
					for(unsigned op=0; op!=inst1->getNumOperands(); ++op)
					{
						Value *OP = inst1->getOperand(op);
						if( Instruction *tmp_inst = dyn_cast<Instruction>(OP) )
						{
							if(inst_ii==tmp_inst){
								float inst1_latency=inst_latency_in_library(inst1);
								if(inst1_latency>INT_MULT*2){
									small=false;
									break_flag=true;
									break;
								}
							}
						}
					}
					if(break_flag==true){
						break;
					}
				}
			}
		}
		if(break_flag==true){
			break;
		}
	}

	return small;
}
