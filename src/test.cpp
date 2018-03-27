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


#include "test.h"
#include "Library.h"
#include "Constant.h"
#include "SetPragma.h"
#include "LoopInterface.h"
#include "ArrayName.h"
#include "GEP.h"
#include "FunctionLatency.h"
#include "SetPragma.h"
#include "Function_II.h"
#include "Dataflow_II.h"
#include "Resource.h"
#include "Power.h"
#include "SetParameter.h"
#include <time.h>

/**************************************Input of benchmarks***************************************/
//bicg
//User Defined: choose suitable values according to the applications.
int function_loop_num[f_num]={2};
int Loops_counter_input[l_num] = {32,32};
int function_array_num[f_num]={5};
int array_size[a_num]={32,32,32,32,32,32};   //q,s,r,A,p (!check before DSE)
int array_dimension[d_num]={1,1,1,2,1};
int Loops_unroll_input[l_num] = {1,1};   //some small loops are unrolled completely in CFG
int Loops_pipeline_input[l_num]={0,0};
int array_partition_type[a_num]={1,1,1,1,1,1};   //block:0, cyclic:1
int array_partition_factor[a_num]={1,1,1,1,1,1}; //when it's equal to the number of array elements, should be complete
int function_pipeline_input[f_num]={0};
int dataflow_input[f_num]={0};
//End definition by users

/*//gemm
//User Defined: choose suitable values according to the applications.
int function_loop_num[f_num]={4};
int Loops_counter_input[l_num] = {16,16,16,16};
int function_array_num[f_num]={3};
int array_size[a_num]={16,16,16,16,16,16};   //C,A,B
int array_dimension[d_num]={2,2,2};
int Loops_unroll_input[l_num] = {1,1,1,1};   //some small loops are unrolled completely in CFG
int Loops_pipeline_input[l_num]={0,0,0,0};
int array_partition_type[a_num]={1,1,1,1,1,1};   //block:0, cyclic:1
int array_partition_factor[a_num]={1,1,1,1,1,1}; //when it's equal to the number of array elements, should be complete
int function_pipeline_input[f_num]={0};
int dataflow_input[f_num]={0};*/
//End definition by users
/************************************************************************************************/

/***************************************Global Variables******************************************/
//Pragma setting
std::map<Function*, int> Function_dataflow;
std::map<Function*, int> Function_pipeline;
std::map<Loop*, int> Loops_counter;
std::map<Loop*, int> Loops_unroll;
std::map<Loop*, int> Loops_pipeline;
std::vector<std::pair<int,int> > array_partition[30]; //0:block, 1:cyclic;
//Parameters
std::map<Instruction*, float> Load_latency;
std::map<Instruction*, float> Store_latency;
std::map<Instruction*,int> inst_reschedule_considered;
std::map<Value*, int> array_number;
std::map<BasicBlock*, int> bb_trip_counter;
std::map<Loop*, float> loop_critical_path;
std::vector<unsigned> Function_read_arg[30];
std::vector<unsigned> Function_write_arg[30];
std::vector<unsigned> Function_tmp_arg[30];
//Estimated performance
std::map<Loop*, float> loop_cycles;
std::map<Loop*, int> loop_II;
std::map<Function*, int> function_cycles;
std::map<Function*, int> function_II;
std::map<Function*, unsigned> Function_index;
//DSE parameters
std::vector<std::pair<int,int> > function_subelement_cycles[30];
std::map<Value*,Value*> array_map_array;
std::vector<Value*> loop_map_array[l_num];
std::map<int,int> optimized_loops;
std::map<Function*,int> optimized_functions;
std::vector<std::pair<int, std::vector<float> > > loop_map_array_type[l_num];
/************************************************************************************************/

/*
 * Definition of member functions for class "Configuration"
 */
Configuration::Configuration(int num):config_num(num){}
void Configuration::set_num_config(int num)
{
	config_num=num;
}
int Configuration::get_num_config()
{
	return config_num;
}
void Configuration::clear_config_vector()
{
	loop_unroll_config.clear();
	loop_pipeline_config.clear();
	array_partition_config.clear();
	function_pipeline_config.clear();
	dataflow_config.clear();
}
void Configuration::set_configuration()
{
	for(int i=0;i<l_num;++i){
		loop_unroll_config.push_back(Loops_unroll_input[i]);
		loop_pipeline_config.push_back(Loops_pipeline_input[i]);
	}
	for(int i=0;i<a_num;++i){
		array_partition_config.push_back(std::make_pair(array_partition_type[i],array_partition_factor[i]));
	}
	for(int i=0;i<f_num;++i){
		function_pipeline_config.push_back(function_pipeline_input[i]);
		dataflow_config.push_back(dataflow_input[i]);
	}
}
void Configuration::get_configuration()
{
	errs()<<"Configuration num: "<<config_num<<"\n";
	errs()<<"Loops unroll input: ";
	for(std::vector<int>::iterator it=loop_unroll_config.begin();it!=loop_unroll_config.end();++it){
		errs()<<*it<<", ";
	}
	errs()<<"\n";
	errs()<<"Loops pipeline input: ";
	for(std::vector<int>::iterator it=loop_pipeline_config.begin();it!=loop_pipeline_config.end();++it){
		errs()<<*it<<", ";
	}
	errs()<<"\n";
	errs()<<"Array partition type input: ";
	for(std::vector<std::pair<int,int> >::iterator it=array_partition_config.begin();it!=array_partition_config.end();++it){
		errs()<<it->first<<", ";
	}
	errs()<<"\n";
	errs()<<"Array partition factor input: ";
	for(std::vector<std::pair<int,int> >::iterator it=array_partition_config.begin();it!=array_partition_config.end();++it){
		errs()<<it->second<<", ";
	}
	errs()<<"\n";
	errs()<<"Function pipeline input: ";
	for(std::vector<int>::iterator it=function_pipeline_config.begin();it!=function_pipeline_config.end();++it){
		errs()<<*it<<", ";
	}
	errs()<<"\n";
	errs()<<"Dataflow input: ";
	for(std::vector<int>::iterator it=dataflow_config.begin();it!=dataflow_config.end();++it){
		errs()<<*it<<", ";
	}
	errs()<<"\n";
}

void Configuration::get_best_loop_unroll_pipeline(ivector &loop_unroll_vec,ivector &loop_pipeline_vec)
{
	loop_unroll_vec=loop_unroll_config;
	loop_pipeline_vec=loop_pipeline_config;
}
void Configuration::get_best_array_partition(pvector &array_partition_vec){
	array_partition_vec=array_partition_config;
}
void Configuration::get_best_function_configuration(ivector &function_pipeline_vec,ivector &function_dataflow_vec){
	function_pipeline_vec=function_pipeline_config;
	function_dataflow_vec=dataflow_config;
}

/*
 * Our Module pass: Test
 */
namespace
{
	clock_t timer_start;
	clock_t timer_end;

	struct Test : public ModulePass
	{
		static char ID;
		Test() : ModulePass(ID) {}

		virtual void getAnalysisUsage(AnalysisUsage &AU) const {
			AU.setPreservesCFG();
			AU.addRequired<LoopInfo>();
			AU.addPreserved<LoopInfo>();
		}

