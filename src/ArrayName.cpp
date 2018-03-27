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


#include "ArrayName.h"


Value *check_PHI_operand(PHINode *phi)
{
	Value *val=NULL;
	Value *val_tmp=NULL;
	for(unsigned i=0, e=phi->getNumIncomingValues();i!=e;i++)
	{
		Value *incoming=phi->getIncomingValue(i);
		if( (!(isa<GetElementPtrInst>(incoming))) && (!(isa<PHINode>(incoming))) )
		{
			val=incoming;
			return val;  //consider most cases to break and continue
		}
		else if(GetElementPtrInst *GEP_incoming=dyn_cast<GetElementPtrInst>(incoming))
		{
			val_tmp=GEP_incoming->getPointerOperand();
			if(isa<PHINode>(val_tmp))
			{
				if(val_tmp==phi){
					val=val_tmp;
					//return val;
				}
				else{
					val=0;
				}
			}
			else{
				val=val_tmp;
				return val;
			}
		}
		else if(isa<PHINode>(incoming))
		{
			val=0;
			continue;
		}
	}
	return val;
}


Value *get_array_name(Instruction *inst)
{
	Value *array_name=NULL;
	Value *op=NULL;

	if(LoadInst *load_inst=dyn_cast<LoadInst>(inst)){
		op=load_inst->getPointerOperand();
	}
	else if(StoreInst *store_inst=dyn_cast<StoreInst>(inst)){
		op=store_inst->getPointerOperand();
	}
	//else errs()<<"This instruction is neither a load nor a store.\n";

	if(GetElementPtrInst *OP_GEP=dyn_cast<GetElementPtrInst>(op)){
		array_name=OP_GEP->getPointerOperand();
	}
	else if(PHINode *OP_PHI=dyn_cast<PHINode>(op)){
		array_name=check_PHI_operand(OP_PHI);
	}
	else if(Instruction *OP_Inst=dyn_cast<Instruction>(op)){
		for(unsigned i=0; i!=OP_Inst->getNumOperands(); ++i){
			Value *OP = OP_Inst->getOperand(i);
			if(GetElementPtrInst *tmp_inst = dyn_cast<GetElementPtrInst>(OP)){
				array_name=tmp_inst->getPointerOperand();
				break;
			}
			else{
				array_name=OP;//Modified
			}
		}
	}
	else{
		array_name=op;
	}
	return array_name;
}
