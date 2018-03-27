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


#include "ComputeCriticalPath.h"
#include "ComputeMemLatency.h"
#include "EvaluateDependency.h"
#include "Library.h"
#include "Power.h"
#include "Constant.h"
#include "UpdateMem.h"
#include "LoopUnrollFactor.h"
#include "ArrayName.h"
#include "Reschedule.h"


void loop_compute_critical_path_gep(Instruction *inst, float latency, unsigned &index_loop, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop, std::map<Instruction*,std::queue<unsigned> > &inst_map_branch)
{
	 unsigned index_inst=0;
	 if(instr_index_loop.count(inst)){
		 index_inst=instr_index_loop[inst];
	 }
	 else{
		 instr_index_loop[inst]=index_loop;
		 index_inst=index_loop;
		 index_loop++;
	 }
	 dependence_loop[index_inst].push_back(std::make_pair(index_inst, latency));
	 GetElementPtrInst *GEP = dyn_cast<GetElementPtrInst>(inst);
	 for(User::op_iterator II=GEP->idx_begin(),IE=GEP->idx_end(); II!=IE; ++II)
	 {
		 if(Instruction *tmp_inst=dyn_cast<Instruction>(*II))
		 {
			 if(instr_index_loop.count(tmp_inst)){
				 unsigned index_tmp_inst=instr_index_loop[tmp_inst];
				 dependence_loop[index_tmp_inst].push_back(std::make_pair(index_inst,0.0));
			 }
			 else{
				 dependence_loop[0].push_back(std::make_pair(index_inst,0.0));
			 }
		 }
		 else{
			 dependence_loop[0].push_back(std::make_pair(index_inst, 0.0));
		 }
	 }
	 if(inst_map_branch.count(inst)){
		 while(!inst_map_branch[inst].empty()){
			 unsigned j=inst_map_branch[inst].front();
			 inst_map_branch[inst].pop();
			 dependence_loop[j].push_back(std::make_pair(index_inst,0.0));
		 }
	 }
}

void loop_compute_critical_path(Instruction * inst, float latency, unsigned &index_loop, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop, std::map<Instruction*,std::queue<unsigned> > &inst_map_branch)
{
	unsigned index_inst=0;
	if(instr_index_loop.count(inst)){
		index_inst=instr_index_loop[inst];
	}
	else{
		instr_index_loop[inst]=index_loop;
		index_inst=index_loop;
		index_loop++;
	}
	dependence_loop[index_inst].push_back(std::make_pair(index_inst, latency));

	for(unsigned op=0; op!=inst->getNumOperands(); ++op)
	{
		Value *OP = inst->getOperand(op);
		if( Instruction *tmp_inst = dyn_cast<Instruction>(OP) )
		{
			 if(instr_index_loop.count(tmp_inst)){
				 unsigned index_tmp_inst=instr_index_loop[tmp_inst];
				 dependence_loop[index_tmp_inst].push_back(std::make_pair(index_inst,0.0));
			 }
			 else{
				 dependence_loop[0].push_back(std::make_pair(index_inst,0.0));
			 }
		}
		else{
			dependence_loop[0].push_back(std::make_pair(index_inst, 0.0));
		}
	}
	if(inst_map_branch.count(inst)){
		while(!inst_map_branch[inst].empty()){
			 unsigned j=inst_map_branch[inst].front();
			 inst_map_branch[inst].pop();
			 dependence_loop[j].push_back(std::make_pair(index_inst,0.0));
		}
	}
}

