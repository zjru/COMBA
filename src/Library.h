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


/*
 * Library.h
 */
#ifndef LIBRARY_H_
#define LIBRARY_H_

/* Default to 100 MHz. */
#if !defined(FREQUENCY100) && !defined(FREQUENCY125) && !defined(FREQUENCY150) && !defined(FREQUENCY200) && !defined(FREQUENCY250)
#  define FREQUENCY100
#endif

#ifdef FREQUENCY100
//T=10ns, F=100MHz      //(reschedule and library finished)
#  define frequency 100      //load,store can deduct some value(large latency, load not chain with small ops). imult chain with add but not write, intdiv and idiv chain with add, write.. can choose suitable library.
#  define INT_ADD 0.2
#  define INT_MULT 1.4    //actual 1.0, cannot chain with load and store. cannot chain with small operations either.
#  define IMULT 6.0       //actual 6.0, can chain with load and store, also small operations. not small and store/load together. not other large operations.
#  define INT_DIV  7.0    //actual 7.0, same with imult.
#  define IDIV  34.0      //actual 36.0, same with imult, but can chain with small and store together. not chain with other large operations.
#  define U_DIV 6.0
#  define UDIV  34.0
#  define FP_ADD   4.0    //not chain
#  define FP_MULT  3.0    //not chain
#  define FP_DIV  10.0    //not chain
#  define SI_TO_FP 4.0    //can chain
#  define FP_TO_SI 1.0
#  define SHIFT 0.05
#  define ALLOCA_LATENCY 1.0
#  define GEP_LATENCY 0.0
#  define CAST_LATENCY 0.2
#  define PHI_LATENCY 1.5//1.0
#  define ICMP_LATENCY 0.15
#  define FCMP_LATENCY 4.0
#  define SELECT_LATENCY 0.05
#  define CALL_LATENCY 1.0
#endif


#ifdef FREQUENCY125
//T=8ns, F=125MHz      //(reschedule and library finished)
#  define frequency 125    //load, store, large operations can chain with small operations.
#  define INT_ADD 0.3
#  define INT_MULT 3.0   //chain with small operations. cannot chain with load and store.
#  define IMULT 6.0      //same with int_mult
#  define INT_DIV  7.0   //same with imult
#  define IDIV  35.0     //actual 36.0, can chain with load and store
#  define U_DIV 6.0      //same with imult
#  define UDIV  35.0     //actual 36.0, can chain with load and store
#  define FP_ADD   5.0   //not chain
#  define FP_MULT  3.0   //actual 4.0, can chain with load and store
#  define FP_DIV  12.0   //not chain
#  define SI_TO_FP 4.0   //not chain
#  define FP_TO_SI 1.0   //not chain
#  define SHIFT 0.1//0.05
#  define ALLOCA_LATENCY 1.0
#  define GEP_LATENCY 0.0
#  define CAST_LATENCY 0.2
#  define PHI_LATENCY 1.5//1.0
#  define ICMP_LATENCY 0.3
#  define FCMP_LATENCY 5.0
#  define SELECT_LATENCY 0.1
#  define CALL_LATENCY 1.0
#endif


#ifdef FREQUENCY150
//T=6.67ns, F=150MHz    //(reschedule and library finished)
#  define frequency 150     //load and write can only chain when there is a shift.
#  define INT_ADD 0.3
#  define INT_MULT 3.0    //actual:3.0, not chain
#  define IMULT 6.0       //actual:6.0, not chain
#  define INT_DIV  7.0    //actual:7.0, not chain, chain with shift
#  define IDIV  36.0      //not chain with load/store/large operations, but chain with small operations
#  define U_DIV 6.0       //actual:6.0, not chain
#  define UDIV  36.0      //same with idiv
#  define FP_ADD   7.0    //actual:7.0, not chain
#  define FP_MULT  4.0    //actual:4.0, not chain
#  define FP_DIV  13.0    //actual:13.0, not chain
#  define SI_TO_FP 5.0
#  define FP_TO_SI 1.7    //actual:2.0, chain with write, not add+write
#  define SHIFT 0.1
#  define ALLOCA_LATENCY 1.0
#  define GEP_LATENCY 0.0
#  define CAST_LATENCY 0.2
#  define PHI_LATENCY 1.5//1.0
#  define ICMP_LATENCY 0.3
#  define FCMP_LATENCY 7.0
#  define SELECT_LATENCY 0.1
#  define CALL_LATENCY 1.0
#endif


#ifdef FREQUENCY200
//T=5ns, F=200MHz  //(reschedule and library finished)
#  define frequency 200 //100,300,400
#  define INT_ADD 0.5
#  define INT_MULT 5.0   //actual 5.0 //load and write can be chained with mul,etc. Therefore estimate the effective latency.
#  define IMULT 7.0      //actual 7.0
#  define INT_DIV  8.0   //actual 8.0 //div can chain with load, cannot chain with other operations.
#  define IDIV  36.0     //not chain
#  define U_DIV 7.0      //actual 7.0 same with imul
#  define UDIV  36.0
#  define FP_ADD   8.0   //not chain
#  define FP_MULT  5.0   //not chain
#  define FP_DIV 16.0    //not chain
#  define SI_TO_FP 6.0
#  define FP_TO_SI 2.5   //after casting, there is a select for div, so add 0.5.
#  define SHIFT 0.2
#  define ALLOCA_LATENCY 1.0
#  define GEP_LATENCY 1.0
#  define CAST_LATENCY 0.4
#  define PHI_LATENCY 1.5
#  define ICMP_LATENCY 0.5
#  define FCMP_LATENCY 8.0//0.5
#  define SELECT_LATENCY 0.2
#  define CALL_LATENCY 1.0
#endif


#ifdef FREQUENCY250
//T=4ns, F=250MHz   //(reschedule and library finished)
#  define frequency 250     //load, store cannot chain. large operations can chain with small operations.
#  define INT_ADD 0.5
#  define INT_MULT 5.0     //chain with small operations
#  define IMULT 7.0        //same with int_mult
#  define INT_DIV  9.0     //before chain, after not chain
#  define IDIV  36.1       //not chain, add 0.2 affect "add,DIV,add" situation.
#  define U_DIV 7.0        //same with imult
#  define UDIV  36.1       //not chain
#  define FP_ADD   9.1     //not chain
#  define FP_MULT  6.1
#  define FP_DIV 30.1
#  define SI_TO_FP 8.1
#  define FP_TO_SI 3.1
#  define SHIFT 0.2
#  define ALLOCA_LATENCY 1.0
#  define GEP_LATENCY 1.0
#  define CAST_LATENCY 0.5
#  define PHI_LATENCY 1.5
#  define ICMP_LATENCY 0.5
#  define FCMP_LATENCY 9.0//0.5
#  define SELECT_LATENCY 0.2
#  define CALL_LATENCY 1.0
#endif


#endif /* LIBRARY_H_ */