		void set_current_best_configuration(Configuration &config)
		{
			std::vector<int> loop_unroll_vec,loop_pipeline_vec,function_pipeline_vec,function_dataflow_vec;
			std::vector<std::pair<int,int> > array_partition_vec;
			config.get_best_array_partition(array_partition_vec);
			config.get_best_loop_unroll_pipeline(loop_unroll_vec,loop_pipeline_vec);
			config.get_best_function_configuration(function_pipeline_vec,function_dataflow_vec);
			int i=0;
			for(std::vector<int>::iterator it=loop_unroll_vec.begin();it!=loop_unroll_vec.end();++it){
				Loops_unroll_input[i]=*it;
				i++;
			}
			int j=0;
			for(std::vector<int>::iterator it=loop_pipeline_vec.begin();it!=loop_pipeline_vec.end();++it){
				Loops_pipeline_input[j]=*it;
				j++;
			}
			int k=0;
			for(std::vector<int>::iterator it=function_pipeline_vec.begin();it!=function_pipeline_vec.end();++it){
				function_pipeline_input[k]=*it;
				k++;
			}
			int m=0;
			for(std::vector<int>::iterator it=function_dataflow_vec.begin();it!=function_dataflow_vec.end();++it){
				dataflow_input[m]=*it;
				m++;
			}
			int n=0;
			for(std::vector<std::pair<int,int> >::iterator it=array_partition_vec.begin();it!=array_partition_vec.end();++it){
				array_partition_type[n]=it->first;
				array_partition_factor[n]=it->second;
				n++;
			}
		}

		//Build the relationship between a loop and the arrays it accesses.
		void set_loop_map_array(Module &M)
		{
			for(int i=0;i<l_num;++i){
				loop_map_array[i].clear();
				loop_map_array_type[i].clear();
			}
			for(auto F=M.begin(), E=M.end(); F!=E; ++F){
				unsigned FnID=F->getIntrinsicID();
				if(FnID==0){
					if(function_is_inline(F)==false){
						LoopInfo &LII = getAnalysis<LoopInfo>(*F);
						LoopInfo* LI;
						LI=&LII;
						unsigned F_index=Function_index[F];
						int loop_index=0;
						for(int i=0;i<(int)F_index;++i){
							loop_index+=function_loop_num[i];
						}
						std::queue<Loop*> loop_queue;
						for (LoopInfo::reverse_iterator i = LI->rbegin(), e = LI->rend(); i != e; ++i){
							Loop *L = *i;
							std::vector<Value*> loop_read;
							std::vector<Value*> loop_write;
							get_loop_input(L,loop_read);
							get_loop_output(L,loop_write);
							std::map<Value*,int> loop_array_counted;
							for(std::vector<Value*>::iterator it=loop_read.begin();it!=loop_read.end();++it){
								Value *array_read=*it;
								if(array_number.count(array_read)){
									if(!loop_array_counted.count(array_read)){
										loop_map_array[loop_index].push_back(array_read);

										int array_index=array_number[array_read];
										int dimension=array_dimension[array_index];
										std::vector<float> vec;
										for(int j=0;j<dimension;j++){
											float m_apt = compute_metric_apt(L, array_read, j);
											vec.push_back(m_apt);
										}
										loop_map_array_type[loop_index].push_back(std::make_pair(array_index,vec));

										loop_array_counted[array_read]=1;
									}
								}
							}
							for(std::vector<Value*>::iterator it=loop_write.begin();it!=loop_write.end();++it){
								Value *array_write=*it;
								if(array_number.count(array_write)){
									if(!loop_array_counted.count(array_write)){
										loop_map_array[loop_index].push_back(array_write);

										int array_index=array_number[array_write];
										int dimension=array_dimension[array_index];
										std::vector<float> vec;
										for(int j=0;j<dimension;j++){
											float m_apt = compute_metric_apt(L, array_write, j);
											vec.push_back(m_apt);
										}
										loop_map_array_type[loop_index].push_back(std::make_pair(array_index,vec));

										loop_array_counted[array_write]=1;
									}
								}
							}
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
							std::vector<Value*> loop_read;
							std::vector<Value*> loop_write;
							get_loop_input(loop_test,loop_read);
							get_loop_output(loop_test,loop_write);
							std::map<Value*,int> loop_array_counted;
							for(std::vector<Value*>::iterator it=loop_read.begin();it!=loop_read.end();++it){
								Value *array_read=*it;
								if(array_number.count(array_read)){
									if(!loop_array_counted.count(array_read)){
										loop_map_array[loop_index].push_back(array_read);

										int array_index=array_number[array_read];
										int dimension=array_dimension[array_index];
										std::vector<float> vec;
										for(int j=0;j<dimension;j++){
											float m_apt = compute_metric_apt(loop_test, array_read, j);
											vec.push_back(m_apt);
										}
										loop_map_array_type[loop_index].push_back(std::make_pair(array_index,vec));

										loop_array_counted[array_read]=1;
									}
								}
							}
							for(std::vector<Value*>::iterator it=loop_write.begin();it!=loop_write.end();++it){
								Value *array_write=*it;
								if(array_number.count(array_write)){
									if(!loop_array_counted.count(array_write)){
										loop_map_array[loop_index].push_back(array_write);

										int array_index=array_number[array_write];
										int dimension=array_dimension[array_index];
										std::vector<float> vec;
										for(int j=0;j<dimension;j++){
											float m_apt = compute_metric_apt(loop_test, array_write, j);
											vec.push_back(m_apt);
										}
										loop_map_array_type[loop_index].push_back(std::make_pair(array_index,vec));

										loop_array_counted[array_write]=1;
									}
								}
							}
							loop_index++;
							std::vector<Loop*> subLoops = loop_test->getSubLoops();
							for (unsigned ii=0; ii<subLoops.size(); ii++){
								 Loop* sub_loop = subLoops.at(ii);
								 loop_queue.push(sub_loop);
							}
						}
					}
				}
			}
		}

