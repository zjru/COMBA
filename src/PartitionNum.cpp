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


#include "PartitionNum.h"
#include "GEP.h"
#include "ArrayName.h"

int get_bank_number(Instruction *inst)
{
	Value *array_name=NULL;
	Value *operand=NULL;
	int Bank_Num=0;
	int offset=0;
	int array_index=0;
	int dim=0;
	int size=0;
	GetElementPtrInst *gep=NULL;

	gep=get_GEP(inst);
	if(gep==NULL){
		Bank_Num=0;
	}
	else{
		array_name=get_array_name(inst);
		if(!array_number.count(array_name)){
			errs()<<"The array loaded is not set correctly for partition.\n";
		}
		else{
			array_index=array_number[array_name];
			dim=array_dimension[array_index];
			int initial=0;
			for(int j=0; j<array_index; ++j){
				initial += array_dimension[j];
			}
			if(dim==1){
				int factor=1;
				unsigned num_indice=gep->getNumIndices();
				operand=gep->getOperand(num_indice);
				offset=compute_gep_operand(operand,false,0);
				if(offset>=0){
					size=array_size[initial];
					for(std::vector< std::pair<int,int> >::iterator it= array_partition[array_index].begin(); it!=array_partition[array_index].end(); ++it)
					{
						//partition type: block
						if(it->first==0){
							factor=it->second;
							Bank_Num=floor((float)offset/(float)(ceil((float)size/(float)factor)));
						}
						//partition type: cyclic
						else if(it->first==1){
							factor=it->second;
							Bank_Num=offset % factor;
						}
						else{
							errs()<<"Something maybe wrong for array setting.\n";
						}
					}
				}
			}
			else if(dim>1){
				int factor=1;
				unsigned num_indice=gep->getNumIndices();
				int indice=num_indice-dim+1;
				int Bank=0;
				int fac=1;
				for(std::vector< std::pair<int,int> >::iterator it= array_partition[array_index].begin(); it!=array_partition[array_index].end(); ++it)
				{
					fac *= factor;
					size=array_size[initial];
					operand=gep->getOperand(indice);
					offset=compute_gep_operand(operand,false,0);
					if(offset>=0){
						if(it->first==0){
							factor=it->second;
							Bank=floor((float)offset/(float)(ceil((float)size/(float)factor)));
						}
						else if(it->first==1){
							factor=it->second;
							Bank=offset % factor;
						}
						Bank_Num += Bank * fac;
						initial++;
						indice++;
					}
					else{
						Bank=0;
						Bank_Num += Bank * fac;
						initial++;
						indice++;
					}
				}
			}
			else{
				errs()<<"Dim is 0, wrong!\n";
			}
		}
	}
	return Bank_Num;

}


int compute_array_element_offset(Instruction *inst, BasicBlock *bb, int base_variable)
{
	int offset=0;
	if(isa<LoadInst>(inst)||isa<StoreInst>(inst))
	{
		GetElementPtrInst *gep=get_GEP(inst);
		if(gep!=NULL){
			Value *array=get_array_name(inst);
			if(array_number.count(array)){
				int array_index=array_number[array];
				int dim=array_dimension[array_index];
				if(dim==1){
					unsigned num_indice=gep->getNumIndices();
					Value *op=gep->getOperand(num_indice);
					offset=compute_gep_operand(op,false,0);
				}
				else if(dim>1){
					int dim_indvar=1;
					for(User::op_iterator ii=gep->idx_begin(), ie=gep->idx_end();ii!=ie; ++ii){
						Value *op_l=*ii;
						if(Instruction *op_inst=dyn_cast<Instruction>(op_l)){
							BasicBlock *bb1=op_inst->getParent();
							if(bb1==bb){
								break;
							}
							dim_indvar++;
						}
						else{
							dim_indvar++;
						}
					}
					Value *op=gep->getOperand(dim_indvar);
					int base=compute_gep_operand(op,false,0);
					int initial=0;
					for(int j=0; j<array_index; ++j){
						initial += array_dimension[j];
					}
					unsigned num_indice=gep->getNumIndices();
					int indice=num_indice;
					int factor=1;
					for(int i=dim; i>0; --i){
						int new_dim=initial+i-1;
						int new_index=0;
						if(indice != dim_indvar){
							Value *operand=gep->getOperand(indice);
							new_index=compute_gep_operand(operand,false,0);
						}
						else{
							new_index=base+base_variable;
						}
						offset += new_index*factor;
						factor*=array_size[new_dim];
						indice--;
					}
				}
				else{
					errs()<<"Something Wrong!\n";
				}
			}
		}
		else{
			offset=0;
		}
	}
	return offset;

}

