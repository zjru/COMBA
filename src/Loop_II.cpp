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


#include "Loop_II.h"
#include "Constant.h"
#include "EvaluateDependency.h"
#include "ComputeCriticalPath.h"
#include "ComputeMemLatency.h"
#include "GEP.h"
#include "FindLatency.h"
#include "ArrayName.h"
#include "PartitionFactor.h"
#include "LoopUnrollFactor.h"
#include "MemOpNum.h"

static bool add_index_loop=true;

//implement RecMII and ResMII....II=max(RecMII, ResMII)
int compute_II(LoopInfo* LI, Loop* L, std::map<Instruction*, unsigned> &instr_index_loop, std::map<Instruction*, unsigned> &load_order_buff, std::vector<std::pair<int,float> > *dependence_loop)
{
	int ResMII = 1;
	int RecMII = 1;
	int II = 1;
	std::map<Instruction *, unsigned>::iterator it;
	for(it=instr_index_loop.begin();it!=instr_index_loop.end();++it)
	{
		Instruction *inst=it->first;
		BasicBlock *bb=inst->getParent();
		if(L->contains(bb)){
			Instruction *indvar=NULL;
			BasicBlock *latch=L->getLoopLatch();
			TerminatorInst *last_inst=latch->getTerminator();
			BranchInst *br_inst=dyn_cast<BranchInst>(last_inst);
			if(br_inst != NULL){
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

				if(indvar->getOpcode()==Instruction::Add){
					add_index_loop=true;
				}
				else if(indvar->getOpcode()==Instruction::Sub){
					add_index_loop=false;
				}
				else{
					add_index_loop=true;
					//errs()<<"This loop is not normal or the block is not loop block";
				}
			}
			PHINode *PHI=NULL;
			BasicBlock *B1=*(L->block_begin());
			for(auto j=B1->begin();j!=B1->end();++j){
				bool break_flag=false;
				if(PHINode *phi1=dyn_cast<PHINode>(j)){
					for(unsigned ii=0, ie=phi1->getNumIncomingValues();ii!=ie;ii++){
					   Value *incoming=phi1->getIncomingValue(ii);
					   Instruction *inst1=dyn_cast<Instruction>(incoming);
					   if(inst1==indvar){
						   PHI=phi1;
						   break_flag=true;
						   break;
					   }
					}
					if(break_flag==true){
						break;
					}
				}
			}
			if(isa<PHINode>(inst))
			{
				PHINode *phi=dyn_cast<PHINode>(inst);
				for(unsigned i=0, e=phi->getNumIncomingValues(); i != e; ++i)
				{
					BasicBlock *inbb = phi->getIncomingBlock(i);
					if(inbb==latch)
					{
						Value *invalue=phi->getIncomingValue(i);
						Instruction *tmp_inst=dyn_cast<Instruction>(invalue);
						if(tmp_inst==NULL){
							RecMII=std::max(1,RecMII);
						}
						else if(tmp_inst==indvar){
							RecMII=std::max(1,RecMII);
						}
						else{
							int distance=1;
							float latency_tmp=find_latency(inst,tmp_inst,instr_index_loop,dependence_loop)-get_inst_latency(inst,instr_index_loop,dependence_loop);
							int rec_tmp=ceil((float)latency_tmp/(float)distance);
							RecMII=std::max(rec_tmp,RecMII);
						}
						//errs()<<"RecMII_in_phi: "<<*phi<<" "<<RecMII<<"\n";
					}
				}

			}
			else if(isa<LoadInst>(inst))
			{
				int rec_tmp=find_max_rec_II(LI,L,inst,PHI,instr_index_loop,dependence_loop);
				RecMII=std::max(rec_tmp,RecMII);
				//errs()<<"RecMII_in_load: "<<rec_tmp<<"\n";
			}//store_store dependency(don't need to consider)
			else if(CallInst *call=dyn_cast<CallInst>(inst)){
				Function *callee = call->getCalledFunction();
				unsigned FnID=callee->getIntrinsicID();
				if(FnID==0){
					int m=function_II[callee];
					RecMII=std::max(m,RecMII);
					unsigned num=call->getNumArgOperands();
					for(unsigned op=0; op<num; ++op){
						Value *OP=call->getArgOperand(op);
						if(callee==OP){
							errs()<<"The operand is the called function.\n";
						}
						else{
							for(Loop::block_iterator BL=L->block_begin(), BF=L->block_end(); BL !=BF; ++BL){
								BasicBlock *BB=*BL;
								for(auto ii=BB->begin();ii!=BB->end();++ii){
									if(isa<StoreInst>(ii)){
										Value *array=get_array_name(ii);
										if(OP==array){
											errs()<<"ResMII_in_call is not feasible due to loop carried dependence.\n";
											float latency_tmp=find_latency(inst,ii,instr_index_loop,dependence_loop);
											int distance=1;
											int rec_tmp=(int)ceil((float)latency_tmp/(float)distance);
											RecMII=std::max(rec_tmp,RecMII);
										}
									}
								}
							}
						}
					}
				}
				//errs()<<"RecMII_in_call: "<<RecMII<<"\n";
			}
			else continue;
		}
	}
	bool flag=test_loop_carried_dependence(LI,L);
	if(flag==true){
		RecMII=RecMII*Loops_unroll[L];
	}
	ResMII=find_ResMII(LI,L,load_order_buff);

	float iteration_latency=0.0;
	iteration_latency=round(update_loopCP(LI,L,load_order_buff));
	II = std::max(ResMII, RecMII);
	if(II>(int)iteration_latency){
		II=(int)iteration_latency;
	}
	loop_II[L]=II;
	return II;
}

