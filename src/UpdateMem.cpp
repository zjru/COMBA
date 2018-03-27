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


#include "UpdateMem.h"
#include "MemOpNum.h"
#include "Constant.h"
#include "EvaluateDependency.h"


void complete_load_update(Loop *L1, Loop *L2, float latency,float latency_delta, unsigned index1,unsigned index2, Instruction* inst,std::vector<std::pair<int,float> > *dependence_loop)
{
	if(L1==NULL){
		if(Loops_unroll[L2]>=Loops_counter[L2]){
			Loop *parent=L2->getParentLoop();
			if(parent==NULL){
				dependence_loop[index1].push_back(std::make_pair(index2,latency_delta));
				Load_latency[inst]=latency_delta;
			}
			else{
				if(Loops_unroll[parent]>=Loops_counter[parent]){
					complete_load_update(L1,parent,latency,latency_delta,index1,index2,inst,dependence_loop);
				}
				else{
					dependence_loop[index1].push_back(std::make_pair(index2,latency));
					Load_latency[inst]=latency;
				}
			}
		}
		else{
			dependence_loop[index1].push_back(std::make_pair(index2,latency));
			Load_latency[inst]=latency;
		}
	}
	else{
		if(L1->contains(L2)){
			if(Loops_unroll[L2]>=Loops_counter[L2]){
				Loop *parent=L2->getParentLoop();
				if(L1==parent){
					dependence_loop[index1].push_back(std::make_pair(index2,latency_delta));
					Load_latency[inst]=latency_delta;
				}
				else{
					complete_load_update(L1,parent,latency,latency_delta,index1,index2,inst,dependence_loop);
				}
			}
			else{
				dependence_loop[index1].push_back(std::make_pair(index2,latency));
				Load_latency[inst]=latency;
			}
		}
		else if(L2->contains(L1)){
			if(Loops_unroll[L1]>=Loops_counter[L1]){
				Loop *parent=L1->getParentLoop();
				if(L2==parent){
					dependence_loop[index1].push_back(std::make_pair(index2,latency_delta));
					Load_latency[inst]=latency_delta;
				}
				else{
					complete_load_update(parent,L2,latency,latency_delta,index1,index2,inst,dependence_loop);
				}
			}
			else{
				dependence_loop[index1].push_back(std::make_pair(index2,latency));
				Load_latency[inst]=latency;
			}
		}
		else if(L1==L2){
			dependence_loop[index1].push_back(std::make_pair(index2,latency));
			Load_latency[inst]=latency;
		}
		else{
			Loop *parent2=L2->getParentLoop();
			Loop *parent1=L1->getParentLoop();
			if(parent1==parent2){
				if(Loops_unroll[L2]>=Loops_counter[L2]){
					if(Loops_unroll[L1]>=Loops_unroll[L1]){
						dependence_loop[index1].push_back(std::make_pair(index2,latency_delta));
						Load_latency[inst]=latency_delta;
					}
					else{
						dependence_loop[index1].push_back(std::make_pair(index2,latency));
						Load_latency[inst]=latency;
					}
				}
				else{
					dependence_loop[index1].push_back(std::make_pair(index2,latency));
					Load_latency[inst]=latency;
				}
			}
			else{
				if(parent2==NULL){
					if(Loops_unroll[L2]>=Loops_counter[L2]){
						dependence_loop[index1].push_back(std::make_pair(index2,latency_delta));
						Load_latency[inst]=latency_delta;
					}
					else{
						dependence_loop[index1].push_back(std::make_pair(index2,latency));
						Load_latency[inst]=latency;
					}
				}
				else{
					if(parent2->contains(L1)){
						if(Loops_unroll[L2]>=Loops_counter[L2]){
							dependence_loop[index1].push_back(std::make_pair(index2,latency_delta));
							Load_latency[inst]=latency_delta;
						}
						else{
							dependence_loop[index1].push_back(std::make_pair(index2,latency));
							Load_latency[inst]=latency;
						}
					}
					else{
						if(Loops_unroll[parent2]>=Loops_counter[parent2]){
							complete_load_update(L1,parent2,latency,latency_delta,index1,index2,inst,dependence_loop);
						}
						else{
							dependence_loop[index1].push_back(std::make_pair(index2,latency));
							Load_latency[inst]=latency;
						}
					}
				}
			}
		}
	}
}


void update_load(LoopInfo* LI, Instruction *inst, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop)
{
	unsigned index_inst=0;
	index_inst=instr_index_loop[inst];
	BasicBlock *bb1=inst->getParent();
	Loop *L1=LI->getLoopFor(bb1);
	float latency=Load_latency[inst];
	std::map<Instruction *, unsigned>::iterator it;
	for(it=instr_index_loop.begin();it!=instr_index_loop.end();++it){
		Instruction *inst_tmp=it->first;
		unsigned index_inst_tmp=it->second;
		if(isa<StoreInst>(inst_tmp)){
			BasicBlock *bb2=inst_tmp->getParent();
			if(bb1!=bb2){
				bool LS_dependency=load_store_dependency(inst,inst_tmp);
				if(LS_dependency==true){
					std::vector< std::pair<int,float> >::iterator i= dependence_loop[index_inst].begin();
					if(i->first==(int)index_inst){
						i->second=0;
					}
					Loop *L2=LI->getLoopFor(bb2);
					if(L1==L2){
						dependence_loop[index_inst_tmp].push_back(std::make_pair(index_inst,latency));
						Load_latency[inst]=latency;
					}
					else{
						int load_num=compute_total_LS_num(LI,inst);
						int store_num=compute_total_LS_num(LI,inst_tmp);
						int delta=load_num-store_num;
						if(delta<0){
							delta=0;
						}
						float latency_delta=ceil((float)delta/(float)port_num_read);
						if(L2==NULL){
							dependence_loop[index_inst_tmp].push_back(std::make_pair(index_inst,latency_delta));
							Load_latency[inst]=latency_delta;
						}
						else{
							complete_load_update(L1,L2,latency,latency_delta,index_inst_tmp,index_inst,inst,dependence_loop);
						}
					}
				}
			}
		}
	}
}


