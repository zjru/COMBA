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


#include "MemOpNum.h"
#include "ArrayName.h"
#include "GEP.h"
#include "LoopUnrollFactor.h"
#include "PartitionNum.h"
#include "ComputeMemLatency.h"

int compute_BB_load_num(BasicBlock *itb, Instruction *inst)
{
	int load_num=1;
	Value *address_index_tmp=NULL;
	Value *address_index=NULL;

	address_index=get_array_name(inst);
	GetElementPtrInst *gep1=get_GEP(inst);
	for(auto iti=itb->begin(); iti!=itb->end();++iti)
	{
		if(isa<LoadInst>(iti))
		{
			address_index_tmp=get_array_name(iti);
			if(address_index_tmp==address_index)
			{
				GetElementPtrInst *gep=get_GEP(iti);
				if(gep!=gep1){
					load_num++;
				}
			}
		}
	}
	return load_num;
}

int compute_BB_store_num(BasicBlock *itb, Instruction *inst)
{
	int store_num=1;
	Value *address_index_tmp=NULL;
	Value *address_index=NULL;

	address_index=get_array_name(inst);
	GetElementPtrInst *gep1=get_GEP(inst);
	for(auto iti=itb->begin(); iti!=itb->end();++iti)
	{
		if(isa<StoreInst>(iti))
		{
			address_index_tmp=get_array_name(iti);
			if(address_index_tmp==address_index)
			{
				GetElementPtrInst *gep=get_GEP(iti);
				if(gep!=gep1){
					store_num++;
				}
			}
		}
	}
	return store_num;
}

int partition_mem_op_num(LoopInfo *LI, BasicBlock *bb, Instruction *inst)
{
	int num=0;
	Value *array_name=get_array_name(inst);
	bool inst_is_load=false;
	bool inst_is_store=false;
	bool both_are_load=false;
	bool both_are_store=false;
	std::vector<int> array_element_counted;
	GetElementPtrInst *gep1=get_GEP(inst);
	Loop *L=LI->getLoopFor(bb);
	int inst_offset=0;
	int same_bank_factor=1;
	bool has_same=false;
	if(gep1!=NULL){
		for(User::op_iterator ii=gep1->idx_begin(), ie=gep1->idx_end();ii!=ie; ++ii){
			Value *op_l=*ii;
			if(Instruction *op_inst=dyn_cast<Instruction>(op_l)){
				BasicBlock *bb1=op_inst->getParent();
				Loop *l1=LI->getLoopFor(bb1);
				bool related=unroll_loop_relation(L,l1);
				if(related==true){
					int a=get_unroll_SameBank_num(bb1,inst,inst,array_element_counted);
					//errs()<<"a: "<<a<<"\n";
					if(a!=0){
						same_bank_factor *= a;
						has_same=true;
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
					 	 has_same=true;
				 	 }
				 	 inst_offset=compute_array_element_offset(inst,bb1,0);
				 }
			}
		}
	}
	if(has_same==true){
		num += same_bank_factor;
	}
	array_element_counted.push_back(inst_offset);

	if(isa<LoadInst>(inst)){
		inst_is_load=true;
	}
	else if(isa<StoreInst>(inst)){
		inst_is_store=true;
	}
	else{
		errs()<<"This inst is neither load nor store.\n";
	}
	for(auto iti=bb->begin();iti!=bb->end();++iti){
		both_are_load=inst_is_load && (isa<LoadInst>(iti));
		both_are_store=inst_is_store && (isa<StoreInst>(iti));
		if(both_are_load || both_are_store){
			Value *array_name_tmp=get_array_name(iti);
			if(array_name==array_name_tmp){
				GetElementPtrInst *gep=get_GEP(iti);
				if(gep!=gep1){
					if(gep!=NULL){
						int same_factor=1;
						bool have_same=false;
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
										have_same=true;
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
										 have_same=true;
									 }
								 }
							}
						}
						if(have_same==true){
							num += same_factor;
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

int compute_total_LS_num(LoopInfo* LI, Instruction *inst)
{
	if((!isa<LoadInst>(inst))&&(!isa<StoreInst>(inst))){
		errs()<<"This instruction is neither load nor store...Not apply.\n";
	}
	int num=0;
	bool array_partition_is_set=false;
	BasicBlock *bb=inst->getParent();
	int unroll_factor=0;
	Loop *L=LI->getLoopFor(bb);
	unroll_factor=total_unroll_factor_ls(LI,L,inst);
	Value *array_name=get_array_name(inst);
	int size=1;
	if(array_number.count(array_name)){
		int array_index=array_number[array_name];
		int dim=array_dimension[array_index];
		int initial=0;
		for(int j=0; j<array_index; ++j){
			initial += array_dimension[j];
		}
		for(int i=0;i<dim;++i){
			int j=initial+i;
			size*=array_size[j];
		}
		for(std::vector< std::pair<int,int> >::iterator it= array_partition[array_index].begin(); it!=array_partition[array_index].end(); ++it)
		{
			if(it->second!=1){
				array_partition_is_set=true;
				break;
			}
		}
	}
	if(array_partition_is_set==false){
		if(isa<LoadInst>(inst)){
			num=compute_BB_load_num(bb,inst)*unroll_factor;
			if(num>size){
				num=size;
			}
		}
		else if(isa<StoreInst>(inst)){
			num=compute_BB_store_num(bb,inst)*unroll_factor;
		}
	}
	else{
   	    num=partition_mem_op_num(LI,bb,inst)+(int)mux_latency(LI,bb,inst)*2;
	}
    return num;
}