		float compute_metric_apt(Loop *L, Value *array, int dim)
		{
			float m_apt=0.0;
			int load_num=0;
			int load_index_max=0;
			int load_index_min=0;
			int store_num=0;
			int store_index_max=0;
			int store_index_min=0;
			int array_index=array_number[array];
			int dimension=array_dimension[array_index];
			for(Loop::block_iterator BI=L->block_begin(), BE=L->block_end(); BI !=BE; ++BI)
			{
				BasicBlock *bb=*BI;
				for(auto iti=bb->begin();iti!=bb->end();++iti)
				{
					if(isa<LoadInst>(iti)){
						Value *array_name=get_array_name(iti);
						if(array==array_name){
							load_num++;
							GetElementPtrInst *gep=get_GEP(iti);
							if(gep==NULL){
								load_index_min=0;
							}
							else{
								unsigned num_indice=gep->getNumIndices();
								int indice=num_indice-dimension+dim+1;
								Value *operand=gep->getOperand(indice);
								int offset=compute_gep_operand(operand,false,0);
								load_index_max=std::max(offset,load_index_max);
								load_index_min=std::min(offset,load_index_min);
							}
						}
					}
					else if(isa<StoreInst>(iti)){
						Value *array_name=get_array_name(iti);
						if(array==array_name){
							store_num++;
							GetElementPtrInst *gep=get_GEP(iti);
							if(gep==NULL){
								store_index_min=0;
							}
							else{
								unsigned num_indice=gep->getNumIndices();
								int indice=num_indice-dimension+dim+1;
								Value *operand=gep->getOperand(indice);
								int offset=compute_gep_operand(operand,false,0);
								store_index_max=std::max(offset,store_index_max);
								store_index_min=std::min(offset,store_index_min);
							}
						}
					}
				}
			}
			float m_apt_l = load_num/(load_index_max-load_index_min+1);
			float m_apt_s = store_num/(store_index_max-store_index_min+1);
			// For most cases, m_apt is not larger than 1...
			if(m_apt_l==1.0){
				if(m_apt_s==1.0 || m_apt_s==0.0){
					m_apt=1.0; //cyclic
				}
				else if(m_apt_s<1.0){
					m_apt=2.0; //both types should be tried.
				}
				else{
					m_apt=0.0;
					//errs()<<"Note: some special case happens.";
				}
			}
			else if(m_apt_l<1.0 && m_apt_l>0.0){
				if(m_apt_s<1.0){
					m_apt=0.5; //block
				}
				else if(m_apt_s==1.0){
					m_apt=2.0; //both types should be tried.
				}
				else{
					m_apt=0.0;
					//errs()<<"Note: some special case happens.";
				}
			}
			else if(m_apt_l==0.0){
				if(m_apt_s==1.0){
					m_apt=1.0; //cyclic
				}
				else if(m_apt_s<1.0 && m_apt_s>0.0){
					m_apt=0.5; //block
				}
				else if(m_apt_s==0.0){
					m_apt=0.0;
					//errs()<<"Note: No loads/stores exist.";
				}
				else{
					m_apt=0.0;
					//errs()<<"Note: some special case happens.";
				}
			}
			else{
				m_apt=0.0;
				//errs()<<"Note: some special case happens.";
			}
			return m_apt;
		}

		int get_subelement_latency(Function *F,int sub_order)
		{
			int sub_latency=0;
			unsigned F_index=Function_index[F];
			for(std::vector<std::pair<int,int> >::iterator it=function_subelement_cycles[F_index].begin();it!=function_subelement_cycles[F_index].end();++it){
				if(it->first==sub_order){
					sub_latency=it->second;
					break;
				}
			}
			return sub_latency;
		}

		int get_loop_order(Function *F, LoopInfo* LI, Loop *L)
		{
			int L_order=0;
			int order=1;
			//unsigned F_index=Function_index[F];
			std::map<Loop*,int> top_loop_counted;
			std::map<Function*,int> function_counted;
			for(inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I){
				Instruction *inst=&*I;
				BasicBlock *bb=inst->getParent();
				Loop *bb_loop=LI->getLoopFor(bb);
				if(bb_loop==NULL){
					if(CallInst *call=dyn_cast<CallInst>(inst)){
						Function *callee = call->getCalledFunction();
						if(callee!=F){
							unsigned FnID=callee->getIntrinsicID();
							if(FnID==0){
								if(!function_counted.count(callee)){
									order++;
									function_counted[callee]=1;
								}
								else{
									function_counted[callee]++;
								}
							}
						}
					}
				}
				else{
					Loop *top_loop=get_top_loop(bb_loop);
					if(top_loop_counted.count(top_loop)){
						top_loop_counted[top_loop]++;
					}
					else{
						if(top_loop==L){
							L_order=order;
							break;
						}
						order++;
						top_loop_counted[top_loop]=1;
					}
				}
			}
			return L_order;
		}

		int get_function_order(Function *F, LoopInfo* LI, Function *F1)
		{
			int F1_order=0;
			int order=1;
			std::map<Function*,int> function_counted;
			if(LI->empty()){
				for(inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I){
					Instruction *inst=&*I;
					if(CallInst *call=dyn_cast<CallInst>(inst)){
						Function *callee = call->getCalledFunction();
						if(callee!=F){
							unsigned FnID=callee->getIntrinsicID();
							if(FnID==0){
								if(!function_counted.count(callee)){
									if(callee==F1){
										F1_order=order;
										break;
									}
									order++;
									function_counted[callee]=1;
								}
								else{
									function_counted[callee]++;
								}
							}
						}
					}
				}
			}
			else{
				std::map<Loop*,int> top_loop_counted;
				for(inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I){
					Instruction *inst=&*I;
					BasicBlock *bb=inst->getParent();
					Loop *bb_loop=LI->getLoopFor(bb);
					if(bb_loop==NULL){
						if(CallInst *call=dyn_cast<CallInst>(inst)){
							Function *callee = call->getCalledFunction();
							if(callee!=F){
								unsigned FnID=callee->getIntrinsicID();
								if(FnID==0){
									if(!function_counted.count(callee)){
										if(callee==F1){
											F1_order=order;
											break;
										}
										order++;
										function_counted[callee]=1;
									}
									else{
										function_counted[callee]++;
									}
								}
							}
						}
					}
					else{
						Loop *top_loop=get_top_loop(bb_loop);
						if(top_loop_counted.count(top_loop)){
							top_loop_counted[top_loop]++;
						}
						else{
							order++;
							top_loop_counted[top_loop]=1;
						}
					}
				}
			}
			return F1_order;
		}