void replace_write_latency(int store_num1, int store_num2, unsigned index, Instruction *inst,std::vector<std::pair<int,float> > *dependence_loop)
{
	float latency=ceil((float)store_num1/(float)port_num_write_diff);
	if(store_num2<=store_num1){
		for(std::vector<std::pair<int,float> >::iterator i=dependence_loop[index].begin();i!=dependence_loop[index].end();++i){
			if(i->first==(int)index){
				i->second=0.0;
			}
		}
		Store_latency[inst]=0.0;
	}
	else{
		for(std::vector<std::pair<int,float> >::iterator i=dependence_loop[index].begin();i!=dependence_loop[index].end();++i){
			if(i->first==(int)index){
				if(i->second<latency){
					i->second=0.0;
					Store_latency[inst]=0.0;
				}
				else{
					i->second=i->second-latency;
					Store_latency[inst]=i->second;
				}
			}
		}
	}
}


void complete_store_update(Loop *L1, Loop *L2, int store_num1, int store_num2, unsigned index, Instruction *inst, std::vector<std::pair<int,float> > *dependence_loop)
{
	if(L1!=L2){
		if(L1==NULL){
			if(Loops_unroll[L2]>=Loops_counter[L2]){
				Loop *parent = L2->getParentLoop();
				if(parent==NULL){
					replace_write_latency(store_num1,store_num2,index,inst,dependence_loop);
				}
				else{
					complete_store_update(L1,parent,store_num1,store_num2,index,inst,dependence_loop);
				}
			}
		}
		else{
			if(L1->contains(L2)){
				if(Loops_unroll[L2]>=Loops_counter[L2]){
					Loop *parent=L2->getParentLoop();
					if(L1==parent){
						replace_write_latency(store_num1,store_num2,index,inst,dependence_loop);
					}
					else{
						complete_store_update(L1,parent,store_num1,store_num2,index,inst,dependence_loop);
					}
				}
			}
			else if(L2->contains(L1)){
				if(Loops_unroll[L1]>=Loops_counter[L1]){
					Loop *parent=L1->getParentLoop();
					if(L2==parent){
						replace_write_latency(store_num1,store_num2,index,inst,dependence_loop);
					}
					else{
						complete_store_update(parent,L2,store_num1,store_num2,index,inst,dependence_loop);
					}
				}
			}
			else{
				Loop *parent2=L2->getParentLoop();
				Loop *parent1=L1->getParentLoop();
				if(parent1==parent2){
					if(Loops_unroll[L2]>=Loops_counter[L2]){
						if(Loops_unroll[L1]>=Loops_unroll[L1]){
							replace_write_latency(store_num1,store_num2,index,inst,dependence_loop);
						}
					}
				}
				else{
					if(parent2==NULL){
						if(Loops_unroll[L2]>=Loops_counter[L2]){
							replace_write_latency(store_num1,store_num2,index,inst,dependence_loop);
						}
					}
					else{
						if(parent2->contains(L1)){
							if(Loops_unroll[L2]>=Loops_counter[L2]){
								replace_write_latency(store_num1,store_num2,index,inst,dependence_loop);
							}
						}
						else{
							if(Loops_unroll[parent2]>=Loops_counter[parent2]){
								complete_store_update(L1,parent2,store_num1,store_num2,index,inst,dependence_loop);
							}
						}
					}
				}
			}
		}
	}
}

void update_store(LoopInfo* LI, Instruction *inst, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop)
{
	BasicBlock *bb1=inst->getParent();
	Loop *L1=LI->getLoopFor(bb1);
	std::map<Instruction *, unsigned>::iterator it;
	for(it=instr_index_loop.begin();it!=instr_index_loop.end();++it){
		Instruction *inst_tmp=it->first;
		unsigned index_inst_tmp=it->second;
		if(isa<StoreInst>(inst_tmp)){
			if(inst_tmp!=inst){
				BasicBlock *bb2=inst_tmp->getParent();
				if(bb1!=bb2){
					bool SS_dependency=store_store_dependency(inst,inst_tmp);
					if(SS_dependency==true){
						//dependence_loop[index_inst_tmp].push_back(std::make_pair(index_inst,0.0));
						Loop *L2=LI->getLoopFor(bb2);
						if(L1!=L2){
							int store_num1=compute_total_LS_num(LI,inst);
							int store_num2=compute_total_LS_num(LI,inst_tmp);
							if(L2==NULL){
								if(Loops_unroll[L1]>=Loops_counter[L1]){
									replace_write_latency(store_num1,store_num2,index_inst_tmp,inst_tmp,dependence_loop);
								}
							}
							else{
								complete_store_update(L1,L2,store_num1,store_num2,index_inst_tmp,inst_tmp,dependence_loop);
							}
						}
					}
				}
			}
		}
	}
}