//when (innermost) parent loop is unrolled, new elements will go to different banks due to differemt options of array partition.
int get_unroll_SameBank_num(BasicBlock *bb, Instruction *inst, Instruction *inst1, std::vector<int> &array_element_counted)
{
	int num=0;
	Value *op=NULL;
	int bank_num=get_bank_number(inst);
	Value *array_name=get_array_name(inst);
	GetElementPtrInst *gep=get_GEP(inst);
	if(inst!=inst1){
		GetElementPtrInst *gep_tmp=get_GEP(inst1);
		if(bb_trip_counter[bb]>=1){
			if((gep_tmp!=NULL)&&(gep!=NULL)){
				if(array_number.count(array_name)){
					int array_index=array_number[array_name];
					int dim=array_dimension[array_index];
					Value *operand=NULL;
					// Array with dimension one
					if(dim==1){
						unsigned num_indice=gep->getNumIndices();
						op=gep->getOperand(num_indice);
						//op=gep->getOperand(1);
						int off=compute_gep_operand(op,false,0);
						unsigned num_indice_tmp=gep_tmp->getNumIndices();
						operand=gep_tmp->getOperand(num_indice_tmp);
						int offset=compute_gep_operand(operand,false,0);
						Value *val=gep_tmp->getPointerOperand();
						if(isa<PHINode>(val)){
						    offset--;
						}
						int initial=0;
						for(int j=0; j<array_index; ++j){
							initial += array_dimension[j];
						}
						int size=array_size[initial];
						if(off>=0&&offset>=0){
							for(int i=0; i<bb_trip_counter[bb]; ++i){
								int offset1=offset + i;
								if(find(array_element_counted.begin(), array_element_counted.end(),offset1)==array_element_counted.end())
								{
									int bank_tmp=0;
									//errs()<<"offset: "<<offset1<<"\n";
									if(offset1<size){
										for(std::vector< std::pair<int,int> >::iterator it= array_partition[array_index].begin(); it!=array_partition[array_index].end(); ++it)
										{
											if(it->first==0){
												int factor=it->second;
												bank_tmp=floor((float)offset1/(float)(ceil((float)size/(float)factor)));
											}
											else if(it->first==1){
												int factor=it->second;
												bank_tmp=offset1 % factor;
											}
											else{
												errs()<<"Something maybe wrong for array setting.\n";
											}
											if(bank_num==bank_tmp){
												num++;
											}
										}
									}
									array_element_counted.push_back(offset1);
								}
							}
						}
						else{
							num+=bb_trip_counter[bb];
						}
					}
					// Array with multi-dimension
					else if(dim>1){
						int dim_indvar=1;
						for(User::op_iterator ii=gep_tmp->idx_begin(), ie=gep_tmp->idx_end();ii!=ie; ++ii)
						{
							Value *op_l=*ii;
							if(Instruction *op_inst=dyn_cast<Instruction>(op_l)){
								BasicBlock *bb1=op_inst->getParent();
								if(bb1==bb){
									break;
								}
								dim_indvar++;
							}
							else{
								dim_indvar++;
							}
						}
						unsigned nn=gep->getNumOperands();
						if(dim_indvar<(int)nn)
						{
							op=gep->getOperand(dim_indvar);
							int off=compute_gep_operand(op,false,0);
							operand=gep_tmp->getOperand(dim_indvar);
							int offset_base=compute_gep_operand(operand,false,0);
							int initial=0;
							for(int j=0; j<array_index; ++j){
								initial += array_dimension[j];
							}
							int initial_indvar=initial+dim_indvar-1;
							int size_indvar=array_size[initial_indvar];
							if(off>=0&&offset_base>=0)
							{
								for(int i=0; i<bb_trip_counter[bb];++i)
								{
									int offset = offset_base + i;
									if(offset<size_indvar)
									{
										int element_offset=compute_array_element_offset(inst1,bb,i);
										if(find(array_element_counted.begin(), array_element_counted.end(),element_offset)==array_element_counted.end())
										{
											int offset1=0;
											unsigned num_indice=gep_tmp->getNumIndices();
											int indice=num_indice-dim+1;
											int fac=1;
											int factor=1;
											int initial1=initial;
											int bank_compare=0;
											for(std::vector< std::pair<int,int> >::iterator it= array_partition[array_index].begin(); it!=array_partition[array_index].end(); ++it)
											{
												fac *= factor;
												int bank_tmp=0;
												int size=array_size[initial1];
												Value *operand1=gep_tmp->getOperand(indice);
												if(indice != dim_indvar){
													offset1=compute_gep_operand(operand1,false,0);
												}
												else{
													offset1=offset;
												}
												if(it->first==0){
													factor=it->second;
													bank_tmp=floor((float)offset1/(float)(ceil((float)size/(float)factor)));
												}
												else if(it->first==1){
													factor=it->second;
													bank_tmp=offset1 % factor;
												}
												else{
													errs()<<"Something maybe wrong for array setting.\n";
												}
												bank_compare += bank_tmp * fac;
												initial1++;
												indice++;
											}
											if(bank_num==bank_compare){
												num++;
											}
											array_element_counted.push_back(element_offset);
										}
									}
								}
							}
							else{
								num+=bb_trip_counter[bb];
							}
						}
					}
					else{
						errs()<<"Dim is 0, wrong!\n";
					}
				}
			}
		}
	}
	else{
		if(bb_trip_counter[bb]>=1)
		{
			if(gep!=NULL)
			{
				if(array_number.count(array_name))
				{
					int array_index=array_number[array_name];
					int dim=array_dimension[array_index];
					// Array with dimension one
					if(dim==1)
					{
						unsigned num_indice=gep->getNumIndices();
						op=gep->getOperand(num_indice);
						int off=compute_gep_operand(op,false,0);
				        Value *val=gep->getPointerOperand();
				        if(isa<PHINode>(val)){
				            off--;
				        }
						int initial=0;
						for(int j=0; j<array_index; ++j){
							initial += array_dimension[j];
						}
						int size=array_size[initial];
						if(off>=0){
							for(int i=0; i<bb_trip_counter[bb]; ++i){
								int offset1=off + i;
								if(find(array_element_counted.begin(), array_element_counted.end(),offset1)==array_element_counted.end()){
									int bank_tmp=0;
									if(offset1<size){
										for(std::vector< std::pair<int,int> >::iterator it= array_partition[array_index].begin(); it!=array_partition[array_index].end(); ++it){
											if(it->first==0){
												int factor=it->second;
												bank_tmp=floor((float)offset1/(float)(ceil((float)size/(float)factor)));
											}
											else if(it->first==1){
												int factor=it->second;
												bank_tmp=offset1 % factor;
											}
											else{
												errs()<<"Something maybe wrong for array setting.\n";
											}
											if(bank_num==bank_tmp){
												num++;
											}
										}
									}
									if(i!=0){
										array_element_counted.push_back(offset1);
									}
								}
							}
						}
						else{
							for(std::vector< std::pair<int,int> >::iterator it= array_partition[array_index].begin(); it!=array_partition[array_index].end(); ++it){
								if(it->second==size){
									num=1;
								}
								else{
									num=bb_trip_counter[bb];
								}
							}
						}
					}
					// Array with multi-dimension
					else if(dim>1)
					{
						int dim_indvar=1;
						bool idx_constant=false;
						for(User::op_iterator ii=gep->idx_begin(), ie=gep->idx_end();ii!=ie; ++ii)
						{
							Value *op_l=*ii;
							if(Instruction *op_inst=dyn_cast<Instruction>(op_l)){
								BasicBlock *bb1=op_inst->getParent();
								if(bb1==bb){
									break;
								}
								dim_indvar++;
							}
							else{
								dim_indvar++;
								if(isa<ConstantInt>(op_l)){
									idx_constant=true;
								}
							}
						}
						unsigned nn=gep->getNumOperands();
						if(dim_indvar<(int)nn)
						{
							op=gep->getOperand(dim_indvar);
							int off_base=compute_gep_operand(op,false,0);
							int initial=0;
							for(int j=0; j<array_index; ++j){
								initial += array_dimension[j];
							}
							int initial_indvar=initial+dim_indvar-1;
							if(idx_constant==true){
								initial_indvar=initial+dim_indvar-2;
							}
							int size_indvar=array_size[initial_indvar];

							if(off_base>=0){
								for(int i=0; i<bb_trip_counter[bb];++i)
								{
									int off = off_base+i;
									if(off<size_indvar){
										int element_offset=compute_array_element_offset(inst,bb,i);
										if(find(array_element_counted.begin(), array_element_counted.end(),element_offset)==array_element_counted.end()){
											int offset1=0;
											unsigned num_indice=gep->getNumIndices();
											int indice=num_indice-dim+1;
											int fac=1;
											int factor=1;
											int initial1=initial;
											int bank_compare=0;
											for(std::vector< std::pair<int,int> >::iterator it= array_partition[array_index].begin(); it!=array_partition[array_index].end(); ++it)
											{
												int bank_tmp=0;
												fac *= factor;
												int size=array_size[initial1];
												Value *operand1=gep->getOperand(indice);
												if(indice != dim_indvar){
													offset1=compute_gep_operand(operand1,false,0);
												}
												else{
													offset1=off;
												}
												if(it->first==0){
													factor=it->second;
													bank_tmp=floor((float)offset1/(float)(ceil((float)size/(float)factor)));
												}
												else if(it->first==1){
													factor=it->second;
													bank_tmp=offset1 % factor;
												}
												else{
													errs()<<"Something maybe wrong for array setting.\n";
												}
												bank_compare += bank_tmp * fac;
												initial1++;
												indice++;
											}
											if(bank_num==bank_compare){
												num++;
											}
											if(i!=0){
												array_element_counted.push_back(element_offset);
											}
										}
									}
								}
							}
							else{
								for(int i=0; i<bb_trip_counter[bb];++i)
								{
									int offset1=0;
									unsigned num_indice=gep->getNumIndices();
									int indice=num_indice-dim+1;
									int fac=1;
									int factor=1;
									int initial1=initial;
									int bank_compare=0;
									for(std::vector< std::pair<int,int> >::iterator it= array_partition[array_index].begin(); it!=array_partition[array_index].end(); ++it)
									{
										int bank_tmp=0;
										fac *= factor;
										int size=array_size[initial1];
										Value *operand1=gep->getOperand(indice);
										if(indice != dim_indvar){
											offset1=compute_gep_operand(operand1,false,0);
											if(it->first==0){
												factor=it->second;
												bank_tmp=floor((float)offset1/(float)(ceil((float)size/(float)factor)));
											}
											else if(it->first==1){
												factor=it->second;
												bank_tmp=offset1 % factor;
											}
											else{
												errs()<<"Something maybe wrong for array setting.\n";
											}
										}
										else{
											if(it->second==size){
												bank_tmp=i;
											}
											else{
												bank_tmp=0;
											}
										}
										bank_compare += bank_tmp * fac;
										initial1++;
										indice++;
									}
									if(bank_num==bank_compare){
										num++;
									}
								}
							}
						}
					}
					else{
						errs()<<"Dim is 0, wrong!\n";
					}
				}
			}
		}
	}

	return num;

}