		//Performance estimation of one design point (for corresponding configuration).
		void run_model(Module &M,unsigned &total_DSP,unsigned &total_BRAM,int &config_num,Top_Results &top_results,Configuration &config)
		{
  			Function_pipeline.clear();
  			Function_dataflow.clear();
  			Loops_unroll.clear();
  			Loops_pipeline.clear();
  			Loops_counter.clear();// loop issues
			for(int i=0;i<30;++i){
				array_partition[i].clear();
				function_subelement_cycles[i].clear();
			}
  			Load_latency.clear();
  			Store_latency.clear();
  			inst_reschedule_considered.clear();
  			function_cycles.clear();
  			function_II.clear();
  			loop_cycles.clear();
  			loop_II.clear();
			std::vector<std::pair<std::string,unsigned> > function_resource[20];
			std::vector<std::pair<unsigned,int> > function_arg_II[30];
			set_dataflow(M);
			set_function_pipeline(M);
			set_array_partition(M);
			for(auto F=M.begin(), E=M.end(); F!=E; ++F){
				unsigned FnID=F->getIntrinsicID();
				if(FnID==0){
					if(function_is_inline(F)==false){
						//errs()<<"***********************************************************************\n";
						//errs()<<"Function "<<Function_index[F]<<": "<<F->getName()<<"\n";
						LoopInfo &LI = getAnalysis<LoopInfo>(*F);
						LoopInfo* LI_pointer;
						LI_pointer=&LI;
						set_loop_counter_ul_pipeline(LI_pointer,F);
						int FnII=1;
						std::map<Instruction*, unsigned> instr_index_loop;
						std::vector<std::pair<int,float> > dependence_loop[INSN_NUM];
						int Fn_tmp_cycles=compute_cycles_in_Fn(F,LI_pointer,instr_index_loop,dependence_loop);
						function_cycles[F]=Fn_tmp_cycles;
						//errs()<<F->getName()<<"Function cycles: "<<function_cycles[F]<<"\n";
						bool dataflow_work=check_dataflow(F,LI_pointer);
						if(Function_dataflow[F]==1){
							if(dataflow_work==true){
								FnII=compute_dataflow_II(F,LI_pointer);
								function_II[F]=FnII;
							}
						}
						else{
							if(Function_pipeline[F]==1){
								FnII=compute_Fn_II(F,LI_pointer,instr_index_loop,dependence_loop,function_arg_II);
								if(FnII>function_cycles[F]){
									FnII=function_cycles[F]+1;
								}
								function_II[F]=FnII;
							}
							else{
								FnII=function_cycles[F]+1;
								function_II[F]=FnII;
							}
						}
						//errs()<<F->getName()<<" II: "<<function_II[F]<<"\n";
						unsigned F_index=Function_index[F];
						function_resource[F_index].push_back(std::make_pair("BRAM",0));
						function_resource[F_index].push_back(std::make_pair("DSP",0));
						function_resource[F_index].push_back(std::make_pair("FF",0));
						function_resource[F_index].push_back(std::make_pair("LUT",0));
						resource_accumulation(LI_pointer,F,function_resource,instr_index_loop,dependence_loop);
						bram_accumulation(LI_pointer,F,function_resource);
						if(!function_resource[F_index].empty()){
							for(std::vector<std::pair<std::string,unsigned> >::iterator ii=function_resource[F_index].begin();ii!=function_resource[F_index].end();++ii){
								std::string resource=ii->first;
								if(resource=="DSP"){
									top_results.top_function_DSP[config_num]=ii->second;
									//top_function_DSP[config_num]=ii->second;
								}
								else if(resource=="BRAM"){
									top_results.top_function_BRAM[config_num]=ii->second;
									//top_function_BRAM[config_num]=ii->second;
								}
							}
						}
						top_results.top_function_cycles[config_num]=function_cycles[F];
						top_results.top_function_II[config_num]=function_II[F];
					}
				}
			}
			//errs()<<"Function cycles: "<<top_results.top_function_cycles[config_num]<<"\n";
			//errs()<<"Function II: "<<top_results.top_function_II[config_num]<<"\n";
			//errs()<<"BRAM usage: "<<top_results.top_function_BRAM[config_num]<<"\n";
			//errs()<<"DSP usage: "<<top_results.top_function_DSP[config_num]<<"\n";
			if((top_results.top_function_DSP[config_num]>total_DSP)&&(top_results.top_function_BRAM[config_num]>total_BRAM)){
				top_results.top_function_cycles[config_num]=inf;
				top_results.top_function_II[config_num]=inf;
			}
			if((top_results.top_function_DSP[config_num]<=total_DSP)&&(top_results.top_function_BRAM[config_num]<=total_BRAM)){
				//errs()<<config.get_num_config()<<"\n";
				if(config.get_num_config()==0){
					config.clear_config_vector();
					config.set_num_config(config_num);
					config.set_configuration();
					config.get_configuration();
				}
				else{
					int current_best_configuration=config.get_num_config();
					int current_best_cycles=top_results.top_function_cycles[current_best_configuration];
					int current_best_II=top_results.top_function_II[current_best_configuration];
					if(top_results.top_function_cycles[config_num]<current_best_cycles){
						if(top_results.top_function_II[config_num]<=current_best_II){
							config.clear_config_vector();
							config.set_num_config(config_num);
							config.set_configuration();
							//config.get_configuration();
						}
						else if(top_results.top_function_II[config_num]>current_best_II){// possible optimal points
							//errs()<<"Possible optimal point: \n";
							//config.get_configuration();
							config.clear_config_vector();
							config.set_num_config(config_num);
							config.set_configuration();
						}
					}
					else if(top_results.top_function_cycles[config_num]==current_best_cycles){
						if(top_results.top_function_II[config_num]<=current_best_II){
							config.clear_config_vector();
							config.set_num_config(config_num);
							config.set_configuration();
							//config.get_configuration();
						}
					}
					else{
						if(top_results.top_function_II[config_num]<current_best_II){//possible optimal points
							/*errs()<<"Possible optimal point: \n";
							errs()<<"Configuration num: "<<config_num<<"\n";
							errs()<<"Loops unroll input: ";
							for(int i=0;i<l_num;++i){
								errs()<<Loops_unroll_input[i]<<", ";
							}
							errs()<<"\n";
							errs()<<"Loops pipeline input: ";
							for(int i=0;i<l_num;++i){
								errs()<<Loops_pipeline_input[i]<<", ";
							}
							errs()<<"\n";
							errs()<<"Array partition type input: ";
							for(int i=0;i<a_num;++i){
								errs()<<array_partition_type[i]<<", ";
							}
							errs()<<"\n";
							errs()<<"Array partition factor input: ";
							for(int i=0;i<a_num;++i){
								errs()<<array_partition_factor[i]<<", ";
							}
							errs()<<"\n";
							errs()<<"Function pipeline input: ";
							for(int i=0;i<f_num;++i){
								errs()<<function_pipeline_input[i]<<", ";
							}
							errs()<<"\n";
							errs()<<"Dataflow input: ";
							for(int i=0;i<f_num;++i){
								errs()<<dataflow_input[i]<<", ";
							}
							errs()<<"\n";*/
						}
					}
				}
			}
			config_num++;
		}

