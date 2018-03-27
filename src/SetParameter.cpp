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


#include "SetParameter.h"
#include "LoopInterface.h"
#include "SetPragma.h"
#include "ArrayName.h"
#include "GEP.h"


void set_loop_map(Loop *L,Loop *parent,Loop *loop,std::map<Loop*,int> loop_index_tmp,std::vector<int> *loop_index)
{
	if(parent==L||L->contains(parent)){
		if(loop_index_tmp.count(parent)){
			int l_index=loop_index_tmp[parent];
			loop_index[l_index].push_back(loop_index_tmp[loop]);
			Loop *parent_parent=parent->getParentLoop();
			if(parent_parent!=NULL){
				set_loop_map(L,parent_parent,loop,loop_index_tmp,loop_index);
			}
		}
		else{
			errs()<<"Wrong@@@\n";
		}
	}
}

void set_loop_index(Function *F, LoopInfo* LI, Loop *L,std::map<Loop*,int> &loop_index_tmp,std::vector<int> *loop_index)
{
	unsigned F_index=Function_index[F];
	int loop_index_begin=0;
	for(int i=0;i<(int)F_index;++i){
		loop_index_begin+=function_loop_num[i];
	}
	std::queue<Loop*> loop_queue;
	for (LoopInfo::reverse_iterator i = LI->rbegin(), e = LI->rend(); i != e; ++i){
		Loop *loop = *i;
		if(loop==L||L->contains(loop)){
			loop_index_tmp[loop]=loop_index_begin;
		}
		loop_index_begin++;
		std::vector<Loop*> subLoops = loop->getSubLoops();
		if(!subLoops.empty()){
			for (unsigned ii=0; ii<subLoops.size(); ii++){
				Loop* sub_loop = subLoops.at(ii);
				loop_queue.push(sub_loop);
			}
		}
	}
	while (!loop_queue.empty())
	{
		Loop* loop_test = loop_queue.front();
		loop_queue.pop();
		if(loop_test==L||L->contains(loop_test)){
			loop_index_tmp[loop_test]=loop_index_begin;
		}
		loop_index_begin++;
		std::vector<Loop*> subLoops = loop_test->getSubLoops();
		for (unsigned ii=0; ii<subLoops.size(); ii++){
			 Loop* sub_loop = subLoops.at(ii);
			 loop_queue.push(sub_loop);
		}
	}
	for(std::map<Loop*,int>::iterator it=loop_index_tmp.begin();it!=loop_index_tmp.end();++it){
		Loop *loop=it->first;
		Loop *parent=loop->getParentLoop();
		if(parent!=NULL){
			set_loop_map(L,parent,loop,loop_index_tmp,loop_index);
		}
	}
}

