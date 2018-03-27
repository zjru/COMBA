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


#include "Reschedule.h"
#include "FindLatency.h"
#include "Cast.h"
#include "GEP.h"


void reschedule_dependence(LoopInfo* LI, int freq, std::map<Instruction*, unsigned> &instr_index_loop,std::vector<std::pair<int,float> > *dependence_loop)
{
	for(std::map<Instruction*,unsigned>::iterator it=instr_index_loop.begin();it!=instr_index_loop.end();++it){
		Instruction *inst=it->first;
		if(!inst_reschedule_considered.count(inst)){
			BasicBlock *bb=inst->getParent();
			unsigned index_inst=it->second;
			if(index_inst!=0){
				for(std::vector<std::pair<int,float> >::iterator i=dependence_loop[index_inst].begin();i!=dependence_loop[index_inst].end();++i){
					if((i->first==(int)index_inst)){
						if(i->second!=0.0){
							if(isa<PHINode>(inst)){
								Loop *L;
								std::vector<Loop*> subLoops;
								if(!LI->empty()){
									L=LI->getLoopFor(bb);
									subLoops = L->getSubLoops();
								}
								if(L==NULL){
									i->second=0.0;
								}
								else if(subLoops.empty()){
									i->second=0.0;
								}
							}
							if(freq==100){
								reschedule_dependence_100(inst,it,i,instr_index_loop,dependence_loop);
							}
							else if(freq==125){
								reschedule_dependence_125(inst,it,i,instr_index_loop,dependence_loop);
							}
							else if(freq==150){
								reschedule_dependence_150(inst,it,i,instr_index_loop,dependence_loop);
							}
							else if(freq==200){
								reschedule_dependence_200(inst,it,i,instr_index_loop,dependence_loop);
							}
							else if(freq==250){
								reschedule_dependence_250(inst,it,i,instr_index_loop,dependence_loop);
							}
						}
					}
				}
			}
		}
	}
}