		//DSE when changing parameters (i.e., type and factor) of array partitioning
		void run_array_configuration(Module &M,int top_array_index,std::vector<std::pair<int,int> > *array_optimization_config,unsigned &total_DSP,unsigned &total_BRAM,int &config_num,Top_Results &top_results,Configuration &config)
		{
			if(!array_optimization_config[top_array_index].empty()){
				int accumulated_dim=0;
				for(int i=0;i<top_array_index;++i){
					int dim_tmp=array_dimension[i];
					accumulated_dim+=dim_tmp;
				}
				int j=0;
				for(std::vector<std::pair<int,int> >::iterator it=array_optimization_config[top_array_index].begin();it!=array_optimization_config[top_array_index].end();++it)
				{
					if((it->first)<=1){
						int new_j=j+accumulated_dim;
						int dim_size=array_size[new_j];
						int bound=get_power_of_two(dim_size);
						array_partition_type[new_j]=it->first;
						int factor_begin=it->second;
						int begin=get_power_of_two(factor_begin);
						for(int t=begin;t<=bound;t++){
							array_partition_factor[new_j]=pow(2,t);
							run_model(M,total_DSP,total_BRAM,config_num,top_results,config);
							int last_config=config_num-2;
							int this_config_num=config_num-1;
							if(top_results.top_function_cycles[last_config]<top_results.top_function_cycles[this_config_num]){
								if(t==bound){
									it->first += 1;
									it->second = 1;
									break;
								}
								else{
									it->second=pow(2,bound);
									break;
								}
							}
							else if(top_results.top_function_cycles[last_config]==top_results.top_function_cycles[this_config_num]){
								if(top_results.top_function_II[last_config]<top_results.top_function_II[this_config_num]){
									if(t==bound){
										it->first += 1;
										it->second = 1;
										break;
									}
									else{
										it->second=pow(2,bound);
										break;
									}
								}
								else{
									if(t==bound){
										it->first += 1;
										it->second = 1;
										break;
									}
									else{
										continue;
									}
								}
							}
							else{
								if(t==bound){
									it->first += 1;
									it->second = 1;
									break;
								}
								else{
									continue;
								}
							}
						}
						std::vector<std::pair<int,int> > array_partition_vec;
						config.get_best_array_partition(array_partition_vec);
						int numth=0;
						if(!array_partition_vec.empty()){
							for(std::vector<std::pair<int,int> >::iterator it=array_partition_vec.begin();it!=array_partition_vec.end();++it){
								if(numth==new_j){
									array_partition_type[new_j]=it->first;
									array_partition_factor[new_j]=it->second;
									break;
								}
								else{
									numth++;
								}
							}
						}
						else{
							errs()<<"Please check: array partition vector is empty!\n";
							array_partition_type[new_j]=1;
							array_partition_factor[new_j]=1;
						}
					}
					j++;
				}
				bool optimized=true;
				for(std::vector<std::pair<int,int> >::iterator it=array_optimization_config[top_array_index].begin();it!=array_optimization_config[top_array_index].end();++it)
				{
					if(it->first<=1){
						optimized=false;
						break;
					}
				}
				if(optimized==true){
					array_optimization_config[top_array_index].clear();
				}
			}
		}


		bool subloop_unroll_complete(int l_index, std::vector<int> *loop_index)
		{
			bool subloop_complete=true;
			if(!loop_index[l_index].empty()){
				for(std::vector<int>::iterator it=loop_index[l_index].begin();it!=loop_index[l_index].end();++it){
					int index=*it;
					if(Loops_unroll_input[index]<Loops_counter_input[index]){
						subloop_complete=false;
						break;
					}
					else{
						if(!loop_index[index].empty()){
							subloop_complete=subloop_unroll_complete(index,loop_index);
						}
					}
				}
			}
			return subloop_complete;
		}

		bool array_config_is_empty(std::vector<std::pair<int,int> > *array_optimization_config)
		{
			bool is_empty=true;
			for(int i=0;i<d_num;++i){
				if(!array_optimization_config[i].empty()){
					is_empty=false;
					break;
				}
			}
			return is_empty;
		}

		//DSE: changing array parameters for corresponding loop configuration
		void run_loop_array_configuration(Module &M,int l_index,Function *top_function,unsigned &total_DSP,unsigned &total_BRAM,int &config_num,Top_Results &top_results,Configuration &config)
		{
			unsigned top_function_index=Function_index[top_function];
			int top_array_num=function_array_num[top_function_index];
			int array_num=0; //array index in array_dimension and array_number
			for(int i=0;i<(int)top_function_index;++i){
				int num=function_array_num[i];
				array_num+=num;
			}
			int largest_array_num=top_array_num+array_num;

			std::vector<std::pair<int,int> > array_optimization_config[d_num];
			for(int i=0;i<d_num;++i){
				array_optimization_config[i].clear();
			}
			std::map<Value*,int> array_counted;
			if(!loop_map_array[l_index].empty()){
				for(std::vector<Value*>::iterator it=loop_map_array[l_index].begin();it!=loop_map_array[l_index].end();++it){
					Value *array=*it;
			    	if(!array_counted.count(array)){
					    if(array_number.count(array)){
					    	int array_index=array_number[array];
							if(array_index>=array_num&&array_index<largest_array_num){
						        int array_dim=array_dimension[array_index];
						        for(int j=0; j<array_dim; ++j){
						        	array_optimization_config[array_index].push_back(std::make_pair(0,1));
						        }
							}
							else{
								if(array_map_array.count(array)){
									Value *m_array=array_map_array[array];
									if(!array_counted.count(m_array)){
										if(array_number.count(m_array)){
											int num=array_number[m_array];
											if(num>=array_num&&num<largest_array_num){
										        int array_dim=array_dimension[num];
										        for(int j=0; j<array_dim; ++j){
										        	array_optimization_config[num].push_back(std::make_pair(0,1));
										        }
											}
										}
										array_counted[m_array]=1;
									}
								}
							}
					    }
					    array_counted[array]=1;
			    	}
				}
			}
			while(array_config_is_empty(array_optimization_config)==false)
			{
				for(int i=0;i<d_num;++i){
					if(array_optimization_config[i].empty()){
						continue;
					}
					else{
						int array_index=i;
						run_array_configuration(M,array_index,array_optimization_config,total_DSP,total_BRAM,config_num,top_results,config);
					}
				}
			}
		}

