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


#include "Function_II.h"
#include "Constant.h"
#include "ArrayName.h"
#include "LoopUnrollFactor.h"
#include "MemOpNum.h"
#include "PartitionFactor.h"


int compute_Fn_II(Function *F, LoopInfo* LI,std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop,std::vector<std::pair<unsigned,int> > *function_arg_II)
{
	int II = 1;

	for(inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I)
	{
		Instruction *inst=&*I;
		if(CallInst *call=dyn_cast<CallInst>(inst)){
			Function *callee = call->getCalledFunction();
			unsigned FnID=callee->getIntrinsicID();
			if(FnID==0){
				int m=function_II[callee];
				II=std::max(m,II);
			}
		}
	}
	int resMII_inFn=find_Fn_ResMII(F,LI,function_arg_II);
	int resMII_amongFn=find_Fns_ResMII(F,LI,instr_index_loop,dependence_loop,function_arg_II);
	II=std::max(resMII_inFn,II);
	II=std::max(resMII_amongFn,II);

	return II;
}


int find_Fn_ResMII(Function *F, LoopInfo* LI, std::vector<std::pair<unsigned,int> > *function_arg_II)
{
	int resMII=0;
	std::map<Value*, int> array_reads_number, array_writes_number;
	std::map<Value*, int> loop_array_rw;
	std::map<Value*, bool> loop_array_ls;
	std::map<Value*, int> sdp_mode;
	int num_max_tmp=0;
	int num_max=0;
	for(auto itb=F->begin();itb!=F->end();++itb){
		Loop *L=LI->getLoopFor(itb);
		for(auto iti=itb->begin();iti!=itb->end();++iti){
			if(isa<LoadInst>(iti)){
				int unroll_factor=total_unroll_factor_ls(LI,L,iti);
				//errs()<<unroll_factor<<"\n";
				bool array_partition_is_set=false;
				Value *array0=get_array_name(iti);
				int size=1;
				if(array_number.count(array0)){
					 int array_index=array_number[array0];
					 int dim=array_dimension[array_index];
					 int initial=0;
					 for(int j=0; j<array_index; ++j){
						 initial += array_dimension[j];
					 }
					 for(int i=0;i<dim;++i){
						 int j=initial+i;
						 size*=array_size[j];
					 }
				}
				if(!loop_array_ls.count(array0)){
					bool ls=false;
					for(auto ii=itb->begin();ii!=itb->end();++ii){
						if(isa<StoreInst>(ii)){
							Value *store_array=get_array_name(ii);
							if(array0==store_array){
								loop_array_ls[array0]=true;
								ls=true;
								break;
							}
						}
					}
					if(ls==false){
						loop_array_ls[array0]=false;
					}
				}
				if(loop_array_ls[array0]==true){
					if(!sdp_mode.count(array0)){
						int total_bank=partition_factor(iti);
						int load_in_one_bank=partition_mem_op_num(LI,itb,iti);
						int load_total=compute_BB_load_num(itb,iti)*unroll_factor;
						if(load_total>size){
							load_total=size;
						}
						int required_bank=(int)ceil((float)load_total/(float)load_in_one_bank);
						if(required_bank==total_bank){
							if(load_total==total_bank){
								sdp_mode[array0]=1;//The memory is SDP mode.
							}
							else{
								sdp_mode[array0]=0;//The estimated clock period may exceeds the target. Also, II is larger than expected (addition).
							}
						}
						else{
							sdp_mode[array0]=2;//compute as usual.
						}
					}
				}
				int num_reads=0;
				if(array_number.count(array0)){
					int array_index=array_number[array0];
					for(std::vector< std::pair<int,int> >::iterator it= array_partition[array_index].begin(); it!=array_partition[array_index].end(); ++it)
					{
						if(it->second!=1){
							array_partition_is_set=true;
							break;
						}
					}
					if(array_partition_is_set==false){
						num_reads=compute_BB_load_num(itb,iti)*unroll_factor;
		            	if(num_reads>size){
		            		num_reads=size;
		            	}
					}
					else{
						num_reads=partition_mem_op_num(LI,itb,iti);
					}
					array_reads_number[array0]=std::max(array_reads_number[array0],num_reads);
				}
			}
			else if(isa<StoreInst>(iti)){
				int unroll_factor=total_unroll_factor_ls(LI,L,iti);
				bool array_partition_is_set=false;
				Value *array1=get_array_name(iti);
				int num_writes=0;
				if(array_number.count(array1)){
					int array_index=array_number[array1];
					for(std::vector< std::pair<int,int> >::iterator it= array_partition[array_index].begin(); it!=array_partition[array_index].end(); ++it)
					{
						if(it->second!=1){
							array_partition_is_set=true;
							break;
						}
					}
					if(array_partition_is_set==false){
						num_writes=compute_BB_store_num(itb,iti)*unroll_factor;
					}
					else{
						num_writes=partition_mem_op_num(LI,itb,iti);
					}
					array_writes_number[array1]=std::max(array_writes_number[array1],num_writes);
				}
			}
		}
	}
	int function_latency=function_cycles[F];
	std::map<Value*, int>::iterator it;
	int res_ii_read_max=0;
	int res_ii_write_max=0;
	for(it=array_reads_number.begin();it!=array_reads_number.end();++it){
		int res_ii=(int)(ceil)((float)it->second/(float)port_num_read);
		res_ii_read_max=std::max(res_ii,res_ii_read_max);
	}
	for(it=array_writes_number.begin();it!=array_writes_number.end();++it){
		int res_ii=(int)(ceil)((float)it->second/(float)port_num_write_diff);
		res_ii_write_max=std::max(res_ii,res_ii_write_max);
	}
	for(it=array_reads_number.begin();it!=array_reads_number.end();++it){
		Value* array=it->first;
		loop_array_rw[array]=res_ii_read_max;
	}
	for(it=array_writes_number.begin();it!=array_writes_number.end();++it){
		Value* array=it->first;
		if(loop_array_ls.count(array)){
			if(loop_array_ls[array]==true){
				if(sdp_mode.count(array)){
					if(sdp_mode[array]==1){
						loop_array_rw[array] = std::max(loop_array_rw[array],res_ii_write_max);
					}
					else{
						loop_array_rw[array] += res_ii_write_max;
					}
				}
				else{
					errs()<<"Please check: something goes wrong!\n";
				}
			}
			else{
				loop_array_rw[array] += res_ii_write_max;
			}
		}
		else{
			loop_array_rw[array] += res_ii_write_max;
		}
	}
	for(it=loop_array_rw.begin();it!=loop_array_rw.end();++it){
		int num=it->second;
		num_max_tmp=std::max(num_max_tmp,num);
	}
	int delta=(int)function_latency-num_max_tmp;
	if(delta<0){
		delta=-delta;
	}
	if(delta<2){
		resMII=num_max_tmp;
	}
	else{
		loop_array_rw.clear();
		for(it=array_reads_number.begin();it!=array_reads_number.end();++it){
			Value* array=it->first;
			int res_ii=(int)(ceil)((float)it->second/(float)port_num_read);
			loop_array_rw[array]=res_ii;
		}
		for(it=array_writes_number.begin();it!=array_writes_number.end();++it){
			Value* array=it->first;
			int res_ii=(int)(ceil)((float)it->second/(float)port_num_write_diff);
			if(loop_array_ls.count(array)){
				if(loop_array_ls[array]==true){
					if(sdp_mode.count(array)){
						if(sdp_mode[array]==1){
							loop_array_rw[array] = std::max(loop_array_rw[array],res_ii);
						}
						else{
							loop_array_rw[array] += res_ii;
						}
					}
					else{
						errs()<<"Please check: something goes wrong!\n";
					}
				}
				else{
					loop_array_rw[array] += res_ii;
				}
			}
			else{
				loop_array_rw[array] += res_ii;
			}
		}
		for(it=loop_array_rw.begin();it!=loop_array_rw.end();++it){
			int num=it->second;
			num_max=std::max(num_max,num);
		}
		resMII= num_max;
	}
	for(it=loop_array_rw.begin();it!=loop_array_rw.end();++it){
		Value *array=it->first;
		int ii=it->second;
		unsigned index_tmp=Function_index[F];
		bool Argnum_is_variable=F->isVarArg();
		if(Argnum_is_variable==false){
			if(!F->arg_empty()){
				for(Function::arg_iterator ArgI=F->arg_begin(),ArgE=F->arg_end();ArgI!=ArgE; ++ArgI){
					Argument *argu=ArgI;
					unsigned argNo=argu->getArgNo();
					Value *val=dyn_cast<Value>(argu);
					Type *arg_type=val->getType();
					bool arg_is_array = arg_type->isPointerTy()||arg_type->isArrayTy();
					if(arg_is_array==true){
						if(val==array){
							function_arg_II[index_tmp].push_back(std::make_pair(argNo,ii));
						}
					}
				}
			}
		}
	}
	return resMII;
}


