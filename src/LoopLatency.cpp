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


#include "LoopLatency.h"
#include "Constant.h"
#include "ArrayName.h"
#include "ComputeCriticalPath.h"
#include "FindLatency.h"
#include "Reschedule.h"
#include "Loop_II.h"
#include "UpdateMem.h"
#include "ComputeFixedPath.h"
#include "Library.h"


bool test_loop_is_perfect(Loop* L)
{
	bool loop_is_perfect=false;
	std::vector<Loop*> subLoops=L->getSubLoops();
	int size = subLoops.size();
	if(subLoops.empty())
	{
		Instruction *indvar=NULL;
		Loop* Loop_parent=L->getParentLoop();
		BasicBlock *head=Loop_parent->getHeader();
		BasicBlock *latch=Loop_parent->getLoopLatch();
		TerminatorInst *last_inst=latch->getTerminator();
		BranchInst *br_inst=dyn_cast<BranchInst>(last_inst);
		CmpInst *cmp=dyn_cast<CmpInst>(br_inst->getCondition());
		Value *cmpOp0=cmp->getOperand(0);
		Value *cmpOp1=cmp->getOperand(1);
		if(Instruction *cmp_op0=dyn_cast<Instruction>(cmpOp0))
		{
			indvar=cmp_op0;
		}
		else if(Instruction *cmp_op1=dyn_cast<Instruction>(cmpOp1))
		{
			indvar=cmp_op1;
		}
		else{
			errs()<<"Not a common conditional inst.\n";
		}
		for(auto iti=latch->begin();iti!=latch->end();++iti)
		{
		   Instruction *inst=iti;
		   if((inst==cmp)||(inst==br_inst)||(inst==indvar))
		   {
			   loop_is_perfect=true;
		   }
		   else{
			   loop_is_perfect=false;
			   break;
		   }
		}
		if(loop_is_perfect==true){
			   for(auto ii=head->begin();ii !=head->end();++ii)
			   {
				   Instruction *inst_h=ii;
				   if(isa<StoreInst>(inst_h)){
					   Value *array=get_array_name(inst_h);
					   for(Loop::block_iterator BI=L->block_begin(), BE=L->block_end(); BI !=BE; ++BI){
						   BasicBlock *bb=*BI;
						   for(auto it=bb->begin();it!=bb->end();++it){
							   if(isa<LoadInst>(it)){
								   Value *array_name=get_array_name(it);
								   if(array_name==array){
									   loop_is_perfect=false;
								   }
							   }
						   }
					   }
				   }
			   }
		}
	}
	else if(size==1){
		Loop* L1 = subLoops.at(0);
		if(Loops_pipeline[L1]==1){
			Instruction *indvar=NULL;
			Loop* Loop_parent=L1->getParentLoop();
			BasicBlock *head=Loop_parent->getHeader();
			BasicBlock *latch=Loop_parent->getLoopLatch();
			TerminatorInst *last_inst=latch->getTerminator();
			BranchInst *br_inst=dyn_cast<BranchInst>(last_inst);
			CmpInst *cmp=dyn_cast<CmpInst>(br_inst->getCondition());
			Value *cmpOp0=cmp->getOperand(0);
			Value *cmpOp1=cmp->getOperand(1);
			if(Instruction *cmp_op0=dyn_cast<Instruction>(cmpOp0))
			{
				indvar=cmp_op0;
			}
			else if(Instruction *cmp_op1=dyn_cast<Instruction>(cmpOp1))
			{
				indvar=cmp_op1;
			}
			else{
				errs()<<"Not a common conditional inst.\n";
			}

			for(auto iti=latch->begin();iti!=latch->end();++iti)
			{
			   Instruction *inst=iti;
			   if((inst==cmp)||(inst==br_inst)||(inst==indvar))
			   {
				   loop_is_perfect=true;
			   }
			   else{
				   loop_is_perfect=false;
				   break;
			   }
			}
			if(loop_is_perfect==true){
			   for(auto ii=head->begin();ii !=head->end();++ii)
			   {
				   Instruction *inst_h=ii;
				   if(isa<StoreInst>(inst_h)){
					   Value *array=get_array_name(inst_h);
					   for(Loop::block_iterator BI=L1->block_begin(), BE=L1->block_end(); BI!=BE; ++BI){
						   BasicBlock *bb=*BI;
						   for(auto it=bb->begin();it!=bb->end();++it){
							   if(isa<LoadInst>(it)){
								   Value *array_name=get_array_name(it);
								   if(array_name==array){
									   loop_is_perfect=false;
								   }
							   }
						   }
					   }
				   }
			   }
			}
		}
		else{
			loop_is_perfect=test_loop_is_perfect(L1);
		}
	}
	else{
		loop_is_perfect=false;
	}
	return loop_is_perfect;
}


