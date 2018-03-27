/*
 *  COMBA
 *  Copyright (C) 2017  RCSL, HKUST
 *  
 *  ONLY FOR ACADEMIC USE, NOT FOR COMMERCIAL USE.
 *  
 *  Please use our tool at academic institutions and non-profit 
 *  research organizations for research use. 
 *  
 ***************************************************************   
 *
 * test.h
 *
 *  Created on: Dec 20, 2016
 *      Author: zjr
 */

#ifndef TEST_H_
#define TEST_H_

#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CFG.h"
#include "llvm/Support/GraphWriter.h"
#include "llvm/IR/Intrinsics.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IRReader/IRReader.h"
#include <bits/stl_map.h>
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/IR/InstrTypes.h"
#include "llvm/PassAnalysisSupport.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <string>
#include <map>
#include <typeinfo>
#include <queue>
#include <algorithm>

using namespace llvm;

typedef std::vector<int> ivector;
typedef std::vector<std::pair<int,int> > pvector;


//User Defined: Input information according to applications
//bicg
#define d_num 5   //dim input size, equal to the number of array
#define a_num 6   //dim*d_num, equal to the addition of all the dimensions
#define l_num 2   //#depth*#loop_num
#define f_num 1   //#functions
/*//gemm
#define d_num 3  //dim input size, equal to the number of array
#define a_num 6  //dim*d_num, equal to the addition of all the dimensions
#define l_num 4  //#depth*#loop_num
#define f_num 1
*/

//Input variables
extern int function_loop_num[];
extern int Loops_counter_input[];
extern int Loops_unroll_input[];
extern int Loops_pipeline_input[];
extern int array_partition_type[];
extern int array_partition_factor[];
extern int function_array_num[];
extern int array_size[];
extern int array_dimension[];
extern int function_pipeline_input[];
extern int dataflow_input[];

//Pragma setting
extern std::map<Function*, int> Function_dataflow;
extern std::map<Function*, int> Function_pipeline;
extern std::map<Loop*, int> Loops_counter;
extern std::map<Loop*, int> Loops_unroll;
extern std::map<Loop*, int> Loops_pipeline;
extern std::vector<std::pair<int,int> > array_partition[30]; //0:block, 1:cyclic;

//Parameters
extern std::map<Instruction*, float> Load_latency;
extern std::map<Instruction*, float> Store_latency;
extern std::map<Instruction*,int> inst_reschedule_considered;
extern std::map<Value*, int> array_number;
extern std::map<BasicBlock*, int> bb_trip_counter;
extern std::map<Loop*, float> loop_critical_path;
extern std::vector<unsigned> Function_read_arg[30];
extern std::vector<unsigned> Function_write_arg[30];
extern std::vector<unsigned> Function_tmp_arg[30];

//Estimated performance
extern std::map<Loop*, float> loop_cycles;
extern std::map<Loop*, int> loop_II;
extern std::map<Function*, int> function_cycles;
extern std::map<Function*, int> function_II;
extern std::map<Function*, unsigned> Function_index;

//DSE parameters
extern std::vector<std::pair<int,int> > function_subelement_cycles[30];
extern std::map<Value*,Value*> array_map_array;
extern std::vector<Value*> loop_map_array[l_num];
extern std::map<int,int> optimized_loops;
extern std::map<Function*,int> optimized_functions;
extern std::vector<std::pair<int, std::vector<float> > > loop_map_array_type[l_num];


class Configuration {
public:
	   Configuration(int num);
	   void set_num_config(int num);
	   int get_num_config();
	   void clear_config_vector();
	   void set_configuration();
	   void get_configuration();
	   void get_best_loop_unroll_pipeline(ivector &loop_unroll_vec,ivector &loop_pipeline_vec);
	   void get_best_array_partition(pvector &array_partition_vec);
	   void get_best_function_configuration(ivector &function_pipeline_vec,ivector &function_dataflow_vec);
private:
	   int config_num;
	   ivector loop_unroll_config;
	   ivector loop_pipeline_config;
	   pvector array_partition_config;
	   ivector function_pipeline_config;
	   ivector dataflow_config;
};


class Top_Results {
public:
		std::map<int,int> top_function_cycles;
		std::map<int,int> top_function_II;
		std::map<int, unsigned> top_function_DSP;
		std::map<int, unsigned> top_function_BRAM;
};


#endif /* TEST_H_ */