bool is_call_argument(CallInst *call, Value *arg)
{
	bool is_arg=false;
	for(unsigned op=0;op<call->getNumArgOperands();++op){
		Value *OP=call->getArgOperand(op);
		Type *OP_type = OP->getType();
		bool is_array = OP_type->isPointerTy()||OP_type->isArrayTy();
		if(is_array==true){
			if(!isa<Instruction>(OP)){
				if(OP==arg){
					is_arg=true;
					break;
				}
			}
			else{
				if(GetElementPtrInst *gep=dyn_cast<GetElementPtrInst>(OP)){
					Value *gep_pointer=gep->getPointerOperand();
					if(gep_pointer==arg){
						is_arg=true;
						break;
					}
				}
			}
		}
	}
	return is_arg;
}


unsigned get_arg_number(CallInst *call, Value *arg)
{
	unsigned argNo=0;
	for(unsigned op=0;op<call->getNumArgOperands();++op){
		Value *OP=call->getArgOperand(op);
		if(arg==OP){
			argNo=op;
			break;
		}
	}
	return argNo;
}


int find_Fns_ResMII(Function *F, LoopInfo* LI, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop,std::vector<std::pair<unsigned,int> > *function_arg_II)
{
	int ResMII=0;
	bool has_subfunction=false;
	for(inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I){
		Instruction *inst=&*I;
		if(CallInst *call=dyn_cast<CallInst>(inst)){
			Function *callee = call->getCalledFunction();
			unsigned FnID=callee->getIntrinsicID();
			if(FnID==0){
				has_subfunction=true;
			}
		}
	}
	std::map<Value*,int> Function_array;
	int Function_array_index=0;
	if(has_subfunction==true){
		for(inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I){
			Instruction *inst=&*I;
			if(CallInst *call=dyn_cast<CallInst>(inst)){
				Function *callee = call->getCalledFunction();
				unsigned FnID=callee->getIntrinsicID();
				if(FnID==0){
					for(unsigned op=0;op<call->getNumArgOperands();++op){
						Value *OP=call->getArgOperand(op);
						Type *OP_type = OP->getType();
						bool is_array = OP_type->isPointerTy()||OP_type->isArrayTy();
						if(is_array==true){
							if(!isa<Instruction>(OP)){
								if(!Function_array.count(OP)){
									Function_array_index++;
									Function_array[OP]=Function_array_index;
								}
							}
							else{
								if(GetElementPtrInst *gep=dyn_cast<GetElementPtrInst>(OP)){
									Value *gep_pointer=gep->getPointerOperand();
									if(!Function_array.count(gep_pointer)){
										Function_array_index++;
										Function_array[gep_pointer]=Function_array_index;
									}
								}
							}
						}
					}
				}
			}
		}
		for(std::map<Value*,int>::iterator it= Function_array.begin();it!=Function_array.end();++it){
			Value *array=it->first;
			//float max_latency=0.0;
			//float min_latency=inf;
			int delta=0;
			for(inst_iterator i = inst_begin(F), e = inst_end(F); i != e; ++i){
				Instruction *inst=&*i;
				if(CallInst *call=dyn_cast<CallInst>(inst)){
					Function *callee = call->getCalledFunction();
					unsigned FnID=callee->getIntrinsicID();
					if(FnID==0){//simply the model and select the longest latency as the II.(In general cases, need to consider not continuous cases and distance between function copies conflict)
						 if(is_call_argument(call,array)==true){
							 unsigned argno=get_arg_number(call,array);
							 unsigned callee_index=Function_index[callee];
							 for(std::vector<std::pair<unsigned,int> >::iterator ii=function_arg_II[callee_index].begin();ii!=function_arg_II[callee_index].end();++ii){
								 if(argno==ii->first){
									 int array_ii=ii->second;
									 delta+=array_ii;
								 }
							 }
						 }
					}
				}
			}
			ResMII=std::max(ResMII,delta);
		}
	}
	return ResMII;
}