float pipeline_iteration_latency(LoopInfo* LI, Loop *L,int II, std::map<Instruction*, unsigned> &instr_index_loop, std::map<Instruction*, unsigned> &load_order_buff, std::vector<std::pair<int,float> > *dependence_loop)
{
	float iteration_latency=0.0;
	iteration_latency=round(update_loopCP(LI,L,load_order_buff));
	std::map<Value*,int> array_read;
	std::map<Value*,int> array_write;
	for(std::map<Instruction*,unsigned>::iterator it=instr_index_loop.begin();it!=instr_index_loop.end();++it)
	{
		Instruction *inst=it->first;
		if(isa<LoadInst>(inst)){
			Value *array_name=get_array_name(inst);
			if(!array_read.count(array_name)){
				if(array_number.count(array_name)){
					array_read[array_name]=array_number[array_name];
				}
			}
		}
		else if(isa<StoreInst>(inst)){
			Value *array_name=get_array_name(inst);
			if(!array_write.count(array_name)){
				if(array_number.count(array_name)){
					array_write[array_name]=array_number[array_name];
				}
			}
		}
	}
	float IL_delta=0.0;
	for(std::map<Value*,int>::iterator it=array_read.begin();it!=array_read.end();++it)
	{
		Value *array=it->first;
		if(array_write.count(array)){
			float latency_min_write=inf;
			float latency_max_write=0.0;
			float latency_max_read=0.0;
			for(std::map<Instruction*,unsigned>::iterator ii=instr_index_loop.begin();ii!=instr_index_loop.end();++ii){
				Instruction *inst=ii->first;
				if(isa<StoreInst>(inst)){
					Value *store_array=get_array_name(inst);
					if(array==store_array){
						float latency_tmp=find_latency_after_inst(inst,instr_index_loop,dependence_loop);
						latency_min_write=std::min(latency_min_write,latency_tmp);
						latency_max_write=std::max(latency_max_write,latency_tmp);
					}
				}
				else if(isa<LoadInst>(inst)){
					Value *load_array=get_array_name(inst);
					if(load_array==array){
						float latency_tmp1=find_latency_after_inst(inst,instr_index_loop,dependence_loop);
						latency_max_read=std::max(latency_max_read,latency_tmp1);
					}
				}
			}
			latency_min_write=round(latency_min_write)-1.0;
			latency_max_write=round(latency_max_write)-1.0;
			latency_max_read=round(latency_max_read);
			int divider=(int)ceil(latency_max_write/(float)II);
			int upper_bound=divider*II;
			IL_delta=std::max((float)upper_bound,IL_delta);
		}
	}
	if(IL_delta>=iteration_latency){
		iteration_latency = IL_delta;//+= IL_delta;
	}
	return round(iteration_latency);
}


float perfect_pipeline_iteration_latency(LoopInfo* LI, Loop *L, int II, std::map<Instruction*, unsigned> &instr_index_loop,std::map<Instruction*, unsigned> &load_order_buff, std::vector<std::pair<int,float> > *dependence_loop)
{
	float usual_latency=pipeline_iteration_latency(LI,L,II,instr_index_loop,load_order_buff,dependence_loop);
	float iteration_latency=round(update_loopCP(LI,L,load_order_buff));
	Loop *parent=L->getParentLoop();
	float new_iteration_latency=round(update_loopCP(LI,parent,load_order_buff));
	if(new_iteration_latency>iteration_latency){
		usual_latency += new_iteration_latency-iteration_latency;
	}
	return usual_latency;
}


