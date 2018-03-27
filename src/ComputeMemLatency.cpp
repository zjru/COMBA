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


#include "ComputeMemLatency.h"
#include "LoopUnrollFactor.h"
#include "GEP.h"
#include "ArrayName.h"
#include "PartitionNum.h"
#include "FindLatency.h"
#include "Library.h"
#include "PartitionFactor.h"
#include "MemOpNum.h"
#include "Constant.h"
#include "Cast.h"
#include "Power.h"


//Array partition influence the number of memory operations.
int partition_load_num(LoopInfo *LI, BasicBlock *bb, Instruction *inst, std::map<Instruction*, unsigned> &instr_index_loop, std::map<Instruction*, unsigned> &load_order_buff)
{
	int num=0;
	Value *array_name=get_array_name(inst);
	GetElementPtrInst *gep1=get_GEP(inst);
	Loop *L=LI->getLoopFor(bb);
	int inst_offset=0;
	int same_bank_factor=1;
	bool same=false;
	std::vector<int> array_element_counted;
	if(gep1!=NULL){
		for(User::op_iterator ii=gep1->idx_begin(), ie=gep1->idx_end();ii!=ie; ++ii){
			Value *op_l=*ii;
			if(Instruction *op_inst=dyn_cast<Instruction>(op_l)){
				BasicBlock *bb1=op_inst->getParent();
				Loop *l1=LI->getLoopFor(bb1);
				bool related=unroll_loop_relation(L,l1);
				if(related==true){
					int a=get_unroll_SameBank_num(bb1,inst,inst,array_element_counted);
					if(a!=0){
						same_bank_factor *= a;
						same=true;
					}
					inst_offset=compute_array_element_offset(inst,bb1,0);
				}
			}
			else{
				BasicBlock *bb1=gep1->getParent();
				Loop *l1=LI->getLoopFor(bb1);
				bool related=unroll_loop_relation(L,l1);
				if(related==true){
					int a=get_unroll_SameBank_num(bb1,inst,inst,array_element_counted);
					if(a!=0){
						same_bank_factor *= a;
						same=true;
					}
					inst_offset=compute_array_element_offset(inst,bb1,0);
				}
			}
		}
	}
	if(same==true){
		num += same_bank_factor;
	}
	array_element_counted.push_back(inst_offset);
	unsigned reorder_index=0;
	if(load_order_buff.count(inst)){
		reorder_index=load_order_buff[inst];
		for(std::map<Instruction*, unsigned>::iterator it=load_order_buff.begin();it!=load_order_buff.end();++it){
			Instruction *inst1=it->first;
			unsigned reorder_index1=it->second;
			if(reorder_index1<reorder_index){
				BasicBlock *bb1=inst1->getParent();
				if(bb1==bb){
					Value *array_name_tmp=get_array_name(inst1);
					if(array_name==array_name_tmp){
						GetElementPtrInst *gep=get_GEP(inst1);
						if(gep!=gep1){
							if(gep!=NULL){
								int same_factor=1;
								bool same1=false;
								for(User::op_iterator ii=gep->idx_begin(), ie=gep->idx_end();ii!=ie; ++ii){
									Value *op_l=*ii;
									if(Instruction *op_inst=dyn_cast<Instruction>(op_l)){
										BasicBlock *bb2=op_inst->getParent();
										Loop *l2=LI->getLoopFor(bb2);
										bool related=unroll_loop_relation(L,l2);
										if(related==true){
											int b=get_unroll_SameBank_num(bb2,inst,inst1,array_element_counted);
											if(b!=0){
												same_factor *= b;
												same1=true;
											}
										}
									}
									else{
										BasicBlock *bb2=gep->getParent();
										Loop *l2=LI->getLoopFor(bb2);
										bool related=unroll_loop_relation(L,l2);
										if(related==true){
											 int b=get_unroll_SameBank_num(bb2,inst,inst1,array_element_counted);
											 if(b!=0){
												 same_factor *= b;
												 same1=true;
											 }
										}
									}
								}
								if(same1==true){
									num += same_factor;
								}
							}
						}
					}
				}
			}
		}
	}
	else{
		errs()<<"This load is not found in this function.\n";
		for(std::map<Instruction*, unsigned>::iterator it=instr_index_loop.begin();it!=instr_index_loop.end();++it)
		{
			Instruction *iti=it->first;
			BasicBlock *itb=iti->getParent();
			if(isa<LoadInst>(iti)){
				if(itb==bb){
					Value *array_name_tmp=get_array_name(iti);
					if(array_name==array_name_tmp){
						GetElementPtrInst *gep=get_GEP(iti);
						if(gep!=gep1){
							if(gep!=NULL){
								int same_factor=1;
								bool same2=false;
								for(User::op_iterator ii=gep->idx_begin(), ie=gep->idx_end();ii!=ie; ++ii){
									Value *op_l=*ii;
									if(Instruction *op_inst=dyn_cast<Instruction>(op_l)){
										BasicBlock *bb2=op_inst->getParent();
										Loop *l2=LI->getLoopFor(bb2);
										bool related=unroll_loop_relation(L,l2);
										if(related==true){
											 int b=get_unroll_SameBank_num(bb2,inst,iti,array_element_counted);
											 if(b!=0){
												 same_factor *= b;
												 same2=true;
											 }
										}
									}
									else{
										BasicBlock *bb2=gep->getParent();
										Loop *l2=LI->getLoopFor(bb2);
										bool related=unroll_loop_relation(L,l2);
										if(related==true){
											 int b=get_unroll_SameBank_num(bb2,inst,iti,array_element_counted);
											 if(b!=0){
												 same_factor *= b;
												 same2=true;
											 }
										}
									}
								}
								if(same2==true){
									num += same_factor;
								}
							}
						}
					}
				}
			}
		}
	}
	if(num==0){
		num=1;
	}
	return num;
}

