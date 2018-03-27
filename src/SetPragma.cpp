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


#include "SetPragma.h"


void set_dataflow(Module &M)
{
	for(auto F=M.begin(), E=M.end(); F!=E; ++F){
		unsigned FnID=F->getIntrinsicID();
		if(FnID==0){
			if(function_is_inline(F)==false){
				unsigned F_index=Function_index[F];
				Function_dataflow[F]=dataflow_input[F_index];
			}
		}
		/*else{
			errs()<<"This is intrinsic function of llvm.\n";
		}*/
	}
	for(auto F=M.begin(), E=M.end(); F!=E; ++F){
		unsigned FnID=F->getIntrinsicID();
		if(FnID==0){
			if(function_is_inline(F)==false){
				for(inst_iterator I=inst_begin(F), IE=inst_end(F); I!=IE; ++I){
					Instruction *inst= &*I;
					if(CallInst *call=dyn_cast<CallInst>(inst)){
						Function *callee=call->getCalledFunction();
						if(Function_dataflow.count(callee)){
							Function_dataflow[callee]=0;
							unsigned callee_index=Function_index[callee];
							dataflow_input[callee_index]=0;
							//StringRef FunName=callee->getName();
							//errs()<<FunName<<": Subfunctions cannot dataflow.\n";
						}
					}
				}
			}

		}
	}
}

bool function_is_inline(Function *F)
{
	 bool inline_function=false;
	 inline_function=F->hasFnAttribute(Attribute::AlwaysInline);
	 return inline_function;
}

bool function_is_noinline(Function *F)
{
	 bool noinline_function=false;
	 noinline_function=F->hasFnAttribute(Attribute::NoInline);
	 return noinline_function;
}

bool has_subFn(Function *F)
{
	bool Fn_has_subFn=false;
	for(inst_iterator I=inst_begin(F), IE=inst_end(F); I!=IE; ++I){
		Instruction *inst= &*I;
		if(CallInst *call=dyn_cast<CallInst>(inst)){
			Function *callee=call->getCalledFunction();
			unsigned FnID=callee->getIntrinsicID();
			if(FnID==0){
				Fn_has_subFn=true;
			}
		}
	}
	return Fn_has_subFn;
}

bool Fn_pipeline_modify(Function *F)
{
	unsigned FnID=F->getIntrinsicID();
	if(FnID==0){
		bool Fn_has_subFn=has_subFn(F);
		if(Fn_has_subFn==true){
			if(Function_pipeline[F]==1){
				for(inst_iterator I=inst_begin(F), IE=inst_end(F); I!=IE; ++I){
					Instruction *inst= &*I;
					if(CallInst *call=dyn_cast<CallInst>(inst)){
						Function *callee=call->getCalledFunction();
						if(Function_pipeline.count(callee)){
							Function_pipeline[callee]=1;
							unsigned callee_index=Function_index[callee];
							function_pipeline_input[callee_index]=1;
							//StringRef FunName=callee->getName();
							//errs()<<FunName<<": Subfunctions should be pipelined when top is pipelined.\n";
							bool sub_modify=Fn_pipeline_modify(callee);
							return sub_modify;
						}
					}
				}
			}
		}
	}
	return true;
}

void set_function_pipeline(Module &M)
{
	for(auto F=M.begin(), E=M.end(); F!=E; ++F){
		unsigned FnID=F->getIntrinsicID();
		if(FnID==0){
			if(function_is_inline(F)==false){
				unsigned F_index=Function_index[F];
				Function_pipeline[F]=function_pipeline_input[F_index];
			}
		}
		/*else{
			errs()<<"This is intrinsic function of llvm.\n";
		}*/
	}
	for(auto F=M.begin(), E=M.end(); F!=E; ++F){
		unsigned FnID=F->getIntrinsicID();
		if(FnID==0){
			if(function_is_inline(F)==false){
				if(Function_dataflow.count(F)){
					if(Function_dataflow[F]==1){
						Function_pipeline[F]=0;
						unsigned F_index=Function_index[F];
					    function_pipeline_input[F_index]=0;
						//StringRef FunName=F->getName();
						//errs()<<"Function pipeline is ignored when dataflow detected.\n";
						//errs()<<"Function: "<<FunName<<" should be top function.\n";
					}
				}
			}
		}
	}
	for(auto F=M.begin(), E=M.end(); F!=E; ++F){
		unsigned FnID=F->getIntrinsicID();
		if(FnID==0){
			if(function_is_inline(F)==false){
				if(Function_pipeline[F]==1){
					for(inst_iterator I=inst_begin(F), IE=inst_end(F); I!=IE; ++I){
						Instruction *inst= &*I;
						if(CallInst *call=dyn_cast<CallInst>(inst)){
							Function *callee=call->getCalledFunction();
							if(Function_pipeline.count(callee)){
								Function_pipeline[callee]=1;
								unsigned callee_index=Function_index[callee];
								function_pipeline_input[callee_index]=1;
								//StringRef FunName=callee->getName();
								//errs()<<"Function: "<<FunName<<": Subfunctions should be pipelined when top is pipelined.\n";
								bool sub_modify=Fn_pipeline_modify(callee);
								if(sub_modify==false){
									errs()<<"Pipeline of subfunction under function is not set correctly.\n";
								}
							}
						}
					}
				}
			}
		}
	}
}

