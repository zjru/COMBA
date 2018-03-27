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


#include "FindLatency.h"
#include "Constant.h"
#include "ComputeCriticalPath.h"

float find_latency_before_inst(Instruction *inst, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop)
{
	float latency=0.0;
	std::map<unsigned, float> loop_inst_till_latency;
	float cp=loop_solveCP(dependence_loop,loop_inst_till_latency);
	if(cp<0.0){
		errs()<<"Please check1...\n";
	}
	for(unsigned op=0;op!=inst->getNumOperands();++op)
	{
		Value *OP = inst->getOperand(op);
		if(Instruction *inst1 = dyn_cast<Instruction>(OP)){
			if(instr_index_loop.count(inst1)){
				unsigned index_inst1=instr_index_loop[inst1];
				latency=std::max(latency,loop_inst_till_latency[index_inst1]);
			}
		}
	}
	//for PHI and load/write instructions, we add "manual dependence" when considering relationship between blocks. Here, we just consider load due to our needs.
	if(isa<LoadInst>(inst)){
		if(instr_index_loop.count(inst)){
			unsigned inst_index=instr_index_loop[inst];
			for(std::map<Instruction*,unsigned>::iterator it=instr_index_loop.begin();it!=instr_index_loop.end();++it){
				unsigned inst_tmp_index=it->second;
				for(std::vector< std::pair<int,float> >::iterator i= dependence_loop[inst_tmp_index].begin(); i!=dependence_loop[inst_tmp_index].end(); ++i){
					if(i->first==(int)inst_index){
						float latency_tmp=loop_inst_till_latency[inst_tmp_index];
						latency=std::max(latency,latency_tmp);
					}
				}
			}
		}
	}
	return round(latency);
}


float find_latency_after_inst(Instruction *inst, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop)
{
	float latency=0.0;
	std::map<unsigned, float> loop_inst_till_latency;
	float cp=loop_solveCP(dependence_loop,loop_inst_till_latency);
	if(cp<0.0){
		errs()<<"Please check2...\n";
	}
	unsigned index_inst=instr_index_loop[inst];
	latency=loop_inst_till_latency[index_inst];
	return round(latency);
}


float find_latency(Instruction *inst_begin, Instruction *inst_end, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop)
{
	std::queue<int> index_queue[INSN_NUM];
	float latency=0.0;
	unsigned index_inst_begin=instr_index_loop[inst_begin];
	unsigned index_inst_end=instr_index_loop[inst_end];
	float *T=new float [INSN_NUM];
	for (int i=0;i<INSN_NUM;++i)
		T[i]=0.0;
	std::map<int,int> pushed_inst;
	if(!dependence_loop[index_inst_begin].empty())
	{
		for(std::vector< std::pair<int,float> >::iterator i= dependence_loop[index_inst_begin].begin(); i!=dependence_loop[index_inst_begin].end(); ++i)
		{
			if(i->first==(int)index_inst_begin)
			{
				if(isa<LoadInst>(inst_begin)){
					latency = 2.0;
				}
				else if(PHINode *phi=dyn_cast<PHINode>(inst_begin)){
					for(unsigned ii=0,ie=phi->getNumIncomingValues();ii!=ie;ii++){
						Value *incoming=phi->getIncomingValue(ii);
						if(isa<LoadInst>(incoming)){
							latency = 2.0;
							break;
						}
						else{
							latency = std::max(i->second, latency);
						}
					}
				}
				else{
					latency = std::max(i->second, latency);
				}
				T[index_inst_begin]=latency;
				continue;
			}
			else{
				if(!pushed_inst.count((int)index_inst_begin)){
					int index_tmp=i->first;
					float latency_tmp=i->second;
					T[index_tmp]=T[index_inst_begin]+latency_tmp;
					index_queue[index_inst_begin].push(index_tmp);
					pushed_inst[index_inst_begin]=1;
				}
				else{
					int index_tmp=i->first;
					float latency_tmp=i->second;
					float T_tmp=T[index_inst_begin]+latency_tmp;
					T[index_tmp]=std::max(T_tmp,T[index_tmp]);
				}
			}
		}
	}
	for(int j=index_inst_begin; j<= (int)index_inst_end; ++j)
	{
		while(!index_queue[j].empty())
		{
			int index_test=index_queue[j].front();
			index_queue[j].pop();
			for(std::vector< std::pair<int,float> >::iterator i= dependence_loop[index_test].begin(); i!=dependence_loop[index_test].end(); ++i)
			{
				float tmp=0.0;
				if(i->first==index_test){
					tmp = T[index_test] + i->second;
					T[index_test]=std::max(tmp,T[index_test]);
				}
				else{
					if(!pushed_inst.count(index_test)){
						int index_tmp=i->first;
						float latency_tmp=i->second;
						T[index_tmp]=T[index_test]+latency_tmp;
						index_queue[index_test].push(index_tmp);
					}
					else{
						int index_tmp=i->first;
						float latency_tmp=i->second;
						float T_tmp=T[index_test]+latency_tmp;
						T[index_tmp]=std::max(T_tmp,T[index_tmp]);
					}
				}
			}
		}
	}

	if(T[index_inst_end] == 0.0){
		//errs()<<"These two instruction have no dependency\n";
		latency=0.0;
	}
	else{
		latency=T[index_inst_end];
	}
	delete []T;
	return latency;
}

float get_inst_latency(Instruction *inst, std::map<Instruction*, unsigned> &instr_index_loop,std::vector<std::pair<int,float> > *dependence_loop)
{
	float latency=0.0;
	if(instr_index_loop.count(inst)){
		unsigned index=instr_index_loop[inst];
		for(std::vector<std::pair<int,float> >::iterator i=dependence_loop[index].begin();i!=dependence_loop[index].end();++i){
			if((i->first==(int)index)){
				latency=i->second;
			}
		}
	}
	return latency;
}

void set_inst_latency(Instruction *inst,float latency,std::map<Instruction*, unsigned> &instr_index_loop,std::vector<std::pair<int,float> > *dependence_loop)
{
	if(instr_index_loop.count(inst)){
		unsigned index=instr_index_loop[inst];
		for(std::vector<std::pair<int,float> >::iterator i=dependence_loop[index].begin();i!=dependence_loop[index].end();++i){
			if((i->first==(int)index)){
				i->second=latency;
			}
		}
	}
}
