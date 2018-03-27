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


#include "GEP.h"

GetElementPtrInst *get_GEP(Instruction *inst)
{
	Value *op=NULL;
	GetElementPtrInst *gep=NULL;
	if(LoadInst *load_inst=dyn_cast<LoadInst>(inst)){
		op=load_inst->getPointerOperand();
	}
	else if(StoreInst *store_inst=dyn_cast<StoreInst>(inst)){
		op=store_inst->getPointerOperand();
	}
	else errs()<<"This instruction is neither a load nor a store.\n";

	if(GetElementPtrInst *OP_GEP=dyn_cast<GetElementPtrInst>(op)){
		gep=OP_GEP;
	}
	else if(PHINode *OP_PHI=dyn_cast<PHINode>(op)){
		for(unsigned i=0, e=OP_PHI->getNumIncomingValues();i!=e;i++){
			Value *incoming=OP_PHI->getIncomingValue(i);
			if(GetElementPtrInst *GEP_incoming=dyn_cast<GetElementPtrInst>(incoming)){
				Value *val=GEP_incoming->getPointerOperand();
				if(val==op){
				   gep=GEP_incoming;
				   break;
				}
				/*if(!isa<PHINode>(val)){
				   gep=GEP_incoming;
				   break;
				}*/
			}
		}
	}
	else if(Instruction *OP_Inst=dyn_cast<Instruction>(op)){
		for(unsigned i=0; i!=OP_Inst->getNumOperands(); ++i){
		  Value *OP = OP_Inst->getOperand(i);
			if(GetElementPtrInst *tmp_inst = dyn_cast<GetElementPtrInst>(OP)){
				gep=tmp_inst;
				break;
			}
		}
	}
	return gep;
}