void reschedule_dependence_100(Instruction *inst, std::map<Instruction*,unsigned>::iterator it, std::vector<std::pair<int,float> >::iterator i, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop)
{
	if(StoreInst *store=dyn_cast<StoreInst>(inst))
	{
		bool large_latency_flag=latency_is_large(inst,instr_index_loop,dependence_loop);
		if(large_latency_flag==false){
			i->second=i->second-0.5;
			for(std::map<Instruction*,unsigned>::iterator ii=instr_index_loop.begin();ii!=instr_index_loop.end();++ii){
				Instruction *inst_tmp=ii->first;
				float op_latency=get_inst_latency(inst_tmp,instr_index_loop,dependence_loop);
				if(op_latency==(float)1.4){
					unsigned opcode=inst_tmp->getOpcode();
					if(opcode==Instruction::Mul){
						set_inst_latency(inst_tmp,1.5,instr_index_loop,dependence_loop);
					}
				}
			}
			Value *op=store->getValueOperand();
			if(Instruction *op_inst_tmp=dyn_cast<Instruction>(op)){
				Instruction *op_inst=NULL;
				if(inst_is_cast(op_inst_tmp)==true){
					op_inst=get_cast_inst(op_inst_tmp);
				}
				else{
					op_inst=op_inst_tmp;
				}
				if(op_inst!=NULL){
					float op_latency=get_inst_latency(op_inst,instr_index_loop,dependence_loop);
					if(op_latency<0.1){
						set_inst_latency(op_inst,0.0,instr_index_loop,dependence_loop);
					}
					else if(op_latency<0.5){
						op_latency=op_latency-0.1;
						set_inst_latency(op_inst,op_latency,instr_index_loop,dependence_loop);
					}
				}
			}
		}
		else{
			Value *op=store->getValueOperand();
			if(Instruction *op_inst_tmp=dyn_cast<Instruction>(op)){
				Instruction *op_inst=NULL;
				if(inst_is_cast(op_inst_tmp)==true){
					op_inst=get_cast_inst(op_inst_tmp);
				}
				else{
					op_inst=op_inst_tmp;
				}
				if(op_inst!=NULL){
					float op_latency=get_inst_latency(op_inst,instr_index_loop,dependence_loop);
					if(op_latency<0.5){
						set_inst_latency(op_inst,0.0,instr_index_loop,dependence_loop);
					}
					else if(op_latency>2.0&&op_latency<30.0){
						unsigned opcode=op_inst->getOpcode();
						if((opcode!=Instruction::FAdd)&&(opcode!=Instruction::FMul)&&(opcode!=Instruction::FDiv))
						{
							op_latency=op_latency-0.5;
							set_inst_latency(op_inst,op_latency,instr_index_loop,dependence_loop);
						}
					}
				}
			}
		}
	}
	else if(isa<LoadInst>(inst))
	{
		GetElementPtrInst *gep=get_GEP(inst);
		if(gep!=NULL){
			bool decrease_flag1=false;
			for(unsigned ii=0;ii<gep->getNumOperands();++ii){
				Value *op=gep->getOperand(ii);
				if(Instruction *op_inst_tmp=dyn_cast<Instruction>(op)){
					if(!isa<PHINode>(op_inst_tmp)){
						Instruction *op_inst=NULL;
						if(inst_is_cast(op_inst_tmp)==true){
							op_inst=get_cast_inst(op_inst_tmp);
						}
						else{
							op_inst=op_inst_tmp;
						}
						if(op_inst!=NULL){
							float op_latency=get_inst_latency(op_inst,instr_index_loop,dependence_loop);
							if(op_latency<0.5){
								set_inst_latency(op_inst,0.0,instr_index_loop,dependence_loop);
							}
							else if(isa<LoadInst>(op_inst)){
								if(decrease_flag1==false){
									i->second=i->second-0.5;
									decrease_flag1=true;
								}
							}
							for(unsigned j=0;j<op_inst->getNumOperands();++j){
								Value *opj=op_inst->getOperand(j);
								if(Instruction *opj_inst=dyn_cast<Instruction>(opj)){
									float opj_latency=get_inst_latency(opj_inst,instr_index_loop,dependence_loop);
									if(opj_latency<0.5){
										set_inst_latency(opj_inst,0.0,instr_index_loop,dependence_loop);
									}
								}
							}
						}
					}
				}
			}
		}
		bool decrease_flag=false;
		bool large_latency_flag=latency_is_large(inst,instr_index_loop,dependence_loop);
		for(std::map<Instruction*,unsigned>::iterator ii=it;ii!=instr_index_loop.end();++ii){
			Instruction *inst1_tmp=ii->first;
			if(decrease_flag==false){
				for(unsigned j=0;j<inst1_tmp->getNumOperands();++j){
					Value *op=inst1_tmp->getOperand(j);
					if(Instruction *op_inst=dyn_cast<Instruction>(op)){
						if(op_inst==inst){
							Instruction *inst1=NULL;
							if(inst_is_cast(inst1_tmp)==true){
								for(std::map<Instruction*,unsigned>::iterator ij=instr_index_loop.begin();ij!=instr_index_loop.end();++ij){
									Instruction *i_tmp=ij->first;
									if(decrease_flag==false){
										for(unsigned t=0;t<i_tmp->getNumOperands();++t){
											Value *op_tmp=i_tmp->getOperand(t);
											if(Instruction *optmp_inst=dyn_cast<Instruction>(op_tmp)){
												if(optmp_inst==inst1_tmp){
													inst1=i_tmp;
													bool break_flag=false;
													for(unsigned k=0;k<inst1->getNumOperands();++k){
														Value *op1=inst1->getOperand(k);
														if(Instruction *op_inst1=dyn_cast<Instruction>(op1)){
															float op1_latency=find_latency_after_inst(op_inst1,instr_index_loop,dependence_loop);
															float inst_latency=find_latency_after_inst(inst,instr_index_loop,dependence_loop);
															if(op1_latency>inst_latency){
																break_flag=true;
																break;
															}
														}
													}
													if(break_flag==false){
														float op_latency=get_inst_latency(inst1,instr_index_loop,dependence_loop);
														if(large_latency_flag==false){
															i->second=i->second-0.5;
															if(op_latency<0.1){
																set_inst_latency(inst1,0.0,instr_index_loop,dependence_loop);
															}
															else if(op_latency<0.5){
																op_latency=op_latency-0.1;
																set_inst_latency(inst1,op_latency,instr_index_loop,dependence_loop);
															}
															decrease_flag=true;
														}
														else{
															if(op_latency<0.5){
																set_inst_latency(inst1,0.0,instr_index_loop,dependence_loop);
															}
															if(op_latency>2.0&&op_latency<30.0){
																unsigned opcode=op_inst->getOpcode();
																if((opcode!=Instruction::FAdd)&&(opcode!=Instruction::FMul)&&(opcode!=Instruction::FDiv))
																{
																	op_latency=op_latency-0.5;
																	set_inst_latency(inst1,op_latency,instr_index_loop,dependence_loop);
																}
															}
														}
													}
													break;
												}
											}
										}
									}
								}
								break;
							}
							else{
								inst1=inst1_tmp;
								bool break_flag=false;
								for(unsigned k=0;k<inst1->getNumOperands();++k){
									Value *op1=inst1->getOperand(k);
									if(Instruction *op_inst1=dyn_cast<Instruction>(op1)){
										float op1_latency=find_latency_after_inst(op_inst1,instr_index_loop,dependence_loop);
										float inst_latency=find_latency_after_inst(inst,instr_index_loop,dependence_loop);
										if(op1_latency>inst_latency){
											break_flag=true;
											break;
										}
									}
								}
								if(break_flag==false){
									float op_latency=get_inst_latency(inst1,instr_index_loop,dependence_loop);
									if(large_latency_flag==false){
										i->second=i->second-0.5;
										if(op_latency<0.1){
											set_inst_latency(inst1,0.0,instr_index_loop,dependence_loop);
										}
										else if(op_latency<0.5){
											op_latency=op_latency-0.1;
											set_inst_latency(inst1,op_latency,instr_index_loop,dependence_loop);
										}
										decrease_flag=true;
									}
									else{
										if(op_latency<0.5){
											set_inst_latency(inst1,0.0,instr_index_loop,dependence_loop);
										}
										if(op_latency>2.0&&op_latency<30.0){
											unsigned opcode=op_inst->getOpcode();
											if((opcode!=Instruction::FAdd)&&(opcode!=Instruction::FMul)&&(opcode!=Instruction::FDiv))
											{
												op_latency=op_latency-0.5;
												set_inst_latency(inst1,op_latency,instr_index_loop,dependence_loop);
											}
										}
									}
								}
								break;
							}
						}
					}
				}
			}
		}
	}
	else
	{
		//for those>2.0, consider chaining with load, store, small operations and int_mult(-0.5)
		if(i->second>2.0&&i->second<30.0){
			for(std::map<Instruction*,unsigned>::iterator ii=instr_index_loop.begin();ii!=instr_index_loop.end();++ii){
				Instruction *inst_tmp=ii->first;
				float op_latency=get_inst_latency(inst_tmp,instr_index_loop,dependence_loop);
				if(op_latency==(float)1.4){
					unsigned opcode=inst_tmp->getOpcode();
					if(opcode==Instruction::Mul){
						set_inst_latency(inst_tmp,1.0,instr_index_loop,dependence_loop);
					}
				}
			}
			float latency=0.0;
			for(unsigned ii=0;ii<inst->getNumOperands();++ii){
				Value *op=inst->getOperand(ii);
				if(Instruction *op_inst=dyn_cast<Instruction>(op)){
					float latency_tmp=find_latency_after_inst(op_inst,instr_index_loop,dependence_loop);
					latency=std::max(latency,latency_tmp);
				}
			}
			for(unsigned ii=0;ii<inst->getNumOperands();++ii){
				Value *op=inst->getOperand(ii);
				if(Instruction *op_inst_tmp=dyn_cast<Instruction>(op)){
					float latency_tmp=find_latency_after_inst(op_inst_tmp,instr_index_loop,dependence_loop);
					if(latency_tmp==latency){
						Instruction *op_inst=NULL;
						if(inst_is_cast(op_inst_tmp)==true){
							op_inst=get_cast_inst(op_inst_tmp);
						}
						else{
							op_inst=op_inst_tmp;
						}
						if(op_inst!=NULL){
							float op_latency=get_inst_latency(op_inst,instr_index_loop,dependence_loop);
							unsigned opcode=inst->getOpcode();
							if((opcode!=Instruction::FAdd)&&(opcode!=Instruction::FMul)&&(opcode!=Instruction::FDiv))
							{
								if(isa<LoadInst>(op_inst)){
									if(num_is_int(op_latency)==true){
										float l=op_latency-0.5;
										set_inst_latency(op_inst,l,instr_index_loop,dependence_loop);
									}
								}
								else{
									if(op_latency<0.5){
										set_inst_latency(op_inst,0.0,instr_index_loop,dependence_loop);
									}
									else if(op_latency>30.0&&op_latency<36.0){
										float l=op_latency+1.0;
										set_inst_latency(op_inst,l,instr_index_loop,dependence_loop);
									}
								}
							}
							else{
								if(op_latency>30.0&&op_latency<36.0){
									float l=op_latency+1.0;
									set_inst_latency(op_inst,l,instr_index_loop,dependence_loop);
								}
							}
						}
					}
				}
			}
			for(std::map<Instruction*,unsigned>::iterator ii=it;ii!=instr_index_loop.end();++ii){
				Instruction *inst1_tmp=ii->first;
				for(unsigned j=0;j<inst1_tmp->getNumOperands();++j){
					Value *op=inst1_tmp->getOperand(j);
					if(Instruction *op_inst=dyn_cast<Instruction>(op)){
						if(op_inst==inst){
							Instruction *inst1=NULL;
							if(inst_is_cast(inst1_tmp)==true){
								for(std::map<Instruction*,unsigned>::iterator ij=instr_index_loop.begin();ij!=instr_index_loop.end();++ij){
									Instruction *i_tmp=ij->first;
									for(unsigned t=0;t<i_tmp->getNumOperands();++t){
										Value *op_tmp=i_tmp->getOperand(t);
										if(Instruction *optmp_inst=dyn_cast<Instruction>(op_tmp)){
											if(optmp_inst==inst1_tmp){
												inst1=i_tmp;
												bool break_flag=false;
												for(unsigned k=0;k<inst1->getNumOperands();++k){
													Value *op1=inst1->getOperand(k);
													if(Instruction *op_inst1=dyn_cast<Instruction>(op1)){
														float op1_latency=find_latency_after_inst(op_inst1,instr_index_loop,dependence_loop);
														float inst_latency=find_latency_after_inst(inst,instr_index_loop,dependence_loop);
														if(op1_latency>inst_latency){
															break_flag=true;
															break;
														}
													}
												}
												if(break_flag==false){
													float op_latency=get_inst_latency(inst1,instr_index_loop,dependence_loop);
													unsigned opcode=inst->getOpcode();
													if((opcode!=Instruction::FAdd)&&(opcode!=Instruction::FMul)&&(opcode!=Instruction::FDiv))
													{
														if(isa<StoreInst>(inst1)){
															if(num_is_int(op_latency)==true){
																float l=op_latency-0.5;
																set_inst_latency(inst1,l,instr_index_loop,dependence_loop);
															}
														}
														else{
															if(op_latency<0.5){
																set_inst_latency(inst1,0.0,instr_index_loop,dependence_loop);
															}
															else if(op_latency>30.0&&op_latency<36.0){
																float l=op_latency+1.0;
																set_inst_latency(inst1,l,instr_index_loop,dependence_loop);
															}
														}
													}
													else{
														if(op_latency>30.0&&op_latency<36.0){
															float l=op_latency+1.0;
															set_inst_latency(inst1,l,instr_index_loop,dependence_loop);
														}
													}
												}
												break;
											}
										}
									}
								}
								break;
							}
							else{
								inst1=inst1_tmp;
								bool break_flag=false;
								for(unsigned k=0;k<inst1->getNumOperands();++k){
									Value *op1=inst1->getOperand(k);
									if(Instruction *op_inst1=dyn_cast<Instruction>(op1)){
										float op1_latency=find_latency_after_inst(op_inst1,instr_index_loop,dependence_loop);
										float inst_latency=find_latency_after_inst(inst,instr_index_loop,dependence_loop);
										if(op1_latency>inst_latency){
											break_flag=true;
											break;
										}
									}
								}
								if(break_flag==false){
									float op_latency=get_inst_latency(inst1,instr_index_loop,dependence_loop);
									unsigned opcode=inst->getOpcode();
									if((opcode!=Instruction::FAdd)&&(opcode!=Instruction::FMul)&&(opcode!=Instruction::FDiv))
									{
										if(isa<StoreInst>(inst1)){
											if(num_is_int(op_latency)==true){
												float l=op_latency-0.5;
												set_inst_latency(inst1,l,instr_index_loop,dependence_loop);
											}
										}
										else{
											if(op_latency<0.5){
												set_inst_latency(inst1,0.0,instr_index_loop,dependence_loop);
											}
											else if(op_latency>30.0&&op_latency<36.0){
												float l=op_latency+1.0;
												set_inst_latency(inst1,l,instr_index_loop,dependence_loop);
											}
										}
									}
									else{
										if(op_latency>30.0&&op_latency<36.0){
											float l=op_latency+1.0;
											set_inst_latency(inst1,l,instr_index_loop,dependence_loop);
										}
									}
								}
								break;
							}
						}
					}
				}
			}
		}
	}
}

