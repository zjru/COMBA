Target of COMBA (for accademic use):
======================================
COMBA is used to estimate the performance of applications when applied with different pragma configurations in HLS tools, such as Vivado HLS. Another function of COMBA is that it can explore the design space quickly and find a high-performance configuration within minutes under given resource constraints.

Please cite our paper if you use COMBA for your academic research:
```
@inproceedings{zhao2017comba,
  title={COMBA: A comprehensive model-based analysis framework for high level synthesis of real applications},
  author={Zhao, Jieru and Feng, Liang and Sinha, Sharad and Zhang, Wei and Liang, Yun and He, Bingsheng},
  booktitle={Computer-Aided Design (ICCAD), 2017 IEEE/ACM International Conference on},
  pages={430--437},
  year={2017},
  organization={IEEE}
}
```

License:
======================================
The source code is released under [GPLv3](https://www.gnu.org/licenses/licenses.en.html) license.
If you have any questions about how to use our tool, please contact Jieru ZHAO (jzhaoao@connect.ust.hk).

For commercial inqueries, please contact Prof. Wei ZHANG (wei.zhang@ust.hk).


Building COMBA:
======================================
 * The platform we use is the LLVM compiler (Version 3.4) with the clang front-end. Please install it at first. The tool can be used directly on LLVM 3.4. 
 * Put the whole folder of "COMBA" under "/llvm-3.4/lib/Transforms".
 * Add "add_subdirectory(COMBA)" in the "/llvm-3.4/lib/Transforms/CMakeLists.txt".




Using COMBA:
======================================
 For testing given applications in the "test" folder (two steps):

  * To compile the source codes, run the bash file "runMyPro.sh".
  * To invoke the tool to analyze one application, run the command:   
	`opt -load /llvm-3.4/Release+Asserts/lib/LLVMTest.so -test < ./test/$name.ll >/dev/null`.   
     $name.ll is the .ll file of the corresponding application, which is bicg.ll by default.


 For testing users' own applications (five steps):

  * To obtain the .ll file, run the command:  
	`clang -O1 -emit-llvm -S $name.c -o $name.ll`.   	   
     $name.c is the application that you want to test and $name.ll is the corresponding .ll file.
  * Put the generated .ll file in the folder "test".
  * Modify the input of the tool as following:
	* In "/src/test.h", the values of four varibles (d_num, a_num, l_num and f_num) need to be modified. 
		* d_num is the number of arrays in the top-function;   
		* a_num is the sum of the number of dimension of each array; 
		* l_num is the sum of loop levels of each loop; 
		* f_num is the number of functions in this application.
    
	* In "/src/test.cpp", at the beginning of this file, five arrays (function_loop_num, Loops_counter_input, function_array_num, array_size and array_dimension) need to be initialized.
		* function_loop_num[i] is the number of loop levels in the (i+1)-th function.  
		For example, if the second function in the application contains two 2-level nested loops, then function_loop_num[1] = 2*2 = 4.
		* Loops_counter_input[i] is the loop bound of the (i+1)-th loop.  
		The order of loops is counted from the first loop to the next loop and from the outer level to the inner level. You can see the function "set_loop_counter_ul_pipeline" in "SetPragma.cpp" to learn the details if you want.
		* function_array_num[i] is the number of arrays in the (i+1)-th function. 
		* array_size[i] is the number of array elements in the (i+1)-th array dimension.   
		For example, if there are three arrays A[8], B[16], C[4][8] and their order is A, C, B, then array_size[0] = 8, array_size[1] = 4 (row), array_size[2] = 8 (column) and array_size[3] = 16.   
		Note that the order of arrays depends on its order of appearance in the LLVM IR (.ll file). If you don't want to look at the control flow graph or check the .ll file, you can also get this information by removing the annotation symbols at the end of the function "set_array_index" in "SetParameter.cpp", doing the rest two steps, exiting the process after getting the information and then come back to this step to finish the input setting. 
		* array_dimension[i] is the number of dimension of the (i+1)-th array.
       
	* In "/src/test.cpp", the other six arrays, Loops_unroll_input[l_num], Loops_pipeline_input[l_num], array_partition_type[a_num], array_partition_factor[a_num], function_pipeline_input[f_num] and dataflow_input[f_num], are the configuration of the beginning point (no pragma is applied). Therefore, initialize them to 0 or 1 (loop/function pipelining and dataflow are 0 and others are 1). 
      
 * To compile the source codes, run the bash file "runMyPro.sh".
 * To invoke the tool to analyze the application, run the command:  
	`opt -load /llvm-3.4/Release+Asserts/lib/LLVMTest.so -test < ./test/$name.ll >/dev/null`.



