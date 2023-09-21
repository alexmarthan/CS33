#include <stdio.h>
#include <stdlib.h>

#include "kernels.h"

// Intentionally Uninitialized Arrays
float in1[T1I+T1S][T1J+T1S][T1K+T1S];
float in2[T2I+T1S][T2J+T1S][T2K+T1S];

float kern1[T1S][T1S][T1S];
float kern2[T2S][T2S][T2S];

float out1[T1I][T1J][T1K];
float out2[T2I][T2J][T2K];

// This function transposes one value from 
void move_one_value(int i1, int j1, int i2, int j2, int Ni, int Nj, char In[Ni][Nj], char Out[Ni][Nj]) {
  Out[i2][j2]=In[i1][j1];
}

void set_to_zero(int i, int j, int k, int Ni, int Nj, int Nk,float array[Ni][Nj][Nk]) {
  array[i][j][k]=0;
}

void macc_element(const float* In, float* Out, const float* Stencil) {
  *Out += (*In) * (*Stencil);
}

int main(int argc, char** argv) {
  char cache_run=0;
  int test_case=1;
  int opt;

  test_case=atoi(argv[1]);

  switch(test_case) {
    case 1: compute_transpose(T1I,T1J,in1,out1); break;
    case 2: compute_stencil(T2I,T2J,T2K,T2S,in2,out2,kern2); break;
    default: break;
  }

  return 0;
}


