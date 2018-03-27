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


#include "InstLibrary.h"
#include "Library.h"
#include "Power.h"

float inst_latency_in_library(Instruction *inst)
{
	float latency=0.0;
	switch(inst->getOpcode()){
		//Terminator Instructions
		case Instruction::Ret:
		case Instruction::Br:
		case Instruction::Switch:
		case Instruction::IndirectBr:
		case Instruction::Invoke:
		case Instruction::Resume:
		case Instruction::Unreachable:
		  break;
		//Standard binary operators
		case Instruction::Add:
		case Instruction::Sub:
		  latency=INT_ADD;
		  break;
		case Instruction::FAdd:
		case Instruction::FSub:
		  latency=FP_ADD;
		  break;
		case Instruction::Mul:
		  Value *opr0;
		  opr0=inst->getOperand(1);
		  if(ConstantInt *cont=dyn_cast<ConstantInt>(opr0)){
			  int opconst=cont->getSExtValue();
			  if(is_power_of_two(opconst)){
				  latency=SHIFT;
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
					  latency=SHL_ADD;
				  }
				  else{
					  if(is_power_of_two(delta)){
						  latency=SHL_ADD;
					  }
					  else{
						  latency=INT_MULT;
					  }
				  }
			  }
		  }
		  else{
			  latency=IMULT;
		  }
		  break;
		case Instruction::FMul:
		  latency=FP_MULT;
		  break;
		case Instruction::SDiv:
		case Instruction::SRem:
		  Value *opr;
		  opr=inst->getOperand(1);
		  if(ConstantInt *cont=dyn_cast<ConstantInt>(opr)){
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
					 SHL_DIV=SELECT_LATENCY+2*INT_ADD;
				  }
				  else if(frequency==200){
					 SHL_DIV=SELECT_LATENCY+INT_ADD;
				  }
				  else if(frequency==250){
					 SHL_DIV=SELECT_LATENCY+INT_ADD;
				  }
				  latency=SHL_DIV;
				  //errs()<<"Ignored: Divided by a contant which is power of two.\n";
			  }
			  else{
				  latency=INT_DIV;
			  }
		  }
		  else{
			  latency=IDIV;
		  }
		  break;
		case Instruction::UDiv:
		case Instruction::URem:
		  Value *opr1;
		  opr1=inst->getOperand(1);
		  if(ConstantInt *cont=dyn_cast<ConstantInt>(opr1)){
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
				  latency=SHL_DIV;
				  //errs()<<"Ignored: Divided by a contant which is power of two.\n";
			  }
			  else{
				  latency=U_DIV;
			  }
		  }
		  else{
			  latency=UDIV;
		  }
		  break;
		case Instruction::FDiv:
		case Instruction::FRem:
		 latency=FP_DIV;
		  break;

		//Logical operators (integer operands)
		case Instruction::Shl:
		case Instruction::LShr:
		case Instruction::AShr:
		 latency=SHIFT;
		  break;
		case Instruction::And:
		case Instruction::Or:
		case Instruction::Xor:
		 latency=INT_ADD;
		  break;
		//Memory operators
		case Instruction::Alloca:
		 latency=ALLOCA_LATENCY;
		 break;
		case Instruction::Load:
		case Instruction::Store:
		 break;

		case Instruction::GetElementPtr:
		 latency=GEP_LATENCY;
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
		 latency=0.0;
		 break;
		case Instruction::PtrToInt:
		case Instruction::IntToPtr:
		case Instruction::BitCast:
		 latency=CAST_LATENCY;
		 break;
		case Instruction::FPToUI:
		case Instruction::FPToSI:
		 latency=FP_TO_SI;
		 break;
		case Instruction::UIToFP:
		case Instruction::SIToFP:
		 latency=SI_TO_FP;
		 break;
		case Instruction::AddrSpaceCast:
		 errs()<<"In AddrSpaceCast, it's not normal.\n";
		 break;
		//Other operators
		case Instruction::ICmp:
		 latency=ICMP_LATENCY;
		  break;
		case Instruction::FCmp:
		 latency=FCMP_LATENCY;
		  break;
		case Instruction::PHI:
		 latency=PHI_LATENCY;
		  break;

		case Instruction::Call:
		  break;

		case Instruction::Select:
		 latency=SELECT_LATENCY;
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
	return latency;
}