bool set_subFn_pipeline(Loop *L)
{
	if(Loops_pipeline[L]==1){
		for(Loop::block_iterator BI=L->block_begin(), BE=L->block_end(); BI !=BE; ++BI){
				BasicBlock *bb=*BI;
				for(auto iti=bb->begin();iti!=bb->end();++iti)
				{
					if(CallInst *call=dyn_cast<CallInst>(iti)){
						Function *callee=call->getCalledFunction();
						if(Function_pipeline.count(callee)){
							Function_pipeline[callee]=1;
							unsigned callee_index=Function_index[callee];
							function_pipeline_input[callee_index]=1;
							//StringRef FunName=callee->getName();
							//errs()<<"Function: "<<FunName<<": is set to function pipelining due to its parent loop pipelining.\n";
							bool sub_mod=Fn_pipeline_modify(callee);
							if(sub_mod==false){
								errs()<<"Pipeline of subfunction under function is not set correctly.\n";
							}
						}
					}
				}
		}
	}
	else{
		std::vector<Loop*> subLoops = L->getSubLoops();
		if( !subLoops.empty() ){
            for (unsigned ii=0; ii<subLoops.size(); ii++)
			{
		        Loop* sub_loop = subLoops.at(ii);
		        bool subFn_pipeline=set_subFn_pipeline(sub_loop);
		        if(subFn_pipeline==false){
		        	errs()<<"Pipeline of subFunctions under loop is not set correctly.\n";
		        }
			}
	    }
	}
	return true;
}

void set_loop_counter_ul_pipeline(LoopInfo* LI, Function *F) //using queue to implement BFS.
{
	unsigned F_index=Function_index[F];
	int loop_index=0;
	for(int i=0;i<(int)F_index;++i){
		loop_index+=function_loop_num[i];
	}
	std::map<Loop*,int> loop_map_index;
	std::queue<Loop*> loop_queue;
	for (LoopInfo::reverse_iterator i = LI->rbegin(), e = LI->rend(); i != e; ++i){
        Loop *L = *i;
		Loops_counter[L] = Loops_counter_input[loop_index];
		Loops_unroll[L] = Loops_unroll_input[loop_index];
		Loops_pipeline[L] = Loops_pipeline_input[loop_index];
		loop_map_index[L]=loop_index;
        loop_index++;
		std::vector<Loop*> subLoops = L->getSubLoops();
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
      	Loops_counter[loop_test] = Loops_counter_input[loop_index];
      	Loops_unroll[loop_test] = Loops_unroll_input[loop_index];
      	Loops_pipeline[loop_test] = Loops_pipeline_input[loop_index];
      	loop_map_index[loop_test]=loop_index;
      	loop_index++;
        std::vector<Loop*> subLoops = loop_test->getSubLoops();
      	for (unsigned ii=0; ii<subLoops.size(); ii++){
      		 Loop* sub_loop = subLoops.at(ii);
      		 loop_queue.push(sub_loop);
      	}
	}
	//Function/loop pipeline influence sub-loops pipelining
	if(Function_pipeline[F]==0){
		for(LoopInfo::reverse_iterator i = LI->rbegin(), e = LI->rend(); i!=e; ++i){
			Loop *L=*i;
			if(Loops_unroll[L]>=Loops_counter[L]){
				Loops_pipeline[L]=0;  //loop pipeline will be ignored if it is unrolled completely.
				int l_index=loop_map_index[L];
				Loops_pipeline_input[l_index]=0;
			}
			std::vector<Loop*> subloops=L->getSubLoops();
			if(!subloops.empty()){
				for(unsigned ii=0; ii< subloops.size(); ii++){
					Loop* sub_loop=subloops.at(ii);
					loop_queue.push(sub_loop);
				}
				while(!loop_queue.empty())
				{
					Loop* sub_loop_tmp=loop_queue.front();
					loop_queue.pop();
					int test_pipeline=0;
					Loop *loop_parent=sub_loop_tmp->getParentLoop();
					while(loop_parent != NULL){
						if(Loops_pipeline[loop_parent]==1){
							test_pipeline++;
						}
						loop_parent=loop_parent->getParentLoop();
					}
					if(test_pipeline>=1)
					{
						Loops_unroll[sub_loop_tmp]=Loops_counter[sub_loop_tmp];
						Loops_pipeline[sub_loop_tmp]=0;
						int sub_l_index=loop_map_index[sub_loop_tmp];
						Loops_unroll_input[sub_l_index]=Loops_unroll[sub_loop_tmp];
						Loops_pipeline_input[sub_l_index]=0;
        		    	  //errs()<<"SubLoop: "<<sub_loop_tmp<<" is unrolled completely due to loop pipeling.\n";
					}
					else if(test_pipeline==0)
					{
        		    	  //Loops_unroll[sub_loop_tmp]=Loops_unroll[sub_loop_tmp];
        		    	  //errs()<<"SubLoop: "<<sub_loop_tmp<<" doesn't change its directives setting.\n";
					}
					else errs()<<"It may be wrong when setting unrolling factor when pipelining\n";
					if(Loops_unroll[sub_loop_tmp]>=Loops_counter[sub_loop_tmp]){
						Loops_pipeline[sub_loop_tmp]=0;
						int sub_l_index=loop_map_index[sub_loop_tmp];
						Loops_pipeline_input[sub_l_index]=0;
					}
					std::vector<Loop*> subLoops_tmp = sub_loop_tmp->getSubLoops();
					for (unsigned ii=0; ii<subLoops_tmp.size(); ii++)
					{
						Loop* sub_loop = subLoops_tmp.at(ii);
						loop_queue.push(sub_loop);
					}
				}
			}
		}
	}
	else if(Function_pipeline[F]==1){
		for(LoopInfo::reverse_iterator i = LI->rbegin(), e = LI->rend(); i!=e; ++i){
			Loop *L=*i;
			Loops_unroll[L]=Loops_counter[L];
			Loops_pipeline[L]=0;
			int l_index=loop_map_index[L];
			Loops_unroll_input[l_index]=Loops_unroll[L];
			Loops_pipeline_input[l_index]=0;
			//errs()<<"Loop: "<<L<<" is unrolled completely due to function pipeling.\n";
			std::vector<Loop*> subloops=L->getSubLoops();
			if(!subloops.empty())
			{
				for(unsigned ii=0; ii< subloops.size(); ii++)
				{
					Loop* sub_loop=subloops.at(ii);
					//errs()<<"Loop: "<<sub_loop<<" is unrolled completely due to function pipeling.\n";
					loop_queue.push(sub_loop);
				}
			}
		}
		while(!loop_queue.empty()){
			Loop *loop_test=loop_queue.front();
			loop_queue.pop();
			Loops_unroll[loop_test]=Loops_counter[loop_test];
			Loops_pipeline[loop_test]=0;
			int ltest_index=loop_map_index[loop_test];
			Loops_unroll_input[ltest_index]=Loops_unroll[loop_test];
			Loops_pipeline_input[ltest_index]=0;
			std::vector<Loop*> subLoops = loop_test->getSubLoops();
			for (unsigned ii=0; ii<subLoops.size(); ii++)
			{
				Loop* sub_loop = subLoops.at(ii);
				loop_queue.push(sub_loop);
			}
		}
	}
	else{
		errs()<<"Function pipeline is not set correctly.\n";
	}
	//Loop pipeline influence sub-function pipelining
	for (LoopInfo::reverse_iterator i = LI->rbegin(), e = LI->rend(); i != e; ++i)
	{
		Loop *L = *i;
        bool set_subFn=set_subFn_pipeline(L);
        if(set_subFn==false){
        	errs()<<"Pipelining of subFun is not set correctly.\n";
        }
	}
}

