/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* gemm.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

/* Include benchmark-specific header. */
#include "gemm.h"

/* Main computational kernel. The whole function will be timed,
   including the call and return. */
void kernel_gemm(int C[NI][NJ], int A[NI][NK],int B[NK][NJ])
{
  int i, j, k;
 int alpha=3;
 int beta=2;
 int ni=NI;
 int nj=NJ;
 int nk=NK;
  kernel_gemm_label3:for (i = 0; i < ni; i++) {
    for (j = 0; j < nj; j++){
    	C[i][j] *= beta;
    }
    for (k = 0; k < nk; k++) {
       kernel_gemm_label1:for (j = 0; j < nj; j++){
    	   C[i][j] += alpha * A[i][k] * B[k][j];
       }
    }
  }

}