void reschedule_dependence_125(Instruction *inst, std::map<Instruction*,unsigned>::iterator it, std::vector<std::pair<int,float> >::iterator i, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop)
{
	if(StoreInst *store=dyn_cast<StoreInst>(inst)){
		bool large_latency_flag=latency_is_large125(inst,instr_index_loop,dependence_loop);
		Value *op=store->getValueOperand();
		if(Instruction *op_inst_tmp=dyn_cast<Instruction>(op)){
			Instruction *op_inst=NULL;
			if(inst_is_cast(op_inst_tmp)==true){
				op_inst=get_cast_inst(op_inst_tmp);
			}
			else{
				op_inst=op_inst_tmp;
			}
			if(op_inst!=NULL){
				float op_latency=get_inst_latency(op_inst,instr_index_loop,dependence_loop);
				if(large_latency_flag==false){
					i->second=i->second-0.5;
					if(op_latency<1.0){
						if(op_latency>0.5){
							op_latency=op_latency-0.5;
							set_inst_latency(op_inst,op_latency,instr_index_loop,dependence_loop);
						}
						else{
							set_inst_latency(op_inst,0.0,instr_index_loop,dependence_loop);
						}
					}
				}
				else{
					if(op_latency<1.0){
						if(op_latency>0.5){
							op_latency=op_latency-0.5;
							set_inst_latency(op_inst,op_latency,instr_index_loop,dependence_loop);
						}
						else{
							set_inst_latency(op_inst,0.0,instr_index_loop,dependence_loop);
						}
					}
					else if(op_latency>30.0){
						op_latency=op_latency-0.5;
						set_inst_latency(op_inst,op_latency,instr_index_loop,dependence_loop);
					}
					else{
						unsigned opcode=op_inst->getOpcode();
						if(opcode==Instruction::FMul){
							op_latency=op_latency-0.5;
							set_inst_latency(op_inst,op_latency,instr_index_loop,dependence_loop);
						}
					}
				}
			}
		}
	}
	else if(isa<LoadInst>(inst)){
		bool decrease_flag1=false;
		GetElementPtrInst *gep=get_GEP(inst);
		if(gep!=NULL){
			for(unsigned ii=0;ii<gep->getNumOperands();++ii){
				Value *op=gep->getOperand(ii);
				if(Instruction *op_inst_tmp=dyn_cast<Instruction>(op)){
					if(!isa<PHINode>(op_inst_tmp)){
						Instruction *op_inst=NULL;
						if(inst_is_cast(op_inst_tmp)==true){
							op_inst=get_cast_inst(op_inst_tmp);
						}
						else{
							op_inst=op_inst_tmp;
						}
						if(op_inst!=NULL){
							float op_latency=get_inst_latency(op_inst,instr_index_loop,dependence_loop);
							if(op_latency<1.0){
								if(op_latency>0.5){
									op_latency=op_latency-0.5;
									set_inst_latency(op_inst,op_latency,instr_index_loop,dependence_loop);
								}
								else{
									set_inst_latency(op_inst,0.0,instr_index_loop,dependence_loop);
								}
							}
							else if(isa<LoadInst>(op_inst)){
								if(decrease_flag1==false){
									i->second=i->second-0.5;
									decrease_flag1=true;
								}
							}
							for(unsigned j=0;j<op_inst->getNumOperands();++j){
								Value *opj=op_inst->getOperand(j);
								if(Instruction *opj_inst=dyn_cast<Instruction>(opj)){
									float opj_latency=get_inst_latency(opj_inst,instr_index_loop,dependence_loop);
									if(opj_latency<1.0){
										if(opj_latency>0.5){
											opj_latency=opj_latency-0.5;
											set_inst_latency(opj_inst,opj_latency,instr_index_loop,dependence_loop);
										}
										else{
											set_inst_latency(opj_inst,0.0,instr_index_loop,dependence_loop);
										}
									}
								}
							}
						}
					}
				}
			}
		}
		bool large_latency_flag=latency_is_large125(inst,instr_index_loop,dependence_loop);
		for(std::map<Instruction*,unsigned>::iterator ii=it;ii!=instr_index_loop.end();++ii){
			Instruction *inst1_tmp=ii->first;
			for(unsigned j=0;j<inst1_tmp->getNumOperands();++j){
				Value *op=inst1_tmp->getOperand(j);
				if(Instruction *op_inst=dyn_cast<Instruction>(op)){
					if(op_inst==inst){
						Instruction *inst1=NULL;
						if(inst_is_cast(inst1_tmp)==true){
							for(std::map<Instruction*,unsigned>::iterator ij=instr_index_loop.begin();ij!=instr_index_loop.end();++ij){
								Instruction *i_tmp=ij->first;
								for(unsigned t=0;t<i_tmp->getNumOperands();++t){
									Value *op_tmp=i_tmp->getOperand(t);
									if(Instruction *optmp_inst=dyn_cast<Instruction>(op_tmp)){
										if(optmp_inst==inst1_tmp){
											inst1=i_tmp;
											reschedule_load_freq125(large_latency_flag,inst,inst1,instr_index_loop,dependence_loop);
											break;
										}
									}
								}
							}
							break;
						}
						else{
							inst1=inst1_tmp;
							reschedule_load_freq125(large_latency_flag,inst,inst1,instr_index_loop,dependence_loop);
							break;
						}
					}
				}
			}
		}
	}
	else{
		if(i->second>2.0){
			float latency=0.0;
			for(unsigned ii=0;ii<inst->getNumOperands();++ii){
				Value *op=inst->getOperand(ii);
				if(Instruction *op_inst=dyn_cast<Instruction>(op)){
					float latency_tmp=find_latency_after_inst(op_inst,instr_index_loop,dependence_loop);
					latency=std::max(latency,latency_tmp);
				}
			}
			for(unsigned ii=0;ii<inst->getNumOperands();++ii){
				Value *op=inst->getOperand(ii);
				if(Instruction *op_inst_tmp=dyn_cast<Instruction>(op)){
					float latency_tmp=find_latency_after_inst(op_inst_tmp,instr_index_loop,dependence_loop);
					if(latency_tmp==latency){
						Instruction *op_inst=NULL;
						if(inst_is_cast(op_inst_tmp)==true){
							op_inst=get_cast_inst(op_inst_tmp);
						}
						else{
							op_inst=op_inst_tmp;
						}
						if(op_inst!=NULL){
							float op_latency=get_inst_latency(op_inst,instr_index_loop,dependence_loop);
							if(op_latency<1.0){
								if(op_latency>0.5){
									op_latency=op_latency-0.5;
									set_inst_latency(op_inst,op_latency,instr_index_loop,dependence_loop);
								}
								else{
									set_inst_latency(op_inst,0.0,instr_index_loop,dependence_loop);
								}
							}
						}
					}
				}
			}
			bool ainst_increase_flag=false;
			for(std::map<Instruction*,unsigned>::iterator ii=it;ii!=instr_index_loop.end();++ii){
				Instruction *inst1_tmp=ii->first;
				if(ainst_increase_flag==false){
					for(unsigned j=0;j<inst1_tmp->getNumOperands();++j){
						Value *op=inst1_tmp->getOperand(j);
						if(Instruction *op_inst=dyn_cast<Instruction>(op)){
							if(op_inst==inst){
								Instruction *inst1=NULL;
								if(inst_is_cast(inst1_tmp)==true){
									for(std::map<Instruction*,unsigned>::iterator ij=instr_index_loop.begin();ij!=instr_index_loop.end();++ij){
										Instruction *i_tmp=ij->first;
										if(ainst_increase_flag==false){
											for(unsigned t=0;t<i_tmp->getNumOperands();++t){
												Value *op_tmp=i_tmp->getOperand(t);
												if(Instruction *optmp_inst=dyn_cast<Instruction>(op_tmp)){
													if(optmp_inst==inst1_tmp){
														inst1=i_tmp;
														bool break_flag=false;
														for(unsigned k=0;k<inst1->getNumOperands();++k){
															Value *op1=inst1->getOperand(k);
															if(Instruction *op_inst1=dyn_cast<Instruction>(op1)){
																float op1_latency=find_latency_after_inst(op_inst1,instr_index_loop,dependence_loop);
																float inst_latency=find_latency_after_inst(inst,instr_index_loop,dependence_loop);
																if(op1_latency>inst_latency){
																	break_flag=true;
																	break;
																}
															}
														}
														if(break_flag==false){
															float op_latency=get_inst_latency(inst1,instr_index_loop,dependence_loop);
															if(op_latency>0.0&&op_latency<1.0){
																i->second=i->second+0.1;
																ainst_increase_flag=true;
															}
														}
														break;
													}
												}
											}
										}
									}
									break;
								}
								else{
									inst1=inst1_tmp;
									bool break_flag=false;
									for(unsigned k=0;k<inst1->getNumOperands();++k){
										Value *op1=inst1->getOperand(k);
										if(Instruction *op_inst1=dyn_cast<Instruction>(op1)){
											float op1_latency=find_latency_after_inst(op_inst1,instr_index_loop,dependence_loop);
											float inst_latency=find_latency_after_inst(inst,instr_index_loop,dependence_loop);
											if(op1_latency>inst_latency){
												break_flag=true;
												break;
											}
										}
									}
									if(break_flag==false){
										float op_latency=get_inst_latency(inst1,instr_index_loop,dependence_loop);
										if(op_latency>0.0&&op_latency<1.0){
											i->second=i->second+0.1;
											ainst_increase_flag=true;
										}
									}
									break;
								}
							}
						}
					}
				}
			}
		}
	}
}