void set_array_index(Module &M)
{
	array_number.clear();
	int index_array=0;
	for(auto F=M.begin(), E=M.end(); F!=E; ++F){
		unsigned FnID=F->getIntrinsicID();
		if(FnID==0){
			if(function_is_inline(F)==false){
				Value *array_name=NULL;
				for(inst_iterator I=inst_begin(F), E=inst_end(F); I != E; ++I){
					Instruction *inst = &*I;
					if(isa<LoadInst>(inst)){
						array_name=get_array_name(inst);
						if(array_name->getValueID()!=Value::GlobalVariableVal){
							if(array_name != NULL){
								if(!array_number.count(array_name)){
									array_number[array_name]=index_array;
									index_array++;
								}
							}
						}
					}
					else if(isa<StoreInst>(inst)){
						array_name=get_array_name(inst);
						if(array_name->getValueID()!=Value::GlobalVariableVal){
							//it=array_partition_block.find(array_name);
							if(array_name != NULL){
								if(!array_number.count(array_name)){
									array_number[array_name]=index_array;
									index_array++;
								}
							}
						}
					}
					else if(CallInst *call=dyn_cast<CallInst>(inst)){
						Function *callee=call->getCalledFunction();
						unsigned FnID=callee->getIntrinsicID();
						if(FnID==0){
							for(unsigned op=0;op<call->getNumArgOperands();++op){
								Value *OP=call->getArgOperand(op);
								Type *OP_type = OP->getType();
								bool is_array = OP_type->isPointerTy()||OP_type->isArrayTy();
								if(is_array==true){
									if(OP->getValueID()!=Value::GlobalVariableVal){
										if(!isa<Instruction>(OP)){
											array_name=OP;
											if(array_name != NULL){
												if(!array_number.count(array_name)){
													array_number[array_name]=index_array;
													index_array++;
												}
											}
										}
										else{
											if(GetElementPtrInst *gep=dyn_cast<GetElementPtrInst>(OP)){
												Value *gep_pointer=gep->getPointerOperand();
												if(gep_pointer->getValueID()!=Value::GlobalVariableVal){
													array_name=gep_pointer;
													if(array_name != NULL){
														if(!array_number.count(array_name)){
															array_number[array_name]=index_array;
															index_array++;
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
	//Test the order of arrays..........
	/*for(std::map<Value*,int>::iterator it=array_number.begin();it!=array_number.end();++it){
		errs()<<(it->first)->getName()<<it->second<<"\n";
	}*/
}

void set_array_map(Module &M)//only map function and its sub_function, lower level (sub_sub) doesn't consider(don't need)
{
	array_map_array.clear();
	for(auto F=M.begin(), E=M.end(); F!=E; ++F){
		unsigned FnID=F->getIntrinsicID();
		if(FnID==0){
			if(function_is_inline(F)==false){
				//unsigned F_index=Function_index[F];
				for(inst_iterator I=inst_begin(F), E=inst_end(F); I != E; ++I){
					Instruction *inst=&*I;
					if(CallInst *call=dyn_cast<CallInst>(inst)){
						Function *callee=call->getCalledFunction();
						unsigned Fnid=callee->getIntrinsicID();
						if(Fnid==0){
							for(unsigned op=0;op<call->getNumArgOperands();++op){
								Value *OP=call->getArgOperand(op);
								Type *OP_type = OP->getType();
								bool is_array = OP_type->isPointerTy()||OP_type->isArrayTy();
								if(is_array==true){
									Value *call_array=NULL;
									if(OP->getValueID()!=Value::GlobalVariableVal){
										if(!isa<Instruction>(OP)){
											call_array=OP;
										}
										else{
											if(GetElementPtrInst *gep=dyn_cast<GetElementPtrInst>(OP)){
												Value *gep_pointer=gep->getPointerOperand();
												if(gep_pointer->getValueID()!=Value::GlobalVariableVal){
													call_array=gep_pointer;
												}
											}
										}
									}
									if(call_array!=NULL&&array_number.count(call_array)){
										unsigned numth=0;
										for(Function::arg_iterator it=callee->arg_begin();it!=callee->arg_end();++it){
											if(numth==op){
												Value *callee_op=dyn_cast<Value>(it);
												if(array_number.count(callee_op)){
													array_map_array[callee_op]=call_array;
												}
											}
											numth++;
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



/*void set_array_map(Module &M)
{
	for(auto F=M.begin(), E=M.end(); F!=E; ++F){
		unsigned FnID=F->getIntrinsicID();
		if(FnID==0){
			if(function_is_inline(F)==false){
				for(inst_iterator I=inst_begin(F), E=inst_end(F); I != E; ++I){
					Instruction *inst = &*I;
					if(CallInst *call=dyn_cast<CallInst>(inst)){
						Function *callee=call->getCalledFunction();
						unsigned Fnid=callee->getIntrinsicID();
						if(Fnid==0){
							if(function_is_inline(callee)==false){
								if(callee!=F){

								}
							}

							int i=0;
							for(Function::arg_iterator it=callee->arg_begin();it!=callee->arg_end();++it){
								Value *arg=dyn_cast<Value>(it);
								Value *op=call->getArgOperand(i);
								Type *op_type=op->getType();
								bool is_array = op_type->isPointerTy()||op_type->isArrayTy();
								Value *array_op;
								if(is_array==true){
									if(!isa<Instruction>(op)){
										array_op=op;
									}
									else{
										if(GetElementPtrInst *gep=dyn_cast<GetElementPtrInst>(op)){
											Value *gep_pointer=gep->getPointerOperand();
											array_op=gep_pointer;
										}
									}
								}
								if(array_op!=NULL&&arg!=NULL){
									if(array_number.count(array_op)){
										if(array_number.count(arg)){
											int arg_index=array_number[arg];
											int op_index=array_number[array_op];
											int arg_dim=array_dimension[arg_index];
											int op_dim=array_dimension[op_index];
											if(op_dim!=arg_dim){
												errs()<<"ERROR: Formal argument is different with actual argument !\n";
											}
											else{
												array_partition[arg_index]=array_partition[op_index];
												//errs()<<"Formal argument is set again due to calling function.\n";
											}
										}
									}
									else{
										if(array_op->getValueID()==Value::GlobalVariableVal){
											if(array_number.count(arg)){
												int arg_index=array_number[arg];
												//int arg_dim=array_dimension[arg_index];
												if(array_partition[arg_index].empty()){
													errs()<<"Check: empty come here...\n";
												}
												if(!array_partition[arg_index].empty()){
													for(std::vector<std::pair<int,int> >::iterator ii=array_partition[arg_index].begin();ii!=array_partition[arg_index].end();++ii){
														ii->second=1;
														//errs()<<"Formal argument is set to factor 1 due to global variable using.\n";
													}
												}

											}
										}
									}
								}
								i++;
							}
							for(inst_iterator I=inst_begin(callee), E=inst_end(callee); I != E; ++I){
								Instruction *inst=&*I;
								if(CallInst *call_inst=dyn_cast<CallInst>(inst)){
									Function *sub_callee=call_inst->getCalledFunction();
									unsigned Fnid=sub_callee->getIntrinsicID();
									if(Fnid==0){
										if(sub_callee!=callee){
											keep_array_consistent(sub_callee,call_inst);
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
}*/

void store_function_IO(Module &M)
{
	for(int i=0;i<30;++i){
		Function_read_arg[i].clear();
		Function_write_arg[i].clear();
		Function_tmp_arg[i].clear();
	}
	for(auto F=M.begin(), E=M.end(); F!=E; ++F){
		unsigned FnID=F->getIntrinsicID();
		if(FnID==0){
			if(function_is_inline(F)==false){
				unsigned index_tmp=Function_index[F];
				//int arg_num=F->arg_size();
				//errs()<<"#Arguments: "<<arg_num<<"\n";
				bool Argnum_is_variable=F->isVarArg();
				if(Argnum_is_variable==false){
					if(!F->arg_empty()){
						for(Function::arg_iterator ArgI=F->arg_begin(),ArgE=F->arg_end();ArgI!=ArgE; ++ArgI)
						{
							Argument *argu=ArgI;
							unsigned argNo=argu->getArgNo();
							bool attr_read_only=argu->onlyReadsMemory();
							if(attr_read_only==true){
								Function_read_arg[index_tmp].push_back(argNo);
							}
							else{
								Value *val=dyn_cast<Value>(argu);
								Type *arg_type=val->getType();
								bool arg_is_array = arg_type->isPointerTy()||arg_type->isArrayTy();
								bool arg_is_tmp = arg_type->isFloatingPointTy()||arg_type->isIntegerTy();
								if(arg_is_tmp==true && arg_is_array==false){
									Function_tmp_arg[index_tmp].push_back(argNo);
								}
								else if(arg_is_array==true){
									Function_write_arg[index_tmp].push_back(argNo);
									for(inst_iterator I=inst_begin(F), E=inst_end(F);I!=E;++I){
										Instruction *inst = &*I;
										if(isa<LoadInst>(inst)){
											Value *array=get_array_name(inst);
											if(array==val){
												Function_read_arg[index_tmp].push_back(argNo);
												break;
											}
										}
									}
								}
								else{
									errs()<<"The "<<argNo<<"-th argument is special one, please check.\n";
								}
							}
						}
					}
					else{
						errs()<<"This function has no formal argument.\n";
					}
				}
				else{
					errs()<<"Warning:May not do HLS because this function takes a variable number of arguments.\n";
				}
			}
		}
	}
}
