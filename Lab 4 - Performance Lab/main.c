#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/mman.h>

#include "kernels.h"
#include "main.h"

void *in, *out, *out_check;
char hostname[HOST_NAME_MAX];
char cache_profile=1;
char do_all=1;


#define PERFLINESZ (200)



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


static struct option long_options[] = {
//    {"direct-path",        no_argument,       nullptr, 'd',},
    {"input",         required_argument, 0, 'i'},
    {"trials",        required_argument, 0, 't'},
    {0, 0, 0, 0,},
};

float calc_mem_energy(long l1, long l2, long l3, long mem) {
  return  (l1*0.05+l2*0.1+l3*0.5+mem*10.0)/1000000000.0;
}

typedef struct {
  int kernel;
  int index;
  int Ni,Nj,Nk;
  int S;
  float orig_msec, orig_mem;
  float best_msec, best_mem;
} TestParams;

void clear3d(int Ni, int Nj, int Nk, float a[Ni][Nj][Nk]) {
  for(int i = 0; i < Ni; ++i) {
    for(int j = 0; j < Nj; ++j) {
      for(int k = 0; k < Nk; ++k) {
        a[i][j][k] = 0;
      }
    }
  }
}

void gen_3d(int Ni, int Nj, int Nk, float a[Ni][Nj][Nk],float normalize) {
  for(int i = 0; i < Ni; ++i) {
    for(int j = 0; j < Nj; ++j) {
      for(int k = 0; k < Nk; ++k) {
        a[i][j][k] = ((float)rand())/(normalize*RAND_MAX/8);
      }
    }
  }
}

static int max_errors_to_print = 5;

char check_2d(int Ni, int Nj, 
              char a[Ni][Nj], char a_check[Ni][Nj], char in[Ni][Nj]) {

  int errors_printed=0;
  char has_errors = 0;
  for(int i = 0; i < Ni; ++i) {
    for(int j = 0; j < Nj; ++j) {
      if( (a[i][j] != a_check[i][j]) ) { 
        has_errors = 1;
        if(errors_printed < max_errors_to_print) {
          if(errors_printed==0) printf("\n");
          printf("Error on index: [%d][%d].",i,j);
          printf("Your output: %01X, Correct output %01X\n", 
              (unsigned char)a[i][j], (unsigned char)a_check[i][j]);
          errors_printed++;
        } else {
          //printed too many errors already, just stop
          if(max_errors_to_print !=0) {
            printf("and many more errors likely exist...\n");
          }
          return 1;
        }
      }
    }
  }
  return has_errors;
}

char check_3d(int Ni, int Nj, int Nk,
              float a[Ni][Nj][Nk], float a_check[Ni][Nj][Nk]) {
  int errors_printed=0;
  char has_errors = 0;
  for(int i = 0; i < Ni; ++i) {
    for(int j = 0; j < Nj; ++j) {
      for(int k = 0; k < Nk; ++k) {
        if( (a[i][j][k] < (a_check[i][j][k] - 0.005)) ||
            (a[i][j][k] > (a_check[i][j][k] + 0.005))    ) {
          has_errors = 1;
          if(errors_printed < max_errors_to_print) {
            if(errors_printed==0) printf("\n");
            printf("Error on index: [%d][%d][%d].",i,j,k);
            printf("Your output: %f, Correct output %f\n",a[i][j][k], a_check[i][j][k]);
            errors_printed++;
          } else {
            //printed too many errors already, just stop
            if(max_errors_to_print !=0) {
              printf("and many more errors likely exist...\n");
            }
            return 1;
          }
        }
      }
    }
  }
  return has_errors;
}


long first_num(char* p) {
  char str[128] = {0};
  
  int found=0;
  char* s=str;
  for(int i = 0; i <1000; i++){
    char c = *p++;
    if(c==0) break;
    if(c==',') continue;
    if(c==' ' || c=='\t') {
      if(found) break;
      else continue;
    }
    found=1;
    *s++=c;
  }

  return strtol(str, &str, 10);
}

long min(long l1, long l2) {
  if(l1 < l2) return l1;
  else return l2;
}