int find_array_offset(LoopInfo* LI, Instruction *inst)
{
	int distance=0;
	int dis=0;
	Value *operand=NULL;
	BasicBlock *bb=inst->getParent();
	Loop *L=LI->getLoopFor(bb);
	GetElementPtrInst *gep=NULL;
	gep=get_GEP(inst);
	if(gep==NULL){
		distance=0;
	}
	else{
		Value *array_name=get_array_name(inst);
		if(!array_number.count(array_name)){
			errs()<<"The array loaded is not set correctly for partition.\n";
		}
		else{
			int array_index=array_number[array_name];
			int dim=array_dimension[array_index];
			if(dim==1){
				unsigned num_indice=gep->getNumIndices();
				operand=gep->getOperand(num_indice);
				dis=compute_gep_operand(operand,false,0);
				distance=dis;
			}
			else{
				int m[10];
				int j=0;
				for(int i=0;i<10;++i){
					m[i]=0;
				}
				for(User::op_iterator ii=gep->idx_begin(), ie=gep->idx_end();ii !=ie; ++ii)
				{
					operand=*ii;
					dis=compute_gep_operand(operand,false,0);
					m[j]=dis;
					if(Instruction *op_inst=dyn_cast<Instruction>(operand)){
						BasicBlock *bb1=op_inst->getParent();
						Loop *L1=LI->getLoopFor(bb1);
						if(L1 != L){
							Loop *L2=L->getParentLoop();
							if(L1==L2){
								m[j]=m[j]*Loops_counter[L];
							}
							else{
								Loop *L3=L2->getParentLoop();
								if(L1==L3){
									m[j]=m[j]*Loops_counter[L]*Loops_counter[L2];
								}
								else{
									errs()<<"Something wrong or nested loop with depth larger than 3.\n";
								}
							}
						}
					}
					j++;
				}
				for(int i=0;i<10;++i){
					distance += m[i];
				}
			}
		}
	}
	return distance;
}


int find_distance(LoopInfo* LI, Instruction *inst_l, Instruction *inst_s)
{
	int distance=0;
	int distance_l=0;
	int distance_s=0;
	BasicBlock *bb1=inst_l->getParent();
	Loop *L_load=LI->getLoopFor(bb1);
	BasicBlock *bb2=inst_s->getParent();
	Loop *L_store=LI->getLoopFor(bb2);
	distance_l=find_array_offset(LI, inst_l);
	distance_s=find_array_offset(LI, inst_s);
	distance = floor(distance_l/Loops_unroll[L_load]) - floor(distance_s/Loops_unroll[L_store]);
	return distance;
}