void reschedule_dependence_150(Instruction *inst, std::map<Instruction*,unsigned>::iterator it, std::vector<std::pair<int,float> >::iterator i, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop)
{
	if(StoreInst *store=dyn_cast<StoreInst>(inst)){
		Value *op=store->getValueOperand();
		if(Instruction *op_inst_tmp=dyn_cast<Instruction>(op)){
			Instruction *op_inst=NULL;
			if(inst_is_cast(op_inst_tmp)==true){
				op_inst=get_cast_inst(op_inst_tmp);
			}
			else{
				op_inst=op_inst_tmp;
			}
			if(op_inst!=NULL){
				float op_latency=get_inst_latency(op_inst,instr_index_loop,dependence_loop);
				if((op_latency>0.0&&op_latency<1.0)||op_latency==(float)1.7){
					if(load_shift_store(inst,instr_index_loop,dependence_loop)){
						i->second=i->second-0.4;
					}
					else{
						if(op_latency==(float)1.7){
							op_latency=op_latency-0.2;
							set_inst_latency(op_inst,op_latency,instr_index_loop,dependence_loop);
						}
						i->second=i->second-0.05;
					}
				}
			}
		}
	}
	else if(isa<LoadInst>(inst)){
		bool decrease_flag1=false;
		GetElementPtrInst *gep=get_GEP(inst);
		if(gep!=NULL){
			for(unsigned ii=0;ii<gep->getNumOperands();++ii){
				Value *op=gep->getOperand(ii);
				if(Instruction *op_inst_tmp=dyn_cast<Instruction>(op)){
					if(!isa<PHINode>(op_inst_tmp)){
						Instruction *op_inst=NULL;
						if(inst_is_cast(op_inst_tmp)==true){
							op_inst=get_cast_inst(op_inst_tmp);
						}
						else{
							op_inst=op_inst_tmp;
						}
						if(op_inst!=NULL){
							float op_latency=get_inst_latency(op_inst,instr_index_loop,dependence_loop);
							if(op_latency<0.5){
								set_inst_latency(op_inst,0.0,instr_index_loop,dependence_loop);
							}
							else if(isa<LoadInst>(op_inst)){
								if(decrease_flag1==false){
									i->second=i->second-1.0;
									decrease_flag1=true;
								}
							}
						}
					}
				}
			}
		}
		bool load_reduce_flag=false;
		for(std::map<Instruction*,unsigned>::iterator ii=it;ii!=instr_index_loop.end();++ii){
			Instruction *inst1_tmp=ii->first;
			if(load_reduce_flag==false){
				for(unsigned j=0;j<inst1_tmp->getNumOperands();++j){
					Value *op=inst1_tmp->getOperand(j);
					if(Instruction *op_inst=dyn_cast<Instruction>(op)){
						if(op_inst==inst){
							Instruction *inst1=NULL;
							if(inst_is_cast(inst1_tmp)==true){
								for(std::map<Instruction*,unsigned>::iterator ij=instr_index_loop.begin();ij!=instr_index_loop.end();++ij){
									Instruction *i_tmp=ij->first;
									if(load_reduce_flag==false){
										for(unsigned t=0;t<i_tmp->getNumOperands();++t){
											Value *op_tmp=i_tmp->getOperand(t);
											if(Instruction *optmp_inst=dyn_cast<Instruction>(op_tmp)){
												if(optmp_inst==inst1_tmp){
													inst1=i_tmp;
													bool break_flag=false;
													for(unsigned k=0;k<inst1->getNumOperands();++k){
														Value *op1=inst1->getOperand(k);
														if(Instruction *op_inst1=dyn_cast<Instruction>(op1)){
															float op1_latency=find_latency_after_inst(op_inst1,instr_index_loop,dependence_loop);
															float inst_latency=find_latency_after_inst(inst,instr_index_loop,dependence_loop);
															if(op1_latency>inst_latency){
																break_flag=true;
																break;
															}
														}
													}
													if(break_flag==false){
														float op_latency=get_inst_latency(inst1,instr_index_loop,dependence_loop);
														if((op_latency>0.0&&op_latency<1.0)||op_latency==(float)1.7){
															if(load_shift_store(inst,instr_index_loop,dependence_loop)){
																i->second=i->second-0.4;
															}
															else{
																if(op_latency==(float)1.7){
																	op_latency=op_latency-0.2;
																	set_inst_latency(inst1,op_latency,instr_index_loop,dependence_loop);
																}
																i->second=i->second-0.05;
															}
															load_reduce_flag=true;
														}
													}
													break;
												}
											}
										}
									}
								}
								break;
							}
							else{
								inst1=inst1_tmp;
								bool break_flag=false;
								for(unsigned k=0;k<inst1->getNumOperands();++k){
									Value *op1=inst1->getOperand(k);
									if(Instruction *op_inst1=dyn_cast<Instruction>(op1)){
										float op1_latency=find_latency_after_inst(op_inst1,instr_index_loop,dependence_loop);
										float inst_latency=find_latency_after_inst(inst,instr_index_loop,dependence_loop);
										if(op1_latency>inst_latency){
											break_flag=true;
											break;
										}
									}
								}
								if(break_flag==false){
									float op_latency=get_inst_latency(inst1,instr_index_loop,dependence_loop);
									if((op_latency>0.0&&op_latency<1.0)||op_latency==(float)1.7){
										if(load_shift_store(inst,instr_index_loop,dependence_loop)){
											i->second=i->second-0.4;
										}
										else{
											if(op_latency==(float)1.7){
												op_latency=op_latency-0.2;
												set_inst_latency(inst1,op_latency,instr_index_loop,dependence_loop);
											}
											i->second=i->second-0.05;
										}
										load_reduce_flag=true;
									}
								}
								break;
							}
						}
					}
				}
			}
		}
	}
	else{
		if(i->second>2.0&&i->second<30.0){
			float latency=0.0;
			for(unsigned ii=0;ii<inst->getNumOperands();++ii){
				Value *op=inst->getOperand(ii);
				if(Instruction *op_inst=dyn_cast<Instruction>(op)){
					float latency_tmp=find_latency_after_inst(op_inst,instr_index_loop,dependence_loop);
					latency=std::max(latency,latency_tmp);
				}
			}
			bool binst_increase_flag=false;
			for(unsigned ii=0;ii<inst->getNumOperands();++ii){
				Value *op=inst->getOperand(ii);
				if(Instruction *op_inst_tmp=dyn_cast<Instruction>(op)){
					float latency_tmp=find_latency_after_inst(op_inst_tmp,instr_index_loop,dependence_loop);
					if(latency_tmp==latency){
						if(binst_increase_flag==false){
							Instruction *op_inst=NULL;
							if(inst_is_cast(op_inst_tmp)==true){
								op_inst=get_cast_inst(op_inst_tmp);
							}
							else{
								op_inst=op_inst_tmp;
							}
							if(op_inst!=NULL){
								float op_latency=get_inst_latency(op_inst,instr_index_loop,dependence_loop);
								//errs()<<op_latency<<": "<<*op_inst<<"\n";
								/*if(op_latency==(float)0.1){
									set_inst_latency(op_inst,0.0,instr_index_loop,dependence_loop);
								}*/
								if(op_latency>0.1&&op_latency<1.0){
									bool before_has_large=false;
									for(unsigned j=0;j<op_inst->getNumOperands();++j){
										Value *opj=op_inst->getOperand(j);
										if(Instruction *opj_inst=dyn_cast<Instruction>(opj)){
											float opj_latency=get_inst_latency(opj_inst,instr_index_loop,dependence_loop);
											if(opj_latency>2.0){
												before_has_large=true;
												break;
											}
										}
									}
									if(before_has_large==true){
										i->second=i->second+0.2;
										binst_increase_flag=true;
									}
								}
							}
						}
					}
				}
			}
			bool ainst_increase_flag=false;
			for(std::map<Instruction*,unsigned>::iterator ii=it;ii!=instr_index_loop.end();++ii){
				Instruction *inst1_tmp=ii->first;
				if(ainst_increase_flag==false){
					for(unsigned j=0;j<inst1_tmp->getNumOperands();++j){
						Value *op=inst1_tmp->getOperand(j);
						if(Instruction *op_inst=dyn_cast<Instruction>(op)){
							if(op_inst==inst){
								Instruction *inst1=NULL;
								if(inst_is_cast(inst1_tmp)==true){
									for(std::map<Instruction*,unsigned>::iterator ij=instr_index_loop.begin();ij!=instr_index_loop.end();++ij){
										Instruction *i_tmp=ij->first;
										if(ainst_increase_flag==false){
											for(unsigned t=0;t<i_tmp->getNumOperands();++t){
												Value *op_tmp=i_tmp->getOperand(t);
												if(Instruction *optmp_inst=dyn_cast<Instruction>(op_tmp)){
													if(optmp_inst==inst1_tmp){
														inst1=i_tmp;
														bool break_flag=false;
														for(unsigned k=0;k<inst1->getNumOperands();++k){
															Value *op1=inst1->getOperand(k);
															if(Instruction *op_inst1=dyn_cast<Instruction>(op1)){
																float op1_latency=find_latency_after_inst(op_inst1,instr_index_loop,dependence_loop);
																float inst_latency=find_latency_after_inst(inst,instr_index_loop,dependence_loop);
																if(op1_latency>inst_latency){
																	break_flag=true;
																	break;
																}
															}
														}
														if(break_flag==false){
															float op_latency=get_inst_latency(inst1,instr_index_loop,dependence_loop);
															if(op_latency>0.1&&op_latency<1.0){
																i->second=i->second+0.2;
																ainst_increase_flag=true;
															}
														}
														break;
													}
												}
											}
										}
									}
									break;
								}
								else{
									inst1=inst1_tmp;
									bool break_flag=false;
									for(unsigned k=0;k<inst1->getNumOperands();++k){
										Value *op1=inst1->getOperand(k);
										if(Instruction *op_inst1=dyn_cast<Instruction>(op1)){
											float op1_latency=find_latency_after_inst(op_inst1,instr_index_loop,dependence_loop);
											float inst_latency=find_latency_after_inst(inst,instr_index_loop,dependence_loop);
											if(op1_latency>inst_latency){
												break_flag=true;
												break;
											}
										}
									}
									if(break_flag==false){
										float op_latency=get_inst_latency(inst1,instr_index_loop,dependence_loop);
										if(op_latency>0.1&&op_latency<1.0){
											i->second=i->second+0.2;
											ainst_increase_flag=true;
										}
									}
									break;
								}
							}
						}
					}
				}
			}
		}
	}
}