void set_array_partition(Module &M)
{
	int size=array_number.size();
	int accumulated_dim=0;
	for(int i=0;i<size;++i){
		for(std::map<Value*,int>::iterator it=array_number.begin();it!=array_number.end();++it){
			if(it->second==i){
				int array_index=it->second;
				int dim=array_dimension[array_index];
				for(int j=0; j<dim;++j){
					int new_j = j+accumulated_dim;
					array_partition[array_index].push_back(std::make_pair(array_partition_type[new_j],array_partition_factor[new_j]));
					//errs()<<"array_index: "<<array_index<<" and type: "<<array_partition_type[new_j]<<" and factor "<<array_partition_factor[new_j]<<"\n";
				}
				accumulated_dim += dim;
			}
		}
	}
	std::map<Function*,int> Function_array_set;
	int order=0;
	for(auto F=M.begin(), E=M.end(); F!=E; ++F){
		unsigned FnID=F->getIntrinsicID();
		if(FnID==0){
			if(function_is_inline(F)==false){
				if(!Function_array_set.count(F)){
					Function_array_set[F]=order;
					order++;
				}
				for(inst_iterator I=inst_begin(F), E=inst_end(F); I != E; ++I){
					Instruction *inst=&*I;
					if(CallInst *call=dyn_cast<CallInst>(inst)){
						Function *callee=call->getCalledFunction();
						unsigned Fnid=callee->getIntrinsicID();
						if(Fnid==0){
							if(Function_array_set.count(callee)){
								if(callee!=F){
									keep_array_consistent(callee,call);
								}
							}
						}
					}
				}
			}
		}
	}
}


void keep_array_consistent(Function *callee, CallInst *call)
{
	int i=0;
	for(Function::arg_iterator it=callee->arg_begin();it!=callee->arg_end();++it){
		Value *arg=dyn_cast<Value>(it);
		Value *op=call->getArgOperand(i);
		Type *op_type=op->getType();
		bool is_array = op_type->isPointerTy()||op_type->isArrayTy();
		Value *array_op=NULL;
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
						//errs()<<arg_index<<" and "<<op_index<<"\n";
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
