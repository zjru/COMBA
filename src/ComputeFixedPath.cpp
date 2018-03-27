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

#include "ComputeFixedPath.h"

void loop_computeCP_gep_fixed_latency(Loop *L, Instruction *inst, float latency, unsigned &index_loop, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop)
{
	unsigned index_inst=0;
	instr_index_loop[inst]=index_loop;
	index_inst=index_loop;
	dependence_loop[index_inst].push_back(std::make_pair(index_inst, 0.0));
	index_loop++;
	GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(inst);
	for(User::op_iterator II=GEP->idx_begin(),IE=GEP->idx_end(); II!=IE; ++II)
	{
		if(Instruction *tmp_inst=dyn_cast<Instruction>(*II))
		{
			BasicBlock *tmp_bb=tmp_inst->getParent();
			if(instr_index_loop.count(tmp_inst)){
				unsigned index_tmp_inst=instr_index_loop[tmp_inst];
				if(L->contains(tmp_bb)){
					dependence_loop[index_tmp_inst].push_back(std::make_pair(index_inst,0.0));
				}
				else{
					dependence_loop[index_tmp_inst].push_back(std::make_pair(index_inst,latency));
				}
			}
			else{
				dependence_loop[0].push_back(std::make_pair(index_inst,latency));
			}
		}
		else{
			dependence_loop[0].push_back(std::make_pair(index_inst, latency));
		}
	}
}

void loop_computeCP_fixed_latency(Loop *L, Instruction * inst, float latency, unsigned &index_loop, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop)
{
	unsigned index_inst=0;
	instr_index_loop[inst]=index_loop;
	index_inst=index_loop;
	dependence_loop[index_inst].push_back(std::make_pair(index_inst, 0.0));
	index_loop++;
	for(unsigned op=0; op!=inst->getNumOperands(); ++op)
	{
		Value *OP = inst->getOperand(op);
		if( Instruction *tmp_inst = dyn_cast<Instruction>(OP) )
		{
			BasicBlock *tmp_bb=tmp_inst->getParent();
			if(instr_index_loop.count(tmp_inst)){
				unsigned index_tmp_inst=instr_index_loop[tmp_inst];
				if(L->contains(tmp_bb)){
					dependence_loop[index_tmp_inst].push_back(std::make_pair(index_inst,0.0));
				}
				else{
					dependence_loop[index_tmp_inst].push_back(std::make_pair(index_inst,latency));
				}
			}
			else{
				dependence_loop[0].push_back(std::make_pair(index_inst,latency));
			}
		}
		else{
			dependence_loop[0].push_back(std::make_pair(index_inst, latency));
		}
	}
}

