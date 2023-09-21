#include "kernels.h"

// Ni,Nj -- Dimensions of the In/Out matricies

#pragma GCC push_options
#pragma GCC optimize ("unroll-loops")

void compute_transpose(int Ni, int Nj, const char In[restrict Ni][Nj], char Out[restrict Nj][Ni]) {

  for (int ii = 0; ii < Ni; ii += 250){ // otherwise 400 maybe
    for (int jj = 0; jj < Nj; jj += 250){
      for (int i = ii; i < ii + 250 ; ++i) {
        for(int j = jj; j < jj + 250 ; ++j) {
          Out[i][j]=In[j][i];
        }
      }
    }
  }
}
// Ni,Nj,Nk -- Dimensions of the output matrix
// S -- width/length/height of the stencil
void compute_stencil(int Ni, int Nj, int Nk, int S, const float In[Ni+S][Nj+S][Nk+S], float Out[Ni][Nj][Nk], const float Stencil[S][S][S]) {
  for(int i = 0; i < Ni; ++i) { 
    for(int j = 0; j < Nj; ++j) { 
      for(int k = 0; k < Nk; ++k) {
        Out[i][j][k] = 0;

        for(int x = 0; x < S; ++x) {
          for(int y = 0; y < S; ++y) {
            for(int z = 0; z < S; ++z) {
              *(&Out[i][j][k]) += (*(&In[i+x][j+y][k+z])) * (*(&Stencil[x][y][z]));
            } 
          } 
        }
      } 
    } 
  }
}


