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


#include "LoopUnrollFactor.h"
#include "GEP.h"


int total_unroll_factor(Loop *L)
{
	int factor=1;
	if(L!=NULL){
		if(Loops_unroll[L]>=Loops_counter[L]){
			Loop *parent=L->getParentLoop();
			int parent_factor=total_unroll_factor(parent);
			factor=factor * Loops_unroll[L] *parent_factor;
		}
		else{
			factor=factor * Loops_unroll[L];
		}
	}
	return factor;
}

bool unroll_loop_relation(Loop *L1, Loop *L2)
{
	bool unroll_related=false;
	if(L1==L2){
		unroll_related=true;
	}
	else{
		if(L2->contains(L1)){
			if(Loops_unroll[L1]>=Loops_counter[L1]){
				Loop *parent=L1->getParentLoop();
				unroll_related=unroll_loop_relation(parent,L2);
			}
		}
	}
	return unroll_related;
}

int total_unroll_factor_ls(LoopInfo *LI, Loop *L, Instruction *inst)
{
	int factor=1;
	int total_factor=total_unroll_factor(L);
	if(isa<LoadInst>(inst)||isa<StoreInst>(inst)){
		GetElementPtrInst *gep=get_GEP(inst);
		if(gep!=NULL){
			for(User::op_iterator II=gep->idx_begin(),IE=gep->idx_end(); II!=IE; ++II)
			{
				if(Instruction *tmp_inst=dyn_cast<Instruction>(*II))
				{
					BasicBlock *bb=tmp_inst->getParent();
					Loop *l=LI->getLoopFor(bb);
					if(l!=NULL){
						bool related=unroll_loop_relation(L,l);
						if(related==true){
							factor *= Loops_unroll[l];
						}
					}
				}
				else{
					 BasicBlock *bb=gep->getParent();
					 Loop *l=LI->getLoopFor(bb);
					 if(l!=NULL){
						 bool related=unroll_loop_relation(L,l);
						 if(related==true){
							 factor *= Loops_unroll[l];
						 }
					 }
				}
			}
		}
		else{
			factor=total_factor;
		}
	}
	return factor;
}


int total_unroll_factor_op(LoopInfo *LI, Loop *L, Instruction *inst)
{
	int factor=1;
	int total_factor=total_unroll_factor(L);
	bool op_is_ls=false;
	std::map<Loop*,int> loop_ul_counted;
	for(unsigned op=0; op!=inst->getNumOperands(); ++op)
	{
		Value *OP = inst->getOperand(op);
		if(Instruction *inst_op=dyn_cast<Instruction>(OP)){
			if(isa<LoadInst>(inst_op)||isa<StoreInst>(inst_op))
			{
				GetElementPtrInst *gep=get_GEP(inst_op);
				if(gep!=NULL)
				{
					for(User::op_iterator II=gep->idx_begin(),IE=gep->idx_end(); II!=IE; ++II)
					{
						if(Instruction *tmp_inst=dyn_cast<Instruction>(*II))
						{
							BasicBlock *bb=tmp_inst->getParent();
							Loop *l=LI->getLoopFor(bb);
							if(l!=NULL){
								if(!loop_ul_counted.count(l)){
									bool related=unroll_loop_relation(L,l);
									if(related==true){
										factor *= Loops_unroll[l];
										loop_ul_counted[l]=factor;
										op_is_ls=true;
									}
								}
							}
						}
					}
				}
			}
			else{
				for(unsigned op1=0; op1!=inst_op->getNumOperands(); ++op1)
				{
					Value *OP1=inst_op->getOperand(op1);
					if(Instruction *inst_op1=dyn_cast<Instruction>(OP1))
					{
						if(isa<LoadInst>(inst_op1)||isa<StoreInst>(inst_op1))
						{
							GetElementPtrInst *gep=get_GEP(inst_op1);
							if(gep!=NULL){
								for(User::op_iterator II=gep->idx_begin(),IE=gep->idx_end(); II!=IE; ++II){
									if(Instruction *tmp_inst=dyn_cast<Instruction>(*II)){
										BasicBlock *bb=tmp_inst->getParent();
										Loop *l=LI->getLoopFor(bb);
										if(l!=NULL){
											if(!loop_ul_counted.count(l)){
												bool related=unroll_loop_relation(L,l);
												if(related==true){
													factor *= Loops_unroll[l];
													loop_ul_counted[l]=factor;
													op_is_ls=true;
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
	if(op_is_ls==false){
		factor=total_factor;
	}
	return factor;
}
