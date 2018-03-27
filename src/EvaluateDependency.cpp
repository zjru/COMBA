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


#include "EvaluateDependency.h"
#include "ArrayName.h"
#include "GEP.h"


bool load_store_dependency(Instruction *inst_l, Instruction *inst_s)
{
	bool LS_flag=false;
	Value *address_index_load=NULL;
	Value *address_index_store=NULL;
	address_index_store=get_array_name(inst_s);
	address_index_load=get_array_name(inst_l);

	if(address_index_store==address_index_load){
		LS_flag=true;
	}
	return LS_flag;
}


bool store_store_dependency(Instruction *inst_s1, Instruction *inst_s2)
{
	bool SS_flag=false;
	Value *address_index_s1=NULL;
	Value *address_index_s2=NULL;
	address_index_s1=get_array_name(inst_s1);
	address_index_s2=get_array_name(inst_s2);

	if(address_index_s1==address_index_s2){
		SS_flag=true;
	}
	return SS_flag;
}


//Don't consider: if L has subloops, when it is unrolled, time should be addition.(seems like loop carried dependent)
//Consider:when it is pipelined, subloops dependence will influence the latency and II.
//Consider:if L has not subloops, this characteristic will influence the latency.
bool test_loop_carried_dependence(LoopInfo* LI, Loop *L)
{
	bool loop_carry=false;
	//int distance;
	//std::vector<Loop*> subLoops = L->getSubLoops();
	BasicBlock *latch=L->getLoopLatch();
	TerminatorInst *last_inst=latch->getTerminator();
	BranchInst *br_inst=dyn_cast<BranchInst>(last_inst);
	Instruction *indvar=NULL;
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
	}
	PHINode *phi=NULL;
	BasicBlock *B1=*(L->block_begin());
	for(auto j=B1->begin();j!=B1->end();++j){
		bool break_flag=false;
		if(PHINode *phi1=dyn_cast<PHINode>(j)){
			for(unsigned ii=0, ie=phi1->getNumIncomingValues();ii!=ie;ii++){
				Value *incoming=phi1->getIncomingValue(ii);
				Instruction *inst1=dyn_cast<Instruction>(incoming);
				if(inst1==indvar){
					phi=phi1;
					break_flag=true;
					break;
				}
			}
			if(break_flag==true){
				break;
			}
		}
	}
	for(Loop::block_iterator BI=L->block_begin(), BE=L->block_end(); BI !=BE; ++BI)
	{
		BasicBlock *bb=*BI;
		for(auto iti=bb->begin();iti!=bb->end();++iti){
			if(isa<LoadInst>(iti)){
				GetElementPtrInst *gep=get_GEP(iti);
				if(gep==NULL){
					continue;
				}
				else{
					int g_phi=get_gep_phi(gep,phi);
					if(g_phi==10){
						continue;//second_iteration_flag=true;
					}
					else{
						Value *g_op=gep->getOperand(g_phi);
						int compute_second=get_phi_second(L, phi);
						int second_iteration=compute_gep_operand(g_op,true,compute_second);
						for(Loop::block_iterator BL=L->block_begin(), BF=L->block_end(); BL !=BF; ++BL){
							BasicBlock *BB=*BL;
							for(auto ii=BB->begin();ii!=BB->end();++ii){
								if(isa<StoreInst>(ii)){
									bool ls_flag=load_store_dependency(iti,ii);
									if(ls_flag==true){
										GetElementPtrInst *gep1=get_GEP(ii);
										if(gep1==NULL){
											continue;
										}
										else{
											int g_phi1=get_gep_phi(gep1,phi);
											if(g_phi1==10){
												continue;
											}
											else{
												Value *g_op1=gep->getOperand(g_phi1);
												int first_iteration1=compute_gep_operand(g_op1,false,0);
												if(second_iteration==first_iteration1){
													return true;
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
			else if(CallInst *call=dyn_cast<CallInst>(iti)){
				Function *callee=call->getCalledFunction();
				unsigned FnID=callee->getIntrinsicID();
				if(FnID==0){
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
											return true;
										}
									}
								}
							}
						  }
					  }
				}
			}
			else continue;
		}
	}
	return loop_carry;
}