		//DSE: changing loop parameters (loop unrolling and loop pipelining)
		void run_loop_configuration(Module &M,Function *F,Function *top_function,std::map<Loop*,int> loop_index_tmp,std::vector<int> *loop_index,int L_order,int compared_latency, bool &op_finished,unsigned &total_DSP,unsigned &total_BRAM,int &config_num,Top_Results &top_results,Configuration &config)
		{
			int loop_num=loop_index_tmp.size();
			std::map<int,int> loop_counted;
			for(std::map<Loop*,int>::iterator it=loop_index_tmp.begin();it!=loop_index_tmp.end();++it){
				int loop_order=loop_num;
				int index=it->second;
				for(std::map<Loop*,int>::iterator ii=loop_index_tmp.begin();ii!=loop_index_tmp.end();++ii){
					int index_tmp=ii->second;
					if(index>index_tmp){
						loop_order--;
					}
				}
				loop_counted[loop_order]=index;
			}
			bool break_flag=false;
			for(int j=1;j<=loop_num;++j){
				int l_index=loop_counted[j];
				if(!optimized_loops.count(l_index)){
					int L_latency=get_subelement_latency(F,L_order);
					if(L_latency>=compared_latency){
						if(loop_index[l_index].empty()){//subloops is empty
							int trip_count=Loops_counter_input[l_index];
							int bound=get_power_of_two(trip_count);
							int actual_bound=pow(2,bound);
							for(int i=0;i<2;++i){
								Loops_pipeline_input[l_index]=i;
								if(Loops_pipeline_input[l_index]==0){
									for(int t=0;t<=bound;++t){
										//errs()<<t<<"\n";
										Loops_unroll_input[l_index]=pow(2,t);
										run_model(M,total_DSP,total_BRAM,config_num,top_results,config);
										run_loop_array_configuration(M,l_index,top_function,total_DSP,total_BRAM,config_num,top_results,config);
									}
									if(trip_count>actual_bound){
										Loops_unroll_input[l_index]=trip_count;
										run_model(M,total_DSP,total_BRAM,config_num,top_results,config);
										run_loop_array_configuration(M,l_index,top_function,total_DSP,total_BRAM,config_num,top_results,config);
									}
								}
								else{
									for(int t=0;t<bound;++t){
										Loops_unroll_input[l_index]=pow(2,t);
										run_model(M,total_DSP,total_BRAM,config_num,top_results,config);
										run_loop_array_configuration(M,l_index,top_function,total_DSP,total_BRAM,config_num,top_results,config);
									}
								}
							}
							set_current_best_configuration(config);
							optimized_loops[l_index]=1;
							run_model(M,total_DSP,total_BRAM,config_num,top_results,config);
						}
						else{
							bool sub_complete=subloop_unroll_complete(l_index,loop_index);
							if(sub_complete==true){
								int trip_count=Loops_counter_input[l_index];
								int bound=get_power_of_two(trip_count);
								int actual_bound=pow(2,bound);
								for(int i=0;i<2;++i){
									Loops_pipeline_input[l_index]=i;
									if(Loops_pipeline_input[l_index]==0){
										for(int t=0;t<=bound;++t){
											Loops_unroll_input[l_index]=pow(2,t);
											run_model(M,total_DSP,total_BRAM,config_num,top_results,config);
											run_loop_array_configuration(M,l_index,top_function,total_DSP,total_BRAM,config_num,top_results,config);
										}
										if(trip_count>actual_bound){
											Loops_unroll_input[l_index]=trip_count;
											run_model(M,total_DSP,total_BRAM,config_num,top_results,config);
											run_loop_array_configuration(M,l_index,top_function,total_DSP,total_BRAM,config_num,top_results,config);
										}
									}
									else{
										for(int t=0;t<bound;++t){
											Loops_unroll_input[l_index]=pow(2,t);
											run_model(M,total_DSP,total_BRAM,config_num,top_results,config);
											run_loop_array_configuration(M,l_index,top_function,total_DSP,total_BRAM,config_num,top_results,config);
										}
									}
								}
								set_current_best_configuration(config);
								optimized_loops[l_index]=1;
								run_model(M,total_DSP,total_BRAM,config_num,top_results,config);
							}
							else{
								int trip_count=Loops_counter_input[l_index];
								int bound=get_power_of_two(trip_count);
								Loops_pipeline_input[l_index]=1;
								for(int t=0;t<bound;++t){
									Loops_unroll_input[l_index]=pow(2,t);
									run_model(M,total_DSP,total_BRAM,config_num,top_results,config);
									run_loop_array_configuration(M,l_index,top_function,total_DSP,total_BRAM,config_num,top_results,config);
								}
								set_current_best_configuration(config);
								optimized_loops[l_index]=1;
								run_model(M,total_DSP,total_BRAM,config_num,top_results,config);
							}
						}
					}
					else{
						break_flag=true;
						break;
					}
				}
			}
			if(break_flag==true){
				op_finished=false;
			}
			else{
				op_finished=true;
			}
		}

		//DSE: changing function parameters (Function pipelining)
		void run_function_configuration(Module &M,Function *F,Function *top_function,int compared_latency, bool &op_finished, unsigned &total_DSP,unsigned &total_BRAM,int &config_num,Top_Results &top_results,Configuration &config)
		{
			unsigned F_index=Function_index[F];
			bool finished=false;
			for(int i=0;i<2;++i){
				function_pipeline_input[F_index]=i;
				if(function_pipeline_input[F_index]==0){
					DSE_function_not_pipeline(M,F,top_function,compared_latency,finished,total_DSP,total_BRAM,config_num,top_results,config);
					if(finished==false){
						op_finished=false;
						break;
					}
					else continue;
				}
				else{
					unsigned top_function_index=Function_index[top_function];
					int top_array_num=function_array_num[top_function_index];
					int array_num=0;
					for(int i=0;i<(int)top_function_index;++i){
						int num=function_array_num[i];
						array_num+=num;
					}
					if(F==top_function){
						for(int i=0;i<top_array_num;++i){
							int top_array_index=i+array_num;
							int accumulated_dim=0;
							for(int j=0;j<top_array_index;++j){
								int dim_tmp=array_dimension[j];
								accumulated_dim+=dim_tmp;
							}
							int dim=array_dimension[top_array_index];
							for(int j=0;j<dim;++j){
								int new_j=j+accumulated_dim;
								array_partition_type[new_j]=1;
								array_partition_factor[new_j]=1;
							}
						}
					}
					else{
						for(Function::arg_iterator it=F->arg_begin();it!=F->arg_end();++it){
							Value *array=dyn_cast<Value>(it);
							if(array_number.count(array)){
								if(array_map_array.count(array)){
									Value *m_array=array_map_array[array];
									if(array_number.count(m_array)){
										int num=array_number[m_array];
										int largest_array_num=top_array_num+array_num;
										if(num>=array_num&&num<largest_array_num){
											int accumulated_dim=0;
											for(int j=0;j<num;++j){
												int dim_tmp=array_dimension[j];
												accumulated_dim+=dim_tmp;
											}
											int dim=array_dimension[num];
											for(int j=0;j<dim;++j){
												int new_j=j+accumulated_dim;
												array_partition_type[new_j]=1;
												array_partition_factor[new_j]=1;
											}
										}
									}
								}
							}
						}
					}
					DSE_function_pipeline(M,F,top_function,compared_latency,finished,total_DSP,total_BRAM,config_num,top_results,config);
					if(finished==true){
						op_finished=true;
					}
					else{
						op_finished=false;
					}
				}
			}
		}

		//DSE: Function pipelining is applied
		void DSE_function_pipeline(Module &M,Function *F,Function *top_function,int compared_latency,bool &op_finished,unsigned &total_DSP,unsigned &total_BRAM,int &config_num,Top_Results &top_results,Configuration &config)
		{
			unsigned F_index=Function_index[F];
			function_pipeline_input[F_index]=1;
			if(dataflow_input[F_index]==1){
				dataflow_input[F_index]=0;
			}
			run_model(M,total_DSP,total_BRAM,config_num,top_results,config);
			unsigned top_function_index=Function_index[top_function];
			int top_array_num=function_array_num[top_function_index];
			int array_num=0; //array index in array_dimension and array_number
			for(int i=0;i<(int)top_function_index;++i){
				int num=function_array_num[i];
				array_num+=num;
			}
			if(F==top_function){
				std::vector<std::pair<int,int> > array_optimization_config[d_num];
				for(int i=0;i<top_array_num;++i){
					int top_array_index=i+array_num;
			        int array_dim=array_dimension[top_array_index];
			        for(int j=0; j<array_dim; ++j){
			        	array_optimization_config[top_array_index].push_back(std::make_pair(0,1));
			        }
				}
				while(array_config_is_empty(array_optimization_config)==false)
				{
					for(int i=0;i<d_num;++i){
						if(array_optimization_config[i].empty()){
							continue;
						}
						else{
							int top_array_index=i;
							run_array_configuration(M,top_array_index,array_optimization_config,total_DSP,total_BRAM,config_num,top_results,config);
						}
					}
				}
			}
			else{
				std::vector<std::pair<int,int> > array_optimization_config[d_num];
				std::map<Value*,int> array_counted;
				for(Function::arg_iterator it=F->arg_begin();it!=F->arg_end();++it){
					Value *array=dyn_cast<Value>(it);
					if(!array_counted.count(array)){
						if(array_number.count(array)){
							if(array_map_array.count(array)){
								Value *m_array=array_map_array[array];
								if(!array_counted.count(m_array)){
									if(array_number.count(m_array)){
										int num=array_number[m_array];
										int largest_array_num=top_array_num+array_num;
										if(num>=array_num&&num<largest_array_num){
									        int array_dim=array_dimension[num];
									        for(int j=0; j<array_dim; ++j){
									        	array_optimization_config[num].push_back(std::make_pair(0,1));
									        }
										}
									}
									array_counted[m_array]=1;
								}
							}
						}
						array_counted[array]=1;
					}
				}
				while(array_config_is_empty(array_optimization_config)==false)
				{
					for(int i=0;i<d_num;++i){
						if(array_optimization_config[i].empty()){
							continue;
						}
						else{
							int array_index=i;
							run_array_configuration(M,array_index,array_optimization_config,total_DSP,total_BRAM,config_num,top_results,config);
						}
					}
				}
			}
			set_current_best_configuration(config);
			run_model(M,total_DSP,total_BRAM,config_num,top_results,config);
			op_finished=true;
		}