void reschedule_dependence_200(Instruction *inst, std::map<Instruction*,unsigned>::iterator it, std::vector<std::pair<int,float> >::iterator i, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop)
{
	if(GetElementPtrInst *gep=dyn_cast<GetElementPtrInst>(inst)){
		unsigned num_indice=gep->getNumIndices();
		if(num_indice<2){
			i->second=0.0;
		}
		else if(num_indice==2){
			Value *op=gep->getOperand(1);
			if(isa<ConstantInt>(op)){
				i->second=0.0;
			}
			else{
				 for(User::op_iterator ii=gep->idx_begin(), ie=gep->idx_end();ii!=ie; ++ii){
					 Value *op=*ii;
					 if(Instruction *op_inst=dyn_cast<Instruction>(op)){
						 if(isa<PHINode>(op_inst)){
							 i->second=0.0;
						 }
						 else{
							 i->second=1.0;
							 break;
						 }
					 }
				 }
				 if(i->second==(float)0.0){
					 Value *pointer=gep->getPointerOperand();
					 for(std::map<Instruction*,unsigned>::iterator ii=instr_index_loop.begin();ii!=it;++ii){
						 Instruction *inst_tmp=ii->first;
						 if(GetElementPtrInst *gep_tmp=dyn_cast<GetElementPtrInst>(inst_tmp)){
							 Value *pointer_tmp=gep_tmp->getPointerOperand();
							 if(pointer==pointer_tmp){
								 float latency_tmp=get_inst_latency(inst_tmp,instr_index_loop,dependence_loop);
								 if(latency_tmp==(float)1.0){
									 i->second=1.0;
									 break;
								 }
							 }
						 }
					 }
				 }
			}
		}
		else if(num_indice>2){
			 for(User::op_iterator ii=gep->idx_begin(), ie=gep->idx_end();ii!=ie; ++ii){
				 Value *op=*ii;
				 if(Instruction *op_inst=dyn_cast<Instruction>(op)){
					 if(isa<PHINode>(op_inst)){
						 i->second=0.0;
					 }
					 else{
						 i->second=1.0;
						 break;
					 }
				 }
			 }
			 if(i->second==(float)0.0){
				 Value *pointer=gep->getPointerOperand();
				 for(std::map<Instruction*,unsigned>::iterator ii=instr_index_loop.begin();ii!=it;++ii){
					 Instruction *inst_tmp=ii->first;
					 if(GetElementPtrInst *gep_tmp=dyn_cast<GetElementPtrInst>(inst_tmp)){
						 Value *pointer_tmp=gep_tmp->getPointerOperand();
						 if(pointer==pointer_tmp){
							 float latency_tmp=get_inst_latency(inst_tmp,instr_index_loop,dependence_loop);
							 if(latency_tmp==(float)1.0){
								 i->second=1.0;
								 break;
							 }
						 }
					 }
				 }
			 }
		}
	}
	else if(StoreInst *store=dyn_cast<StoreInst>(inst)){
		Value *op=store->getValueOperand();
		if(Instruction *op_inst_tmp=dyn_cast<Instruction>(op)){
			Instruction *op_inst=NULL;
			if(inst_is_cast(op_inst_tmp)==true){
				op_inst=get_cast_inst(op_inst_tmp);
			}
			else{
				op_inst=op_inst_tmp;
			}
			if(op_inst!=NULL){
				float op_latency=get_inst_latency(op_inst,instr_index_loop,dependence_loop);
				if(op_latency<0.5){
					i->second=i->second-op_latency;
				}
				else if(op_latency<=(float)1.0){
					i->second=i->second-0.5;
				}
				else if(op_latency>1.0&&op_latency<30.0&&!isa<LoadInst>(op_inst)){
					unsigned opcode=op_inst->getOpcode();
					if((opcode!=Instruction::FAdd)&&(opcode!=Instruction::FMul)&&(opcode!=Instruction::FDiv)&&(opcode!=Instruction::SIToFP)&&(opcode!=Instruction::UIToFP))
					{
						if(opcode==Instruction::SDiv||opcode==Instruction::SRem||opcode==Instruction::UDiv||opcode==Instruction::URem){
							if(op_latency>10.0&&op_latency<5.0){
								i->second=i->second-1.0;
							}
						}
						else{
							i->second=i->second-1.0;
						}
					}
				}
			}
		}
	}
	else if(isa<LoadInst>(inst)){
		GetElementPtrInst *gep=get_GEP(inst);
		if(gep!=NULL){
			for(unsigned ii=0;ii<gep->getNumOperands();++ii){
				Value *op=gep->getOperand(ii);
				if(Instruction *op_inst_tmp=dyn_cast<Instruction>(op)){
					if(!isa<PHINode>(op_inst_tmp)){
						Instruction *op_inst=NULL;
						if(inst_is_cast(op_inst_tmp)==true){
							op_inst=get_cast_inst(op_inst_tmp);
						}
						else{
							op_inst=op_inst_tmp;
						}
						if(op_inst!=NULL){
							float op_latency=get_inst_latency(op_inst,instr_index_loop,dependence_loop);
							if(op_latency<0.5){
								set_inst_latency(op_inst,0.0,instr_index_loop,dependence_loop);
							}
							else if(op_latency<30.0&&!isa<LoadInst>(op_inst)){
								float l=op_latency-0.5;
								set_inst_latency(op_inst,l,instr_index_loop,dependence_loop);
							}
						}
					}
				}
			}
		}
		bool load_reduce_flag=false;
		for(std::map<Instruction*,unsigned>::iterator ii=it;ii!=instr_index_loop.end();++ii)
		{
			Instruction *inst1_tmp=ii->first;
			if(load_reduce_flag==false){
				for(unsigned j=0;j<inst1_tmp->getNumOperands();++j){
					Value *op=inst1_tmp->getOperand(j);
					if(Instruction *op_inst=dyn_cast<Instruction>(op)){
						if(op_inst==inst){
							Instruction *inst1=NULL;
							if(inst_is_cast(inst1_tmp)==true){
								for(std::map<Instruction*,unsigned>::iterator ij=instr_index_loop.begin();ij!=instr_index_loop.end();++ij){
									Instruction *i_tmp=ij->first;
									if(load_reduce_flag==false){
										for(unsigned t=0;t<i_tmp->getNumOperands();++t){
											Value *op_tmp=i_tmp->getOperand(t);
											if(Instruction *optmp_inst=dyn_cast<Instruction>(op_tmp)){
												if(optmp_inst==inst1_tmp){
													inst1=i_tmp;
													bool break_flag=false;
													for(unsigned k=0;k<inst1->getNumOperands();++k){
														Value *op1=inst1->getOperand(k);
														if(Instruction *op_inst1=dyn_cast<Instruction>(op1)){
															float op1_latency=find_latency_after_inst(op_inst1,instr_index_loop,dependence_loop);
															float inst_latency=find_latency_after_inst(inst,instr_index_loop,dependence_loop);
															if(op1_latency>inst_latency){
																break_flag=true;
																break;
															}
														}
													}
													if(break_flag==false){
														float op_latency=get_inst_latency(inst1,instr_index_loop,dependence_loop);
														if(op_latency<0.5){
															set_inst_latency(inst1,0.0,instr_index_loop,dependence_loop);
														}
														unsigned opcode=inst1->getOpcode();
														if((op_latency>1.0)&&(op_latency<30.0)){
															if((opcode!=Instruction::FAdd)&&(opcode!=Instruction::FMul)&&(opcode!=Instruction::FDiv)&&(opcode!=Instruction::SIToFP)&&(opcode!=Instruction::UIToFP))
															{
																i->second=i->second-1.0;
																load_reduce_flag=true;
															}
														}
													}
													break;
												}
											}
										}
									}
								}
								break;
							}
							else{
								inst1=inst1_tmp;
								bool break_flag=false;
								for(unsigned k=0;k<inst1->getNumOperands();++k){
									Value *op1=inst1->getOperand(k);
									if(Instruction *op_inst1=dyn_cast<Instruction>(op1)){
										float op1_latency=find_latency_after_inst(op_inst1,instr_index_loop,dependence_loop);
										float inst_latency=find_latency_after_inst(inst,instr_index_loop,dependence_loop);
										if(op1_latency>inst_latency){
											break_flag=true;
											break;
										}
									}
								}
								if(break_flag==false){
									float op_latency=get_inst_latency(inst1,instr_index_loop,dependence_loop);
									if(op_latency<0.5){
										set_inst_latency(inst1,0.0,instr_index_loop,dependence_loop);
									}
									unsigned opcode=inst1->getOpcode();
									if((op_latency>1.0)&&(op_latency<30.0)){
										if((opcode!=Instruction::FAdd)&&(opcode!=Instruction::FMul)&&(opcode!=Instruction::FDiv)&&(opcode!=Instruction::SIToFP)&&(opcode!=Instruction::UIToFP))
										{
											i->second=i->second-1.0;
											load_reduce_flag=true;
										}
									}
								}
								break;
							}
						}
					}
				}
			}
		}
	}
	else{
		unsigned opcode=inst->getOpcode();
		bool not_chain=false;
		if(i->second>2.0&&i->second<30.0){
			if(opcode==Instruction::SDiv||opcode==Instruction::SRem||opcode==Instruction::UDiv||opcode==Instruction::URem){
				if(i->second>10.0&&i->second<5.0){
					not_chain=true;
				}
			}
			if(not_chain==false){
				if((opcode!=Instruction::FAdd)&&(opcode!=Instruction::FMul)&&(opcode!=Instruction::FDiv)&&(opcode!=Instruction::SIToFP)&&(opcode!=Instruction::UIToFP)){
					float latency=0.0;
					for(unsigned ii=0;ii<inst->getNumOperands();++ii){
						Value *op=inst->getOperand(ii);
						if(Instruction *op_inst=dyn_cast<Instruction>(op)){
							float latency_tmp=find_latency_after_inst(op_inst,instr_index_loop,dependence_loop);
							latency=std::max(latency,latency_tmp);
						}
					}
					for(unsigned ii=0;ii<inst->getNumOperands();++ii){
						Value *op=inst->getOperand(ii);
						if(Instruction *op_inst_tmp=dyn_cast<Instruction>(op)){
							float latency_tmp=find_latency_after_inst(op_inst_tmp,instr_index_loop,dependence_loop);
							if(latency_tmp==latency){
								Instruction *op_inst=NULL;
								if(inst_is_cast(op_inst_tmp)==true){
									op_inst=get_cast_inst(op_inst_tmp);
								}
								else{
									op_inst=op_inst_tmp;
								}
								if(op_inst!=NULL){
									if(!isa<LoadInst>(op_inst)&&!isa<StoreInst>(op_inst)){
										float op_latency=get_inst_latency(op_inst,instr_index_loop,dependence_loop);
										if(op_latency<0.5){
											set_inst_latency(op_inst,0.0,instr_index_loop,dependence_loop);
										}
										else if(op_latency<30.0){
											op_latency=op_latency-0.5;
											set_inst_latency(op_inst,op_latency,instr_index_loop,dependence_loop);
										}
									}
								}
							}
						}
					}
					for(std::map<Instruction*,unsigned>::iterator ii=it;ii!=instr_index_loop.end();++ii)
					{
						Instruction *inst1_tmp=ii->first;
						for(unsigned j=0;j<inst1_tmp->getNumOperands();++j){
							Value *op=inst1_tmp->getOperand(j);
							if(Instruction *op_inst=dyn_cast<Instruction>(op)){
								if(op_inst==inst){
									Instruction *inst1=NULL;
									if(inst_is_cast(inst1_tmp)==true){
										for(std::map<Instruction*,unsigned>::iterator ij=instr_index_loop.begin();ij!=instr_index_loop.end();++ij){
											Instruction *i_tmp=ij->first;
											for(unsigned t=0;t<i_tmp->getNumOperands();++t){
												Value *op_tmp=i_tmp->getOperand(t);
												if(Instruction *optmp_inst=dyn_cast<Instruction>(op_tmp)){
													if(optmp_inst==inst1_tmp){
														inst1=i_tmp;
														bool break_flag=false;
														for(unsigned k=0;k<inst1->getNumOperands();++k){
															Value *op1=inst1->getOperand(k);
															if(Instruction *op_inst1=dyn_cast<Instruction>(op1)){
																float op1_latency=find_latency_after_inst(op_inst1,instr_index_loop,dependence_loop);
																float inst_latency=find_latency_after_inst(inst,instr_index_loop,dependence_loop);
																if(op1_latency>inst_latency){
																	break_flag=true;
																	break;
																}
															}
														}
														if(break_flag==false){
															if(!isa<LoadInst>(inst1)&&!isa<StoreInst>(inst1)){
																float op_latency=get_inst_latency(inst1,instr_index_loop,dependence_loop);
																if(op_latency<0.5){
																	set_inst_latency(inst1,0.0,instr_index_loop,dependence_loop);
																}
																else if(op_latency<30.0){
																	op_latency=op_latency-0.5;
																	set_inst_latency(inst1,op_latency,instr_index_loop,dependence_loop);
																}
															}
														}
														break;
													}
												}
											}
										}
										break;
									}
									else{
										inst1=inst1_tmp;
										bool break_flag=false;
										for(unsigned k=0;k<inst1->getNumOperands();++k){
											Value *op1=inst1->getOperand(k);
											if(Instruction *op_inst1=dyn_cast<Instruction>(op1)){
												float op1_latency=find_latency_after_inst(op_inst1,instr_index_loop,dependence_loop);
												float inst_latency=find_latency_after_inst(inst,instr_index_loop,dependence_loop);
												if(op1_latency>inst_latency){
													break_flag=true;
													break;
												}
											}
										}
										if(break_flag==false){
											if(!isa<LoadInst>(inst1)&&!isa<StoreInst>(inst1)){
												float op_latency=get_inst_latency(inst1,instr_index_loop,dependence_loop);
												if(op_latency<0.5){
													set_inst_latency(inst1,0.0,instr_index_loop,dependence_loop);
												}
												else if(op_latency<30.0){
													op_latency=op_latency-0.5;
													set_inst_latency(inst1,op_latency,instr_index_loop,dependence_loop);
												}
											}
										}
										break;
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

void reschedule_dependence_250(Instruction *inst, std::map<Instruction*,unsigned>::iterator it, std::vector<std::pair<int,float> >::iterator i, std::map<Instruction*, unsigned> &instr_index_loop, std::vector<std::pair<int,float> > *dependence_loop)
{
	if(GetElementPtrInst *gep=dyn_cast<GetElementPtrInst>(inst)){
		unsigned num_indice=gep->getNumIndices();
		Value *op=gep->getOperand(1);
		if(num_indice<2){
			i->second = 0.0;
			if(Instruction *op_inst=dyn_cast<Instruction>(op)){
				if(!isa<PHINode>(op_inst)){
					int constant_num=0;
					for(unsigned ii=0;ii<op_inst->getNumOperands();++ii){
						Value *op1=op_inst->getOperand(ii);
						if(isa<Constant>(op1)){
							constant_num++;
						}
					}
					if(constant_num==0){
						i->second=1.0;
					}
				}
			}
			 if(i->second==(float)0.0){
				 Value *pointer=gep->getPointerOperand();
				 for(std::map<Instruction*,unsigned>::iterator ii=instr_index_loop.begin();ii!=it;++ii){
					 Instruction *inst_tmp=ii->first;
					 if(GetElementPtrInst *gep_tmp=dyn_cast<GetElementPtrInst>(inst_tmp)){
						 Value *pointer_tmp=gep_tmp->getPointerOperand();
						 if(pointer==pointer_tmp){
							 float latency_tmp=get_inst_latency(inst_tmp,instr_index_loop,dependence_loop);
							 if(latency_tmp==(float)1.0){
								 i->second=1.0;
								 break;
							 }
						 }
					 }
				 }
			 }
		}
		else if(num_indice==2){
			if(isa<Constant>(op)){
				i->second = 0.0;
				Value *op_nc=gep->getOperand(2);
				if(Instruction *op_inst=dyn_cast<Instruction>(op_nc)){
					if(!isa<PHINode>(op_inst)){
						int constant_num=0;
						for(unsigned ii=0;ii<op_inst->getNumOperands();++ii){
							Value *op1=op_inst->getOperand(ii);
							if(isa<Constant>(op1)){
								constant_num++;
							}
						}
						if(constant_num==0){
							i->second=1.0;
						}
					}
				}
				 if(i->second==(float)0.0){
					 Value *pointer=gep->getPointerOperand();
					 for(std::map<Instruction*,unsigned>::iterator ii=instr_index_loop.begin();ii!=it;++ii){
						 Instruction *inst_tmp=ii->first;
						 if(GetElementPtrInst *gep_tmp=dyn_cast<GetElementPtrInst>(inst_tmp)){
							 Value *pointer_tmp=gep_tmp->getPointerOperand();
							 if(pointer==pointer_tmp){
								 float latency_tmp=get_inst_latency(inst_tmp,instr_index_loop,dependence_loop);
								 if(latency_tmp==(float)1.0){
									 i->second=1.0;
									 break;
								 }
							 }
						 }
					 }
				 }
			}
		}
	}
	else if(isa<LoadInst>(inst)){
		GetElementPtrInst *gep=get_GEP(inst);
		if(gep!=NULL){
			for(unsigned ii=0;ii<gep->getNumOperands();++ii){
				Value *op=gep->getOperand(ii);
				if(Instruction *op_inst_tmp=dyn_cast<Instruction>(op)){
					if(!isa<PHINode>(op_inst_tmp)){
						Instruction *op_inst=NULL;
						if(inst_is_cast(op_inst_tmp)==true){
							op_inst=get_cast_inst(op_inst_tmp);
						}
						else{
							op_inst=op_inst_tmp;
						}
						if(op_inst!=NULL){
							float op_latency=get_inst_latency(op_inst,instr_index_loop,dependence_loop);
							if(op_latency<1.0){
								set_inst_latency(op_inst,0.0,instr_index_loop,dependence_loop);
							}
						}
					}
				}
			}
		}
	}
	else if(!isa<StoreInst>(inst)){
		float latency_tmp=i->second-0.1;
		if(num_is_int(latency_tmp)==true){
			int num=1;
			for(unsigned ii=0;ii<inst->getNumOperands();++ii){
				Value *op=inst->getOperand(ii);
				if(Instruction *op_inst_tmp=dyn_cast<Instruction>(op)){
					Instruction *op_inst=NULL;
					if(inst_is_cast(op_inst_tmp)==true){
						op_inst=get_cast_inst(op_inst_tmp);
					}
					else{
						op_inst=op_inst_tmp;
					}
					if(op_inst!=NULL){
						float op_latency=get_inst_latency(op_inst,instr_index_loop,dependence_loop);
						if((op_latency<1.0)&&(op_latency>0.0)){
							if(num>0){
								op_latency+=0.1;
								set_inst_latency(op_inst,op_latency,instr_index_loop,dependence_loop);
								num--;
							}
						}
					}
				}
			}
			int num1=1;
			for(std::map<Instruction*,unsigned>::iterator ii=it;ii!=instr_index_loop.end();++ii){
				Instruction *inst1_tmp=ii->first;
				for(unsigned j=0;j<inst1_tmp->getNumOperands();++j){
					Value *op=inst1_tmp->getOperand(j);
					if(Instruction *op_inst=dyn_cast<Instruction>(op)){
						if(op_inst==inst){
							Instruction *inst1=NULL;
							if(inst_is_cast(inst1_tmp)==true){
								for(std::map<Instruction*,unsigned>::iterator ij=instr_index_loop.begin();ij!=instr_index_loop.end();++ij){
									Instruction *i_tmp=ij->first;
									for(unsigned t=0;t<i_tmp->getNumOperands();++t){
										Value *op_tmp=i_tmp->getOperand(t);
										if(Instruction *optmp_inst=dyn_cast<Instruction>(op_tmp)){
											if(optmp_inst==inst1_tmp){
												inst1=i_tmp;
												float op_latency=get_inst_latency(inst1,instr_index_loop,dependence_loop);
												if((op_latency<1.0)&&(op_latency>0.0)){
													if(num1>0){
														op_latency+=0.1;
														set_inst_latency(inst1,op_latency,instr_index_loop,dependence_loop);
														num1--;
													}
												}
											}
										}
									}
								}
								break;
							}
							else{
								inst1=inst1_tmp;
								float op_latency=get_inst_latency(inst1,instr_index_loop,dependence_loop);
								if((op_latency<1.0)&&(op_latency>0.0)){
									if(num1>0){
										op_latency+=0.1;
										set_inst_latency(inst1,op_latency,instr_index_loop,dependence_loop);
										num1--;
									}
								}
							}
						}
					}
				}
			}
		}
		else{
			if(i->second>=(float)2.0){
				float latency=0.0;
				for(unsigned ii=0;ii<inst->getNumOperands();++ii){
					Value *op=inst->getOperand(ii);
					if(Instruction *op_inst=dyn_cast<Instruction>(op)){
						float latency_tmp=find_latency_after_inst(op_inst,instr_index_loop,dependence_loop);
						latency=std::max(latency,latency_tmp);
					}
				}
				bool binst_decrease_flag=false;
				unsigned opcode=inst->getOpcode();

				if((opcode==Instruction::SDiv)&&(opcode==Instruction::SRem)){
					for(unsigned ii=0;ii<inst->getNumOperands();++ii){
						Value *op=inst->getOperand(ii);
						if(Instruction *op_inst_tmp=dyn_cast<Instruction>(op)){
							float latency_tmp=find_latency_after_inst(op_inst_tmp,instr_index_loop,dependence_loop);
							if(latency_tmp==latency){
								if(binst_decrease_flag==false){
									Instruction *op_inst=NULL;
									if(inst_is_cast(op_inst_tmp)==true){
										op_inst=get_cast_inst(op_inst_tmp);
									}
									else{
										op_inst=op_inst_tmp;
									}
									if(op_inst!=NULL){
										float op_latency=get_inst_latency(op_inst,instr_index_loop,dependence_loop);
										if(op_latency<0.5){
											i->second=i->second-op_latency;
											binst_decrease_flag=true;
										}
										else if((op_latency<1.0)&&(op_latency>0.0)){
											i->second=i->second-0.5;
											binst_decrease_flag=true;
										}
									}
								}
							}
						}
					}
				}
				else{
					for(unsigned ii=0;ii<inst->getNumOperands();++ii){
						Value *op=inst->getOperand(ii);
						if(Instruction *op_inst_tmp=dyn_cast<Instruction>(op)){
							float latency_tmp=find_latency_after_inst(op_inst_tmp,instr_index_loop,dependence_loop);
							if(latency_tmp==latency){
								if(binst_decrease_flag==false){
									Instruction *op_inst=NULL;
									if(inst_is_cast(op_inst_tmp)==true){
										op_inst=get_cast_inst(op_inst_tmp);
									}
									else{
										op_inst=op_inst_tmp;
									}
									if(op_inst!=NULL){
										float op_latency=get_inst_latency(op_inst,instr_index_loop,dependence_loop);
										if(op_latency<0.5){
											i->second=i->second-op_latency;
											binst_decrease_flag=true;
										}
										else if((op_latency<1.0)&&(op_latency>0.0)){
											i->second=i->second-0.5;
											binst_decrease_flag=true;
										}
									}
								}
							}
						}
					}
					bool ainst_decrease_flag=false;
					for(std::map<Instruction*,unsigned>::iterator ii=it;ii!=instr_index_loop.end();++ii){
						Instruction *inst1_tmp=ii->first;
						if(ainst_decrease_flag==false){
							for(unsigned j=0;j<inst1_tmp->getNumOperands();++j){
								Value *op=inst1_tmp->getOperand(j);
								if(Instruction *op_inst=dyn_cast<Instruction>(op)){
									if(op_inst==inst){
										Instruction *inst1=NULL;
										if(inst_is_cast(inst1_tmp)==true){
											for(std::map<Instruction*,unsigned>::iterator ij=instr_index_loop.begin();ij!=instr_index_loop.end();++ij){
												Instruction *i_tmp=ij->first;
												if(ainst_decrease_flag==false){
													for(unsigned t=0;t<i_tmp->getNumOperands();++t){
														Value *op_tmp=i_tmp->getOperand(t);
														if(Instruction *optmp_inst=dyn_cast<Instruction>(op_tmp)){
															if(optmp_inst==inst1_tmp){
																inst1=i_tmp;
																bool break_flag=false;
																for(unsigned k=0;k<inst1->getNumOperands();++k){
																	Value *op1=inst1->getOperand(k);
																	if(Instruction *op_inst1=dyn_cast<Instruction>(op1)){
																		float op1_latency=find_latency_after_inst(op_inst1,instr_index_loop,dependence_loop);
																		float inst_latency=find_latency_after_inst(inst,instr_index_loop,dependence_loop);
																		if(op1_latency>inst_latency){
																			break_flag=true;
																			break;
																		}
																	}
																}
																if(break_flag==false){
																	float op_latency=get_inst_latency(inst1,instr_index_loop,dependence_loop);
																	if(op_latency<0.5){
																		i->second=i->second-op_latency;
																		ainst_decrease_flag=true;
																	}
																	else if((op_latency<1.0)&&(op_latency>0.0)){
																		i->second=i->second-0.5;
																		ainst_decrease_flag=true;
																	}
																}
																break;
															}
														}
													}
												}
											}
											break;
										}
										else{
											inst1=inst1_tmp;
											bool break_flag=false;
											for(unsigned k=0;k<inst1->getNumOperands();++k){
												Value *op1=inst1->getOperand(k);
												if(Instruction *op_inst1=dyn_cast<Instruction>(op1)){
													float op1_latency=find_latency_after_inst(op_inst1,instr_index_loop,dependence_loop);
													float inst_latency=find_latency_after_inst(inst,instr_index_loop,dependence_loop);
													if(op1_latency>inst_latency){
														break_flag=true;
														break;
													}
												}
											}
											if(break_flag==false){
												float op_latency=get_inst_latency(inst1,instr_index_loop,dependence_loop);
												if(op_latency<0.5){
													i->second=i->second-op_latency;
													ainst_decrease_flag=true;
												}
												else if((op_latency<1.0)&&(op_latency>0.0)){
													i->second=i->second-0.5;
													ainst_decrease_flag=true;
												}
											}
											break;
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

bool num_is_int(float number)
{
	if(number==(float)0.0){
		return true;
	}
	else if(number>=(float)1.0){
		float num=number-1.0;
		return num_is_int(num);
	}
	else{
		return false;
	}
}


bool latency_is_large(Instruction *inst,std::map<Instruction*, unsigned> &instr_index_loop,std::vector<std::pair<int,float> > *dependence_loop)
{
	bool large_latency_flag=false;
	if(isa<LoadInst>(inst)){
		float latency_from_load=0.0;
		for(std::map<Instruction*,unsigned>::iterator ii=instr_index_loop.begin();ii!=instr_index_loop.end();++ii){
			Instruction *inst_tmp=ii->first;
			float latency_from_load_tmp=find_latency(inst,inst_tmp,instr_index_loop,dependence_loop);
			if(latency_from_load_tmp!=0){
				latency_from_load=std::max(latency_from_load,latency_from_load_tmp);
			}
		}
		if(latency_from_load>5.0){
			large_latency_flag=true;
		}
	}
	else if(isa<StoreInst>(inst)){
		float latency_till_inst=find_latency_after_inst(inst,instr_index_loop,dependence_loop);
		if(latency_till_inst>5.0){
			large_latency_flag=true;
		}
	}
	return large_latency_flag;
}


bool latency_is_large125(Instruction *inst,std::map<Instruction*, unsigned> &instr_index_loop,std::vector<std::pair<int,float> > *dependence_loop)
{
	bool large_latency_flag=false;
	if(isa<LoadInst>(inst)){
		float latency_from_load=0.0;
		for(std::map<Instruction*,unsigned>::iterator ii=instr_index_loop.begin();ii!=instr_index_loop.end();++ii){
			Instruction *inst_tmp=ii->first;
			float latency_from_load_tmp=find_latency(inst,inst_tmp,instr_index_loop,dependence_loop);//+load_latency-2.0;
			if(latency_from_load_tmp!=0){
				latency_from_load=std::max(latency_from_load,latency_from_load_tmp);
			}
		}
		if(latency_from_load>=(float)6.0){
			large_latency_flag=true;
		}
	}
	else if(isa<StoreInst>(inst)){
		float latency_till_inst=find_latency_after_inst(inst,instr_index_loop,dependence_loop);
		if(latency_till_inst>=(float)6.0){
			large_latency_flag=true;
		}
	}
	return large_latency_flag;
}


void reschedule_load_freq125(bool large_latency_flag,Instruction *inst,Instruction *inst1,std::map<Instruction*, unsigned> &instr_index_loop,std::vector<std::pair<int,float> > *dependence_loop)
{
	bool break_flag=false;
	for(unsigned k=0;k<inst1->getNumOperands();++k){
		Value *op=inst1->getOperand(k);
		if(Instruction *op_inst=dyn_cast<Instruction>(op)){
			float op_latency=find_latency_after_inst(op_inst,instr_index_loop,dependence_loop);
			float inst_latency=find_latency_after_inst(inst,instr_index_loop,dependence_loop);
			if(op_latency>inst_latency){
				break_flag=true;
				break;
			}
		}
	}
	if(break_flag==false){
		float op_latency=get_inst_latency(inst1,instr_index_loop,dependence_loop);
		if(large_latency_flag==false){
			float load_latency=get_inst_latency(inst,instr_index_loop,dependence_loop);
			if(num_is_int(load_latency)){
				float l=load_latency-0.5;
				set_inst_latency(inst,l,instr_index_loop,dependence_loop);
			}
			if(op_latency<1.0){
				if(op_latency>0.5){
					op_latency=op_latency-0.5;
					set_inst_latency(inst1,op_latency,instr_index_loop,dependence_loop);
				}
				else{
					set_inst_latency(inst1,0.0,instr_index_loop,dependence_loop);
				}
			}
		}
		else{
			if(op_latency<1.0){
				if(op_latency>0.5){
					op_latency=op_latency-0.5;
					set_inst_latency(inst1,op_latency,instr_index_loop,dependence_loop);
				}
				else{
					set_inst_latency(inst1,0.0,instr_index_loop,dependence_loop);
				}
			}
			else if(op_latency>30.0){
				op_latency=op_latency-0.5;
				set_inst_latency(inst1,op_latency,instr_index_loop,dependence_loop);
			}
			else{
				unsigned opcode=inst1->getOpcode();
				if(opcode==Instruction::FMul){
					op_latency=op_latency-0.5;
					set_inst_latency(inst1,op_latency,instr_index_loop,dependence_loop);
				}
			}
		}
	}
}


bool load_shift_store(Instruction*inst,std::map<Instruction*, unsigned> &instr_index_loop,std::vector<std::pair<int,float> > *dependence_loop)
{
	if(isa<LoadInst>(inst)){
		float latency_from_load=0.0;
		float load_latency=Load_latency[inst];
		for(std::map<Instruction*,unsigned>::iterator ii=instr_index_loop.begin();ii!=instr_index_loop.end();++ii){
			Instruction *inst_tmp=ii->first;
			if(isa<StoreInst>(inst_tmp)){
				float latency_from_load_tmp=find_latency(inst,inst_tmp,instr_index_loop,dependence_loop)+load_latency-2.0;
				if(latency_from_load_tmp!=0){
					latency_from_load=std::max(latency_from_load,latency_from_load_tmp);
				}
			}
		}
		if(latency_from_load<=3.1){
			return true;
		}
		else{
			return false;
		}
	}
	else if(isa<StoreInst>(inst)){
		float latency_till_inst=find_latency_after_inst(inst,instr_index_loop,dependence_loop);
		if(latency_till_inst<=3.1){
			return true;
		}
		else{
			return false;
		}
	}
	else{
		return false;
	}
}


bool before_inst_large(Instruction *inst,std::map<Instruction*, unsigned> &instr_index_loop,std::vector<std::pair<int,float> > *dependence_loop)
{
	bool before_large=false;
	for(unsigned j=0;j<inst->getNumOperands();++j){
		Value *op=inst->getOperand(j);
		if(Instruction *op_inst=dyn_cast<Instruction>(op)){
			float latency_tmp=get_inst_latency(op_inst,instr_index_loop,dependence_loop);
			unsigned opcode=op_inst->getOpcode();
			if(latency_tmp>=2.0&&latency_tmp<30.0){
				if((opcode!=Instruction::FAdd)&&(opcode!=Instruction::FMul)&&(opcode!=Instruction::FDiv)&&(opcode!=Instruction::SIToFP)&&(opcode!=Instruction::UIToFP)){
					before_large=true;
				}
			}
		}
	}
	return before_large;
}


bool after_inst_large(Instruction *inst,std::map<Instruction*, unsigned> &instr_index_loop,std::vector<std::pair<int,float> > *dependence_loop)
{
	bool after_large=false;
	for(std::map<Instruction*,unsigned>::iterator ii=instr_index_loop.begin();ii!=instr_index_loop.end();++ii){
		Instruction *inst_tmp=ii->first;
		for(unsigned j=0;j<inst_tmp->getNumOperands();++j){
			Value *op=inst_tmp->getOperand(j);
			if(Instruction *op_inst=dyn_cast<Instruction>(op)){
				if(op_inst==inst){
					float latency_tmp=get_inst_latency(inst_tmp,instr_index_loop,dependence_loop);
					if(latency_tmp>2.0){
						after_large=true;
					}
				}
			}
		}
	}
	return after_large;
}


