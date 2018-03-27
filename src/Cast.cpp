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


#include "Cast.h"

bool inst_is_cast(Instruction *inst)
{
	unsigned opcode=inst->getOpcode();
	if((opcode==Instruction::Trunc)||(opcode==Instruction::ZExt)||(opcode==Instruction::SExt)||(opcode==Instruction::FPTrunc)||(opcode==Instruction::FPExt)){
		return true;
	}
	else{
		return false;
	}

}

Instruction *get_cast_inst(Instruction *inst)
{
	Instruction *op_inst=NULL;
	Value *op=inst->getOperand(0);
	op_inst=dyn_cast<Instruction>(op);
	return op_inst;
}
