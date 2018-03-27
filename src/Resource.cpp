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


#include "Resource.h"
#include "Constant.h"
#include "Library.h"
#include "Power.h"
#include "FindLatency.h"
#include "ComputeCriticalPath.h"
#include "Reschedule.h"
#include "LoopUnrollFactor.h"
#include "Cast.h"
#include "FunctionLatency.h"
#include "Dataflow_II.h"
#include "ArrayName.h"


void inst_map_resource(Instruction *inst, bool &shareable_unit,std::map<Instruction*,std::string> &inst_unit)
{
	switch(inst->getOpcode()) //DSP amount will not change when frequency changes but LUT and FF will change(not too much)
	{
		case Instruction::Add:
		case Instruction::Sub:
			shareable_unit=false;
			if(!inst_unit.count(inst)){
				inst_unit[inst]="IntAdd";
			}
			break;
		case Instruction::FAdd:
		case Instruction::FSub:
			shareable_unit=true;
			if(!inst_unit.count(inst)){
				inst_unit[inst]="FAdd";
			}
			break;
		case Instruction::Mul:
		{
			Value *op1;
			op1=inst->getOperand(1);
			if(ConstantInt *cont=dyn_cast<ConstantInt>(op1)){
				int opconst=cont->getSExtValue();
				if(is_power_of_two(opconst)){  //shift+add: 32 LUT
					shareable_unit=false;
					if(!inst_unit.count(inst)){
						inst_unit[inst]="IntAdd";
					}
				}
				else{ //int_mul:2 DSP, 4 LUT
					int bound_power_two=closest_bound_power_two(opconst);
					if(opconst<0){
						opconst=-opconst;
					}
					int delta=opconst-bound_power_two;
					if(delta<0){
						delta=-delta;
					}
					if(delta==1||(is_power_of_two(delta))){
						shareable_unit=false;
						if(!inst_unit.count(inst)){
							inst_unit[inst]="IntAdd";
						}
					}
					else{
						shareable_unit=false;
						if(!inst_unit.count(inst)){
							inst_unit[inst]="IntMul";
						}
					}
				}
			}
			else{ //Mul: 4 DSP, 0 LUT
				shareable_unit=false;//true; //pipelined core, release in the next cycle
				if(!inst_unit.count(inst)){
					inst_unit[inst]="IMul";
				}
			}
			break;
		}
		case Instruction::SDiv:
		case Instruction::SRem:
		case Instruction::UDiv:
		case Instruction::URem:
		{
			Value *op1;
			op1=inst->getOperand(1);
			if(ConstantInt *cont=dyn_cast<ConstantInt>(op1)){
				int opconst=cont->getSExtValue();
				if(is_power_of_two(opconst)){  //96 LUT
					shareable_unit=false;
					if(!inst_unit.count(inst)){
						inst_unit[inst]="IntDiv";
					}
				}
				else{ //use mul unit
					shareable_unit=false;
					if(!inst_unit.count(inst)){
						inst_unit[inst]="IMul";
					}
				}
			}
			else{ //div unit: 320 LUT, 288 FF
				shareable_unit=true;
				if(!inst_unit.count(inst)){
					inst_unit[inst]="IDiv";
				}
			}
			break;
		}
		case Instruction::FMul://3 DSP,138 LUT,128 FF
			shareable_unit=true;
			if(!inst_unit.count(inst)){
				inst_unit[inst]="FMul";
			}
			break;
		case Instruction::FDiv://801 LUT, 453 FF
		case Instruction::FRem:
		{
			Value *op1;
			op1=inst->getOperand(1);
			if(ConstantInt *cont=dyn_cast<ConstantInt>(op1)){
				int opconst=cont->getSExtValue();
				if(is_power_of_two(opconst)){
					shareable_unit=true;
					if(!inst_unit.count(inst)){
						inst_unit[inst]="FMul";
					}
				}
				else{
					shareable_unit=true;
					if(!inst_unit.count(inst)){
						inst_unit[inst]="FDiv";
					}
				}
			}
			else{
				shareable_unit=true;
				if(!inst_unit.count(inst)){
					inst_unit[inst]="FDiv";
				}
			}
			break;
		}
		case Instruction::And:
		case Instruction::Or:
		case Instruction::Xor: //2 16-add
			shareable_unit=false;
			if(!inst_unit.count(inst)){
				inst_unit[inst]="IntAdd";
			}
			break;
		case Instruction::ICmp:
		case Instruction::FCmp:
			shareable_unit=false;
			if(!inst_unit.count(inst)){
				inst_unit[inst]="IntAdd";
			}
			break;
		case Instruction::PtrToInt:
		case Instruction::IntToPtr:
		case Instruction::FPToUI:
		case Instruction::FPToSI:
		case Instruction::UIToFP:
		case Instruction::SIToFP: //339 LUT, 168 FF
			shareable_unit=true;
			if(!inst_unit.count(inst)){
				inst_unit[inst]="Cast";
			}
			break;
		case Instruction::Call:
			shareable_unit=false;
			if(!inst_unit.count(inst)){
				inst_unit[inst]="Call";
			}
			break;
		case Instruction::Shl:
		case Instruction::LShr:
		case Instruction::AShr:
		case Instruction::Ret:
		case Instruction::Br:
		case Instruction::Switch:
		case Instruction::IndirectBr:
		case Instruction::Invoke:
		case Instruction::Resume:
		case Instruction::Unreachable:
		case Instruction::Fence:
		case Instruction::AtomicCmpXchg:
		case Instruction::AtomicRMW:
		case Instruction::AddrSpaceCast:
		case Instruction::UserOp1:
		case Instruction::UserOp2:
		case Instruction::VAArg:
		case Instruction::ExtractElement:
		case Instruction::InsertElement:
		case Instruction::ShuffleVector:
		case Instruction::ExtractValue:
		case Instruction::InsertValue:
		case Instruction::LandingPad:
		case Instruction::Alloca:
		case Instruction::Load:
		case Instruction::Store:
		case Instruction::GetElementPtr:
		case Instruction::PHI:
		case Instruction::Select:
		case Instruction::Trunc:
		case Instruction::ZExt:
		case Instruction::SExt:
		case Instruction::FPTrunc:
		case Instruction::FPExt:
		case Instruction::BitCast:
			break;
		default:
			puts("It is something cannot be handled\n");
			//exit(0);
	}
}