float run(TestParams* p, char check_func, char* is_broken) {
  uint64_t start_time, total_time;

  int Ni = p->Ni, Nj = p->Nj, Nk=p->Nk;
  int S = p->S;

  void* kern = malloc(sizeof(float) * S * S * S);
  // Generate the inputs 
  gen_3d(S,S,S,kern,S*S*S);

  if(check_func) {
    if(p->kernel==0) {
      transpose_check(Ni,Nj,in,out_check);
    } else {
      stencil_check(Ni,Nj,Nk,S,in,out_check,kern);
    }
  }

  start_time = read_usec();
  if(p->kernel==0) {
    compute_transpose(Ni,Nj,in,out);
  } else {
    compute_stencil(Ni,Nj,Nk,S,in,out,kern);
  }
  total_time = read_usec() - start_time;

  if(check_func) {
    if(p->kernel==0) {
      if(check_2d(Ni,Nj,out,out_check,in)) {
        *is_broken=1;
      } 
    } else {
      if(check_3d(Ni,Nj,Nk,out,out_check)) {
        *is_broken=1;
      } 
    }
  }

  double total_msec = total_time / 1000.0;
  float speedup = p->orig_msec / total_msec ;
  p->best_msec = fmin(p->best_msec,total_msec);

  double computations;
 
  if(p->kernel==0) {
    computations = ((double)Ni)*Nj;
  } else {
    computations = ((double)Ni)*Nj*Nk*S*S*S;
  } 

  float ghz=2.0;
  double clocks = total_time*1000.0 * ghz;

  free(kern);


  // Now run the plain binary using system calls and parse the standard out
  long insts=0, vec_insts=0, l1=0,l2=0,l3=0,mem=0;
  FILE *fp;
  char cmd[240];

  // Check if we are on the right machine before running perf, otherwise it
  // will just crash and make us look bad!
  if(cache_profile) {
    sprintf(cmd,"perf stat -e instructions,simd_fp_256.packed_single,L1-dcache-loads,l2_rqsts.all_demand_data_rd,l2_rqsts.all_rfo,l2_rqsts.all_pf,LLC-loads,LLC-stores,LLC-prefetches,node-loads,node-stores,node-prefetches ./plain %d 2>&1",p->index);
    
    if ((fp = popen(cmd, "r")) == NULL) {
        printf("Could not execute: \"%s\", did it get compiled?\n",cmd);
        exit(-1);
    }

    char buf[PERFLINESZ];
    fgets(buf, PERFLINESZ, fp); //discard three lines lazily
    fgets(buf, PERFLINESZ, fp);
    fgets(buf, PERFLINESZ, fp);

    fgets(buf, PERFLINESZ, fp); insts=first_num(buf);

    fgets(buf, PERFLINESZ, fp); vec_insts=first_num(buf);

    fgets(buf, PERFLINESZ, fp); l1=first_num(buf);

    fgets(buf, PERFLINESZ, fp); l2+=first_num(buf);
    fgets(buf, PERFLINESZ, fp); l2+=first_num(buf);
    fgets(buf, PERFLINESZ, fp); l2+=first_num(buf);

    fgets(buf, PERFLINESZ, fp); l3+=first_num(buf);
    fgets(buf, PERFLINESZ, fp); l3+=first_num(buf);
    fgets(buf, PERFLINESZ, fp); l3+=first_num(buf);

    fgets(buf, PERFLINESZ, fp); mem+=first_num(buf);
    fgets(buf, PERFLINESZ, fp); mem+=first_num(buf);
    fgets(buf, PERFLINESZ, fp); mem+=first_num(buf);
  } 

  //printf("%d,%d,%d,%d\n",l1,l2,l3,mem);

  float mem_energy=calc_mem_energy(l1,l2,l3,mem);
  p->best_mem = fmin(p->best_mem,mem_energy);
  float energyup=p->orig_mem/mem_energy;

  printf(" | %8.1f %6.3f %8.1f %11.1f | %7.1f %7.1f %7.2f %7.2f %8.5f | %7.1f %8.1f\n",
          total_msec, clocks/computations, insts/1000000.0, vec_insts/1000000.0, 
          l1/1000000.0, l2/1000000.0, l3/1000000.0, mem/1000000.0, mem_energy,
          speedup, energyup);

  if(cache_profile) {
    if (pclose(fp)) {
      printf("\"%s\" exited with nonzero error status\n",cmd);
      exit(-1);
    }
  }

  return total_time;
}



TestParams Tests[NUM_TESTS] = {
   {0,1,T1I,T1J,T1K,T1S,    530, 0.09, 10000000.00,10000000.00},
   {1,2,T2I,T2J,T2K,T2S,   4250, 0.34, 10000000.00,10000000.00}};

char run_test(int i, char check_func) {
  if(i<0 || i >= NUM_TESTS) {
    printf("Bad Test Case: %d, exiting\n",i+1);
    exit(0);
  }
  char is_broken=0;

  if(Tests[i].kernel==0) {
     printf("%2d %7s | %5d %5d %7s %2s",
          i,"Transp.",Tests[i].Ni,Tests[i].Nj, "-", "-");
  } else {
     printf("%2d %7s | %5d %5d %7d %2d",
          i,"Stencil",Tests[i].Ni,Tests[i].Nj,Tests[i].Nk, Tests[i].S);
  }

  //printf("Test %d with Ni,Nj,Nk=%d,%d,%d; S=%d -- ",i+1,
  //        Tests[i].Ni,Tests[i].Nj,Tests[i].Nk,Tests[i].S);
  fflush(stdout);
  run(&(Tests[i]),check_func,&is_broken);
  return is_broken;
}

float interp(float s, float l, float lgrade, 
                      float h, float hgrade) {
  return (s - l) * (hgrade - lgrade) / (h -l) + lgrade;
}