int compute_gep_operand(Value *op, bool second_iteration_flag, int compute_second)
{
	int dis_l=0;
	int dis_tmp0=0;
	int dis_tmp1=0;
	Value *opr0=NULL;
	Value *opr1=NULL;
	if(Instruction *inst_test=dyn_cast<Instruction>(op))
	{
	   switch(inst_test->getOpcode())
	   {
		   case Instruction::Add:
		   case Instruction::FAdd:
			   opr0=inst_test->getOperand(0);
			   opr1=inst_test->getOperand(1);
			   if(isa<Instruction>(opr0)){
				   dis_tmp0=compute_gep_operand(opr0,second_iteration_flag,compute_second);
			   }
			   else if(ConstantInt *cont=dyn_cast<ConstantInt>(opr0)){
				   dis_tmp0=cont->getSExtValue();
			   }
			   else{
				   errs()<<"The GEP operand is neither integer nor inst. \n";
			   }
			   if(isa<Instruction>(opr1)){
				   dis_tmp1=compute_gep_operand(opr1,second_iteration_flag,compute_second);
			   }
			   else if(ConstantInt *cont1=dyn_cast<ConstantInt>(opr1)){
				   dis_tmp1=cont1->getSExtValue();
			   }
			   else{
				   errs()<<"The GEP operand is neither integer nor inst. \n";
			   }
			   dis_l = dis_tmp0 + dis_tmp1;
			   break;

		   case Instruction::Sub:
		   case Instruction::FSub:
			   opr0=inst_test->getOperand(0);
			   opr1=inst_test->getOperand(1);
			   if(isa<Instruction>(opr0)){
				   dis_tmp0=compute_gep_operand(opr0,second_iteration_flag,compute_second);
			   }
			   else if(ConstantInt *cont=dyn_cast<ConstantInt>(opr0)){
				   dis_tmp0=cont->getSExtValue();
			   }
			   else{
				   errs()<<"The GEP operand is neither integer nor inst. \n";
			   }
			   if(isa<Instruction>(opr1)){
				   dis_tmp1=compute_gep_operand(opr1,second_iteration_flag,compute_second);
			   }
			   else if(ConstantInt *cont1=dyn_cast<ConstantInt>(opr1)){
				   dis_tmp1=cont1->getSExtValue();
			   }
			   else{
				   errs()<<"The GEP operand is neither integer nor inst. \n";
			   }
			   dis_l = dis_tmp0 - dis_tmp1;
			   break;
		   case Instruction::Mul:
		   case Instruction::FMul:
			   opr0=inst_test->getOperand(0);
			   opr1=inst_test->getOperand(1);
			   if(isa<Instruction>(opr0)){
				   dis_tmp0=compute_gep_operand(opr0,second_iteration_flag,compute_second);
			   }
			   else if(ConstantInt *cont=dyn_cast<ConstantInt>(opr0)){
				   dis_tmp0=cont->getSExtValue();
			   }
			   else{
				   errs()<<"The GEP operand is neither integer nor inst. \n";
			   }
			   if(isa<Instruction>(opr1)){
				   dis_tmp1=compute_gep_operand(opr1,second_iteration_flag,compute_second);
			   }
			   else if(ConstantInt *cont1=dyn_cast<ConstantInt>(opr1)){
				   dis_tmp1=cont1->getSExtValue();
			   }
			   else{
				   errs()<<"The GEP operand is neither integer nor inst. \n";
			   }
			   dis_l = dis_tmp0 * dis_tmp1;
			   break;

		   case Instruction::UDiv:
		   case Instruction::SDiv:
		   case Instruction::FDiv:
			   opr0=inst_test->getOperand(0);
			   opr1=inst_test->getOperand(1);
			   if(isa<Instruction>(opr0)){
				   dis_tmp0=compute_gep_operand(opr0,second_iteration_flag,compute_second);
			   }
			   else if(ConstantInt *cont=dyn_cast<ConstantInt>(opr0)){
				   dis_tmp0=cont->getSExtValue();
			   }
			   else{
				   errs()<<"The GEP operand is neither integer nor inst. \n";
			   }
			   if(isa<Instruction>(opr1)){
				   dis_tmp1=compute_gep_operand(opr1,second_iteration_flag,compute_second);
			   }
			   else if(ConstantInt *cont1=dyn_cast<ConstantInt>(opr1)){
				   dis_tmp1=cont1->getSExtValue();
			   }
			   else{
				   errs()<<"The GEP operand is neither integer nor inst. \n";
			   }
			   dis_l = dis_tmp0/dis_tmp1;
			   break;

		   case Instruction::URem:
		   case Instruction::SRem:
		   case Instruction::FRem:
			   opr0=inst_test->getOperand(0);
			   opr1=inst_test->getOperand(1);
			   if(isa<Instruction>(opr0)){
				   dis_tmp0=compute_gep_operand(opr0,second_iteration_flag,compute_second);
			   }
			   else if(ConstantInt *cont=dyn_cast<ConstantInt>(opr0)){
				   dis_tmp0=cont->getSExtValue();
			   }
			   else{
				   errs()<<"The GEP operand is neither integer nor inst. \n";
			   }
			   if(isa<Instruction>(opr1)){
				   dis_tmp1=compute_gep_operand(opr1,second_iteration_flag,compute_second);
			   }
			   else if(ConstantInt *cont1=dyn_cast<ConstantInt>(opr1)){
				   dis_tmp1=cont1->getSExtValue();
			   }
			   else{
				   errs()<<"The GEP operand is neither integer nor inst. \n";
			   }
			   dis_l = dis_tmp0 % dis_tmp1;
			   break;

		   case Instruction::Shl:
			   opr0=inst_test->getOperand(0);
			   opr1=inst_test->getOperand(1);
			   if(isa<Instruction>(opr0)){
				   dis_tmp0=compute_gep_operand(opr0,second_iteration_flag,compute_second);
			   }
			   else if(ConstantInt *cont=dyn_cast<ConstantInt>(opr0)){
				   dis_tmp0=cont->getSExtValue();
			   }
			   else{
				   errs()<<"The GEP operand is neither integer nor inst. \n";
			   }
			   if(isa<Instruction>(opr1)){
				   dis_tmp1=compute_gep_operand(opr1,second_iteration_flag,compute_second);
			   }
			   else if(ConstantInt *cont1=dyn_cast<ConstantInt>(opr1)){
				   dis_tmp1=cont1->getSExtValue();
			   }
			   else{
				   errs()<<"The GEP operand is neither integer nor inst. \n";
			   }
			   dis_l = dis_tmp0 << dis_tmp1;
			   break;

		   case Instruction::LShr:
		   case Instruction::AShr:
			   opr0=inst_test->getOperand(0);
			   opr1=inst_test->getOperand(1);
			   if(isa<Instruction>(opr0)){
				   dis_tmp0=compute_gep_operand(opr0,second_iteration_flag,compute_second);
			   }
			   else if(ConstantInt *cont=dyn_cast<ConstantInt>(opr0)){
				   dis_tmp0=cont->getSExtValue();
			   }
			   else{
				   errs()<<"The GEP operand is neither integer nor inst. \n";
			   }
			   if(isa<Instruction>(opr1)){
				   dis_tmp1=compute_gep_operand(opr1,second_iteration_flag,compute_second);
			   }
			   else if(ConstantInt *cont1=dyn_cast<ConstantInt>(opr1)){
				   dis_tmp1=cont1->getSExtValue();
			   }
			   else{
				   errs()<<"The GEP operand is neither integer nor inst. \n";
			   }
			   dis_l = dis_tmp0 >> dis_tmp1;
			   break;

		   case Instruction::Or:
			   opr0=inst_test->getOperand(0);
			   opr1=inst_test->getOperand(1);
			   if(isa<Instruction>(opr0)){
				   dis_tmp0=compute_gep_operand(opr0,second_iteration_flag,compute_second);
			   }
			   else if(ConstantInt *cont=dyn_cast<ConstantInt>(opr0)){
				   dis_tmp0=cont->getSExtValue();
			   }
			   else{
				   errs()<<"The GEP operand is neither integer nor inst. \n";
			   }
			   if(isa<Instruction>(opr1)){
				   dis_tmp1=compute_gep_operand(opr1,second_iteration_flag,compute_second);
			   }
			   else if(ConstantInt *cont1=dyn_cast<ConstantInt>(opr1)){
				   dis_tmp1=cont1->getSExtValue();
			   }
			   else{
				   errs()<<"The GEP operand is neither integer nor inst. \n";
			   }
			   dis_l = dis_tmp0|dis_tmp1;
			   break;

		   case Instruction::And:
			   opr0=inst_test->getOperand(0);
			   opr1=inst_test->getOperand(1);
			   if(isa<Instruction>(opr0)){
				   dis_tmp0=compute_gep_operand(opr0,second_iteration_flag,compute_second);
			   }
			   else if(ConstantInt *cont=dyn_cast<ConstantInt>(opr0)){
				   dis_tmp0=cont->getSExtValue();
			   }
			   else{
				   errs()<<"The GEP operand is neither integer nor inst. \n";
			   }
			   if(isa<Instruction>(opr1)){
				   dis_tmp1=compute_gep_operand(opr1,second_iteration_flag,compute_second);
			   }
			   else if(ConstantInt *cont1=dyn_cast<ConstantInt>(opr1)){
				   dis_tmp1=cont1->getSExtValue();
			   }
			   else{
				   errs()<<"The GEP operand is neither integer nor inst. \n";
			   }
			   dis_l = dis_tmp0 & dis_tmp1;
			   break;

		   case Instruction::Xor:
			   opr0=inst_test->getOperand(0);
			   opr1=inst_test->getOperand(1);
			   if(isa<Instruction>(opr0)){
				   dis_tmp0=compute_gep_operand(opr0,second_iteration_flag,compute_second);
			   }
			   else if(ConstantInt *cont=dyn_cast<ConstantInt>(opr0)){
				   dis_tmp0=cont->getSExtValue();
			   }
			   else{
				   errs()<<"The GEP operand is neither integer nor inst. \n";
			   }
			   if(isa<Instruction>(opr1)){
				   dis_tmp1=compute_gep_operand(opr1,second_iteration_flag,compute_second);
			   }
			   else if(ConstantInt *cont1=dyn_cast<ConstantInt>(opr1)){
				   dis_tmp1=cont1->getSExtValue();
			   }
			   else{
				   errs()<<"The GEP operand is neither integer nor inst. \n";
			   }
			   dis_l = dis_tmp0 ^ dis_tmp1;
			   break;

		   case Instruction::Trunc:
		   case Instruction::ZExt:
		   case Instruction::SExt:
		   case Instruction::FPToUI:
		   case Instruction::FPToSI:
		   case Instruction::UIToFP:
		   case Instruction::SIToFP:
		   case Instruction::FPTrunc:
		   case Instruction::FPExt:
		   case Instruction::PtrToInt:
		   case Instruction::IntToPtr:
		   case Instruction::BitCast:
			   opr0=inst_test->getOperand(0);
			   if(isa<Instruction>(opr0)){
				   dis_tmp0=compute_gep_operand(opr0,second_iteration_flag,compute_second);
			   }
			   else if(ConstantInt *cont=dyn_cast<ConstantInt>(opr0)){
				   dis_tmp0=cont->getSExtValue();
			   }
			   else{
				   errs()<<"The GEP operand is neither integer nor inst. \n";
			   }
			   dis_l=dis_tmp0;
			   break;

		   case Instruction::PHI:
			   PHINode *PHI;
			   PHI=dyn_cast<PHINode>(inst_test);
			   if(second_iteration_flag==false){
			   for(unsigned ii=0, ie=PHI->getNumIncomingValues();ii!=ie;ii++)
			   {
				   Value *incoming=PHI->getIncomingValue(ii);
				   if(ConstantInt *cont=dyn_cast<ConstantInt>(incoming)){
					   dis_tmp0=cont->getSExtValue();
					   dis_l=dis_tmp0;
				   }
			   }
			   }
			   else{
				   dis_l=compute_second;
			   }
			   break;
		   default:
			   dis_l=-1;
			   //errs()<<"It's not normal\n";
			   //exit(0);
	   }
	}
	else if(ConstantInt *cont=dyn_cast<ConstantInt>(op)){
	   dis_l=cont->getSExtValue();
	}
	else{
	   errs()<<"The GEP operand is neither integer nor inst. \n";
	}
	return dis_l;
}


