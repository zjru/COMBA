/**
 * This version is stamped on May 10, 2016
 *
 * Contact:
 *   Louis-Noel Pouchet <pouchet.ohio-state.edu>
 *   Tomofumi Yuki <tomofumi.yuki.fr>
 *
 * Web address: http://polybench.sourceforge.net
 */
/* bicg.c: this file is part of PolyBench/C */

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <math.h>

#include "bicg.h"

void kernel_bicg(int A[N][M], int s[M], int q[N], int p[M], int r[N])
{
  int i, j;
  int m=M;
  int n=N;
  kernel_bicg_label2:for (i = 0; i < n; i++)
  {
    kernel_bicg_label0:for (j = 0; j < m; j++)
	{
    	    s[j] = s[j] + r[i] * A[i][j];
	    q[i] = q[i] + A[i][j] * p[j];
	}
  }

}