void get_unit_resource(Instruction *inst, std::map<Instruction*,std::string> &inst_unit, std::map<std::string, unsigned> &resource_usage,std::vector<std::pair<std::string,unsigned> > *function_resource)
{
	if(inst_unit.count(inst)){
		std::string unit=inst_unit[inst];
		int judge=0;
		if(unit=="IntAdd"){
			judge=1;
		}
		else if(unit=="IntMul"){
			judge=2;
		}
		else if(unit=="IMul"){
			judge=3;
		}
		else if(unit=="IntDiv"){
			judge=4;
		}
		else if(unit=="IDiv"){
			judge=5;
		}
		else if(unit=="FAdd"){
			judge=6;
		}
		else if(unit=="FMul"){
			judge=7;
		}
		else if(unit=="FDiv"){
			judge=8;
		}
		else if(unit=="Cast"){
			judge=9;
		}
		else if(unit=="Call"){
			judge=10;
		}
		switch(judge)
		{
			case 1:
				resource_usage["DSP"]+=0;
				resource_usage["LUT"]+=32;
				resource_usage["FF"]+=0;
				break;
			case 2:
				resource_usage["DSP"]+=2;
				resource_usage["LUT"]+=4;
				resource_usage["FF"]+=0;
				break;
			case 3:
				resource_usage["DSP"]+=4;
				resource_usage["LUT"]+=0;
				resource_usage["FF"]+=0;
				break;
			case 4:
				resource_usage["DSP"]+=0;
				resource_usage["LUT"]+=96;
				resource_usage["FF"]+=0;
				break;
			case 5:
				resource_usage["DSP"]+=0;
				resource_usage["LUT"]+=320;
				resource_usage["FF"]+=288;
				break;
			case 6:
				resource_usage["DSP"]+=2;
				resource_usage["LUT"]+=214;
				resource_usage["FF"]+=227;
				break;
			case 7:
				resource_usage["DSP"]+=3;
				resource_usage["LUT"]+=138;
				resource_usage["FF"]+=128;
				break;
			case 8:
				resource_usage["DSP"]+=0;
				resource_usage["LUT"]+=801;
				resource_usage["FF"]+=453;
				break;
			case 9:
				resource_usage["DSP"]+=0;
				resource_usage["LUT"]+=339;
				resource_usage["FF"]+=168;
				break;
			case 10:
				if(CallInst *call=dyn_cast<CallInst>(inst)){
					Function *callee=call->getCalledFunction();
					unsigned FnID=callee->getIntrinsicID();
					if(FnID==0){
						if(Function_index.count(callee)){
							unsigned callee_index=Function_index[callee];
							for(std::vector<std::pair<std::string,unsigned> >::iterator it=function_resource[callee_index].begin();it!=function_resource[callee_index].end();++it){
								std::string resource_name=it->first;
								if(resource_name=="DSP"){
									resource_usage["DSP"]+=it->second;
								}
								else if(resource_name=="LUT"){
									resource_usage["LUT"]+=it->second;
								}
								else if(resource_name=="FF"){
									resource_usage["FF"]+=it->second;
								}
							}
						}
					}
				}
				break;
			default:
				break;
		}
	}
	else{
		//errs()<<"This inst has not been scheduled for resource\n";
	}
}