void loop_computeCP_call_fixed_latency(Loop *L, Instruction *inst, float latency, unsigned &index_loop, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop)
{
	CallInst *call_inst = dyn_cast<CallInst>(inst);
	Function *callee=call_inst->getCalledFunction();;
	unsigned in=0;
	instr_index_loop[inst]=index_loop;
	in=index_loop;
	dependence_loop[in].push_back(std::make_pair(in,0.0));
	index_loop++;
	if(function_cycles.count(callee)){
		unsigned num=0;
		num=call_inst->getNumArgOperands();
		for(unsigned op=0; op<num; ++op){
			int test_dependence_loop=0;
			bool is_constant=false;
			bool is_array_rw=false;
			Value *OP=call_inst->getArgOperand(op);
			if(callee==OP){
				errs()<<"The operand is the called function.\n";
			}
			Type *OP_type = OP->getType();
			is_constant = OP_type->isFloatingPointTy()||OP_type->isIntegerTy();
			is_array_rw = OP_type->isPointerTy()||OP_type->isArrayTy();
			if(is_array_rw==true){
				std::map<Instruction*, unsigned>::iterator t;
				for(t=instr_index_loop.begin();t!=instr_index_loop.end();++t){
					Instruction *inst1=t->first;
					unsigned in1=t->second;
					if((isa<CallInst>(inst1))&&(inst1 != inst)){
						CallInst *call1=dyn_cast<CallInst>(inst1);
						for(unsigned op1=0; op1 < call1->getNumArgOperands(); ++op1)
						{
							Value *OP1=call1->getArgOperand(op1);
							if(OP1==OP){
								BasicBlock *tmp_bb=inst1->getParent();
								if(L->contains(tmp_bb)){
									dependence_loop[in1].push_back(std::make_pair(in,0.0));
								}
								else{
									dependence_loop[in1].push_back(std::make_pair(in,latency));
								}
								test_dependence_loop++;
								break;
							}
						}
					}
				}
				if(test_dependence_loop==0){
					if(Instruction *tmp_inst=dyn_cast<Instruction>(OP)){
						BasicBlock *tmp_bb=tmp_inst->getParent();
						if(instr_index_loop.count(tmp_inst)){
							unsigned index_t_inst=instr_index_loop[tmp_inst];
							if(L->contains(tmp_bb)){
								dependence_loop[index_t_inst].push_back(std::make_pair(in,0.0));
							}
							else{
								dependence_loop[index_t_inst].push_back(std::make_pair(in,latency));
							}
						}
						else{
							dependence_loop[0].push_back(std::make_pair(in,latency));
						}
					}
					else{
						dependence_loop[0].push_back(std::make_pair(in, latency));
					}
				}
			}
			else if(is_constant==true){
				if( Instruction *tmp_inst = dyn_cast<Instruction>(OP) )
				{
					BasicBlock *tmp_bb=tmp_inst->getParent();
					if(instr_index_loop.count(tmp_inst)){
						unsigned index_t_inst=instr_index_loop[tmp_inst];
						if(L->contains(tmp_bb)){
							dependence_loop[index_t_inst].push_back(std::make_pair(in,0.0));
						}
						else{
							dependence_loop[index_t_inst].push_back(std::make_pair(in,latency));
						}
					}
					else{
						dependence_loop[0].push_back(std::make_pair(in,latency));
					}
				}
				else{
					dependence_loop[0].push_back(std::make_pair(in, latency));
				}
			}
			else{
				errs()<<"Please check this special argument.\n";
			}
		}
	}
	else{
		dependence_loop[0].push_back(std::make_pair(in, latency));
	}
}

void loop_computeCP_phi_fixed_latency(Loop *L, Instruction *inst, float latency, unsigned &index_loop, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop)
{
	unsigned in=0;
	instr_index_loop[inst]=index_loop;
	in=index_loop;
	dependence_loop[in].push_back(std::make_pair(in,0.0));
	index_loop++;
	PHINode *PHI=dyn_cast<PHINode>(inst);
	for(unsigned ii=0,ie=PHI->getNumIncomingValues();ii!=ie;++ii){
		BasicBlock *bb_incoming=PHI->getIncomingBlock(ii);
		Value *incoming=PHI->getIncomingValue(ii);
		if(L!=NULL){
			BasicBlock *latch=L->getLoopLatch();
			if(bb_incoming != latch){
				if(Instruction *inst_phi=dyn_cast<Instruction>(incoming)){
					BasicBlock *tmp_bb=inst_phi->getParent();
					if(instr_index_loop.count(inst_phi)){
						unsigned index_phi=instr_index_loop[inst_phi];
						if(L->contains(tmp_bb)){
							dependence_loop[index_phi].push_back(std::make_pair(in,0.0));
						}
						else{
							dependence_loop[index_phi].push_back(std::make_pair(in,latency));
						}
					}
					else{
						dependence_loop[0].push_back(std::make_pair(in,latency));
						Instruction *ins=NULL;
						for(unsigned u=1;u<in;++u){
							for(std::map<Instruction *, unsigned>::iterator im=instr_index_loop.begin();im!=instr_index_loop.end();++im){
								if(im->second==u){
									ins=im->first;
									break;
								}
							}
							BasicBlock *bb_ins=ins->getParent();
							if(L->contains(bb_ins)){
								continue;
							}
							else{
								dependence_loop[u].push_back(std::make_pair(in,latency));
							}
						}
					}
				}
				else{
					dependence_loop[0].push_back(std::make_pair(in,latency));
					Instruction *ins=NULL;
					for(unsigned u=1;u<in;++u){
						for(std::map<Instruction *, unsigned>::iterator it=instr_index_loop.begin();it!=instr_index_loop.end();++it){
							if(it->second==u){
								ins=it->first;
								break;
							}
						}
						BasicBlock *bb_ins=ins->getParent();
						if(L->contains(bb_ins)){
							continue;
						}
						else{
							dependence_loop[u].push_back(std::make_pair(in,latency));
						}
					}
				}
			}
		}
		else{
			dependence_loop[0].push_back(std::make_pair(in,latency));
		}
	}
}