int find_max_rec_II(LoopInfo* LI, Loop *L, Instruction *inst, PHINode *phi, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop)
{
	int distance=0;
	float latency=0.0;
	int recMII=1;
	int recMII_tmp=0;
	bool ls_flag=false;
	unsigned index_ls=instr_index_loop[inst];
	int distance_i=0;
	int first_iteration=0;
	int second_iteration=0;
	int g_phi=0;
	if(isa<LoadInst>(inst)){
		GetElementPtrInst *gep=get_GEP(inst);
		if(gep!=NULL){
			g_phi=get_gep_phi(gep,phi);
			if(g_phi!=10){
				Value *g_op=gep->getOperand(g_phi);
				int compute_second=get_phi_second(L, phi);
				second_iteration=compute_gep_operand(g_op,true,compute_second);
				first_iteration=compute_gep_operand(g_op,false,0);
				distance_i=second_iteration-first_iteration;
			}
		}
	}
	std::map<Instruction *, unsigned>::iterator it;
	for(it=instr_index_loop.begin();it!=instr_index_loop.end();++it)
	{
		int distance_store=0;
		int first_i_store=0;
		Instruction *tmp_inst=it->first;
		unsigned tmp_index=instr_index_loop[tmp_inst];
		if(isa<StoreInst>(tmp_inst)){
			ls_flag=load_store_dependency(inst,tmp_inst);
			if(ls_flag==true){
				distance=find_distance(LI,inst,tmp_inst);
				GetElementPtrInst *gep_store=get_GEP(tmp_inst);
				if(g_phi!=10){
					if(gep_store!=NULL){
						Value *s_op=gep_store->getOperand(g_phi);
						first_i_store=compute_gep_operand(s_op,false,0);
					}
					distance_store=first_i_store-first_iteration;
				}
				if(distance_store!=distance_i){
					 int total_bank=partition_factor(inst);
					 BasicBlock *bb=inst->getParent();
					 Loop *loop=LI->getLoopFor(bb);
					 int unroll_factor=total_unroll_factor_ls(LI,loop,inst);
					 int load_in_one_bank=partition_mem_op_num(LI,bb,inst);
					 int load_total=compute_BB_load_num(bb,inst)*unroll_factor;
					 Value *array=get_array_name(inst);
					 int array_index=array_number[array];
					 int dim=array_dimension[array_index];
					 int initial=0;
					 for(int j=0; j<array_index; ++j){
						 initial += array_dimension[j];
					 }
					 int size=1;
					 for(int i=0;i<dim;++i){
						 int j=initial+i;
						 size*=array_size[j];
					 }
					 if(load_total>size){
						 load_total=size;
					 }
					 int bank_used=(int)ceil((float)load_total/(float)load_in_one_bank);
					 if(bank_used<total_bank){//incur loop-carried dependency, because loop will access all the banks which contains the banks used in the next iteration.
						 latency=find_latency(inst,tmp_inst,instr_index_loop,dependence_loop)+mux_latency(LI,bb,inst);
						 recMII_tmp=(int)ceil((float)latency/1.0);
						 recMII=std::max(recMII,recMII_tmp);
					 }
					 else{
						 continue;
					 }
				}
				else{
					float latency_tmp=find_latency(inst,tmp_inst,instr_index_loop,dependence_loop);
					std::map<unsigned, float> loop_inst_till_latency;
					float cp=loop_solveCP(dependence_loop,loop_inst_till_latency);
					if(cp<0.0){
						errs()<<"Please check4...\n";
					}
					//errs()<<"latency_tmp: "<<latency_tmp<<"\n";
					if(latency_tmp==0.0){
						float a=loop_inst_till_latency[index_ls];
						float b=loop_inst_till_latency[tmp_index];
						if(add_index_loop==true){
							if(a<b){
								if(distance<0){
									latency=b-a;
								}
								else{
									latency=1.0;
								}
							}
							else if(a>b){
								if(distance>0){
									latency=a-b;
								}
								else{
									latency=1.0;
								}
							}
							else{
								latency=1.0;
							}
						}
						else{
							if(a<b){
								if(distance>0){
									latency=b-a;
								}
								else{
									latency=1.0;
								}
							}
							else if(a>b){
								if(distance<0){
									latency=a-b;
								}
								else{
									latency=1.0;
								}
							}
							else{
								latency=1.0;
							}

						}
					}
					else{
						latency=latency_tmp;
					}
					//errs()<<"latency: "<<latency<<"\n";

					if(add_index_loop==true){
						if(distance<0){
							distance=-distance;
							recMII_tmp=(int)ceil((float)latency/(float)distance);
						}
						else{
							recMII_tmp=1;
						}
					}
					else{
						if(distance>0){
							recMII_tmp=(int)ceil((float)latency/(float)distance);
						}
						else{
							recMII_tmp=1;
						}
					}
					recMII=std::max(recMII,recMII_tmp);
				}
			}
			else{
				recMII=std::max(recMII,1);
			}
		}
	}
	return recMII;
}


