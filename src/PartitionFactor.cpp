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


#include "PartitionFactor.h"
#include "ArrayName.h"


int partition_factor(Instruction *inst)
{
	int factor=1;
	Value *array_name=get_array_name(inst);
	if(array_number.count(array_name)){
		int array_index=array_number[array_name];
		int dim=array_dimension[array_index];
		if(dim==1){
			std::vector< std::pair<int,int> >::iterator it = array_partition[array_index].begin();
			factor=it->second;
		}
		else if(dim>1){
			int fac=1;
			for(std::vector< std::pair<int,int> >::iterator it= array_partition[array_index].begin(); it!=array_partition[array_index].end(); ++it){
				fac=it->second;
				factor *=fac;
			}
		}
		else{
			errs()<<"Dim is 0, wrong!\n";
		}
	}
	return factor;
}