void compute_resource(Instruction *inst,std::map<std::string, unsigned> &loop_resource, std::vector<std::pair<std::string,unsigned> > *function_resource, std::map<Instruction*, std::string> &inst_unit, std::map<Instruction*,std::string> &scheduled_inst_unit, std::map<Instruction*, unsigned> &instr_index, std::vector<std::pair<int,float> > *dependence)
{
	bool shareable=false;
	inst_map_resource(inst,shareable,inst_unit);
	std::map<std::string, unsigned> resource_usage;
	get_unit_resource(inst,inst_unit,resource_usage,function_resource);
	std::string unit=inst_unit[inst];
	if(shareable==false||unit=="IMul"){
		loop_resource["DSP"]+=resource_usage["DSP"];
		//loop_resource["BRAM"]+=resource_usage["BRAM"];
		loop_resource["LUT"]+=resource_usage["LUT"];
		loop_resource["FF"]+=resource_usage["FF"];
	}
	else{
		if(!scheduled_inst_unit.count(inst)){
			scheduled_inst_unit[inst]=unit;
			for(std::map<Instruction*,unsigned>::iterator it=instr_index.begin();it!=instr_index.end();++it){
				Instruction *inst1=it->first;
				if(inst!=inst1){
					if(inst_unit.count(inst1)){
						std::string inst1_unit=inst_unit[inst1];
						if(inst1_unit==unit){
							float latency1=find_latency(inst1,inst,instr_index,dependence);
							float latency2=find_latency(inst,inst1,instr_index,dependence);
							if(latency1!=0.0||latency2!=0.0){
								scheduled_inst_unit[inst1]=inst1_unit;
							}
						}
					}
				}
			}
			loop_resource["DSP"]+=resource_usage["DSP"];
			//loop_resource["BRAM"]+=resource_usage["BRAM"];
			loop_resource["LUT"]+=resource_usage["LUT"];
			loop_resource["FF"]+=resource_usage["FF"];
		}
	}
}

bool loop_pipeline_is_set(Loop *L)
{
	bool pipelined=false;
	if(L!=NULL){
		if(Loops_pipeline[L]==1){
			pipelined=true;
		}
		else{
			Loop *L_parent=L->getParentLoop();
			pipelined=loop_pipeline_is_set(L_parent);
		}
	}
	return pipelined;
}

Loop *get_pipelined_loop(Loop *L)
{
	Loop *pipelined_loop=NULL;
	if(L!=NULL){
		if(Loops_pipeline[L]==1){
			pipelined_loop=L;
		}
		else{
			Loop *L_parent=L->getParentLoop();
			pipelined_loop=get_pipelined_loop(L_parent);
		}
	}
	return pipelined_loop;
}

void count_related_loops(Loop *L, std::map<Loop*,unsigned> &Loop_counted)
{
	if(!Loop_counted.count(L)){
		Loop_counted[L]=1;
		std::vector<Loop*> subLoops = L->getSubLoops();
		if(!subLoops.empty()){
			int subLoop_num = subLoops.size();
			for(int i=0;i<subLoop_num;++i){
				Loop *subloop=subLoops.at(i);
				count_related_loops(subloop,Loop_counted);
			}
		}
	}
	else{
		Loop_counted[L]++;
		std::vector<Loop*> subLoops = L->getSubLoops();
		if(!subLoops.empty()){
			int subLoop_num = subLoops.size();
			for(int i=0;i<subLoop_num;++i){
				Loop *subloop=subLoops.at(i);
				count_related_loops(subloop,Loop_counted);
			}
		}
	}
}

bool all_loop_unrolled(Loop *L)
{
	bool unrolled=false;
	if(L!=NULL){
		if(Loops_unroll[L]>=Loops_counter[L]){
			Loop *L_parent=L->getParentLoop();
			if(L_parent==NULL){
				unrolled=true;
			}
			else{
				if(Loops_unroll[L_parent]>=Loops_counter[L]){
					unrolled=all_loop_unrolled(L_parent);
				}
				else{
					unrolled=false;
				}
			}
		}
		else{
			unrolled=false;
		}
	}
	return unrolled;
}