float grade(float s) {
  if(s<1)   return 0;
  if(s<6)  return interp(s,   1,  0,  6,  60);
  if(s<25)  return interp(s,  6, 60,  25,  80);
  if(s<60)  return interp(s,  25, 80,  60,  90);
  return 100;
}

float grade_extra(float s) {
  if(s<1)   return 0;
  if(s<10)  return interp(s,   1,  0,  10,  60);
  if(s<100)  return interp(s,  10, 60,  20,  80);
  if(s<400)  return interp(s,  25, 80,  60,  90);
  return 100;
}

int main(int argc, char** argv) {
  char check_func=1;
  int test_case=1;


  int opt;
  int num_trials=1;

  int result = gethostname(hostname, HOST_NAME_MAX);
  if (result) {
    perror("gethostname");
    return EXIT_FAILURE;
  }

  //Parse options:
  while ((opt = getopt_long(argc, argv, "i:t:", long_options, 0)) != -1) {
    switch (opt) {
      case 'i':
        if(*optarg == 'a') {
          do_all=1;
        } else if(*optarg == 'g') {
          do_all=2;
        } else {
          do_all=0;
          test_case=atoi(optarg);
        }
        break;
      case 't': 
        num_trials=atoi(optarg);
    }
  }

  reticulate_splines(do_all);

  //printf("Num Trials: %d (change with --trials)\n", num_trials);
  printf("\n");

  //setup the input array and generate some random values  
  srand (time(NULL));
  int total=0;
  int max=0;
  for(int i = 0; i < NUM_TESTS; ++i) {
    int S = Tests[i].S;
    total= (Tests[i].Ni+S)*(Tests[i].Nj+S)*(Tests[i].Nk+S);
    if(total>max) {
      max=total;
    }
  }
  max=(max+4095)>>12<<12;
  in = aligned_alloc(4096,sizeof(float) * max); 
  out = aligned_alloc(4096,sizeof(float) * max); 
  out_check = aligned_alloc(4096,sizeof(float) * max); 

  gen_3d(max,1,1,in,1);
  mprotect(in,sizeof(float)*max,PROT_READ);
  //---------------------------------------------------------

  int benchmarks_failed = 0;

  printf("%2s %7s | %5s %5s %7s %2s | %8s %6s %8s %10s | %7s %7s %7s %7s %8s | %7s %8s\n",
          "T#", "Kernel",
          "Ni","Nj","Nk", "S", 
          "Time(ms)", "CPE", "#Inst(M)", "#VecInst(M)", 
          "#L1(M)", "#L2(M)", "#L3(M)", "#Mem(M)", "MemEn(j)",
          "Speedup", "Energyup");


  if(do_all) {
    for(int t = 0; t < num_trials; ++t) {
      if(t!=0) printf("\n");
      for(int i = 0; i < NUM_TESTS; i++) {
        benchmarks_failed+=run_test(i,check_func);
      }
    }

    if(!cache_profile) {
      printf("Quitting before grade computation, as you're not on lnxsrv07\n");
      exit(0);
    }

    double speedup = ((double)Tests[0].orig_msec / (double)Tests[0].best_msec);
    double enup = ((double)Tests[0].orig_mem / (double)Tests[0].best_mem);

    printf("Speedup: %0.2f\n", speedup);
    printf("Energyup: %0.2f\n", enup);

    float ed=speedup * enup;
    printf("Energy-Delay Improvement: %0.2f\n", ed);


    double speedup1 = ((double)Tests[1].orig_msec / (double)Tests[1].best_msec);
    double enup1 = ((double)Tests[1].orig_mem / (double)Tests[1].best_mem);

    printf("Stencil (Extra Credit) Speedup: %0.2f\n", speedup1);
    printf("Stencil (Extra Credit) Energyup: %0.2f\n", enup1);

    float ed1=speedup1 * enup1;
    printf("Stencil (Extra Credit) Energy-Delay Improvement: %0.2f\n", ed1);

    if(benchmarks_failed) {
      printf("Number of Benchmarks FAILED: %d\n",benchmarks_failed);
      printf("No grade given, because of incorrect execution.\n");
    } else {

      float tg = fmin(grade(ed),100.0);
      printf("Transpose Score: %0.1f\n",tg);
      float sg = 0.25 * fmin(grade_extra(ed1),100.0);
      printf("Stencil (Extra Credit) Score: %0.1f\n",sg);
      printf("Grade: %0.1f\n",tg+sg);
    }
  } else {
    for(int t = 0; t < num_trials; ++t) {
      run_test(test_case-1,check_func);
    }
    double speedup = ((double)Tests[test_case-1].orig_msec / (double)Tests[test_case-1].best_msec);
    printf("Speedup: %0.2f\n\n", speedup);

    double energyup = ((double)Tests[test_case-1].orig_mem / (double)Tests[test_case-1].best_mem);
    printf("Energyup: %0.2f\n\n", energyup);

    printf("Energy-Delay Improvement: %0.2f\n", speedup*energyup);

    printf("No grade given, because only one test is run.\n");
    printf(" ... To see your grade, run all tests with \"-i a\"\n");
  }
}