int get_phi_second(Loop *L, PHINode *PHI)
{
	int num=0;
	for(unsigned i=0, e=PHI->getNumIncomingValues();i!=e;i++){
	   Value *incoming=PHI->getIncomingValue(i);
	   if(ConstantInt *cont=dyn_cast<ConstantInt>(incoming)){
		   num += cont->getSExtValue();
	   }
	}
	for(unsigned ii=0, ie=PHI->getNumIncomingValues();ii!=ie;ii++){
	   Value *incoming=PHI->getIncomingValue(ii);
	   BasicBlock *bb_incoming=PHI->getIncomingBlock(ii);
	   BasicBlock *latch=L->getLoopLatch();
	   if(bb_incoming==latch){
		   if(Instruction *inst=dyn_cast<Instruction>(incoming)){
			   Value *op0=inst->getOperand(0);
			   PHINode *inst0=dyn_cast<PHINode>(op0);
			   if(inst0==PHI){
				   for(unsigned j=1; j<inst->getNumOperands(); ++j){
					   Value *op=inst->getOperand(j);
					   if(ConstantInt *cont=dyn_cast<ConstantInt>(op)){
						   num += cont->getSExtValue();
					   }
				   }
			   }
		   }
	   }
	}
	return num;
}


PHINode *get_phi(Instruction *inst, PHINode *PHI)
{
	PHINode *phi=NULL;
	for(unsigned i=0; i!= inst->getNumOperands();++i){
		Value *op=inst->getOperand(i);
		Instruction *tmp_inst=dyn_cast<Instruction>(op);
		if(tmp_inst != NULL){
			if(isa<PHINode>(tmp_inst)){
				phi=dyn_cast<PHINode>(tmp_inst);
				if(phi==PHI){
					break;
				}
			}
			else{
				phi=get_phi(tmp_inst,PHI);
				if(phi==PHI){
					break;
				}
			}
		}
	}
	return phi;
}


int get_gep_phi(GetElementPtrInst *gep, PHINode *PHI)
{
	int num=1;
	PHINode *phi=NULL;
	for(User::op_iterator II=gep->idx_begin(),IE=gep->idx_end(); II!=IE; ++II){
		Instruction *tmp_inst=dyn_cast<Instruction>(*II);
		if(tmp_inst != NULL){
			if(isa<PHINode>(tmp_inst)){
				phi=dyn_cast<PHINode>(tmp_inst);
				if(phi==PHI){
					break;
				}
			}
			else{
				phi=get_phi(tmp_inst,PHI);
				if(phi==PHI){
					break;
				}
			}
		}
		num++;
	}
	if(phi!=PHI){
		num=10;
	}
	return num;
}