int partition_store_num(LoopInfo* LI, BasicBlock *bb, Instruction *inst, unsigned &index_loop, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop)
{
	int num=0;
	float latency_before_inst=find_latency_before_inst(inst,instr_index_loop,dependence_loop);
	Value *array_name=get_array_name(inst);
	Loop *L=LI->getLoopFor(bb);
	int unroll_factor=total_unroll_factor_ls(LI,L,inst);
	GetElementPtrInst *gep1=get_GEP(inst);
	std::vector<int> array_element_counted;
	int inst_offset=0;
	int same_bank_factor=1;
	bool same=false;
	if(gep1!=NULL){
		for(User::op_iterator ii=gep1->idx_begin(), ie=gep1->idx_end();ii!=ie; ++ii){
			Value *op_s=*ii;
			if(Instruction *op_inst=dyn_cast<Instruction>(op_s)){
				BasicBlock *bb1=op_inst->getParent();
				Loop *l1=LI->getLoopFor(bb1);
				bool related=unroll_loop_relation(L,l1);
				if(related==true){
					int a=get_unroll_SameBank_num(bb1,inst,inst,array_element_counted);
					if(a!=0){
						same_bank_factor *= a;
						same=true;
					}
					inst_offset=compute_array_element_offset(inst,bb1,0);
				}
			}
			else{
				BasicBlock *bb1=gep1->getParent();
				Loop *l1=LI->getLoopFor(bb1);
				bool related=unroll_loop_relation(L,l1);
				if(related==true){
					int a=get_unroll_SameBank_num(bb1,inst,inst,array_element_counted);
					if(a!=0){
						same_bank_factor *= a;
						same=true;
					}
					inst_offset=compute_array_element_offset(inst,bb1,0);
				}
			}
		}
	}
	if(same==true){
		num += same_bank_factor;
	}
	array_element_counted.push_back(inst_offset);

	for(std::map<Instruction*, unsigned>::iterator it=instr_index_loop.begin();it!=instr_index_loop.end();++it)
	{
		Instruction *inst1=it->first;
		if(isa<StoreInst>(inst1)){
			if(inst!=inst1){
				BasicBlock *bb1=inst1->getParent();
				if(bb1==bb){
					Value *address=get_array_name(inst1);
					if(address==array_name){
						GetElementPtrInst *gep=get_GEP(inst1);
						if(gep!=gep1){
							float latency_after_store=find_latency_after_inst(inst1,instr_index_loop,dependence_loop);
							float latency_before_store=find_latency_before_inst(inst1,instr_index_loop,dependence_loop);
							if(unroll_factor==1){
								if((latency_before_inst>latency_before_store)&&(latency_before_inst<latency_after_store)){
									if(gep!=NULL){
										int same_factor=1;
										bool same1=false;
										for(User::op_iterator ii=gep->idx_begin(), ie=gep->idx_end();ii!=ie; ++ii){
											 Value *op_s=*ii;
											 if(Instruction *op_inst=dyn_cast<Instruction>(op_s)){
												 BasicBlock *bb2=op_inst->getParent();
												 Loop *l2=LI->getLoopFor(bb2);
												 bool related=unroll_loop_relation(L,l2);
												 if(related==true){
													 int b=get_unroll_SameBank_num(bb2,inst,inst1,array_element_counted);
													 if(b!=0){
														 same_factor *= b;
														 same1=true;
													 }
												 }
											 }
											 else{
												 BasicBlock *bb2=gep->getParent();
												 Loop *l2=LI->getLoopFor(bb2);
												 bool related=unroll_loop_relation(L,l2);
												 if(related==true){
													 int b=get_unroll_SameBank_num(bb2,inst,inst1,array_element_counted);
													 if(b!=0){
														 same_factor *= b;
														 same1=true;
													 }
												 }
											 }
										}
										if(same1==true){
											num += same_factor;
										}
									}
								}
								else if(latency_before_inst==latency_before_store){
									if(index_loop>instr_index_loop[inst1]){
										if(gep!=NULL){
											 int same_factor=1;
											 bool same1=false;
											 for(User::op_iterator ii=gep->idx_begin(), ie=gep->idx_end();ii!=ie; ++ii){
												 Value *op_s=*ii;
												 if(Instruction *op_inst=dyn_cast<Instruction>(op_s)){
													 BasicBlock *bb2=op_inst->getParent();
													 Loop *l2=LI->getLoopFor(bb2);
													 bool related=unroll_loop_relation(L,l2);
													 if(related==true){
														 int b=get_unroll_SameBank_num(bb2,inst,inst1,array_element_counted);
														 if(b!=0){
															 same_factor *= b;
															 same1=true;
														 }
													 }
												 }
												 else{
													 BasicBlock *bb2=gep->getParent();
													 Loop *l2=LI->getLoopFor(bb2);
													 bool related=unroll_loop_relation(L,l2);
													 if(related==true){
														 int b=get_unroll_SameBank_num(bb2,inst,inst1,array_element_counted);
														 if(b!=0){
															 same_factor *= b;
															 same1=true;
														 }
													 }
												 }
											 }
											 if(same1==true){
												 num += same_factor;
											 }
										}
									}
								}
							}
							else{
								if(latency_before_inst>latency_before_store){
									if(gep!=NULL){
										int same_factor=1;
										bool same1=false;
										for(User::op_iterator ii=gep->idx_begin(), ie=gep->idx_end();ii!=ie; ++ii){
											 Value *op_s=*ii;
											 if(Instruction *op_inst=dyn_cast<Instruction>(op_s)){
												 BasicBlock *bb2=op_inst->getParent();
												 Loop *l2=LI->getLoopFor(bb2);
												 bool related=unroll_loop_relation(L,l2);
												 if(related==true){
													 int b=get_unroll_SameBank_num(bb2,inst,inst1,array_element_counted);
													 if(b!=0){
														 same_factor *= b;
														 same1=true;
													 }
												 }
											 }
											 else{
												 BasicBlock *bb2=gep->getParent();
												 Loop *l2=LI->getLoopFor(bb2);
												 bool related=unroll_loop_relation(L,l2);
												 if(related==true){
													 int b=get_unroll_SameBank_num(bb2,inst,inst1,array_element_counted);
													 if(b!=0){
														 same_factor *= b;
														 same1=true;
													 }
												 }
											 }
										}
										if(same1==true){
											num += same_factor;
										}
									}
								}
								else if(latency_before_inst==latency_before_store){
									if(index_loop>instr_index_loop[inst1]){
										if(gep!=NULL){
											 int same_factor=1;
											 bool same1=false;
											 for(User::op_iterator ii=gep->idx_begin(), ie=gep->idx_end();ii!=ie; ++ii){
												 Value *op_s=*ii;
												 if(Instruction *op_inst=dyn_cast<Instruction>(op_s)){
													 BasicBlock *bb2=op_inst->getParent();
													 Loop *l2=LI->getLoopFor(bb2);
													 bool related=unroll_loop_relation(L,l2);
													 if(related==true){
														 int b=get_unroll_SameBank_num(bb2,inst,inst1,array_element_counted);
														 if(b!=0){
															 same_factor *= b;
															 same1=true;
														 }
													 }
												 }
												 else{
													 BasicBlock *bb2=gep->getParent();
													 Loop *l2=LI->getLoopFor(bb2);
													 bool related=unroll_loop_relation(L,l2);
													 if(related==true){
														 int b=get_unroll_SameBank_num(bb2,inst,inst1,array_element_counted);
														 if(b!=0){
															 same_factor *= b;
															 same1=true;
														 }
													 }
												 }
											 }
											 if(same1==true){
												 num += same_factor;
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
	}
	if(num==0){
	num=1;
	}
	return num;
}


//compute the latency of load instruction according to asap strategy.
int compute_asap_load_num(BasicBlock *bb, Instruction *inst, std::map<Instruction*, unsigned> &instr_index_loop, std::map<Instruction*, unsigned> &load_order_buff)
{
	int load_num=1;
	Value *address_index_tmp=NULL;
	Value *address_index=NULL;
	address_index=get_array_name(inst);
	unsigned reorder_index=0;
	GetElementPtrInst *gep=get_GEP(inst);
	if(load_order_buff.count(inst)){
		reorder_index=load_order_buff[inst];
		for(std::map<Instruction*, unsigned>::iterator it=load_order_buff.begin();it!=load_order_buff.end();++it){
			Instruction *inst1=it->first;
			GetElementPtrInst *gep1=get_GEP(inst1);
			if(gep1!=gep){
				unsigned reorder_index1=it->second;
				if(reorder_index1<reorder_index){
					BasicBlock *bb1=inst1->getParent();
					if(bb1==bb){
						address_index_tmp=get_array_name(inst1);
						if(address_index_tmp==address_index){
							load_num++;
						}
					}

				}
			}
		}
	}
	else{
		errs()<<"This load is not found in this function.\n";
		for(std::map<Instruction*, unsigned>::iterator it=instr_index_loop.begin();it!=instr_index_loop.end();++it)
		{
			Instruction *inst1=it->first;
			BasicBlock *bb1=inst1->getParent();
			if(bb1==bb){
				if(isa<LoadInst>(inst1)){
					address_index_tmp=get_array_name(inst1);
					if(address_index_tmp==address_index){
						GetElementPtrInst *gep1=get_GEP(inst1);
						if(gep!=gep1){
							load_num++;
						}
					}
				}
			}
		}
	}
	return load_num;

}


int compute_asap_store_num(LoopInfo* LI, BasicBlock *bb, Instruction *inst, unsigned &index_loop, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop)
{
	int num_writes=1;
	float latency_before_inst=0.0;
	Value *array_name=get_array_name(inst);
	latency_before_inst=find_latency_before_inst(inst,instr_index_loop,dependence_loop);
	Loop *L=LI->getLoopFor(bb);
	int unroll_factor=total_unroll_factor_ls(LI,L,inst);
	GetElementPtrInst *gep=get_GEP(inst);

	//consider store into the same array with limited ports.
	for(std::map<Instruction*, unsigned>::iterator it=instr_index_loop.begin();it!=instr_index_loop.end();++it)
	{
		float latency_before_store=0.0;
		float latency_after_store=0.0;
		Instruction *inst1=it->first;
		if(isa<StoreInst>(inst1)){
			if(inst!=inst1){
				BasicBlock *bb1=inst1->getParent();
				if(bb1==bb){
					Value *address=get_array_name(inst1);
					if(address==array_name){
						GetElementPtrInst *gep1=get_GEP(inst1);
						if(gep1!=gep){
							latency_before_store=find_latency_before_inst(inst1,instr_index_loop,dependence_loop);
							latency_after_store=find_latency_after_inst(inst1,instr_index_loop,dependence_loop);
							if(unroll_factor==1){
								if((latency_before_inst>latency_before_store)&&(latency_before_inst<latency_after_store)){
									num_writes++;
								}
								else if(latency_before_inst==latency_before_store){
									if(index_loop>instr_index_loop[inst1]){
										num_writes++;
									}
								}
							}
							else{
								if(latency_before_inst>latency_before_store){
									num_writes++;
								}
								else if(latency_before_inst==latency_before_store){
									if(index_loop>instr_index_loop[inst1]){
										num_writes++;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return num_writes;

}


float mux_latency(LoopInfo* LI, BasicBlock *bb, Instruction *inst)
{
	float mux_delay=0.0;
	Loop *L=LI->getLoopFor(bb);
	int unroll_factor=total_unroll_factor_ls(LI,L,inst);
	Value *array=get_array_name(inst);
	bool array_partition_is_set=false;
	if(array_number.count(array)){
		int array_index=array_number[array];
		for(std::vector< std::pair<int,int> >::iterator it= array_partition[array_index].begin(); it!=array_partition[array_index].end(); ++it)
		{
			if(it->second!=1){
				array_partition_is_set=true;
				break;
			}
		}
	}
	if(array_partition_is_set==true){
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
		int mux_stage=0;
		float mux_cycle=INT_ADD;
		//more load/store than what we need will use multiplexer/switch, and generate cost.
		if(isa<LoadInst>(inst)){
			int total_bank=partition_factor(inst);
			int load_in_one_bank=partition_mem_op_num(LI,bb,inst);
			int load_total=compute_BB_load_num(bb,inst)*unroll_factor;
			if(load_total>size){
				load_total=size;
			}
			int required_bank=(int)ceil((float)load_total/(float)load_in_one_bank);
			bool ls=false;
			for(auto iti=bb->begin();iti!=bb->end();++iti){
				if(isa<StoreInst>(iti)){
					Value *store_array=get_array_name(iti);
					if(array==store_array){
						ls=true;
						break;
					}
				}
			}
			if(required_bank==total_bank){//best case: no delay
				mux_delay=0.0;
				if(total_bank==size){
					mux_delay+=-1.0;
				}
				if(ls==true){
					if(load_total!=total_bank){//True dual-port memory, delay cost. (by experiments) eg. 4 banks for 8 elements.
						mux_delay+=mux_cycle*6;
					}
				}
			}
			else{
				if(total_bank==size){//when the array is partitioned completely, the cost of multiplexers needs to be considered.
					mux_stage=get_power_of_two(size);
					mux_delay+=ceil(mux_cycle*mux_stage)*4;
				}
				else{//multiplexers are needed
					mux_stage=get_power_of_two(total_bank);
					if(frequency==100){
						mux_delay+=ceil(mux_cycle*mux_stage)*2;
					}
					else if(frequency==125){
						mux_delay+=ceil(mux_cycle*mux_stage)*2;
					}
					else if(frequency==150){
						mux_delay+=ceil(mux_cycle*mux_stage)*2;
					}
					else if(frequency==200){
						mux_delay+=ceil(mux_cycle*mux_stage)*2;
					}
					else if(frequency==250){
						mux_delay+=ceil(mux_cycle*mux_stage)*2;
					}
				}
			}
		}
		else if(isa<StoreInst>(inst))
		{
			int total_bank=partition_factor(inst);
			int store_in_one_bank=partition_mem_op_num(LI,bb,inst);
			int store_total=compute_BB_store_num(bb,inst)*unroll_factor;
			if(store_total>size){
				store_total=size;
			}
			int required_bank=(int)ceil((float)store_total/(float)store_in_one_bank);//actual #bank with required data.
			bool ls=false;
			for(auto iti=bb->begin();iti!=bb->end();++iti){
				if(isa<LoadInst>(iti)){
					Value *load_array=get_array_name(iti);
					if(array==load_array){
						ls=true;
						break;
					}
				}
			}
			if(required_bank==total_bank){//best case: no delay
				mux_delay=0.0;
				if(ls==true){
					if(store_total!=total_bank){
						mux_delay+=mux_cycle*6;
					}
				}
			}
			else{
				if(total_bank==size){//partition completely;
					mux_stage=get_power_of_two(size);
					mux_delay+=ceil(mux_cycle*mux_stage)*4;
				}
				else{//not good, may cause delay
					mux_stage=get_power_of_two(total_bank);
					if(frequency==100){
						mux_delay+=ceil(mux_cycle*mux_stage)*2;
					}
					else if(frequency==125){
						mux_delay+=ceil(mux_cycle*mux_stage)*2;
					}
					else if(frequency==150){
						mux_delay+=ceil(mux_cycle*mux_stage)*2;
					}
					else if(frequency==200){
						mux_delay+=ceil(mux_cycle*mux_stage)*2;
					}
					else if(frequency==250){
						mux_delay+=ceil(mux_cycle*mux_stage)*2;
					}
				}
			}
		}
	}
	else{
		mux_delay=0.0;
	}
	return mux_delay;
}


float get_load_latency(LoopInfo* LI, Instruction *inst, std::map<Instruction*, unsigned> &instr_index_loop, std::map<Instruction*, unsigned> &load_order_buff)
{
	bool array_partition_is_set=false;
	//compute latency of load at first, then compute_critical_path
	float load_latency=1.0;
	BasicBlock *bb=inst->getParent();
	Loop *L=LI->getLoopFor(bb);
	int unroll_factor=1;
	unroll_factor=total_unroll_factor_ls(LI,L,inst);
	int num_r=0;
	Value *array_name=get_array_name(inst);
	if(array_number.count(array_name)){
		int array_index=array_number[array_name];
		for(std::vector< std::pair<int,int> >::iterator it= array_partition[array_index].begin(); it!=array_partition[array_index].end(); ++it)
		{
			if(it->second!=1){
				array_partition_is_set=true;
				break;
			}
		}
	}
	if(array_partition_is_set==false){
		num_r=compute_asap_load_num(bb,inst,instr_index_loop,load_order_buff)*unroll_factor;
		load_latency= 1.0 + ceil((float)num_r/(float)port_num_read);
	}
	else{
		num_r=partition_load_num(LI,bb,inst,instr_index_loop,load_order_buff);
		load_latency= 1.0 + ceil((float)num_r/(float)port_num_read)+mux_latency(LI,bb,inst);
	}
	return load_latency;
}


bool load_before_store_partitioned(LoopInfo* LI, Instruction *inst, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop)
{
	 bool partitioned=false;
	 BasicBlock *bb=inst->getParent();
	 if(StoreInst *store=dyn_cast<StoreInst>(inst)){
		 Value *op=store->getValueOperand();
		 if(Instruction *op_inst=dyn_cast<Instruction>(op)){
			 for(auto iti=bb->begin();iti!=bb->end();++iti){
				 if(isa<LoadInst>(iti)){
					 float latency=find_latency(iti,op_inst,instr_index_loop,dependence_loop);
					 if(latency>0.0){
						 bool array_partition_is_set=false;
			             Value *array_name=get_array_name(iti);
			             if(array_number.count(array_name)){
			            	 int array_index=array_number[array_name];
			            	 for(std::vector< std::pair<int,int> >::iterator it= array_partition[array_index].begin(); it!=array_partition[array_index].end(); ++it)
			            	 {
			            		 if(it->second!=1){
			            			 array_partition_is_set=true;
			            			 break;
			            		 }
			            	 }
			             }
						 if(array_partition_is_set==true){
							 float mux=mux_latency(LI,bb,iti);
							 if(mux<=0.0){
								 partitioned=true;
							 }
							 else{
								 partitioned=false;
								 break;
							 }
						 }
						 else{
							 partitioned=false;
							 break;
						 }
					 }
				 }
			 }
		 }
	 }
	 return partitioned;
}

//compute the latency of store in the last copy(consider loop unrolling)in one iteration.
float get_store_latency(LoopInfo* LI, Instruction *inst, unsigned &index_loop, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop)
{
	bool array_partition_is_set=false;
	Value *array_name=get_array_name(inst);
	BasicBlock *bb=inst->getParent();
	bool bb_load_array=false;
	for(auto iti=bb->begin();iti!=bb->end();++iti){
		if(isa<LoadInst>(iti)){
			bb_load_array=true;
			break;
		}
	}
	//consider store into and load from the same array with port-conflict.
	//consider no load in this block and with load in this block
	float store_latency=0.0;
	int unroll_factor=1;
	int num_w=0;
	if(array_number.count(array_name)){
		int array_index=array_number[array_name];
		for(std::vector< std::pair<int,int> >::iterator it= array_partition[array_index].begin(); it!=array_partition[array_index].end(); ++it)
		{
			if(it->second!=1){
				array_partition_is_set=true;
				break;
			}
		}
	}
	if(bb_load_array==true){
		 bool load_partitioned=load_before_store_partitioned(LI,inst,instr_index_loop,dependence_loop);
		 if(load_partitioned==false){
			 unroll_factor=1;
		 }
		 else{
			 Loop *L=LI->getLoopFor(bb);
			 unroll_factor=total_unroll_factor_ls(LI,L,inst);
		 }
	}
	else{
		Loop *L=LI->getLoopFor(bb);
		unroll_factor=total_unroll_factor_ls(LI,L,inst);
	}
	if(array_partition_is_set==false){
		num_w=compute_asap_store_num(LI,bb,inst,index_loop,instr_index_loop,dependence_loop)*unroll_factor;
		store_latency = ceil((float)num_w/(float)port_num_write_diff);
	}
	else{
		num_w=partition_store_num(LI,bb,inst,index_loop,instr_index_loop,dependence_loop);
		store_latency = ceil((float)num_w/(float)port_num_write_diff)+mux_latency(LI,bb,inst);
	}
	return store_latency;
}


PHINode *get_previous_phi(Instruction *inst)
{
	 PHINode *phi=NULL;
	 if(PHINode *phi_inst=dyn_cast<PHINode>(inst)){
		 phi=phi_inst;
	 }
	 else{
		 Value *op=inst->getOperand(0);
		 if(Instruction *op_inst=dyn_cast<Instruction>(op)){
			 phi=get_previous_phi(op_inst);
		 }
	 }
	 return phi;
}

bool loop_unroll_effective(Loop *L)
{
	 bool effective=false;
	 std::vector<Loop*> subLoops=L->getSubLoops();
	 if(subLoops.empty()){
		 effective=true;
	 }
	 else{
		 int subLoop_num = subLoops.size();
		 for(int j=0;j<subLoop_num;j++)
		 {
			 Loop* sub_loop=subLoops.at(j);
			 if(Loops_unroll[sub_loop]>=Loops_counter[sub_loop]){
				 effective=loop_unroll_effective(sub_loop);
			 }
		 }
	 }
	 return effective;
}

bool loop_carry_load_store_dependency(LoopInfo *LI, Instruction *load, Instruction *store)
{
	 bool lc_ls=false;
	 Value *load_name=get_array_name(load);
	 Value *store_name=get_array_name(store);
	 BasicBlock *bb=store->getParent();
	 int bb_total_store=compute_BB_store_num(bb,store);
	 GetElementPtrInst *gep_store=get_GEP(store);
	 if(gep_store!=NULL){
		 if(bb_total_store<=1){//for common cases...
			 if(load_name==store_name){
				 GetElementPtrInst *gep_load=get_GEP(load);
				 if(gep_load!=gep_store){
					 if(gep_store!=NULL && gep_load!=NULL){
       				 for(User::op_iterator II=gep_store->idx_begin(),IE=gep_store->idx_end(); II!=IE; ++II){
       					 if(Instruction *idx_store=dyn_cast<Instruction>(*II)){
       						 PHINode *phi=get_previous_phi(idx_store);
       						 if(phi!=NULL){
           						 BasicBlock *store_bb=phi->getParent();
           						 Loop *store_loop=LI->getLoopFor(store_bb);
           						 if(store_loop!=NULL){
               						 bool unroll_effective=loop_unroll_effective(store_loop);
               						 if(Loops_unroll[store_loop]>1 && unroll_effective==true){
               							 int store_operand=compute_gep_operand(*II,false,0);
               							 for(User::op_iterator III=gep_load->idx_begin(),IIE=gep_load->idx_end();III!=IIE;++III){
               								 if(Instruction *idx_load=dyn_cast<Instruction>(*III)){
               									 PHINode *phi_load=get_previous_phi(idx_load);
               									 if(phi_load==phi){
               										 int load_operand=compute_gep_operand(*III,false,0);
               										 int ls_diff=store_operand-load_operand;
               										 if(ls_diff>0){
               											 lc_ls=true;
               										 }
               										 break;
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
			 }
		 }
	 }
	 return lc_ls;
}

void reset_sameArray_store_latency(LoopInfo *LI, BasicBlock *bb, Instruction *inst, int unroll_factor, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop)
{
	Value *array_name=get_array_name(inst);
	bool load_store_SameArray=false;
	bool bb_load_array=false;
	float latency_after_inst=find_latency_after_inst(inst,instr_index_loop,dependence_loop);
	float largest_load_latency=0.0;
	for(auto iti=bb->begin();iti!=bb->end();++iti){
		if(isa<LoadInst>(iti)){
			bb_load_array=true;
			Value *load_array=get_array_name(iti);
			bool ls_lc=loop_carry_load_store_dependency(LI,iti,inst);
			if(ls_lc==false){
				if(load_array==array_name){
					load_store_SameArray=true;
					float load_latency_tmp=find_latency_after_inst(iti,instr_index_loop,dependence_loop);
					largest_load_latency=std::max(load_latency_tmp,largest_load_latency);
				}
			}
		}
	}
	if(bb_load_array==false){
		for(std::map<Instruction*, unsigned>::iterator it=instr_index_loop.begin();it!=instr_index_loop.end();++it){
			Instruction *load_inst=it->first;
			if(isa<LoadInst>(load_inst)){
				Value *load_array=get_array_name(load_inst);
				if(load_array==array_name){
					float load_latency_tmp=find_latency_after_inst(load_inst,instr_index_loop,dependence_loop);
					largest_load_latency=std::max(load_latency_tmp,largest_load_latency);
				}
			}
		}
	}
	if(largest_load_latency>1.0){
		largest_load_latency=largest_load_latency-1.0;
	}

    if(load_store_SameArray==true||bb_load_array==false)
    {
		float total_num=0.0;
		float one_num=0.0;
		bool array_partition_is_set=false;
		float inst_latency=Store_latency[inst];
		if(array_number.count(array_name)){
			int array_index=array_number[array_name];
			for(std::vector< std::pair<int,int> >::iterator it= array_partition[array_index].begin(); it!=array_partition[array_index].end(); ++it)
			{
				if(it->second!=1){
					array_partition_is_set=true;
					break;
				}
			}
		}
		if(array_partition_is_set==false){
			total_num=(float)(compute_BB_store_num(bb,inst)*unroll_factor);
			one_num=(float)compute_BB_store_num(bb,inst);
		}
		else{
			int ttt=partition_mem_op_num(LI,bb,inst);
			int bank_num=get_bank_number(inst);
			for(auto iti=bb->begin();iti!=bb->end();++iti){
				if(isa<StoreInst>(iti)){
					Value *array_store=get_array_name(iti);
					if(array_store==array_name){
						int bank_store=get_bank_number(iti);
						if(bank_store==bank_num){
							one_num++;
						}
					}
				}
			}
			total_num=ttt-one_num;
		}
		float oneminus_latency=ceil((float)one_num/(float)port_num_write_diff)+mux_latency(LI,bb,inst);
		float latency_of_all_store=0.0;
		float delta=0.0;
		if(Store_latency[inst]>oneminus_latency){
			latency_of_all_store=Store_latency[inst];
		}
		else{
			latency_of_all_store=floor((float)total_num/(float)port_num_write_diff) + mux_latency(LI,bb,inst) + Store_latency[inst];
			delta=ceil((float)one_num/(float)port_num_write_diff) + mux_latency(LI,bb,inst) - Store_latency[inst];
		}
		if(delta<0.0){
			errs()<<"Please check this...\n";
			delta=0.0;
		}
        float latency_initial=latency_after_inst-latency_of_all_store;
        if(latency_initial<0.0){
			inst_latency=latency_of_all_store;
			set_inst_latency(inst,inst_latency,instr_index_loop,dependence_loop);
        }
        else{
			if(latency_initial<largest_load_latency){
				inst_latency+=largest_load_latency-latency_initial+delta;
				set_inst_latency(inst,inst_latency,instr_index_loop,dependence_loop);
			}
			else{
				if(largest_load_latency==0.0){
					 if(bb_load_array==true){
						 //errs()<<"Please check1 if the load array is partitioned completely...\n";
					 }
					 inst_latency=latency_of_all_store;
					 set_inst_latency(inst,inst_latency,instr_index_loop,dependence_loop);
				}
			}
        }
    }
    /*load, store dependency may cause loop unrolling/pipelining not useful as expected
      Common cases are considered: loop index increase as i++
    */
    GetElementPtrInst *gep_store=get_GEP(inst);
    if(gep_store!=NULL && unroll_factor!=0){
        for(auto iti=bb->begin();iti!=bb->end();++iti){
			if(isa<LoadInst>(iti)){
				bool lc_ls=loop_carry_load_store_dependency(LI,iti,inst);
				if(lc_ls==true){
					GetElementPtrInst *gep_load=get_GEP(iti);
					if(gep_load!=NULL){
						for(User::op_iterator II=gep_store->idx_begin(),IE=gep_store->idx_end(); II!=IE; ++II){
							if(Instruction *idx_store=dyn_cast<Instruction>(*II)){
								PHINode *phi=get_previous_phi(idx_store);
								if(phi!=NULL){
									BasicBlock *itb=phi->getParent();
									Loop *itb_parent=LI->getLoopFor(itb);
									if(itb_parent!=NULL){
										bool unroll_effective=loop_unroll_effective(itb_parent);
										if(Loops_unroll[itb_parent]>1 && unroll_effective==true){
											int u_factor=Loops_unroll[itb_parent];
											int store_operand=compute_gep_operand(*II,false,0);
											for(User::op_iterator III=gep_load->idx_begin(),IIE=gep_load->idx_end();III!=IIE;++III){
												if(Instruction *idx_load=dyn_cast<Instruction>(*III)){
													PHINode *phi_load=get_previous_phi(idx_load);
													if(phi_load==phi){
														int load_operand=compute_gep_operand(*III,false,0);
														int ls_diff=store_operand-load_operand;
														if(ls_diff>0){
															int multiplier=(int)ceil((float)u_factor/(float)ls_diff)-1;
															float ls_latency=find_latency(iti,inst,instr_index_loop,dependence_loop);
															float delta_l_latency=multiplier*(ls_latency);
															float l_latency=delta_l_latency+get_inst_latency(iti,instr_index_loop,dependence_loop);
															set_inst_latency(iti,l_latency,instr_index_loop,dependence_loop);
														}
														break;
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
			}
        }
    }
}

void update_after_store(LoopInfo* LI, Instruction *inst, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop)
{
	BasicBlock *bb=inst->getParent();
	Value *array_name=get_array_name(inst);
	Loop *L=LI->getLoopFor(bb);
	int unroll_factor=total_unroll_factor_ls(LI,L,inst);
	unroll_factor=unroll_factor-1;
	float latency_before_inst=find_latency_before_inst(inst,instr_index_loop,dependence_loop);
	if(isa<StoreInst>(inst)){
		for(std::map<Instruction*, unsigned>::iterator it=instr_index_loop.begin();it!=instr_index_loop.end();++it){
			Instruction *inst1=it->first;
			if(isa<StoreInst>(inst1)){
				BasicBlock *bb1=inst1->getParent();
				if(bb==bb1){
					Value *address=get_array_name(inst1);
					if(array_name==address){
						if(inst!=inst1){
							float latency_before_store=find_latency_before_inst(inst1,instr_index_loop,dependence_loop);
							if((latency_before_store>latency_before_inst)){
								unsigned index1=instr_index_loop[inst1];
								float store_latency=get_store_latency(LI,inst1,index1,instr_index_loop,dependence_loop);
								Store_latency[inst1]=store_latency;
								set_inst_latency(inst1,store_latency,instr_index_loop,dependence_loop);
								reset_sameArray_store_latency(LI,bb,inst1,unroll_factor,instr_index_loop,dependence_loop);
							}
						}
					}
				}
			}
		}
		reset_sameArray_store_latency(LI,bb,inst,unroll_factor,instr_index_loop,dependence_loop);
	}
}


void update_after_load(Instruction *inst, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop)
{
	if(isa<LoadInst>(inst)){
		GetElementPtrInst *gep=get_GEP(inst);
		if(gep!=NULL){
			for(User::op_iterator II=gep->idx_begin(),IE=gep->idx_end(); II!=IE; ++II)
			{
				if(Instruction *tmp_inst=dyn_cast<Instruction>(*II)){
					if(inst_is_cast(tmp_inst)){
						Instruction *inst1=get_cast_inst(tmp_inst);
						if(inst1!=NULL){
							if(isa<LoadInst>(inst1)){
								 Load_latency[inst1]=2.0;
								 set_inst_latency(inst1,2.0,instr_index_loop,dependence_loop);
							}
						}
					}
					else{
						if(isa<LoadInst>(tmp_inst)){
							Load_latency[tmp_inst]=2.0;
							set_inst_latency(tmp_inst,2.0,instr_index_loop,dependence_loop);
						}
					}
				}
			}
		}
	}
}