void loop_compute_critical_path_call(Instruction *inst, unsigned &index_loop, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop, std::map<Instruction*,std::queue<unsigned> > &inst_map_branch)
{
	float latency_tmp=0.0;
	CallInst *call_inst = dyn_cast<CallInst>(inst);
	Function *callee=call_inst->getCalledFunction();
	unsigned in=0;
	if(instr_index_loop.count(inst)){
		in=instr_index_loop[inst];
	}
	else{
		instr_index_loop[inst]=index_loop;
		in=index_loop;
		index_loop++;
	}
	if(function_cycles.count(callee))
	{
		latency_tmp=(float)function_cycles[callee];
		BasicBlock *bbb=inst->getParent();
		Function *f=bbb->getParent();
		if(Function_pipeline[f]==1){
			latency_tmp=latency_tmp-2;
		}
		dependence_loop[in].push_back(std::make_pair(in,latency_tmp));
		unsigned num=call_inst->getNumArgOperands();
		for(unsigned op=0; op<num; ++op)
		{
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
								dependence_loop[in1].push_back(std::make_pair(in,0.0));
								test_dependence_loop++;
								break;
							}
						}
					}
				}
				if(test_dependence_loop==0){
					if(Instruction *tmp_inst=dyn_cast<Instruction>(OP)){
						if(instr_index_loop.count(tmp_inst)){
							unsigned index_t_inst=instr_index_loop[tmp_inst];
							dependence_loop[index_t_inst].push_back(std::make_pair(in,0.0));
						}
						else{
							dependence_loop[0].push_back(std::make_pair(in,0.0));
						}
					}
					else{
					   dependence_loop[0].push_back(std::make_pair(in, 0.0));
					}
				}
			}
			else if(is_constant==true){
				if( Instruction *tmp_inst = dyn_cast<Instruction>(OP) )
				{
					if(instr_index_loop.count(tmp_inst)){
						unsigned index_t_inst=instr_index_loop[tmp_inst];
						dependence_loop[index_t_inst].push_back(std::make_pair(in,0.0));
					}
					else{
						dependence_loop[0].push_back(std::make_pair(in,0.0));
					}
				}
				else{
					dependence_loop[0].push_back(std::make_pair(in, 0.0));
				}
			}
			else{
				errs()<<"Please check this special argument.\n";
			}
		}
	}
	else{
		dependence_loop[in].push_back(std::make_pair(in,CALL_LATENCY));
		dependence_loop[0].push_back(std::make_pair(in,0.0));
	}
	if(inst_map_branch.count(inst)){
		while(!inst_map_branch[inst].empty()){
			unsigned j=inst_map_branch[inst].front();
			inst_map_branch[inst].pop();
			dependence_loop[j].push_back(std::make_pair(in,0.0));
		}
	}
}

void loop_compute_critical_path_branch(LoopInfo* LI, Instruction *inst, unsigned &index_loop, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop, std::map<Instruction*,std::queue<unsigned> > &inst_map_branch)
{
	BranchInst *branch=dyn_cast<BranchInst>(inst);
	if(branch->isConditional()==true)
	{
		BasicBlock *bb=inst->getParent();
		Loop *L=LI->getLoopFor(bb);
		BasicBlock *header;
		if(L!=NULL){
			header=L->getHeader();
		}
		bool is_loop_branch=false;
		for(unsigned j=0;j<branch->getNumSuccessors();++j)
		{
			BasicBlock *bb_suc=branch->getSuccessor(j);
			if(bb_suc==bb||bb_suc==header){
				is_loop_branch=true;
			}
		}
		if(is_loop_branch==false){//if-else branch
			unsigned index_inst;
			if(instr_index_loop.count(inst)){
				index_inst=instr_index_loop[inst];
			}
			else{
				instr_index_loop[inst]=index_loop;
				index_inst=index_loop;
				index_loop++;
			}
			dependence_loop[index_inst].push_back(std::make_pair(index_inst, 0.0));
			Value *con=branch->getCondition();
			if(Instruction *con_inst=dyn_cast<Instruction>(con)){
				if(instr_index_loop.count(con_inst)){
					unsigned con_index=instr_index_loop[con_inst];
					dependence_loop[con_index].push_back(std::make_pair(index_inst,0.0));
				}
				else{
					dependence_loop[0].push_back(std::make_pair(index_inst,0.0));
				}
			}
			else{
				dependence_loop[0].push_back(std::make_pair(index_inst,0.0));
			}
			unsigned num_suc=branch->getNumSuccessors();
			if(num_suc!=2){
				errs()<<"Wrong: Branch is not right...\n";
			}
			else{
				for(unsigned i=0;i<num_suc;++i){
					BasicBlock *suc=branch->getSuccessor(i);
					BasicBlock::iterator it=suc->begin();
					Instruction *inst_suc=it;
					inst_map_branch[inst_suc].push(index_inst);
				}
			}
		}
	}
}