		//DSE: Function pipelining is not applied
		void DSE_function_not_pipeline(Module &M,Function *F,Function *top_function,int compared_latency,bool &op_finished,unsigned &total_DSP,unsigned &total_BRAM,int &config_num,Top_Results &top_results,Configuration &config)
		{
			unsigned F_index=Function_index[F];
			function_pipeline_input[F_index]=0;
			dataflow_input[F_index]=Function_dataflow[F];
			run_model(M,total_DSP,total_BRAM,config_num,top_results,config);
			int subelement_num=function_subelement_cycles[F_index].size();
			if(subelement_num==0){//no loops and sub_functions
				unsigned top_function_index=Function_index[top_function];
				int top_array_num=function_array_num[top_function_index];
				int array_num=0;
				for(int i=0;i<(int)top_function_index;++i){
					int num=function_array_num[i];
					array_num+=num;
				}
				if(F==top_function){
					std::vector<std::pair<int,int> > array_optimization_config[d_num];
					for(int i=0;i<top_array_num;++i){
						int top_array_index=i+array_num;
				        int array_dim=array_dimension[top_array_index];
				        for(int j=0; j<array_dim; ++j){
				        	array_optimization_config[top_array_index].push_back(std::make_pair(0,1));
				        }
					}
					while(array_config_is_empty(array_optimization_config)==false)
					{
						for(int i=0;i<d_num;++i){
							if(array_optimization_config[i].empty()){
								continue;
							}
							else{
								int top_array_index=i;
								run_array_configuration(M,top_array_index,array_optimization_config,total_DSP,total_BRAM,config_num,top_results,config);
							}
						}
					}
				}
				else{
					std::vector<std::pair<int,int> > array_optimization_config[d_num];
					std::map<Value*,int> array_counted;
					for(Function::arg_iterator it=F->arg_begin();it!=F->arg_end();++it){
						Value *array=dyn_cast<Value>(it);
						if(!array_counted.count(array)){
							if(array_number.count(array)){
								if(array_map_array.count(array)){
									Value *m_array=array_map_array[array];
									if(!array_counted.count(m_array)){
										if(array_number.count(m_array)){
											int num=array_number[m_array];
											int largest_array_num=top_array_num+array_num;
											if(num>=array_num&&num<largest_array_num){
										        int array_dim=array_dimension[num];
										        for(int j=0; j<array_dim; ++j){
										        	array_optimization_config[num].push_back(std::make_pair(0,1));
										        }
											}
										}
										array_counted[m_array]=1;
									}
								}
							}
							array_counted[array]=1;
						}
					}
					while(array_config_is_empty(array_optimization_config)==false)
					{
						for(int i=0;i<d_num;++i){
							if(array_optimization_config[i].empty()){
								continue;
							}
							else{
								int array_index=i;
								run_array_configuration(M,array_index,array_optimization_config,total_DSP,total_BRAM,config_num,top_results,config);
							}
						}
					}
				}
				set_current_best_configuration(config);
				run_model(M,total_DSP,total_BRAM,config_num,top_results,config);
				op_finished=true;
			}
			else{
				std::vector<std::pair<int,int> > optimize_elements;
				optimize_elements=function_subelement_cycles[F_index];
				while(!optimize_elements.empty()&&function_cycles[F]>=compared_latency){
					optimize_elements=function_subelement_cycles[F_index];
					int vector_size=optimize_elements.size();
					std::map<int,int> subelement_counted;
					subelement_counted.clear();
					for(std::vector<std::pair<int,int> >::iterator it=optimize_elements.begin();it!=optimize_elements.end();++it){
						int optimization_order=vector_size;
						int sub_cycles=it->second;
						int sub_order=it->first;
						for(std::vector<std::pair<int,int> >::iterator ii=optimize_elements.begin();ii!=optimize_elements.end();++ii){
							if(it!=ii){
								int sub_other_cycles=ii->second;
								if(sub_cycles>sub_other_cycles){
									optimization_order--;
								}
								else if(sub_cycles==sub_other_cycles){
									int sub_other_order=ii->first;
									if(!subelement_counted.count(sub_other_order)){
										optimization_order--;
									}
								}
							}
						}
						if(!subelement_counted.count(sub_order)){
							subelement_counted[sub_order]=optimization_order;
						}
					}
					int max_order=0;
					int second_max_order=0;
					int second_max_latency=0;
					for(std::map<int,int>::iterator it=subelement_counted.begin();it!=subelement_counted.end();++it){
						if(it->second==1){
							max_order=it->first;
						}
						else if(it->second==2){
							second_max_order=it->first;
							second_max_latency=get_subelement_latency(F,second_max_order);
						}
					}
					LoopInfo &LII = getAnalysis<LoopInfo>(*F);
					LoopInfo* LI;
					LI=&LII;
					if(LI->empty()){
						for(inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I){
							Instruction *inst=&*I;
							if(CallInst *call=dyn_cast<CallInst>(inst)){
								Function *callee = call->getCalledFunction();
								if(callee!=F){
									unsigned FnID=callee->getIntrinsicID();
									if(FnID==0){
										int f_order=get_function_order(F,LI,callee);
										if(f_order==max_order){//optimize callee
											bool finished=false;
											run_function_configuration(M,callee,top_function,second_max_latency,finished,total_DSP,total_BRAM,config_num,top_results,config);
											set_current_best_configuration(config);
											if(finished==true){//erase it from the vector
												std::vector<std::pair<int,int> >::iterator iter;
												for(iter=optimize_elements.begin();iter!=optimize_elements.end();++iter){
													if(iter->first==max_order){
														break;
													}
												}
												if(iter==optimize_elements.end()){
													errs()<<callee->getName()<<".\n";
													errs()<<"Wrong: this function pair doesn't exist in the vector.\n";
													break;
												}
												else{
													errs()<<callee->getName()<<" is optimized.\n";
													optimized_functions[callee]=1;
													optimize_elements.erase(iter);
												}
											}
											run_model(M,total_DSP,total_BRAM,config_num,top_results,config);
										}
									}
								}
							}
						}
					}
					else{
						for(inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I){
							Instruction *inst=&*I;
							BasicBlock *bb=inst->getParent();
							Loop *bb_loop=LI->getLoopFor(bb);
							if(bb_loop==NULL){
								if(CallInst *call=dyn_cast<CallInst>(inst)){
									Function *callee = call->getCalledFunction();
									if(callee!=F){
										unsigned FnID=callee->getIntrinsicID();
										if(FnID==0){
											int f_order=get_function_order(F,LI,callee);
											if(f_order==max_order){//optimize callee
												bool finished=false;
												run_function_configuration(M,callee,top_function,second_max_latency,finished,total_DSP,total_BRAM,config_num,top_results,config);
												set_current_best_configuration(config);
												if(finished==true){
													std::vector<std::pair<int,int> >::iterator iter;
													for(iter=optimize_elements.begin();iter!=optimize_elements.end();++iter){
														if(iter->first==max_order){
															break;
														}
													}
													if(iter==optimize_elements.end()){
														errs()<<callee->getName()<<".\n";
														errs()<<"Wrong: this function pair doesn't exist in the vector.\n";
													}
													else{
														errs()<<callee->getName()<<" is optimized.\n";
														optimized_functions[callee]=1;
														optimize_elements.erase(iter);
													}
												}
												run_model(M,total_DSP,total_BRAM,config_num,top_results,config);
											}
										}
									}
								}
							}
						}
						for(LoopInfo::reverse_iterator i=LI->rbegin(),e=LI->rend();i!=e;++i){
							Loop *L=*i;
							int l_order=get_loop_order(F,LI,L);
							if(l_order==max_order){//optimize loop
								std::vector<int> loop_index[l_num];
								std::map<Loop*,int> loop_index_tmp;
								set_loop_index(F,LI,L,loop_index_tmp,loop_index);
								int L_index=loop_index_tmp[L];
								bool finished=false;
								run_loop_configuration(M,F,top_function,loop_index_tmp,loop_index,max_order,second_max_latency,finished,total_DSP,total_BRAM,config_num,top_results,config);
								set_current_best_configuration(config);
								//config.get_configuration();
								if(finished==true){
									std::vector<std::pair<int,int> >::iterator iter;
									for(iter=optimize_elements.begin();iter!=optimize_elements.end();++iter){
										if(iter->first==max_order){
											break;
										}
									}
									if(iter==optimize_elements.end()){
										errs()<<"Wrong: this function pair doesn't exist in the vector.\n";
									}
									else{
										errs()<<"Loop "<<max_order<<" is optimized.\n";
										optimized_loops[L_index]=1;
										optimize_elements.erase(iter);
									}
								}
								run_model(M,total_DSP,total_BRAM,config_num,top_results,config);
							}
						}
					}
				}
				if(optimize_elements.empty()){
					op_finished=true;
				}
			}
		}