void compute_loop_resource(LoopInfo* LI, Loop *L,std::vector<std::pair<std::string,unsigned> > *function_resource, std::map<std::string, unsigned> &loop_resource, std::map<Instruction*, unsigned> &load_buff,std::map<Loop*,unsigned> &Loop_counted)
{
	if(!Loop_counted.count(L)){
		if(Loops_pipeline[L]==1){//no share.
			std::map<Instruction*,std::string> inst_unit;
			std::map<Instruction*, unsigned> instr_index;
			std::vector<std::pair<int,float> > dependence[INSN_NUM];
			std::map<Instruction*,std::queue<unsigned> > inst_map_branch;
			dependence_set(LI,L,instr_index,load_buff,dependence,inst_map_branch);
			reschedule_dependence(LI,frequency,instr_index,dependence);
			std::map<Instruction*,unsigned> shareable_unit_count;
			//int L_factor=Loops_unroll[L];
			std::map<std::string, unsigned> loop_resource_tmp;
			for(Loop::block_iterator BI=L->block_begin(), BE=L->block_end(); BI !=BE; ++BI){
				BasicBlock *bb=*BI;
				Loop *L_bb=LI->getLoopFor(bb);
				//int unroll_factor=total_unroll_factor(L_bb);
				for(auto iti=bb->begin();iti!=bb->end();++iti){
					Instruction *inst=iti;
					bool shareable=false;
					inst_map_resource(inst,shareable,inst_unit);
					std::map<std::string, unsigned> resource_usage;
					get_unit_resource(inst,inst_unit,resource_usage,function_resource);
					if(!resource_usage.empty()){
						int unroll_factor=total_unroll_factor_op(LI,L_bb,iti);
						loop_resource_tmp["DSP"]+=resource_usage["DSP"]*unroll_factor;
						loop_resource_tmp["LUT"]+=resource_usage["LUT"]*unroll_factor;
						loop_resource_tmp["FF"]+=resource_usage["FF"]*unroll_factor;
						if(shareable==true){
							shareable_unit_count[inst]+=unroll_factor*1;
						}
					}
				}
			}
			if(loop_II.count(L)){
				int II=loop_II[L];
				for(std::map<Instruction*,unsigned>::iterator it=shareable_unit_count.begin();it!=shareable_unit_count.end();++it){
					Instruction *inst=it->first;
					std::string unit=inst_unit[inst];
					float latency_max_in_loop=0.0;
					for(std::map<Instruction*,unsigned>::iterator ij=instr_index.begin();ij!=instr_index.end();++ij){
						Instruction *inst_tmp=ij->first;
						if(inst_is_cast(inst_tmp)){
							continue;
						}
						else{
							float latency_inst_inst=find_latency(inst_tmp,inst,instr_index,dependence);
							float latency_inst_tmp=get_inst_latency(inst,instr_index,dependence);
							if(latency_inst_inst>latency_inst_tmp){
								if(isa<LoadInst>(inst_tmp)){
									latency_inst_inst=latency_inst_inst-2.0+Load_latency[inst_tmp]-latency_inst_tmp;
								}
								else{
									latency_inst_inst=latency_inst_inst-latency_inst_tmp;
								}
							}
							latency_max_in_loop=std::max(latency_inst_inst,latency_max_in_loop);
						}
					}
					if(latency_max_in_loop<1){
						latency_max_in_loop=1;
					}
					if(latency_max_in_loop>0){
						if(II>latency_max_in_loop){
							II=latency_max_in_loop;
						}
					}
					unsigned share_unit_num=it->second;
					unsigned used_unit_num=(unsigned)ceil((float)share_unit_num/(float)II);
					unsigned delta=share_unit_num-used_unit_num;
					std::map<std::string, unsigned> resource_usage;
					get_unit_resource(inst,inst_unit,resource_usage,function_resource);
					loop_resource_tmp["DSP"]=loop_resource_tmp["DSP"]-resource_usage["DSP"]*delta;
					loop_resource_tmp["LUT"]=loop_resource_tmp["LUT"]-resource_usage["LUT"]*delta;
					loop_resource_tmp["FF"]=loop_resource_tmp["FF"]-resource_usage["FF"]*delta;
				}
			}
			count_related_loops(L,Loop_counted);
			loop_resource["DSP"]+=loop_resource_tmp["DSP"];//*L_factor;
			loop_resource["LUT"]+=loop_resource_tmp["LUT"];//*L_factor;
			loop_resource["FF"]+=loop_resource_tmp["FF"];//*L_factor;
		}
		else{
			for(Loop::block_iterator BI=L->block_begin(), BE=L->block_end(); BI !=BE; ++BI){
				BasicBlock *bb=*BI;
				Loop *L_bb=LI->getLoopFor(bb);
				bool pipelined=loop_pipeline_is_set(L_bb);
				if(pipelined==true){
					Loop *pipelined_loop=get_pipelined_loop(L_bb);
					if(pipelined_loop!=NULL){
						if(!Loop_counted.count(pipelined_loop)){
							std::map<std::string, unsigned> pipelined_loop_resource;
							compute_loop_resource(LI,pipelined_loop,function_resource,pipelined_loop_resource,load_buff,Loop_counted);
							loop_resource["DSP"]+=pipelined_loop_resource["DSP"];
							loop_resource["LUT"]+=pipelined_loop_resource["LUT"];
							loop_resource["FF"]+=pipelined_loop_resource["FF"];
						}
					}
					else{
						errs()<<"Please check this error.\n";
					}
				}
				else{
					if(!Loop_counted.count(L_bb)){
						Loop_counted[L_bb]=1;
						std::vector<Loop*> sub_L_bb = L_bb->getSubLoops();
			//////////////////////////////empty sub_loops/////////////////////////////
						if(sub_L_bb.empty()){
							std::map<std::string, unsigned> loop_resource_tmp;
							std::map<Instruction*,std::string> inst_unit;
							for(Loop::block_iterator bi=L_bb->block_begin(), be=L_bb->block_end(); bi!=be; ++bi){
								BasicBlock *sub_bb=*bi;
								for(auto iti=sub_bb->begin();iti!=sub_bb->end();++iti){
									bool shareable=false;
									inst_map_resource(iti,shareable,inst_unit);
								}
							}
							std::map<Instruction*, unsigned> instr_index;
							std::vector<std::pair<int,float> > dependence[INSN_NUM];
							std::map<Instruction*,std::queue<unsigned> > inst_map_branch;
							dependence_set(LI,L_bb,instr_index,load_buff,dependence,inst_map_branch);
							for(Loop::block_iterator bi=L_bb->block_begin(), be=L_bb->block_end(); bi!=be; ++bi){
								BasicBlock *sub_bb=*bi;
								for(auto iti=sub_bb->begin();iti!=sub_bb->end();++iti){
									Instruction *inst=iti;
									bool shareable=false;
									inst_map_resource(iti,shareable,inst_unit);
									std::string unit=inst_unit[inst];
									std::map<std::string, unsigned> resource_usage;
									std::map<Instruction*,std::string> scheduled_inst_unit;
									compute_resource(inst,resource_usage,function_resource,inst_unit,scheduled_inst_unit,instr_index,dependence);
									if(!resource_usage.empty()){
										int unroll_factor=total_unroll_factor_op(LI,L_bb,iti);
										if(shareable==false){
											loop_resource_tmp["DSP"]+=resource_usage["DSP"]*unroll_factor;
											loop_resource_tmp["LUT"]+=resource_usage["LUT"]*unroll_factor;
											loop_resource_tmp["FF"]+=resource_usage["FF"]*unroll_factor;
										}
										else{
											float latency_max_in_loop=0.0;
											for(std::map<Instruction*,unsigned>::iterator ij=instr_index.begin();ij!=instr_index.end();++ij){
												Instruction *inst_tmp=ij->first;
												if(inst_is_cast(inst_tmp)){
													continue;
												}
												else{
													float latency_inst_inst=find_latency(inst_tmp,inst,instr_index,dependence);
													float latency_inst_tmp=get_inst_latency(inst,instr_index,dependence);
													if(latency_inst_inst>latency_inst_tmp){
														if(isa<LoadInst>(inst_tmp)){
															latency_inst_inst=latency_inst_inst-2.0+Load_latency[inst_tmp]-latency_inst_tmp;
														}
														else{
															latency_inst_inst=latency_inst_inst-latency_inst_tmp;
														}
													}
													latency_max_in_loop=std::max(latency_inst_inst,latency_max_in_loop);
												}
											}
											unsigned used_unit_num=1;
											if(latency_max_in_loop<1){
												latency_max_in_loop=1;
											}
											if(latency_max_in_loop>0){
												unsigned share_unit_num=unroll_factor;
												used_unit_num=(unsigned)ceil((float)share_unit_num/(float)latency_max_in_loop);
											}
											loop_resource_tmp["DSP"]+=resource_usage["DSP"]*used_unit_num;
											loop_resource_tmp["LUT"]+=resource_usage["LUT"]*used_unit_num;
											loop_resource_tmp["FF"]+=resource_usage["FF"]*used_unit_num;
										}
									}
								}
							}
							loop_resource["DSP"]+=loop_resource_tmp["DSP"];
							loop_resource["LUT"]+=loop_resource_tmp["LUT"];
							loop_resource["FF"]+=loop_resource_tmp["FF"];
						}
		/////////////////////////////not empty sub_loops////////////////////////////////////
						else{
			    			for(Loop::block_iterator bi=L_bb->block_begin(), be=L_bb->block_end(); bi!=be; ++bi)
			    			{
			    				BasicBlock *sub_bb=*bi;
			    				bool bb_in_loop_flag=false;
			    				Loop *subloop=NULL;
			    				for(unsigned j=0;j<sub_L_bb.size();++j){
			    					Loop *subloop_bb=sub_L_bb.at(j);
			    					if(subloop_bb->contains(sub_bb)){
			    						bb_in_loop_flag=true;
			    						subloop=subloop_bb;
			    						break;
			    					}
			    				}
								if(bb_in_loop_flag==false){
									std::map<Instruction*,std::string> inst_unit;
									for(auto iti=sub_bb->begin();iti!=sub_bb->end();++iti){
										bool shareable=false;
										inst_map_resource(iti,shareable,inst_unit);
									}
									unsigned index=1;
									std::map<Instruction*, unsigned> instr_index;
									std::vector<std::pair<int,float> > dependence[INSN_NUM];
									std::map<Instruction*,std::queue<unsigned> > inst_map_branch;
									for(auto iti=sub_bb->begin();iti!=sub_bb->end();++iti){
				      					update(LI,iti,index,instr_index,load_buff,dependence,inst_map_branch);
									}
			    					std::map<Instruction*,std::string> scheduled_inst_unit;
			    					std::map<std::string, unsigned> loop_resource_tmp;
									for(auto iti=sub_bb->begin();iti!=sub_bb->end();++iti){
										Instruction *inst=iti;
										bool shareable=false;
										inst_map_resource(iti,shareable,inst_unit);
										std::string unit=inst_unit[inst];
										std::map<std::string, unsigned> resource_usage;
										compute_resource(inst,resource_usage,function_resource,inst_unit,scheduled_inst_unit,instr_index,dependence);
										if(!resource_usage.empty()){
											int unroll_factor=total_unroll_factor_op(LI,L_bb,iti);
											if(shareable==false){
												loop_resource_tmp["DSP"]+=resource_usage["DSP"]*unroll_factor;
												loop_resource_tmp["LUT"]+=resource_usage["LUT"]*unroll_factor;
												loop_resource_tmp["FF"]+=resource_usage["FF"]*unroll_factor;
											}
											unsigned used_unit_num=1;
											float latency_max_in_loop=find_latency_before_inst(inst,instr_index,dependence)-1.0;
											if(latency_max_in_loop<1){
												latency_max_in_loop=1;
											}
											if(latency_max_in_loop>0){
												unsigned share_unit_num=unroll_factor;
												used_unit_num=(unsigned)ceil((float)share_unit_num/(float)latency_max_in_loop);
											}
											loop_resource_tmp["DSP"]+=resource_usage["DSP"]*used_unit_num;
											loop_resource_tmp["LUT"]+=resource_usage["LUT"]*used_unit_num;
											loop_resource_tmp["FF"]+=resource_usage["FF"]*used_unit_num;
										}
									}
									loop_resource["DSP"]+=loop_resource_tmp["DSP"];
									loop_resource["LUT"]+=loop_resource_tmp["LUT"];
									loop_resource["FF"]+=loop_resource_tmp["FF"];
								}
								else{
									if(Loop_counted.count(subloop)){
										Loop_counted[subloop]++;
									}
									else{
										if(Loops_pipeline[subloop]==1){
											std::map<std::string, unsigned> subloop_resource;
											compute_loop_resource(LI,subloop,function_resource,subloop_resource,load_buff,Loop_counted);
											count_related_loops(subloop,Loop_counted);
											loop_resource["DSP"]+=subloop_resource["DSP"];
											loop_resource["LUT"]+=subloop_resource["LUT"];
											loop_resource["FF"]+=subloop_resource["FF"];
										}
										else{
											std::map<std::string, unsigned> subloop_resource;
											compute_loop_resource(LI,subloop,function_resource,subloop_resource,load_buff,Loop_counted);
											loop_resource["DSP"]+=subloop_resource["DSP"];
											loop_resource["LUT"]+=subloop_resource["LUT"];
											loop_resource["FF"]+=subloop_resource["FF"];
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

void resource_accumulation(LoopInfo* LI,Function *F,std::vector<std::pair<std::string,unsigned> > *function_resource, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop)
{
	if(Function_index.count(F)){
		unsigned F_index=Function_index[F];
		if(Function_pipeline[F]==1){
			//no share. then use II to get the minimum resource usage.(the amount could be used to share)
			std::map<Instruction*,unsigned> shareable_unit_count;
			std::map<Instruction*,std::string> inst_unit;
			for(auto itb=F->begin();itb!=F->end();++itb){
				BasicBlock *bb=itb;
				int unroll_factor=1;
				for(auto iti=itb->begin();iti!=itb->end();++iti){
					Instruction *inst=iti;
					bool shareable=false;
					inst_map_resource(inst,shareable,inst_unit);
					std::map<std::string, unsigned> resource_usage;
					get_unit_resource(inst,inst_unit,resource_usage,function_resource);
					if(!resource_usage.empty()){
						if(!LI->empty()){
							Loop *L=LI->getLoopFor(bb);
							if(L!=NULL){
								unroll_factor=total_unroll_factor_op(LI,L,iti);
							}
						}
						for(std::vector<std::pair<std::string,unsigned> >::iterator ii=function_resource[F_index].begin();ii!=function_resource[F_index].end();++ii){
							std::string resource=ii->first;
							if(resource=="DSP"){
								ii->second+=resource_usage["DSP"]*unroll_factor;
							}
							else if(resource=="LUT"){
								ii->second+=resource_usage["LUT"]*unroll_factor;
							}
							else if(resource=="FF"){
								ii->second+=resource_usage["FF"]*unroll_factor;
							}
						}
						if(shareable==true){
							//std::string share_unit=inst_unit[inst];
							shareable_unit_count[inst]+=unroll_factor*1;
						}
					}
				}
			}
			if(function_II.count(F)){
				int II=function_II[F];
				for(std::map<Instruction*,unsigned>::iterator it=shareable_unit_count.begin();it!=shareable_unit_count.end();++it){
					Instruction *inst=it->first;
					std::string unit=inst_unit[inst];
					float latency_max_in_function=0.0;
					for(std::map<Instruction*,unsigned>::iterator ij=instr_index_loop.begin();ij!=instr_index_loop.end();++ij){
						Instruction *inst_tmp=ij->first;
						if(inst_is_cast(inst_tmp)){
							continue;
						}
						else{
							float latency_inst_inst=find_latency(inst_tmp,inst,instr_index_loop,dependence_loop);
							float latency_inst_tmp=get_inst_latency(inst,instr_index_loop,dependence_loop);
							if(latency_inst_inst>latency_inst_tmp){
								if(isa<LoadInst>(inst_tmp)){
									latency_inst_inst=latency_inst_inst-2.0+Load_latency[inst_tmp]-latency_inst_tmp;
								}
								else{
									latency_inst_inst=latency_inst_inst-latency_inst_tmp;
								}
							}
							latency_max_in_function=std::max(latency_inst_inst,latency_max_in_function);
						}
					}
					if(latency_max_in_function<1){
						latency_max_in_function=1;
					}
					if(latency_max_in_function>0){
						if(II>latency_max_in_function){
							II=latency_max_in_function;
						}
					}
					unsigned share_unit_num=it->second;
					unsigned used_unit_num=(unsigned)ceil((float)share_unit_num/(float)II);
					unsigned delta=share_unit_num-used_unit_num;
					std::map<std::string, unsigned> resource_usage;
					get_unit_resource(inst,inst_unit,resource_usage,function_resource);
					for(std::vector<std::pair<std::string,unsigned> >::iterator ii=function_resource[F_index].begin();ii!=function_resource[F_index].end();++ii){
						std::string resource=ii->first;
						if(resource=="DSP"){
							ii->second=ii->second-resource_usage["DSP"]*delta;
							errs()<<ii->second<<"\n";
						}
						else if(resource=="LUT"){
							ii->second=ii->second-resource_usage["LUT"]*delta;
						}
						else if(resource=="FF"){
							ii->second=ii->second-resource_usage["FF"]*delta;
						}
					}
				}
			}
		}
		else{
			std::map<Instruction*, unsigned> load_buff;
			reorder_load(F,load_buff);
			if(LI->empty()){
				std::map<Instruction*,std::string> inst_unit;
				for(auto itb=F->begin();itb!=F->end();++itb){
					for(auto iti=itb->begin();iti!=itb->end();++iti){
						Instruction *inst=iti;
						bool shareable=false;
						inst_map_resource(inst,shareable,inst_unit);
					}
				}
				std::map<Instruction*,std::string> scheduled_inst_unit;
				for(auto itb=F->begin();itb!=F->end();++itb){
					for(auto iti=itb->begin();iti!=itb->end();++iti){
						Instruction *inst=iti;
						std::map<std::string, unsigned> inst_resource;
						compute_resource(inst,inst_resource,function_resource,inst_unit,scheduled_inst_unit,instr_index_loop,dependence_loop);
						for(std::vector<std::pair<std::string,unsigned> >::iterator ii=function_resource[F_index].begin();ii!=function_resource[F_index].end();++ii){
							std::string resource=ii->first;
							if(resource=="DSP"){
								ii->second+=inst_resource["DSP"];
							}
							else if(resource=="LUT"){
								ii->second+=inst_resource["LUT"];
							}
							else if(resource=="FF"){
								ii->second+=inst_resource["FF"];
							}
						}
					}
				}
			}
			else{
				std::map<Loop*,int> loop_resource_counted;
				for(auto itb=F->begin();itb!=F->end();++itb){
					bool bb_in_loop_flag=false;
					Loop *loop=NULL;
					for(LoopInfo::reverse_iterator i=LI->rbegin(),e=LI->rend();i!=e;++i){
						Loop *L=*i;
						if(L->contains(itb)){
							bb_in_loop_flag=true;
							loop=L;
							break;
						}
					}
					if(bb_in_loop_flag==false){
						std::map<Instruction*,std::string> inst_unit;
						for(auto iti=itb->begin();iti!=itb->end();++iti){
							Instruction *inst=iti;
							bool shareable=false;
							inst_map_resource(inst,shareable,inst_unit);
						}
						unsigned index=1;
						std::map<Instruction*, unsigned> instr_index;
    					std::vector<std::pair<int,float> > dependence[INSN_NUM];
    					std::map<Instruction*,std::queue<unsigned> > inst_map_branch;
						for(auto iti=itb->begin();iti!=itb->end();++iti){
	      					update(LI,iti,index,instr_index,load_buff,dependence,inst_map_branch);
						}
    					std::map<Instruction*,std::string> scheduled_inst_unit;
						for(auto iti=itb->begin();iti!=itb->end();++iti){
							Instruction *inst=iti;
							std::map<std::string, unsigned> inst_resource;
							compute_resource(inst,inst_resource,function_resource,inst_unit,scheduled_inst_unit,instr_index,dependence);
							for(std::vector<std::pair<std::string,unsigned> >::iterator ii=function_resource[F_index].begin();ii!=function_resource[F_index].end();++ii){
								std::string resource=ii->first;
								if(resource=="DSP"){
									ii->second+=inst_resource["DSP"];
								}
								else if(resource=="LUT"){
									ii->second+=inst_resource["LUT"];
								}
								else if(resource=="FF"){
									ii->second+=inst_resource["FF"];
								}
							}
						}
					}
					else{
						if(loop_resource_counted.count(loop)){
							++loop_resource_counted[loop];
						}
						else{
							loop_resource_counted[loop]=1;
							std::map<std::string, unsigned> loop_resource;
							std::map<Loop*,unsigned> Loop_counted;
							compute_loop_resource(LI,loop,function_resource,loop_resource,load_buff,Loop_counted);
							for(std::vector<std::pair<std::string,unsigned> >::iterator ii=function_resource[F_index].begin();ii!=function_resource[F_index].end();++ii){
								std::string resource=ii->first;
								if(resource=="DSP"){
									ii->second+=loop_resource["DSP"];
								}
								else if(resource=="LUT"){
									ii->second+=loop_resource["LUT"];
								}
								else if(resource=="FF"){
									ii->second+=loop_resource["FF"];
								}
							}
						}
					}
				}
			}
		}
	}
	else{
		errs()<<"Please check whether this function is considered.\n";
	}
}

//BRAM usage only need to be considered within this function: global memory and local memory it used. Don't need to consider other called function's BRAM usage.
void bram_accumulation(LoopInfo* LI,Function *F,std::vector<std::pair<std::string,unsigned> > *function_resource)
{
	std::map<Value*,int> function_argument;
	std::map<Value*,int> bram_counted_array;
	if(Function_index.count(F)){
		unsigned F_index=Function_index[F];
		if(!F->arg_empty()){
			for(Function::arg_iterator ArgI=F->arg_begin(),ArgE=F->arg_end();ArgI!=ArgE; ++ArgI){
				//Argument *arg=ArgI;
				if(Value *val=dyn_cast<Value>(ArgI)){
					function_argument[val]=1;
				}
			}
		}
		for(inst_iterator I=inst_begin(F), E=inst_end(F);I!=E;++I){
			Instruction *inst = &*I;
			if(isa<LoadInst>(inst)||isa<StoreInst>(inst)){
				Value *array=get_array_name(inst);
				if(array!=NULL){
					if(!bram_counted_array.count(array)){
						if(!function_argument.count(array)){
							if(array_number.count(array)){
								int true_simple_factor=2;
								int ls_num=0;
								for(inst_iterator II=inst_begin(F), IE=inst_end(F);II!=IE;++II){
									Instruction *inst_ii = &*II;
									if((isa<LoadInst>(inst)&&isa<LoadInst>(inst_ii))||(isa<StoreInst>(inst)&&isa<StoreInst>(inst_ii))){
										Value *array_ii=get_array_name(inst_ii);
										if(array_ii==array){
											BasicBlock *bb=inst_ii->getParent();
											int unroll_factor=1;
											//Loop *L=LI->getLoopFor(bb);
											if(!LI->empty()){
												Loop *L=LI->getLoopFor(bb);
												if(L!=NULL){
													unroll_factor=total_unroll_factor_ls(LI,L,inst_ii);
												}
											}
											ls_num+=unroll_factor;
										}
									}
								}
								if(ls_num==1){
									true_simple_factor=1;
								}
								int array_index=array_number[array];
								int dim=array_dimension[array_index];
								 int initial=0;
								 for(int j=0; j<array_index; ++j){
									 initial += array_dimension[j];
								 }
								int size=1;
								for(int i=0;i<dim;++i){
									int j=initial+i;
									size*=array_size[j];
								}
								int partition_num=1;
								for(std::vector< std::pair<int,int> >::iterator it= array_partition[array_index].begin(); it!=array_partition[array_index].end(); ++it)
								{
									partition_num*=it->second;
								}
								//suppose only for int and float (32 bits), char:8(use LUT ram), long, double: 64
								int bank_num=(int)ceil(32*(float)size/((float)partition_num*18000));
								int BRAM_num=bank_num*partition_num*true_simple_factor;
								for(std::vector<std::pair<std::string,unsigned> >::iterator ii=function_resource[F_index].begin();ii!=function_resource[F_index].end();++ii){
									std::string resource=ii->first;
									if(resource=="BRAM"){
										ii->second+=BRAM_num;
									}
								}
							}
							else{//should be global memory...estimate generally...
								int true_simple_factor=2;
								int ls_num=0;
								for(inst_iterator II=inst_begin(F), IE=inst_end(F);II!=IE;++II){
									Instruction *inst_ii = &*II;
									if((isa<LoadInst>(inst)&&isa<LoadInst>(inst_ii))||(isa<StoreInst>(inst)&&isa<StoreInst>(inst_ii))){
										Value *array_ii=get_array_name(inst_ii);
										if(array_ii==array){
											BasicBlock *bb=inst_ii->getParent();
											int unroll_factor=1;
											if(!LI->empty()){
												Loop *L=LI->getLoopFor(bb);
												if(L!=NULL){
													unroll_factor=total_unroll_factor_ls(LI,L,inst_ii);
												}
											}
											ls_num+=unroll_factor;
										}
									}
								}
								if(ls_num==1){
									true_simple_factor=1;
								}
								for(std::vector<std::pair<std::string,unsigned> >::iterator ii=function_resource[F_index].begin();ii!=function_resource[F_index].end();++ii){
									std::string resource=ii->first;
									if(resource=="BRAM"){
										ii->second+=true_simple_factor;
									}
								}
							}
							bram_counted_array[array]=1;
						}
					}
					else{
						bram_counted_array[array]++;
					}
				}
			}
		}
		if(Function_dataflow[F]==1){//if dataflow works, BRAM becomes ping-pong memory, double
			  bool dataflow_work=check_dataflow(F,LI);
			  if(dataflow_work==true){
					for(std::vector<std::pair<std::string,unsigned> >::iterator ii=function_resource[F_index].begin();ii!=function_resource[F_index].end();++ii){
						std::string resource=ii->first;
						if(resource=="BRAM"){
							ii->second=ii->second*2;
						}
					}
			  }
		}
	}
}