void loop_compute_critical_path_switch(LoopInfo* LI, Instruction *inst, unsigned &index_loop, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop, std::map<Instruction*,std::queue<unsigned> > &inst_map_branch)
{
	if(SwitchInst *switch_inst=dyn_cast<SwitchInst>(inst))
	{
		unsigned index_inst;
		if(instr_index_loop.count(inst)){
			index_inst=instr_index_loop[inst];
		}
		else{
			instr_index_loop[inst]=index_loop;
			index_inst=index_loop;
			index_loop++;
		}
		dependence_loop[index_inst].push_back(std::make_pair(index_inst, 0.0));
		Value *con=switch_inst->getCondition();
		if(Instruction *con_inst=dyn_cast<Instruction>(con))
		{
			if(instr_index_loop.count(con_inst)){
				unsigned con_index=instr_index_loop[con_inst];
				dependence_loop[con_index].push_back(std::make_pair(index_inst,0.0));
			}
			else{
				dependence_loop[0].push_back(std::make_pair(index_inst,0.0));
			}
		}
		else{
			dependence_loop[0].push_back(std::make_pair(index_inst,0.0));
		}
		for(unsigned i=0;i<switch_inst->getNumSuccessors();++i){
			BasicBlock *suc=switch_inst->getSuccessor(i);
			BasicBlock::iterator it=suc->begin();
			Instruction *inst_suc=it;
			inst_map_branch[inst_suc].push(index_inst);
		}
	}
}