float compute_cycles_in_loop(LoopInfo* LI, Loop* L, std::map<Instruction*, unsigned> &load_order_buff)
{
	//float constant=1.0;
	int II=1;
	std::vector<Loop*> subLoops = L->getSubLoops();
	int subLoop_num = subLoops.size();
	float tmp_sum = 0.0;

	if ( subLoops.empty() )
	{
		if(Loops_pipeline[L]==1)
		{
			std::map<Instruction*, unsigned> instr_index_loop;
			std::vector<std::pair<int,float> > dependence_loop[INSN_NUM];
			std::map<Instruction*,std::queue<unsigned> > inst_map_branch;
			dependence_set(LI,L,instr_index_loop,load_order_buff,dependence_loop,inst_map_branch);
			reschedule_dependence(LI,frequency,instr_index_loop,dependence_loop);
			II=compute_II(LI,L,instr_index_loop,load_order_buff,dependence_loop);
			float pipeline_latency=pipeline_iteration_latency(LI,L,II,instr_index_loop,load_order_buff,dependence_loop);
			tmp_sum += pipeline_latency + II*(ceil((float)Loops_counter[L]/(float)Loops_unroll[L])-1);
		}
		else if(Loops_pipeline[L]==0){
			if(Loops_unroll[L]>=Loops_counter[L]){
				errs()<<"Single Loop "<<L<<" is unrolled completely.\n";
			}
			else{
				float update_loop_cp=round(update_loopCP(LI,L,load_order_buff));
				tmp_sum += update_loop_cp * ceil((float)Loops_counter[L]/(float)Loops_unroll[L]);
			}
		}
		else{
			errs()<<"Loop pipelining is not set correctly\n";
		}
		//errs()<<"subLoop: "<<L<<"\n";
		//errs()<<"Loop cycles: "<<tmp_sum<<"\n";
		loop_cycles[L]=tmp_sum;
		//tmp_sum += constant;
	}
	else
	{
		bool loop_is_perfect=test_loop_is_perfect(L);
		if(loop_is_perfect==false){
			//errs()<<"Imperfect Loop: May change into perfect loop (loop merge or add if-else within inner loop) to improve performance.\n";
		}
		if(Loops_pipeline[L]==1){
			std::map<Instruction*, unsigned> instr_index_loop;
			std::vector<std::pair<int,float> > dependence_loop[INSN_NUM];
			std::map<Instruction*,std::queue<unsigned> > inst_map_branch;
			dependence_set(LI,L,instr_index_loop,load_order_buff,dependence_loop,inst_map_branch);
			reschedule_dependence(LI,frequency,instr_index_loop,dependence_loop);
			II=compute_II(LI,L,instr_index_loop,load_order_buff,dependence_loop);
			float pipeline_latency=pipeline_iteration_latency(LI,L,II,instr_index_loop,load_order_buff,dependence_loop);
			//errs()<<"Pipelined latency: "<<pipeline_latency<<"\n";
			//errs()<<II<<"\n";
			tmp_sum += pipeline_latency + II*(ceil((float)Loops_counter[L]/(float)Loops_unroll[L])-1);
		}
		else if(Loops_pipeline[L]==0){
			Loop* sub_loop_first=subLoops.at(0);
			if((loop_is_perfect==true) && (Loops_unroll[L]==1) && (Loops_pipeline[sub_loop_first]==1)){
				std::map<Instruction*, unsigned> instr_index_subloop;
				std::vector<std::pair<int,float> > dependence_subloop[INSN_NUM];
				std::map<Instruction*,std::queue<unsigned> > inst_map_branch;
				dependence_set(LI,sub_loop_first,instr_index_subloop,load_order_buff,dependence_subloop,inst_map_branch);
				reschedule_dependence(LI,frequency,instr_index_subloop,dependence_subloop);
				int II_subloop=compute_II(LI,sub_loop_first,instr_index_subloop,load_order_buff,dependence_subloop);
				float subloop_cp=perfect_pipeline_iteration_latency(LI,sub_loop_first,II_subloop,instr_index_subloop,load_order_buff,dependence_subloop);
				//errs()<<"Pipelined latency: "<<subloop_cp<<"\n";
				//errs()<<II_subloop<<"\n";
				tmp_sum += subloop_cp + II_subloop*((float)Loops_counter[L]/(float)Loops_unroll[L] * ceil((float)Loops_counter[sub_loop_first]/(float)Loops_unroll[sub_loop_first])-1);
			}
			else{
				if(Loops_unroll[L]>=Loops_counter[L]){
					errs()<<"Loop "<<L<<" is unrolled completely and just to calculate its subloops.\n";
				}
				else{
					std::map<Instruction*, unsigned> instr_index_loop;
					std::vector<std::pair<int,float> > dependence_loop[INSN_NUM];
					std::map<Instruction*,std::queue<unsigned> > inst_map_branch;
					std::map<unsigned, float> loop_inst_till_latency;
					std::map<Loop*,int> updated_loop;
					unsigned index_loop=1;
					for (int i=0;i<INSN_NUM;++i)
					{
						std::vector<std::pair<int,float> > vec;
						dependence_loop[i].swap(vec);
					}
					for(Loop::block_iterator BI=L->block_begin(), BE=L->block_end(); BI !=BE; ++BI)
					{
						BasicBlock *bb=*BI;
						bool bb_in_loop_flag=false;
						Loop *subloop=NULL;
						for(int j=0; j<subLoop_num; j++){
							Loop* sub_loop = subLoops.at(j);
							if(sub_loop->contains(bb)){
								bb_in_loop_flag = true;
								subloop=sub_loop;
								break;
							}
						}
						if(bb_in_loop_flag==false){
							for(auto iti=bb->begin();iti!=bb->end();++iti){
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
							 if(updated_loop.count(subloop)){
								 ++updated_loop[subloop];
							 }
							 else{
								 updated_loop[subloop]=1;
								 Loop_insert(LI, subloop, index_loop, instr_index_loop,load_order_buff, dependence_loop,inst_map_branch);
							 }
						}
					}
					reschedule_dependence(LI,frequency,instr_index_loop,dependence_loop);
					float critical_path_loop = loop_solveCP(dependence_loop,loop_inst_till_latency);
					tmp_sum = round(critical_path_loop) * ceil((float)Loops_counter[L]/(float)Loops_unroll[L]);
				}
			}
		}
		else{
			errs()<<"Pipeline directive is not set correctly.\n";
		}
		loop_cycles[L]=tmp_sum;
		//errs()<<"Loop: "<<L<<"\n";
		//errs()<<"Loop cycles: "<<tmp_sum<<"\n";
	}
	return tmp_sum;
}


//insert loop (not unrolled completely) as a "big" node in the critical path
void Loop_insert(LoopInfo* LI, Loop *L, unsigned &index_loop, std::map<Instruction*, unsigned> &instr_index_loop, std::map<Instruction*, unsigned> &load_order_buff, std::vector<std::pair<int,float> > *dependence_loop, std::map<Instruction*,std::queue<unsigned> > &inst_map_branch)
{
	 std::map<Loop*,int> updated_loop;
	 std::vector<Loop*> subLoops=L->getSubLoops();
	 int subLoop_num = subLoops.size();
	 if(subLoops.empty()){
		 if(Loops_unroll[L]>=Loops_counter[L]){
			 for(Loop::block_iterator BI=L->block_begin(), BE=L->block_end(); BI !=BE; ++BI){
				 BasicBlock *bb=*BI;
				 for(auto iti=bb->begin();iti!=bb->end();++iti)
				 {
				 	update(LI,iti,index_loop,instr_index_loop,load_order_buff,dependence_loop,inst_map_branch);
					if(isa<LoadInst>(iti)){
						update_load(LI,iti,instr_index_loop,dependence_loop);
					}
					else if(isa<StoreInst>(iti)){
						update_store(LI,iti,instr_index_loop,dependence_loop);
					}
				 }
			 }
		 }
		 else{
			 Loop *parent=L->getParentLoop();
			 int factor=1;
			 if(parent!=NULL){
				 factor=Loops_unroll[parent];
			 }
			 float latency=compute_cycles_in_loop(LI,L,load_order_buff)*factor;
			 loop_cycles[L]=latency;
			 for(Loop::block_iterator BI=L->block_begin(), BE=L->block_end(); BI !=BE; ++BI)
			 {
				 BasicBlock *bb=*BI;
				 for(auto iti=bb->begin();iti!=bb->end();++iti)
				 {
					 update_fix_latency(L,iti,latency,index_loop,instr_index_loop,dependence_loop);
					 inst_reschedule_considered[iti]=1;
				 }
			 }
		 }
	 }
	 else{
		 if(Loops_unroll[L]>=Loops_counter[L]){
			 for(Loop::block_iterator BI=L->block_begin();BI != L->block_end(); ++BI){
				 BasicBlock *bb=*BI;
				 Loop *subloop=NULL;
				 bool bb_in_loop_flag=false;
				 for(int j=0; j<subLoop_num; j++){
					 Loop* sub_loop = subLoops.at(j);
					 if(sub_loop->contains(bb)){
						 bb_in_loop_flag = true;
						 subloop=sub_loop;
						 break;
					 }
				 }
				 if(bb_in_loop_flag==false){
					 for(auto iti=bb->begin();iti!=bb->end();++iti){
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
					 if(updated_loop.count(subloop)){
						 ++updated_loop[subloop];
					 }
					 else{
						 updated_loop[subloop]=1;
						 Loop_insert(LI, subloop, index_loop, instr_index_loop,load_order_buff,dependence_loop,inst_map_branch);
					 }
				 }
			 }
		 }
		 else{
			 Loop *parent=L->getParentLoop();
			 int factor=1;
			 if(parent!=NULL){
				 factor=Loops_unroll[parent];
			 }
			 float latency=compute_cycles_in_loop(LI,L,load_order_buff)*factor;
			 loop_cycles[L]=latency;
			 for(Loop::block_iterator BI=L->block_begin();BI != L->block_end(); ++BI){
				 BasicBlock *bb=*BI;
				 for(auto iti=bb->begin();iti!=bb->end();++iti)
				 {
					 update_fix_latency(L,iti,latency,index_loop,instr_index_loop,dependence_loop);
					 inst_reschedule_considered[iti]=1;
				 }
			 }
		 }
	 }
}