		void set_default_array()
		{
			int size=array_number.size();
			int accumulated_dim=0;
			for(int i=0;i<size;++i){
				int dim=array_dimension[i];
				for(int j=0; j<dim;++j){
					int new_j = j+accumulated_dim;
					array_partition_type[new_j]=1;
					array_partition_factor[new_j]=1;
				}
				accumulated_dim += dim;
			}
		}

		void set_default_loop()
		{
			for(int i=0;i<l_num;++i){
				Loops_unroll_input[i]=1;
				Loops_pipeline_input[i]=0;
			}
		}

		void set_default_function()
		{
			for(int i=0;i<f_num;++i){
				function_pipeline_input[i]=0;
				dataflow_input[i]=0;
			}
		}

		bool runOnModule(Module &M) override
		{
			errs()<<"In this module"<<"\n";
			/*********Parameters used to compare different configuration*********/
			unsigned total_DSP=2160;// parameter of total DSP.
			unsigned total_BRAM=2584;
			/****************************DSE setting*****************************/
			unsigned index_function=0;
			Function_index.clear();
			std::map<Function*,int> function_counted;
			for(auto F=M.begin(), E=M.end(); F!=E; ++F){
				unsigned FnID=F->getIntrinsicID();
				if(FnID==0){
					if(function_is_inline(F)==false){
						if(!function_counted.count(F)){
							Function_index[F]=index_function;
							index_function++;
							function_counted[F]=1;
						}
						else{
							function_counted[F]++;
						}
					}
				}
			}
			optimized_loops.clear();
			optimized_functions.clear();
			set_array_index(M);
			set_array_map(M);
			store_function_IO(M);
			set_loop_map_array(M);
			Function_pipeline.clear();
			Function_dataflow.clear();

			int config_num=1;
			Configuration config(0);
			Top_Results top_results;
			Function *top_function;
			unsigned top_index=index_function-1;
  			for(std::map<Function*,unsigned>::iterator it=Function_index.begin();it!=Function_index.end();++it){
  				if(it->second==top_index){
  					top_function=it->first;
  				}
  			}
  			bool op_finished=false;
  			timer_start=clock();

  			for(int j=1;j>=0;--j){
  				Function_dataflow[top_function]=j;
  				if(Function_dataflow[top_function]==0){//break;
  					for(int i=0;i<2;++i){
  						errs()<<"***********************************************************************\n";
  						errs()<<"<"<<config_num<<"> Directives Setting"<<"\n";
  						Function_pipeline[top_function]=i;
  						if(Function_pipeline[top_function]==1){
  							DSE_function_pipeline(M,top_function,top_function,0,op_finished,total_DSP,total_BRAM,config_num,top_results,config);
  							int best_configuration=config.get_num_config();
  							errs()<<"Best configuration is ";
  							config.get_configuration();
  							errs()<<"Function cycles: "<<top_results.top_function_cycles[best_configuration]<<"\n";
  							errs()<<"Function II: "<<top_results.top_function_II[best_configuration]<<"\n";
  							errs()<<"BRAM usage: "<<top_results.top_function_BRAM[best_configuration]<<"\n";
  							errs()<<"DSP usage: "<<top_results.top_function_DSP[best_configuration]<<"\n";
  						}
  						else{
  							DSE_function_not_pipeline(M,top_function,top_function,0,op_finished,total_DSP,total_BRAM,config_num,top_results,config);
  						}
  					}
  				}
  				else{
					LoopInfo &LI = getAnalysis<LoopInfo>(*top_function);
					LoopInfo* LI_pointer;
					LI_pointer=&LI;
  					bool dataflow_work=check_dataflow(top_function,LI_pointer);
  					if(dataflow_work==true){
  						Function_pipeline[top_function]=0;
						Function_dataflow[top_function]=1;
						DSE_function_not_pipeline(M,top_function,top_function,0,op_finished,total_DSP,total_BRAM,config_num,top_results,config);
  					}
  				}
  			}

  			timer_end=clock();
			double time_consume;
			time_consume=(double)(timer_end-timer_start)/CLOCKS_PER_SEC;
			errs() << "total time consumed: " << time_consume << "\n";
			if(op_finished==false){
				errs()<<"DSE not finished...\n";
			}
  			return false;
		}
   };
}

char Test::ID = 0;
static RegisterPass<Test> X("test", "Test Pass");