void update(LoopInfo* LI, Instruction *inst, unsigned &index_loop, std::map<Instruction*, unsigned> &instr_index_loop, std::map<Instruction*, unsigned> &load_order_buff, std::vector<std::pair<int,float> > *dependence_loop, std::map<Instruction*,std::queue<unsigned> > &inst_map_branch)
{
	switch(inst->getOpcode())
	{
		//Terminator Instructions
		case Instruction::Ret: break;
		case Instruction::Br:
			loop_compute_critical_path_branch(LI,inst,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
			break;
		case Instruction::Switch:
			loop_compute_critical_path_switch(LI,inst,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
			break;
		case Instruction::IndirectBr:
			break;
		case Instruction::Invoke:      errs()<<"WARNING: in Invoke. \n";	    break;
		case Instruction::Resume:      errs()<<"WARNING: in Resume. \n";	    break;
		case Instruction::Unreachable: errs()<<"WARNING: in Unreachable. \n";	break;

		//Standard binary operators
		case Instruction::Add:
		case Instruction::Sub:
			loop_compute_critical_path(inst,INT_ADD,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
			break;
		case Instruction::FAdd:
		case Instruction::FSub:
			loop_compute_critical_path(inst,FP_ADD,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
			break;
		case Instruction::Mul:
			Value *opr0;
			opr0=inst->getOperand(1);
			if(ConstantInt *cont=dyn_cast<ConstantInt>(opr0))
			{
				int opconst=cont->getSExtValue();
				if(is_power_of_two(opconst)){
					loop_compute_critical_path(inst,SHIFT,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
					//errs()<<"Ignored: multiplied by a contant which is power of two.\n";
				}
				else{
					float SHL_ADD=0.0;
					if(frequency==100){
						SHL_ADD=SHIFT+INT_ADD;
					}
					else if(frequency==125){
						SHL_ADD=SHIFT+INT_ADD;
					}
					else if(frequency==150){
						SHL_ADD=SHIFT+INT_ADD;
					}
					else if(frequency==200){
						SHL_ADD=SHIFT+INT_ADD;
					}
					else if(frequency==250){
						SHL_ADD=SHIFT+INT_ADD+0.2;
					}
					int bound_power_two=closest_bound_power_two(opconst);
					if(opconst<0){
						opconst=-opconst;
					}
					int delta=opconst-bound_power_two;
					if(delta<0){
						delta=-delta;
					}
					if(delta==1){
						loop_compute_critical_path(inst,SHL_ADD,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
					}
					else{
						if(is_power_of_two(delta)){
							loop_compute_critical_path(inst,SHL_ADD,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
						}
						else{
							loop_compute_critical_path(inst,INT_MULT,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
						}
					}
				}
			}
			else{
				loop_compute_critical_path(inst,IMULT,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
			}
			break;
		case Instruction::FMul:
			loop_compute_critical_path(inst,FP_MULT,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
			break;
		case Instruction::SDiv:
		case Instruction::SRem:
			Value *opr;
			opr=inst->getOperand(1);
			if(ConstantInt *cont=dyn_cast<ConstantInt>(opr))
			{
				int opconst=cont->getSExtValue();
				if(is_power_of_two(opconst)){
					float SHL_DIV=0.0;
					if(frequency==100){
						SHL_DIV=SELECT_LATENCY+2*INT_ADD;
					}
					else if(frequency==125){
						SHL_DIV=SELECT_LATENCY+2*INT_ADD;
					}
					else if(frequency==150){
						SHL_DIV=SELECT_LATENCY+INT_ADD;
					}
					else if(frequency==200){
						SHL_DIV=SELECT_LATENCY+INT_ADD;
					}
					else if(frequency==250){
						SHL_DIV=SELECT_LATENCY+INT_ADD;
					}
					loop_compute_critical_path(inst,SHL_DIV,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
					//errs()<<"Ignored: Divided by a contant which is power of two.\n";
				}
				else{
					loop_compute_critical_path(inst,INT_DIV,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
				}
			}
			else{
				loop_compute_critical_path(inst,IDIV,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
			}
			break;
		case Instruction::UDiv:
		case Instruction::URem:
			Value *opr1;
			opr1=inst->getOperand(1);
			if(ConstantInt *cont=dyn_cast<ConstantInt>(opr1))
			{
				int opconst=cont->getSExtValue();
				if(is_power_of_two(opconst)){
					float SHL_DIV=0.0;
					if(frequency==100){
						SHL_DIV=SELECT_LATENCY+2*INT_ADD;
					}
					else if(frequency==125){
						SHL_DIV=SELECT_LATENCY+2*INT_ADD;
					}
					else if(frequency==150){
						SHL_DIV=SELECT_LATENCY+INT_ADD;
					}
					else if(frequency==200){
						SHL_DIV=SELECT_LATENCY+INT_ADD;
					}
					else if(frequency==250){
						SHL_DIV=SELECT_LATENCY+INT_ADD;
					}
					loop_compute_critical_path(inst,SHL_DIV,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
					//errs()<<"Ignored: Divided by a contant which is power of two.\n";
				}
				else{
					loop_compute_critical_path(inst,U_DIV,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
				}
			}
			else{
				loop_compute_critical_path(inst,UDIV,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
			}
			break;
		case Instruction::FDiv:
		case Instruction::FRem:
			Value *opr2;
			opr2=inst->getOperand(1);
			if(ConstantInt *cont=dyn_cast<ConstantInt>(opr2)){
				int opconst=cont->getSExtValue();
				if(is_power_of_two(opconst)){
					loop_compute_critical_path(inst,FP_MULT,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
				}
				else{
					loop_compute_critical_path(inst,FP_DIV,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
				}
			}
			else{
				loop_compute_critical_path(inst,FP_DIV,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
			}
			break;
		//Logical operators (integer operands)
		case Instruction::Shl:
		case Instruction::LShr:
		case Instruction::AShr:
			loop_compute_critical_path(inst,SHIFT,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
			break;
		case Instruction::And:
		case Instruction::Or:
		case Instruction::Xor:
			loop_compute_critical_path(inst,INT_ADD,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
			break;

		//Memory operators
		case Instruction::Alloca:
		{
			unsigned in;
			instr_index_loop[inst]=index_loop;
			in=index_loop;
			dependence_loop[in].push_back(std::make_pair(in,ALLOCA_LATENCY));
			dependence_loop[0].push_back(std::make_pair(in,0));
			if(inst_map_branch.count(inst)){
				while(!inst_map_branch[inst].empty()){
					unsigned j=inst_map_branch[inst].front();
					inst_map_branch[inst].pop();
					dependence_loop[j].push_back(std::make_pair(in,0.0));
				}
			}
			index_loop++;
			break;
		}
		case Instruction::Load:
		{
			float load_latency;
			load_latency=get_load_latency(LI,inst,instr_index_loop,load_order_buff);
			Load_latency[inst]=load_latency;
			loop_compute_critical_path(inst,load_latency,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
			update_after_load(inst,instr_index_loop,dependence_loop);
			break;
		}
		case Instruction::Store:
		{
			float store_latency;
			store_latency=get_store_latency(LI,inst,index_loop,instr_index_loop,dependence_loop);
			Store_latency[inst]=store_latency;
			loop_compute_critical_path(inst,store_latency,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
			update_after_store(LI,inst,instr_index_loop,dependence_loop);
		    break;
		}

		case Instruction::GetElementPtr:
			loop_compute_critical_path_gep(inst,GEP_LATENCY,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
			break;

		case Instruction::Fence: break;
		case Instruction::AtomicCmpXchg: break;
		case Instruction::AtomicRMW: break;

		//Cast operators
		case Instruction::Trunc:
		case Instruction::ZExt:
		case Instruction::SExt:
		case Instruction::FPTrunc:
		case Instruction::FPExt:
			loop_compute_critical_path(inst,0.0,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
			break;
		case Instruction::PtrToInt:
		case Instruction::IntToPtr:
		case Instruction::BitCast:
		loop_compute_critical_path(inst,CAST_LATENCY,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
		break;
		case Instruction::FPToUI:
		case Instruction::FPToSI:
			loop_compute_critical_path(inst,FP_TO_SI,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
			break;
		case Instruction::UIToFP:
		case Instruction::SIToFP:
			loop_compute_critical_path(inst,SI_TO_FP,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
			break;
		case Instruction::AddrSpaceCast:
			errs()<<"In AddrSpaceCast, it's not normal.\n";
			break;

		//Other operators
		case Instruction::ICmp:
			loop_compute_critical_path(inst,ICMP_LATENCY,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
			break;
		case Instruction::FCmp:
			loop_compute_critical_path(inst,FCMP_LATENCY,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
			break;
		case Instruction::PHI:
		{
			unsigned in;
			instr_index_loop[inst]=index_loop;
			in=index_loop;
			dependence_loop[in].push_back(std::make_pair(in,PHI_LATENCY));
			index_loop++;
			PHINode *PHI;
			PHI=dyn_cast<PHINode>(inst);
			BasicBlock *bb;
			bb=inst->getParent();
			Loop* loop;
			loop=LI->getLoopFor(bb);
			if(inst_map_branch.count(inst)){
				while(!inst_map_branch[inst].empty()){
					unsigned j=inst_map_branch[inst].front();
					inst_map_branch[inst].pop();
					dependence_loop[j].push_back(std::make_pair(in,0.0));
				}
			}
			for(unsigned ii=0,ie=PHI->getNumIncomingValues();ii!=ie;++ii)
			{
				BasicBlock *bb_incoming=PHI->getIncomingBlock(ii);
				Value *incoming=PHI->getIncomingValue(ii);
				if(loop!=NULL){
					BasicBlock *latch=loop->getLoopLatch();
					if(bb_incoming != latch){
						if(Instruction *inst_phi=dyn_cast<Instruction>(incoming))
						{
							if(instr_index_loop.count(inst_phi)){
								unsigned index_phi=instr_index_loop[inst_phi];
								dependence_loop[index_phi].push_back(std::make_pair(in,0));
							}
							else{
								dependence_loop[0].push_back(std::make_pair(in,0));
							}
						}
						else{
							dependence_loop[0].push_back(std::make_pair(in,0));
						}
					}
				}
				else{//consider if-else and switch cases.
					if(bb_incoming!=bb){
						if(Instruction *inst_phi=dyn_cast<Instruction>(incoming))
						{
							if(instr_index_loop.count(inst_phi)){
								unsigned index_phi=instr_index_loop[inst_phi];
								dependence_loop[index_phi].push_back(std::make_pair(in,0));
							}
							else{
								dependence_loop[0].push_back(std::make_pair(in,0));
							}
						}
						else{
							dependence_loop[0].push_back(std::make_pair(in,0));
						}
					}
				}
			}
			break;
		}

		case Instruction::Call:
			loop_compute_critical_path_call(inst,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
			break;

		case Instruction::Select:
			loop_compute_critical_path(inst,SELECT_LATENCY,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
			break;

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

		default:
			puts("It is something cannot be handled\n");
			exit(0);

	}
}

float loop_solveCP(std::vector<std::pair<int,float> > *dependence, std::map<unsigned, float> &loop_inst_till_latency)
{
	 float *F=new float [INSN_NUM];

	 for (int i=0;i<INSN_NUM;++i)
	 		F[i]=0.0;

	 for(int v=1;v<INSN_NUM;++v){
		 if(dependence[v].empty())
		 {
			 continue;
		 }
		 else{
		     float w=dependence[v].begin()->second;
		     F[v]=w;
	         for(int u=0;u<v;++u){
			     for(std::vector< std::pair<int,float> >::iterator i= dependence[u].begin(); i!=dependence[u].end(); ++i)
	             {
                   if(i->first==v)
                   {
                   	float x=i->second;
           	        F[v]=std::max(F[v],F[u]+w+x);
                   }
                   else continue;
	             }
	         }
	     }
	 }
	 float cp=0;
	 for(int i=0; i<INSN_NUM; ++i){
		 cp=std::max(F[i],cp);
	     loop_inst_till_latency[i]=F[i];
	 }
	 delete []F;
	 return cp;
}

void dependence_set(LoopInfo* LI, Loop *L, std::map<Instruction*, unsigned> &instr_index_loop, std::map<Instruction*, unsigned> &load_order_buff, std::vector<std::pair<int,float> > *dependence_loop, std::map<Instruction*,std::queue<unsigned> > &inst_map_branch)
{
    unsigned index_loop=1;
	for(Loop::block_iterator BI=L->block_begin(), BE=L->block_end(); BI !=BE; ++BI){
		BasicBlock *bb=*BI;
	        for(auto iti=bb->begin();iti!=bb->end();++iti)
	        {
	            update(LI,iti,index_loop,instr_index_loop,load_order_buff,dependence_loop,inst_map_branch);
				if(isa<LoadInst>(iti)){
					int index=instr_index_loop[iti];
					std::vector<std::pair<int,float> > ::iterator it=dependence_loop[index].begin();
					update_load(LI,iti,instr_index_loop,dependence_loop);
					std::vector<std::pair<int,float> > ::iterator it1=dependence_loop[index].begin();
				}
				else if(isa<StoreInst>(iti)){
					update_store(LI,iti,instr_index_loop,dependence_loop);
				}
	        }
	}
}

float loopCP(LoopInfo* LI, Loop* L, std::map<Instruction*, unsigned> &load_order_buff)
{
	std::map<Instruction*, unsigned> instr_loop;
	std::vector<std::pair<int,float> > dependence[INSN_NUM];
	std::map<Instruction*,std::queue<unsigned> > inst_map_branch;
	std::map<unsigned, float> loop_inst_till_latency;
    dependence_set(LI,L,instr_loop,load_order_buff,dependence,inst_map_branch);
	reschedule_dependence(LI,frequency,instr_loop,dependence);
    float loop_cp=loop_solveCP(dependence,loop_inst_till_latency);
	/*for(unsigned i=0;i<INSN_NUM;++i){
		if(loop_inst_till_latency[i]!=0){
			errs()<<i<<": "<<loop_inst_till_latency[i]<<"\n";
		}
	}*/
	return loop_cp;
}

float dependent_loopCP(LoopInfo* LI, Loop *L, std::map<Instruction*, unsigned> &load_order_buff)
{
	std::map<Instruction*, unsigned> instr_index_loop;
	std::vector<std::pair<int,float> > dependence_loop[INSN_NUM];
	std::map<Instruction*,std::queue<unsigned> > inst_map_branch;
	std::map<unsigned, float> loop_inst_till_latency;
	float loop_cp=0.0;
	unsigned index_loop=1;
	for(Loop::block_iterator BI=L->block_begin(), BE=L->block_end(); BI !=BE; ++BI)
	{
		BasicBlock *bb=*BI;
		for(auto iti=bb->begin();iti!=bb->end();++iti)
		{
			if(isa<LoadInst>(iti)){
				bool array_partition_is_set=false;
				Loop *parent=LI->getLoopFor(bb);
				int num_reads=0;
				float load_latency=1.0;
				int total_factor=total_unroll_factor_ls(LI,parent,iti);
				int unroll_factor= (int)ceil((float)total_factor/(float)Loops_unroll[L]);
                Value *array_name=get_array_name(iti);
                if(array_number.count(array_name)){
	            	 int array_index=array_number[array_name];
	            	 for(std::vector< std::pair<int,int> >::iterator it= array_partition[array_index].begin(); it!=array_partition[array_index].end(); ++it){
	            		 if(it->second!=1){
	            			 array_partition_is_set=true;
	            			 break;
	            		 }
	            	 }
	             }
	             if(array_partition_is_set==false){
	            	 num_reads=compute_asap_load_num(bb,iti,instr_index_loop,load_order_buff)*unroll_factor;//compute_BB_load_num(bb,iti)*unroll_factor;
	            	 load_latency= 1.0 + ceil((float)num_reads/(float)port_num_read);
	             }
	             else{
	            	 int num_reads=partition_load_num(LI,bb,iti,instr_index_loop,load_order_buff);
	            	 load_latency= 1.0 + ceil((float)num_reads/(float)port_num_read)+mux_latency(LI,bb,iti);
	             }
	             Load_latency[iti]=load_latency;
                 loop_compute_critical_path(iti,load_latency,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
                 update_after_load(iti,instr_index_loop,dependence_loop);
                 update_load(LI,iti,instr_index_loop,dependence_loop);
			}
			else if(isa<StoreInst>(iti)){
				 bool array_partition_is_set=false;
				 Loop *parent=LI->getLoopFor(bb);
				 int num_writes=0;
				 int total_factor=total_unroll_factor_ls(LI,parent,iti);
				 int unroll_factor= (int)ceil((float)total_factor/(float)Loops_unroll[L]);
				 float store_latency=0.0;
				 Value *array_name=get_array_name(iti);
				 if(store_latency==0.0){
					 unroll_factor=1;
				 }
				 if(array_number.count(array_name)){
					 int array_index=array_number[array_name];
					 for(std::vector< std::pair<int,int> >::iterator it= array_partition[array_index].begin(); it!=array_partition[array_index].end(); ++it){
						 if(it->second!=1){
							 array_partition_is_set=true;;
							 break;
						 }
					 }
				 }
				 if(array_partition_is_set==false){
					 num_writes=compute_asap_store_num(LI,bb,iti,index_loop,instr_index_loop,dependence_loop)+unroll_factor;
					 store_latency += ceil((float)num_writes/(float)port_num_write_diff);
				 }
				 else{
					 int num_writes=partition_store_num(LI,bb,iti,index_loop,instr_index_loop,dependence_loop);
					 store_latency += ceil((float)num_writes/(float)port_num_write_diff)+mux_latency(LI,bb,iti);
				 }
				 Store_latency[iti]=store_latency;
				 loop_compute_critical_path(iti,store_latency,index_loop,instr_index_loop,dependence_loop,inst_map_branch);
				 update_after_store(LI,iti,instr_index_loop,dependence_loop);
				 update_store(LI,iti,instr_index_loop,dependence_loop);
			}
			else{
				update(LI,iti,index_loop,instr_index_loop,load_order_buff,dependence_loop,inst_map_branch);
			}
		}
	}
	reschedule_dependence(LI,frequency,instr_index_loop,dependence_loop);
	loop_cp=loop_solveCP(dependence_loop,loop_inst_till_latency);
	return loop_cp;
}

float update_loopCP(LoopInfo* LI, Loop *L, std::map<Instruction*, unsigned> &load_order_buff)
{
	float loop_cp=0.0;
	std::vector<Loop*> subLoops = L->getSubLoops();
	int subLoop_num  = subLoops.size();
	bool loop_dependence_flag=test_loop_carried_dependence(LI,L);
	if(subLoops.empty()){
		if(loop_dependence_flag==false){
			loop_cp=loopCP(LI,L,load_order_buff);
		}
		else{
			loop_cp=dependent_loopCP(LI,L,load_order_buff)*Loops_unroll[L];
		}
	}
	else{  //outerloop is pipelined, calculate its critical path. (inner loops are all unrolled completely)
		float error=0;
		for(int j=0; j<subLoop_num; j++){
			bool subloop_dependence=false;
			Loop* sub_loop=subLoops.at(j);
			float update_subloop=update_loopCP(LI,sub_loop,load_order_buff);
			float cp_subloop=loopCP(LI,sub_loop,load_order_buff);
			if(cp_subloop!=update_subloop){
				subloop_dependence=true;
			}
			if(subloop_dependence==true){
				error += update_subloop - cp_subloop;
			}
		}
		if(loop_dependence_flag==false){
			loop_cp=loopCP(LI,L,load_order_buff)+error;
		}
		else{
			loop_cp=dependent_loopCP(LI,L,load_order_buff)+error;
			loop_cp=loop_cp*Loops_unroll[L];
		}
	}
	loop_critical_path[L]=loop_cp;
	return loop_cp;
}