void update_fix_latency(Loop *L, Instruction *inst, float latency, unsigned &index_loop, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop)
{
	switch(inst->getOpcode())
	{
		//Terminator Instructions
		case Instruction::Ret: break;
		case Instruction::Br:
		case Instruction::Switch:
		case Instruction::IndirectBr:
			break;
		case Instruction::Invoke:      errs()<<"WARNING: in Invoke. \n";	    break;
		case Instruction::Resume:      errs()<<"WARNING: in Resume. \n";	    break;
		case Instruction::Unreachable: errs()<<"WARNING: in Unreachable. \n";	break;
		//Standard binary operators
		case Instruction::Add:
		case Instruction::Sub:
		case Instruction::FAdd:
		case Instruction::FSub:
		case Instruction::Mul:
		case Instruction::FMul:
		case Instruction::SDiv:
		case Instruction::SRem:
		case Instruction::UDiv:
		case Instruction::URem:
		case Instruction::FDiv:
		case Instruction::FRem:
		case Instruction::Shl:
		case Instruction::LShr:
		case Instruction::AShr:
		case Instruction::And:
		case Instruction::Or:
		case Instruction::Xor:
		case Instruction::Load:
		case Instruction::Store:
		case Instruction::Trunc:
		case Instruction::ZExt:
		case Instruction::SExt:
		case Instruction::FPTrunc:
		case Instruction::FPExt:
		case Instruction::PtrToInt:
		case Instruction::IntToPtr:
		case Instruction::BitCast:
		case Instruction::FPToUI:
		case Instruction::FPToSI:
		case Instruction::UIToFP:
		case Instruction::SIToFP:
		case Instruction::ICmp:
		case Instruction::FCmp:
		case Instruction::Select:
			loop_computeCP_fixed_latency(L, inst, latency,index_loop,instr_index_loop,dependence_loop);
			break;
		case Instruction::Alloca:
		{
			unsigned in;
			instr_index_loop[inst]=index_loop;
			in=index_loop;
			dependence_loop[in].push_back(std::make_pair(in,0));
			dependence_loop[0].push_back(std::make_pair(in,latency));
			index_loop++;
			break;
		}
		case Instruction::GetElementPtr:
			loop_computeCP_gep_fixed_latency(L, inst, latency,index_loop,instr_index_loop,dependence_loop);
			break;
		case Instruction::Call:
			loop_computeCP_call_fixed_latency(L, inst, latency,index_loop,instr_index_loop,dependence_loop);
			break;
		case Instruction::AddrSpaceCast:
		case Instruction::Fence:
		case Instruction::AtomicCmpXchg:
		case Instruction::AtomicRMW:
		case Instruction::UserOp1:
		case Instruction::UserOp2:
		case Instruction::VAArg:
		case Instruction::ExtractElement:
		case Instruction::InsertElement:
		case Instruction::ShuffleVector:
		case Instruction::ExtractValue:
		case Instruction::InsertValue:
		case Instruction::LandingPad:
			break;
		case Instruction::PHI:
			loop_computeCP_phi_fixed_latency(L, inst, latency,index_loop,instr_index_loop,dependence_loop);
			break;
		default:
			puts("It is something cannot be handled\n");
			exit(0);
	}
}