//If read from and write to the same array, Vivado HLS will add them together to calculate the ResMII;
//If read from and write to different arrays, Vivado HLS will use the maximum one to calculate.
int find_ResMII(LoopInfo* LI, Loop *L, std::map<Instruction*, unsigned> &load_order_buff)
{
	int resMII=1;
	std::map<Value*, int> array_reads_number, array_writes_number;
	std::map<Value*, int> loop_array_rw;
	std::map<Value*, int> loop_array_rw_tmp;
	std::map<Value*, bool> loop_array_ls;
	std::map<Value*, int> sdp_mode;
	int num_max=0;
	int num_max_tmp=0;

	for(Loop::block_iterator BI=L->block_begin(), BE=L->block_end(); BI !=BE; ++BI)
	{
		BasicBlock *bb=*BI;
		Loop *L_bb=LI->getLoopFor(bb);
		for(auto iti=bb->begin();iti!=bb->end();++iti){
			if(isa<LoadInst>(iti)){
				int unroll_factor=total_unroll_factor_ls(LI,L_bb,iti);
				//errs()<<unroll_factor<<"\n";
				bool array_partition_is_set=false;
				Value *array0=get_array_name(iti);
				int size=1;
				if(array_number.count(array0)){
					 int array_index=array_number[array0];
					 int dim=array_dimension[array_index];
					 int initial=0;
					 for(int j=0; j<array_index; ++j){
						 initial += array_dimension[j];
					 }
					 for(int i=0;i<dim;++i){
						 int j=initial+i;
						 size*=array_size[j];
					 }
				}
				if(!loop_array_ls.count(array0)){
					bool ls=false;
					for(auto ii=bb->begin();ii!=bb->end();++ii){
						if(isa<StoreInst>(ii)){
							Value *store_array=get_array_name(ii);
							if(array0==store_array){
								loop_array_ls[array0]=true;
								ls=true;
								break;
							}
						}
					}
					if(ls==false){
						loop_array_ls[array0]=false;
					}
				}
				if(loop_array_ls[array0]==true){
					if(!sdp_mode.count(array0)){
						int total_bank=partition_factor(iti);
						int load_in_one_bank=partition_mem_op_num(LI,bb,iti);
						int load_total=compute_BB_load_num(bb,iti)*unroll_factor;
						if(load_total>size){
							load_total=size;
						}
						int required_bank=(int)ceil((float)load_total/(float)load_in_one_bank);
						if(required_bank==total_bank){
							if(load_total==total_bank){
								sdp_mode[array0]=1;//The memory is SDP mode.
							}
							else{
								sdp_mode[array0]=0;//The estimated clock period may exceeds the target. Also, II is larger than expected (addition).
							}
						}
						else{
							sdp_mode[array0]=2;//compute as usual.
						}
					}
				}
				int num_reads=0;
				if(array_number.count(array0)){
					int array_index=array_number[array0];
					for(std::vector< std::pair<int,int> >::iterator it= array_partition[array_index].begin(); it!=array_partition[array_index].end(); ++it)
					{
						if(it->second!=1){
							array_partition_is_set=true;
							break;
						}
					}
					if(array_partition_is_set==false){
						num_reads=compute_BB_load_num(bb,iti)*unroll_factor;
						if(num_reads>size){
							num_reads=size;
						}
					}
					else{
						num_reads=partition_mem_op_num(LI,bb,iti);
					}
					array_reads_number[array0]=std::max(array_reads_number[array0],num_reads);
				}
			}
			else if(isa<StoreInst>(iti)){
				int unroll_factor=total_unroll_factor_ls(LI,L_bb,iti);
				bool array_partition_is_set=false;
				Value *array1=get_array_name(iti);
				int num_writes=0;
				if(array_number.count(array1)){
					int array_index=array_number[array1];
					for(std::vector< std::pair<int,int> >::iterator it= array_partition[array_index].begin(); it!=array_partition[array_index].end(); ++it)
					{
						if(it->second!=1){
							array_partition_is_set=true;
							break;
						}
					}
					if(array_partition_is_set==false){
						num_writes=compute_BB_store_num(bb,iti)*unroll_factor;
					}
					else{
						num_writes=partition_mem_op_num(LI,bb,iti);
					}
					array_writes_number[array1]=std::max(array_writes_number[array1],num_writes);
				}
			}
		}
	}
	float iteration_latency=0.0;
	iteration_latency=round(update_loopCP(LI,L,load_order_buff));
	std::map<Value*, int>::iterator it;
	int res_ii_read_max=0;
	int res_ii_write_max=0;
	for(it=array_reads_number.begin();it!=array_reads_number.end();++it){
		int res_ii=(int)(ceil)((float)it->second/(float)port_num_read);
		res_ii_read_max=std::max(res_ii,res_ii_read_max);
	}
	for(it=array_writes_number.begin();it!=array_writes_number.end();++it){
		int res_ii=(int)(ceil)((float)it->second/(float)port_num_write_diff);
		res_ii_write_max=std::max(res_ii,res_ii_write_max);
	}
	for(it=array_reads_number.begin();it!=array_reads_number.end();++it){
		Value* array=it->first;
		loop_array_rw_tmp[array]=res_ii_read_max;
	}
	for(it=array_writes_number.begin();it!=array_writes_number.end();++it){
		Value* array=it->first;
		if(loop_array_ls.count(array)){
			if(loop_array_ls[array]==true){
				if(sdp_mode.count(array)){
					if(sdp_mode[array]==1){
						loop_array_rw_tmp[array] = std::max(loop_array_rw_tmp[array],res_ii_write_max);
					}
					else{
						loop_array_rw_tmp[array] += res_ii_write_max;
					}
				}
				else{
					errs()<<"Please check: something goes wrong!\n";
				}
			}
			else{
				loop_array_rw_tmp[array] += res_ii_write_max;
			}
		}
		else{
			loop_array_rw_tmp[array] += res_ii_write_max;
		}
	}
	for(it=loop_array_rw_tmp.begin();it!=loop_array_rw_tmp.end();++it){
		int num=it->second;
		num_max_tmp=std::max(num_max_tmp,num);
	}
	int delta=(int)iteration_latency-num_max_tmp;
	if(delta<0){
		delta=-delta;
	}
	if(delta<2){
		resMII=num_max_tmp;
	}
	else{
		for(it=array_reads_number.begin();it!=array_reads_number.end();++it){
			Value* array=it->first;
			int res_ii=(int)(ceil)((float)it->second/(float)port_num_read);
			loop_array_rw[array]=res_ii;
		}
		for(it=array_writes_number.begin();it!=array_writes_number.end();++it){
			Value* array=it->first;
			int res_ii=(int)(ceil)((float)it->second/(float)port_num_write_diff);
			if(loop_array_ls.count(array)){
				if(loop_array_ls[array]==true){
					if(sdp_mode.count(array)){
						if(sdp_mode[array]==1){
							loop_array_rw[array] = std::max(loop_array_rw[array],res_ii);
						}
						else{
							loop_array_rw[array] += res_ii;
						}
					}
					else{
						errs()<<"Please check: something goes wrong!\n";
					}
				}
				else{
					loop_array_rw[array] += res_ii;
				}
			}
			else{
				loop_array_rw[array] += res_ii;
			}
		}
		for(it=loop_array_rw.begin();it!=loop_array_rw.end();++it){
			int num=it->second;
			num_max=std::max(num_max,num);
		}
		resMII= num_max;
	}
	return resMII;
}


