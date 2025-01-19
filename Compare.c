// File: c:/bsd/rigel/sort/integersorters/Compare.c
// Date: Fri May 22 09:36:07 2009/ Sat Dec 28 12:23:32 2013/
// Fri Jul 07 13:40:56 2017 / Tue Jan 02 15:57:33 2018
// Thu Dec 12 11:28:59 2024
// (C) OntoOO/ Dennis de Champeaux
/*
Copyright (c) 2018, Dennis de Champeaux.  All rights reserved.

The copyright holders hereby grant to any person obtaining a copy of
this software (the "Software") and/or its associated documentation
files (the Documentation), the irrevocable (except in the case of
breach of this license) no-cost, royalty free, rights to use the
Software for non-commercial evaluation/test purposes only, while
subject to the following conditions:

. Redistributions of the Software in source code must retain the above
copyright notice, this list of conditions and the following
disclaimers.

. Redistributions of the Software in binary form must reproduce the
above copyright notice, this list of conditions and the following
disclaimers in the documentation and/or other materials provided with
the distribution.

. Redistributions of the Documentation must retain the above copyright
notice, this list of conditions and the following disclaimers.

The name of the copyright holder, may not be used to endorse or
promote products derived from this Software or the Documentation
without specific prior written permission of the copyright holder.
 
THE SOFTWARE AND DOCUMENTATION ARE PROVIDED "AS IS", WITHOUT WARRANTY
OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE CONTRIBUTORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
IN CONNECTION WITH THE SOFTWARE OR DOCUMENTATION OR THE USE OF OR
OTHER DEALINGS WITH THE SOFTWARE OR DOCUMENTATION.
*/

/*
Wed Jan 03 18:10:58 2018
Member algorithms have been replaced by better versions. Cut2f has been 
added.  Numerous algorithms have been deprecated.  They may still work, 
but they may require upgrades regarding where pivots are safeguarded.
The real algorithms, for real sorting, are the versions that require 
a comparison function.  They have better parallel versions because they
do the first partitioning in parallel with two threads.
 */

/*
This file has a proof-of-concept implementation of a 4-partition
algorithm cut4 that sorts integer arrays.  A library version needs to
replace the integers with objects that support a compare function.

Since the cut4 overhead is large it delegates smaller arrays to a
two partition sorter cut2f, which in turn delegates smaller arrays to cut2, 
which delegates in turn to quicksort0, and from there to insertionsort. 

Finding good pivot(s) impacts 4 partition sorting like 2-partition
sorting.  The larger the array the more overhead is devoted to finding 
good pivots.

This C-file contains many other sorting algorithms, comparison functions and
infrastructure: 
** The main body has calls to several functions.  Un-comment one of
them, re/compile this file, run it and one obtains output on the screen.
-- The functions come in three groups:
   --- Testing an algorithm
   --- Validating a algorihm
   --- Comparison of two sorting algorithms on arrays of different
   sizes with uniform distributed integer data.

-- Testing of an algorithm consists of filling an array, running an
   algorithm and checking that the output is sorted

-- Validate analgorithm is done by comparing its output against the output
   of another sorting algorithm.  

-- Comparing two algorithms must deal with the limited accuracy of the
   clock() function.  The following procedure was implemented in all
   comparison functions;
   Repeat for increasing array sizes:
      Repeat 3-times:
        * Measure the time to fill X arrays (different inputs)
	* Measure the time to fill and sort X arrays for algorithm P
	* Measure the time to fill and sort X arrays for algorithm Q
	* Subtract from the fill and sort times the fill times
	* Calculate the ratios
   At least two out of three ratios are typically very equal.

   Here a (partial) list of what is available::
   ** Utility functions findValue and check
   ** Test functions for different sort algorithms
   ** Validation functions
   ** Timing comparison functions for two different sort algorithms
   ** fillarray, which fills arrays with uniform random data
   ** quicksort0 itself a hybrid 
   ** insertionsort
   ** cut2, our "best" enhanced Quicksort
   ** cut2x (named cut3duplicates elsewhere), replaced by dflgm
   ** tps a simple two pivot three partition sorter
   ** dflgm Dutch flag type one pivot, three partition sorter,
      which is invoked when duplicates are found/ when label 
      invariants are not satisfied, e.g. when all elements are the same  
   ** cut3x, cut3y, cut3z (replaced by tps)
   // ** cut4s, cut4f, more compact than cut4 but not as fast
   ** cut4 
   ** heapc, heapsort for integers
   ** bentley, Bentley & McIlroy's qsort function, improved and
      the defect in the Cygwin library is repaired 
   ** sortc, calling the broken qsort function in our Cygwin library
   ** dpqSort, the Dual-Pivot Quicksort algorithm
   ** Bentley-McIlroy testbench with the addition of the slopes family
   ** comparison functions for the testbench
   ** Adp_SymPSort and callChensort from Jingchao Chen 
   ** _quicksort/ sortlinux/ callLQ from a linux/gcc library by Douglas C. Schmidt
   ** util functions for compareZeros00, for generating distributions with:
      -- constant data with a varying percentage of uniform noise
      -- sorted data with a varying percentage of uniform noise
      -- inversely sorted data with a varying percentage of uniform noise
   ** mergesort copying with parallel version
   ** mergeSortDDC non-copying mergesort 
   ** pthread functionality for cut4p
   ** cut4p, parallel version of cut4
||||||||||||||||||||||||||||||||||||||||||||||||||||||||
Thu Dec 12 11:29:52 2024
Deleting most and replacing with the modified versions from C7 
 */

/*
++++ delete merge sort from this file +++
*** Hebe
*** AI Journal 1 & 2
*** QS edit
*/

#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

#include "Isort.c"
#include "Hsort.c"
#include "Dsort.c"
#include "D3sort.c"
#include "Qusort.c"
#include "C2sort.c"
#include "C2LR.c"
#include "C4.c"
#include "CD4.c"
#include "C7.c"
#include "C3pp.c"
#include "MergeSort.c"

#include "Bentley.c"
#include "LQ.c"


// #include "Dsort2.c" // not used

// int probeParamCut3x = 1000000; // for parallel version


void fillarray();
void testInsertionsort();
void testHeapSort();
void testD3sort();
// void testQuicksort0();
// void testMyQS();
void testCut2();
void testC2LR();
void testCut4();
void testCD4();
void testC7();
void testLinux();
void testSortcB();
void testC3pp();

void testDpqSort();

// void testPart3();

void validateQuicksort0();
void validateCut2();
void validateC2LR();
void validateCut4();
void validateCD4();
void validateC7();
void validateC3pp();

/*
void validateHeapSort();
 void validateSortc();
void validateSortcB();
*/
// void validateDpqSort();
/*
void validatePart3();
void validateChenSort();
*/
// void exploreParams();
void timeTest();
void timeTest2();


// void showSuffle2();

// void musserTest();
void heapc();
// void quicksort0();
// int med();
// void iswap();

/*

void tps();
void tpsc();
*/

void dflglTest();
void dflgmTest();

void insertionsort();
void cut4();
void cut4c();
void cut4d();
void cut4dc();
void sortlinux();
void sortcB();

void sortc();
void dpqSort();
void mergeSort();
// void chenSort();
// void parMergeSort2();
// void parMergeSort4();
void callLQ();
void callBentley();
// void callChensort();
// void callMergesort();
void callDpq();
// void part3();

void dflgm();
// void cut4p();
// void cut4pc();
// void showDistribution();

int main (int argc, char *argv[]) {
  printf("Running compare ...\n");
  // testInsertionsort();
  // testHeapSort();
  // testD3sort();
  // testQuicksort0();
  // testCut2();
  // testC2LR();
  // testCut4();
  // testCD4();
  // testC7();
  // testLinux();
  // testSortcB();
  // testC3pp();

  // testCut4p(); // parallel
  // testSortc(); // ??
  // testDpqSort();
  // testPart3();

  // validateQuicksort0();
  // validateCut2();
  // validateC2LR();
  // validateCut4();
  // validateCD4();
  // validateC7();
  // validateC3pp();
  // validateMyQS();
  // validateDflglTest();
  // validateDflgmTest();
  // validateCut4p();
  // validateHeapSort();
  // validateSortc();
  // validateSortcB();
  // validateDpqSort();
  // validatePart3();
  // validateMergeSort();
  // exploreParams();

  timeTest();
  // timeTest2();

  // compareDpqSortAgainstDdCSort2(); // using Bentley stuff <<<<<<<<<<<<<<<<<<<<<
  // showSuffle2();
  // compareSortcBAgainstCut4(); // using Bentley stuff +++++++
  // compareSortcQAgainstCut2(); // !!!!native qsort/ LQ/ B&M <-> cut2 using Bentley stuff
  // compareSortcBAgainstCut2(); // B&M <-> cut2 using Bentley stuff  <<<<<<<<<<
  // compareChenSortAgainstCut2(); // using Bentley stuff
  // musserTest();
  // compare00LQAgainstFourSort();
  // compare00BentleyAgainstFourSort();
  // compare00ChenAgainstFourSort();
  // compare00MergeAgainstFourSort();
  // compare00DpqAgainstFourSort();
  // compare00DpqAgainstTPS();
  // compare00DpqAgainstSixSort();
  // showDistribution();
  return 0;
} // end main
/*
int findValue(int *A, int N, int M, int value) {
  int i;
  for (i = N; i <= M; i++) 
    if (A[i] == value) return i;
  return -1;
}
*/

void check(int *A, int N, int M) 
{
  int i, x, y;
  int cnt = 0;
  for (i = N + 1; i <= M; i++) {
    x = A[i-1]; y = A[i];
    if (y < x) { cnt++;
      printf("Error element i=%i less than preceding element: A[i]=%i\n",i,y);
    }
  }
  printf("Check finished # errors: %i \n", cnt);
} // end check


// int size2 = 1024*1024*16 *8;
// int size2 = 1024*1024*16 *2;
int size2 = 1024*1024*16;
// int size2 = 1024 * 512; 
// int size2 = 1024;

void sortcB(int *A, int N, int M)
{
  int compareInt();
  bentley(&A[N], M-N+1, sizeof(int), compareInt);
} // end sortcB

void callBentley(int *A, int size, int (*compare1)() ) {
  bentley(A, size, sizeof(int), compare1);
} // end callBentley



void testAlgorithm(char* label, void (*alg1)() ) {
  int siz = size2;
  // int siz = 1024*1024*16;
  // int siz = 1024*1024*8; 
  // int siz = 1024*1024*4; 
  // int siz = 1024*1024*3;
  // int siz = 1024*1024*2;
  // int siz = 1024*1024; 
  // int siz = 1024*512;
  // int siz = 1024*256;
  // int siz = 1024*128; 
  // int siz = 1024 * 64;
  // int siz = 1024 * 32;
  // int siz = 1024 * 16;
  // int siz = 1024 * 8;
  // int siz = 1024 * 4;
  // int siz = 1024 * 3;
  // int siz = 1024 * 2;
  // int siz = 1024; 
  // int siz = 512;
  // int siz = 200;

  printf("%s on size %d \n", label, siz);
  // int A[siz];
  int *A = malloc (sizeof(int) * siz);
  fillarray(A, siz, 100);
  (*alg1)(A, 0, siz-1);
  check(A, 0, siz-1);
  free(A);
} // end testAlgorithm

void testInsertionsort() {
  testAlgorithm("Running  testInsertionsort...", insertionsort);
}

void testHeapSort() {
  testAlgorithm("Running testHeapSort ...", heapc);
}

void testD3sort() {
  testAlgorithm("Running testD3Sort ...", dflgm0);
}

void testQuicksort0() {
  testAlgorithm("Running testQuicksort0 ...", quicksort0);
}

void testCut2() {
  testAlgorithm("Running testCut2 ...", cut2);
}

void testC2LR() {
  testAlgorithm("Running testC2LR ...", cut2lr);
}
void testCut4() {
  testAlgorithm("Running testCut4 ...", cut4);
}

void testCD4() {
  testAlgorithm("Running testCD4 ...", cut4d);
}

void testC7() {
  testAlgorithm("Running testC7 ...", cut7);
}

void testC3pp() {
  testAlgorithm("Running testC3pp ...", c3pp);
}


void testLinux() {
 testAlgorithm("Running testLinux ...", sortlinux);
}

void testSortcB() {
 testAlgorithm("Running testSortcB ...", sortcB);
}
void testDpqSort() {
  testAlgorithm("Running testDpqSort ...", dpqSort);
}

void testMergeSort() {
  testAlgorithm("Running testMergeSort ...", mergeSort);
} // end testMergeSort

void validateAlgorithm(char* label, void (*alg1)(), void (*alg2)() ) {
int siz = size2;
  // int siz = 16 * 1024 * 1024;
  // int siz = 3 * 1024 * 1024;;
  // int siz = 2 * 1024 * 1024;
  // int siz = 1024 * 1024;
  // int siz = 512 * 1024;
  // int siz = 256 * 1024;
  // int siz = 128 * 1024;
  // int siz = 64 * 1024;
  // int siz = 32 * 1024;
  // int siz = 16 * 1024;
  // int siz = 1024;
  // int siz = 512;
  printf("siz: %i %s", siz, label);
  // int A[siz];
  int *A = malloc (sizeof(int) * siz);
  fillarray(A, siz, 100);
  (*alg1)(A, 0, siz-1);
  int *B = malloc (sizeof(int) * siz);
  fillarray(B, siz, 100);
  (*alg2)(B, 0, siz-1);
  int i; int foundError = 0;
  for (i = 0; i < siz; i++)
    if ( A[i] != B[i] ) {
      printf("validate error i: %i A[i]: %i B[i]: %d \n", i, A[i], B[i]);
      foundError = 1;
    }
  if ( !foundError ) 
    printf("NO error found ...\n");
  free(A); free(B);
} // end validateAlgorithm

void validateQuicksort0() {
  validateAlgorithm("Running validate Quicksort0...\n",
		    heapc, quicksort0);
}

void validateCut2() {
  validateAlgorithm("Running validate Cut2 ...\n",
		    quicksort0, cut2);
}

void validateC2LR() {
  validateAlgorithm("Running validate C2LR ...\n",
		    cut2, cut2lr);
}

void validateCut4() {
  validateAlgorithm("Running validate Cut4 ...\n",
		    cut2lr, cut4);
}

void validateCD4() {
  validateAlgorithm("Running validate CD4 ...\n",
		    cut4, cut4d);
}

void validateC7() {
  validateAlgorithm("Running validate C7 ...\n",
		    cut4, cut7);
}

void validateC3pp() {
  validateAlgorithm("Running validate C3pp ...\n",
		    cut2, c3pp);
}

void validateDpqSort() {
  validateAlgorithm("Running validate DpqSort ...\n",
		    cut2, dpqSort);
}

// /*
void validateMergeSort() {
  validateAlgorithm("Running validate MergeSort ...\n",
		    cut2, mergeSort);
} // end validateMergeSort()
// */


/* Validating parMergeSort2 with two threads
void validateMergeSort0() {
  validateAlgorithm("Running validate parMergeSort2 ...\n",
		    cut2, parMergeSort2);
} // end validateMergeSort0()
*/
/* Validating parMergeSort2 with four threads
void validateMergeSort0() {
  validateAlgorithm("Running validate parMergeSort4 ...\n",
		    cut2, parMergeSort4);
} // end validateMergeSort0()
// */

void timeTest() {
  // run an algorithm and report the time used
  printf("timeTest() on size: ");
  int cut4Time, T;
  int seed;
  //  int seedLimit = 300000;
  int seedLimit = 1;
  /* One can increase reps provided other processes do not 
     mess up the average.
     Just check each new timing generated that it has not been 
     'contamined' by a back ground process like a browser.
     It is machine dependent.  An i3 box had 30 stable 
     measurements.
   */
  int reps = 5;
  int z;
  // int k;
  int siz = size2;
  // int siz = 1024 * 1024 * 16;
  // siz = 1024 *1024 *2;
  printf("%i\n", siz);
  int *A = malloc (sizeof(int) * siz);

  // warm up the process
  fillarray(A, siz, 666); 
  int sumTimes = 0;
  float sumTimesf = 0;
  for (z = 0; z < reps; z++) { // repeat to check stability
    cut4Time = 0;
    int TFill = clock();
    for (seed = 0; seed < seedLimit; seed++) 
      fillarray(A, siz, seed);
      // for ( k = 0; k < siz; k++ ) A[k] = 0;
      // for ( k = 0; k < siz; k++ ) A[k] = k%5;
      // for ( k = 0; k < siz; k++ ) A[k] = siz-k;
    TFill = clock() - TFill;
    T = clock();
    for (seed = 0; seed < seedLimit; seed++) { 
      fillarray(A, siz, seed);
      // for ( k = 0; k < siz; k++ ) A[k] = 0;
      // for ( k = 0; k < siz; k++ ) A[k] = k%5;
      // for ( k = 0; k < siz; k++ ) A[k] = siz-k;

      // heapc(A, 0, siz-1);
      // dflgm0(A, 0, siz-1);
      // quicksort0(A, 0, siz-1);
      // cut2(A, 0, siz-1);  
      // cut2lr(A, 0, siz-1); 
      // cut4d(A, 0, siz-1); 
      dpqSort(A, 0, siz-1);
      // cut4(A, 0, siz-1);  
      // cut7(A, 0, siz-1); 
      // c3pp(A, 0, siz-1); // my 2-pivot
      // dflglTest(A, 0, siz-1);
      // dflgmTest(A, 0, siz-1);  
      // mergeSort(A, 0, siz-1);  

      // sortlinux(A, 0, siz-1);    // LQ
      // sortcB(A, 0, siz-1);    
      // part3(A, 0, siz-1);    
    }
    cut4Time = (clock() - T - TFill);
    float ct = (float) cut4Time;
    printf("algorithm time: %i  %f \n", z, ct/CLOCKS_PER_SEC);
    sumTimes = sumTimes + cut4Time;
    // printf("z %i average time: %f\n", z, sumTimesf/(1+z));
  }

  sumTimesf = (float)sumTimes/ reps;
  printf("%s %f %s", "average time: ", (sumTimesf/CLOCKS_PER_SEC), "\n");
  free(A);

} // end timeTest()

void timeTest2() {
  // run an algorithm and report the time used on two distributions
  printf("timeTest2() cut4\n");
  int cut4Time, T, TFill;
  int seed;
  //  int seedLimit = 300000;
  int seedLimit = 30;
  int z, k;
  // int siz = 1024 * 1024 * 16;
  int siz = 1024 * 1024;
  int *A = malloc(sizeof(int) * siz);

  // warm up the process
  fillarray(A, siz, 666); 
  int sumTimes = 0;
  for (z = 0; z < 3; z++) { // repeat to check stability
    cut4Time = 0;
    TFill = clock();
    for (seed = 0; seed < seedLimit; seed++) 
      fillarray(A, siz, seed);
      // for ( k = 0; k < siz; k++ ) A[k] = 0;
      // for ( k = 0; k < siz; k++ ) A[k] = k%5;
      // for ( k = 0; k < siz; k++ ) A[k] = siz-k;
    TFill = clock() - TFill;
    T = clock();
    for (seed = 0; seed < seedLimit; seed++) { 
      fillarray(A, siz, seed);
      cut4(A, 0, siz-1);  
    }
    cut4Time = clock() - T - TFill;
    printf("cut4Time: %d \n", cut4Time);
    sumTimes = sumTimes + cut4Time;
  }
  printf("%s %d %s", "sumTimes: ", sumTimes, "\n");
  // ---------- second distribution --------------------
  sumTimes = 0;
  for (z = 0; z < 3; z++) { // repeat to check stability
    cut4Time = 0;
    TFill = clock();
    for (seed = 0; seed < seedLimit; seed++) 
      // fillarray(A, siz, seed);
      // for ( k = 0; k < siz; k++ ) A[k] = 0;
      // for ( k = 0; k < siz; k++ ) A[k] = k%5;
      for ( k = 0; k < siz; k++ ) A[k] = siz-k;
      // for ( k = 0; k < siz; k++ ) A[k] = k;
    TFill = clock() - TFill;
    T = clock();
    for (seed = 0; seed < seedLimit; seed++) { 
      // fillarray(A, siz, seed);
      // for ( k = 0; k < siz; k++ ) A[k] = 0;
      // for ( k = 0; k < siz; k++ ) A[k] = k%5;
      for ( k = 0; k < siz; k++ ) A[k] = siz-k;
      // for ( k = 0; k < siz; k++ ) A[k] = k;
      cut4(A, 0, siz-1);  
    }
    cut4Time = clock() - T - TFill;
    printf("cut4Time: %d \n", cut4Time);
    sumTimes = sumTimes + cut4Time;
  }
  printf("%s %d %s", "sumTimes: ", sumTimes, "\n");
  free(A);

} // end timeTest2()

void compareAlgorithms(char *label, void (*alg1)(), void (*alg2)() ) {
  printf(label);
  int alg1Time, alg2Time, T;
  int seed;
  int z;
  // int siz = 1024;
  // int seedLimit = 32 * 1024;
  int siz = 1024 * 1024;
  int seedLimit = 32;
  int limit = 1024 * 1024 * 16 + 1;
  while (siz <= limit) {
    printf("%s %d %s %d %s", "siz: ", siz, " seedLimit: ", seedLimit, "\n");
    // int A[siz];
    int *A = malloc (sizeof(int) * siz);
    // warm up the process
    for (seed = 0; seed < seedLimit; seed++) 
      fillarray(A, siz, seed);
    for (z = 0; z < 3; z++) { // repeat to check stability
      alg1Time = 0; alg2Time = 0;
      int TFill = clock();
      for (seed = 0; seed < seedLimit; seed++) 
	fillarray(A, siz, seed);
      TFill = clock() - TFill;
      T = clock();
      for (seed = 0; seed < seedLimit; seed++) { 
	fillarray(A, siz, seed);
	(*alg1)(A, 0, siz-1); 
      }
      alg1Time = clock() - T - TFill;
      T = clock();
      for (seed = 0; seed < seedLimit; seed++) { 
	fillarray(A, siz, seed);
	(*alg2)(A, 0, siz-1);
      }
      alg2Time = clock() - T - TFill;
      printf("%s %d %s", "siz: ", siz, " ");
      printf("%s %d %s", "alg1Time: ", alg1Time, " ");
      printf("%s %d %s", "alg2Time: ", alg2Time, " ");
      float frac = 0;
      if ( alg1Time != 0 ) frac = alg2Time / ( 1.0 * alg1Time );
      printf("%s %f %s", "frac: ", frac, "\n");
    }
    free(A);
    siz = siz * 2;
    seedLimit = seedLimit / 2;
  }
} // end compareAlgorithms

/*
void musserTest() {
  char* label = "Musser test";
  // (char *label, int (*alg1)(), int (*alg2)() ) {
  printf(label);
  int alg1Time, alg2Time, T;
  int seed;
  int i, z;
  int siz = 1024;
  int limit = 1024 * 1024 * 16 + 1;
  int seedLimit = 32 * 1024;
  while (siz <= limit) {
    printf("%s %d %s %d %s", "siz: ", siz, " seedLimit: ", seedLimit, "\n");
    // int A[siz];
    int *A = malloc (sizeof(int) * siz);
    // warm up the process
    for (seed = 0; seed < seedLimit; seed++) 
      fillarray(A, siz, seed);
    for (z = 0; z < 3; z++) { // repeat to check stability
      alg1Time = 0; alg2Time = 0;
      int TFill = clock();
      for (seed = 0; seed < seedLimit; seed++) 
	fillarray(A, siz, seed);
      TFill = clock() - TFill;
      T = clock();
      for (seed = 0; seed < seedLimit; seed++) { 
	fillarray(A, siz, seed);
	cut4(A, 0, siz-1); 
      }
      alg1Time = clock() - T - TFill;
      T = clock();
      for (seed = 0; seed < seedLimit; seed++) { 
	// fillarray(A, siz, seed);
	int k = siz/2;
	for (i = 0; i < k; i=i+2) A[i] = i;
	for (i = 1; i < k; i=i+2) A[i] = i-1+k;
	for (i = k; i < siz; i++) A[i] = 1 + 2 * (i-k);
	cut4(A, 0, siz-1);
      }
      alg2Time = clock() - T - TFill;
      printf("%s %d %s", "siz: ", siz, " ");
      printf("%s %d %s", "alg1Time: ", alg1Time, " ");
      printf("%s %d %s", "alg2Time: ", alg2Time, " ");
      float frac = 0;
      if ( alg1Time != 0 ) frac = alg2Time / ( 1.0 * alg1Time );
      printf("%s %f %s", "frac: ", frac, "\n");
    }
    free(A);
    siz = siz * 2;
    seedLimit = seedLimit / 2;
  }
} // end musserTest()
*/

void fillarray(int *A, int lng, int startv) {
  const int range = 1024*1024*32;
  srand(startv);
  int i;
  for ( i = 0; i < lng; i++) A[i] = rand()%range; 
  // for ( i = 0; i < lng; i++) A[i] = 0;
  /* for testing dflgm
  int val = 1000000000;
  A[(lng-1)/2] = val;
  int delta = lng/10;
  for ( i = 0; i < lng; i = i + delta ) A[i] = val;
  */
} // end of fillarray



int compareInt (const void * a, const void * b)
{
  // return ( *(int*)a - *(int*)b );
  int x = *(int*)a; int y = *(int*)b;
  return ( x < y ? -1 : ( x == y ? 0 : 1));
} // end compareInt


// sortc(int *A, int size)
void sortc(int *A, int N, int M)
{
  qsort(&A[N], M-N+1, sizeof(int), compareInt);
} // end sortc


// dpqSort is the Dual-Pivot Quicksort algorithm

int INSERTION_SORT_THRESHOLD = 47;

void dualPivotQuicksort();
void dpqSort(int *a, int left, int right) {
  // Use insertion sort on tiny arrays
  if (right - left + 1 < INSERTION_SORT_THRESHOLD) {
    int k, ak, j;
    for (k = left + 1; k <= right; k++) {
      ak = a[k];
      for (j = k - 1; j >= left && ak < a[j]; j--) {
	a[j + 1] = a[j];
      }
      a[j + 1] = ak;
    }
  } else { // Use Dual-Pivot Quicksort on large arrays
    dualPivotQuicksort(a, left, right);
  }
} // end dpqSort


/**
 * Sorts the specified range of the array by Dual-Pivot Quicksort.
 *
 * @param a the array to be sorted
 * @param left the index of the first element, inclusive, to be sorted
 * @param right the index of the last element, inclusive, to be sorted
 * @param leftmost indicates if this part is the leftmost in the range <---  gone
 */

void dualPivotQuicksort(int *a, int left, int right) {
  int length = right - left + 1;
  
  // Inexpensive approximation of length / 7
  // int seventh = (length >> 3) + (length >> 6) + 1;
  int seventh = length/7;
  
  /*
   * Sort five evenly spaced elements around (and including) the
   * center element in the range. These elements will be used for
   * pivot selection as described below. The choice for spacing
   * these elements was empirically determined to work well on
   * a wide variety of inputs.
   */
  // int e3 = (left + right) >>> 1; // The midpoint
  int e3 = left + (length>>1); // (left + right)/2;
  int e2 = e3 - seventh;
  int e1 = e2 - seventh;
  int e4 = e3 + seventh;
  int e5 = e4 + seventh;
  
  // Sort these elements using insertion sort
  if (a[e2] < a[e1]) { int t = a[e2]; a[e2] = a[e1]; a[e1] = t; }
  
  if (a[e3] < a[e2]) { int t = a[e3]; a[e3] = a[e2]; a[e2] = t;
    if (t < a[e1]) { a[e2] = a[e1]; a[e1] = t; }
  }
  if (a[e4] < a[e3]) { int t = a[e4]; a[e4] = a[e3]; a[e3] = t;
    if (t < a[e2]) { a[e3] = a[e2]; a[e2] = t;
      if (t < a[e1]) { a[e2] = a[e1]; a[e1] = t; }
    }
  }
  if (a[e5] < a[e4]) { int t = a[e5]; a[e5] = a[e4]; a[e4] = t;
    if (t < a[e3]) { a[e4] = a[e3]; a[e3] = t;
      if (t < a[e2]) { a[e3] = a[e2]; a[e2] = t;
	if (t < a[e1]) { a[e2] = a[e1]; a[e1] = t; }
      }
    }
  }
  
  // Pointers
  int less  = left;  // The index of the first element of center part
  int great = right; // The index before the first element of right part
  
  if (a[e1] != a[e2] && a[e2] != a[e3] && a[e3] != a[e4] && a[e4] != a[e5]) {
    /*
     * Use the second and fourth of the five sorted elements as pivots.
     * These values are inexpensive approximations of the first and
     * second terciles of the array. Note that pivot1 <= pivot2.
     */
    int pivot1 = a[e2];
    int pivot2 = a[e4];
    
    /*
     * The first and the last elements to be sorted are moved to the
     * locations formerly occupied by the pivots. When partitioning
     * is complete, the pivots are swapped back into their final
     * positions, and excluded from subsequent sorting.
     */
    a[e2] = a[left];
    a[e4] = a[right];
    
    /*
     * Skip elements, which are less or greater than pivot values.
     */
    while (a[++less] < pivot1);
    while (a[--great] > pivot2);
  
    /*
     * Partitioning:
     *
     *   left part           center part                   right part
     * +--------------------------------------------------------------+
     * |  < pivot1  |  pivot1 <= && <= pivot2  |    ?    |  > pivot2  |
     * +--------------------------------------------------------------+
     *               ^                          ^       ^
     *               |                          |       |
     *              less                        k     great
     *
     * Invariants:
     *
     *              all in (left, less)   < pivot1
     *    pivot1 <= all in [less, k)     <= pivot2
     *              all in (great, right) > pivot2
     *
     * Pointer k is the first index of ?-part.
     */
    int k, ak;
  outer:
    for (k = less - 1; ++k <= great; ) {
      ak = a[k];
      if (ak < pivot1) { // Move a[k] to left part
	a[k] = a[less];
	/*
	 * Here and below we use "a[i] = b; i++;" instead
	 * of "a[i++] = b;" due to performance issue.
	 */
	a[less] = ak;
	++less;
      } else if (ak > pivot2) { // Move a[k] to right part
	while (a[great] > pivot2) {
	  if (great-- == k) {
	    // break outer;
	    goto outer;
	  }
	}
	if (a[great] < pivot1) { // a[great] <= pivot2
	  a[k] = a[less];
	  a[less] = a[great];
	  ++less;
	} else { // pivot1 <= a[great] <= pivot2
	  a[k] = a[great];
	}
	/*
	 * Here and below we use "a[i] = b; i--;" instead
	 * of "a[i--] = b;" due to performance issue.
	 */
	a[great] = ak;
	--great;
      }
    }
  
    // Swap pivots into their final positions
    a[left]  = a[less  - 1]; a[less  - 1] = pivot1;
    a[right] = a[great + 1]; a[great + 1] = pivot2;
    
    // Sort left and right parts recursively, excluding known pivots
    dpqSort(a, left, less - 2);
    dpqSort(a, great + 2, right);
    /* Tests suggest (DdC) to simply sort the middle segment with:
       dpqSort(a, less, great);
       return;
    */
  
    /*
     * If center part is too large (comprises > 4/7 of the array),
     * swap internal pivot values to ends.
     */
    if (less < e1 && e5 < great) {
      /*
       * Skip elements, which are equal to pivot values.
       */
      while (a[less] == pivot1) {
	++less;
      }
      
      while (a[great] == pivot2) {
	--great;
      }
      
      /*
       * Partitioning:
       *
       *   left part         center part                  right part
       * +----------------------------------------------------------+
       * | == pivot1 |  pivot1 < && < pivot2  |    ?    | == pivot2 |
       * +----------------------------------------------------------+
       *              ^                        ^       ^
       *              |                        |       |
       *             less                      k     great
       *
       * Invariants:
       *
       *              all in (*,  less) == pivot1
       *     pivot1 < all in [less,  k)  < pivot2
       *              all in (great, *) == pivot2
       *
       * Pointer k is the first index of ?-part.
       */
      int k, ak;
    outer2:
      for (k = less - 1; ++k <= great; ) {
	ak = a[k];
	if (ak == pivot1) { // Move a[k] to left part
	  a[k] = a[less];
	  a[less] = ak;
	  ++less;
	} else if (ak == pivot2) { // Move a[k] to right part
	  while (a[great] == pivot2) {
	    if (great-- == k) {
	      // break outer;
	      goto outer2;
	    }
	  }
	  if (a[great] == pivot1) { // a[great] < pivot2
	    a[k] = a[less];
	    /*
	     * Even though a[great] equals to pivot1, the
	     * assignment a[less] = pivot1 may be incorrect,
	     * if a[great] and pivot1 are floating-point zeros
	     * of different signs. Therefore in float and
	     * double sorting methods we have to use more
	     * accurate assignment a[less] = a[great].
	     */
	    // a[less] = pivot1;
	    a[less] = a[great]; // mandatory for objects
	    ++less;
	  } else { // pivot1 < a[great] < pivot2
	    a[k] = a[great];
	  }
	  a[great] = ak;
	  --great;
	}
      }
    }
    
    // Sort center part recursively
    dpqSort(a, less, great);
  
  } else { // Partitioning with one pivot
    /*
     * Use the third of the five sorted elements as pivot.
     * This value is inexpensive approximation of the median.
     */
    int pivot = a[e3];
    
    /*
     * Partitioning degenerates to the traditional 3-way
     * (or "Dutch National Flag") schema:
     *
     *   left part    center part              right part
     * +-------------------------------------------------+
     * |  < pivot  |   == pivot   |     ?    |  > pivot  |
     * +-------------------------------------------------+
     *              ^              ^        ^
     *              |              |        |
     *             less            k      great
     *
     * Invariants:
     *
     *   all in (left, less)   < pivot
     *   all in [less, k)     == pivot
     *   all in (great, right) > pivot
     *
     * Pointer k is the first index of ?-part.
     */
    int k, ak;
    for (k = less; k <= great; ++k) {
      if (a[k] == pivot) {
	continue;
      }
      ak = a[k];
      if (ak < pivot) { // Move a[k] to left part
	a[k] = a[less];
	a[less] = ak;
	++less;
      } else { // a[k] > pivot - Move a[k] to right part
	while (a[great] > pivot) {
	  --great;
	}
	if (a[great] < pivot) { // a[great] <= pivot
	  a[k] = a[less];
	  a[less] = a[great];
	  ++less;
	} else { // a[great] == pivot
	  /*
	   * Even though a[great] equals to pivot, the
	   * assignment a[k] = pivot may be incorrect,
	   * if a[great] and pivot are floating-point
	   * zeros of different signs. Therefore in float
	   * and double sorting methods we have to use
	   * more accurate assignment a[k] = a[great].
	   */
	  // a[k] = pivot;
	  a[k] = a[great]; // mandatory for objects
	}
	a[great] = ak;
	--great;
      }
    }
    
    /*
     * Sort left and right parts recursively.
     * All elements from center part are equal
     * and, therefore, already sorted.
     */
    dpqSort(a, left, less - 1);
    dpqSort(a, great + 1, right);
  }
} // end dualPivotQuicksort


void callDpq(int *A, int size) {
  dpqSort(A, 0, size-1);
} // end callDpq

// Bentley file content generators

// Here the modifiers
void reverse2();
void reverse(int *A, int n) {
  reverse2(A, 0, n-1);
}
void reverse2(int *A, int start, int end) {
  int x;
  while ( start < end ) {
    x = A[start]; A[start++] = A[end]; A[end--] = x;
  }    
}
void reverseFront(int *A, int n) {
  reverse2(A, 0, n/2);
}
void reverseBack(int *A, int n) {
  reverse2(A, n/2, n-1);
}
void tweakSort(int *A, int n) {
  cut4(A, 0, n-1);
  // cut2(A, 0, n-1);
}
void dither(int *A, int n) {
  int k;
  // for (k = 0; k < n; k++) A[k] = A[k] + (k % 5);
  for (k = 0; k < n; k++) A[k] = A[k] - (k % 5);
}

// Here the families
void sawtooth(int *A, int n, int m, int tweak) {
  // int *A = malloc (sizeof(int) * n);
  int k;
  // for (k = 0; k < n; k++) A[k] = k % m;
  for (k = 0; k < n; k++) A[k] = -(k % m);
  if ( tweak <= 0 ) return;
  if ( tweak == 1 ) { reverse(A, n); return; }
  if ( tweak == 2 ) { reverseFront(A, n); return; }
  if ( tweak == 3 ) { reverseBack(A, n); return; }
  if ( tweak == 4 ) { tweakSort(A, n); return; }
  dither(A, n);
} // end sawtooth

void rand2(int *A, int n, int m, int tweak, int seed) {
  srand(seed);
  int k;
  // for (k = 0; k < n; k++) A[k] = rand() % m;
  for (k = 0; k < n; k++) A[k] = -(rand() % m);
  if ( tweak <= 0 ) return;
  if ( tweak == 1 ) { reverse(A, n); return; }
  if ( tweak == 2 ) { reverseFront(A, n); return; }
  if ( tweak == 3 ) { reverseBack(A, n); return; }
  if ( tweak == 4 ) { tweakSort(A, n); return; }
  dither(A, n);
} // end rand2

void stagger(int *A, int n, int m, int tweak) {
  int k;
  // for (k = 0; k < n; k++) A[k] = (k*m+k) % n;
  for (k = 0; k < n; k++) A[k] = -((k*m+k) % n);
  if ( tweak <= 0 ) return;
  if ( tweak == 1 ) { reverse(A, n); return; }
  if ( tweak == 2 ) { reverseFront(A, n); return; }
  if ( tweak == 3 ) { reverseBack(A, n); return; }
  if ( tweak == 4 ) { tweakSort(A, n); return; }
  dither(A, n);
} // end stagger

void plateau(int *A, int n, int m, int tweak) {
  int k;
  // for (k = 0; k < n; k++) A[k] = ( k <= m ? k : m );
  for (k = 0; k < n; k++) A[k] = -( k <= m ? k : m );
  if ( tweak <= 0 ) return;
  if ( tweak == 1 ) { reverse(A, n); return; }
  if ( tweak == 2 ) { reverseFront(A, n); return; }
  if ( tweak == 3 ) { reverseBack(A, n); return; }
  if ( tweak == 4 ) { tweakSort(A, n); return; }
  dither(A, n);
} // end plateau

// We did NOT use this generator
void shuffle(int *A, int n, int m, int tweak, int seed) {
  srand(seed);
  int k, i, j;
  i = j = 0;
  for (k = 0; k < n; k++) {
    if ( 0 == ( rand() %m ) ) { 
      j = j+2; A[k] = j;
    } else {
      i = i+2; A[k] = i; 
    }
    A[k] = -A[k]; // or delete
  }
  if ( tweak <= 0 ) return;
  if ( tweak == 1 ) { reverse(A, n); return; }
  if ( tweak == 2 ) { reverseFront(A, n); return; }
  if ( tweak == 3 ) { reverseBack(A, n); return; }
  if ( tweak == 4 ) { tweakSort(A, n); return; }
  dither(A, n);
} // end shuffle

void slopes(int *A, int n, int m, int tweak) {
  int k, i, b, ak;
  i = k = b = 0; ak = 1;
  while ( k < n ) {
    if (1000000 < ak) ak = k; else
    if (ak < -1000000) ak = -k;

    A[k] = -(ak + b); ak = A[k];
    k++; i++; b++;
    if ( 11 == b ) { b = 0; }
    if ( m == i ) { ak = ak*2; i = 0; }
  }
  if ( tweak <= 0 ) return;
  if ( tweak == 1 ) { reverse(A, n); return; }
  if ( tweak == 2 ) { reverseFront(A, n); return; }
  if ( tweak == 3 ) { reverseBack(A, n); return; }
  if ( tweak == 4 ) { tweakSort(A, n); return; }
  dither(A, n);
} // end slopes

// We did NOT use this generator
void shuffle2(int n, int m, int seed) {
  srand(seed);
  int k, i, j, z, cnt;
  i = j = cnt = 0; 
  for (k = 0; k < n; k++) {
    if ( 0 == ( rand() %m ) ) { 
      j = j+2; z = j;
    } else {
      i = i+2; z = i; 
    }
  }
} // end shuffle2

void showSuffle2() {
  shuffle2(500, 64, 777);
} // end showSuffle2

// Kushagra 3-pivot sorter

void iswap(int p, int q, int *x) {
  int t = x[p];
  x[p] = x[q];
  x[q] = t;
} // end of iswap

void partition3();
void part3(int *A, int N, int M) {
  partition3(A, N, M);
} // end part3

void partition3(int *A, int left, int right) {
  int L;
Again:
  // printf("partition3 left %i right %i\n", left, right);
    L = right - left;
    if ( L <= 12 ) { 
      insertionsort(A, left, right);
      return;
    }
    int e = L/6;
    int mid = left + L/2;
                               // B[0] = A[left]; 
    iswap(left+1, left+e, A);  // B[1] = A[left+e]; 
    iswap(left+2, mid-e, A);   // B[2] = A[mid-e];
    iswap(left+3, mid, A);     // B[3] = A[mid];
    iswap(left+4, mid+e, A);   // B[4] = A[mid+e]; 
    iswap(left+5, right-e, A); // B[5] = A[right-e]; 
    iswap(left+6, right, A);   // B[6] = A[right];
    // insertionSortB(B, 0, 6);
    insertionsort(A, left, left + 6);
    // A[left] = B[0]; A[left+e] = B[1]; A[mid-e] = B[2];
    // A[mid] = B[3];
    // A[mid+e] = B[4]; A[right-e] = B[5]; A[right] = B[6];
    iswap(left, left+1, A); // iswap(left, left+e, A);   // -> p
    iswap(left+1, left+3, A); // iswap(left+1, mid, A);    // -> q
    iswap(right, left+5, A);// iswap(right, right-e, A); // -> r 

    int a = left + 2; int b = a;
    int c = right - 1; int d = c;
    int p = A[left]; int q = A[left + 1]; int r = A[right];
    while ( b <= c ) {
        while ( A[b] < q && b <= c ) {
        // while ( compar(A[b], q) < 0 && b <= c ) {
	   if ( A[b] < p ) {
	   // if ( compar(A[b], p) < 0 ) {
              // swap(A[a], A[b]); 
	      iswap(a, b, A); 
              a++;
           }
           b++;
        }
        while ( A[c] > q && b <= c ) {
	// while ( compar(A[c], q) > 0 && b <= c ) {
	   if ( A[c] > r ) {
	   // if ( compar(A[c], r) > 0 ) {
              // swap(A[c], A[d]);
	      iswap(c, d, A); 
              d--;
           }
           c--;
        }
        if ( b <= c ) {
           if ( A[b] > r ) {
	   // if ( compar(A[b], r) > 0 ) {
             if ( A[c] < p ) {
	     // if ( compar(A[c], p) < 0 ) {
                // swap(A[b],A[a]); swap(A[a],A[c]);
		iswap(a, b, A); iswap(a, c, A);   		    
                a++;
             } else
                // swap(A[b],A[c]);
		iswap(c, b, A);
             // swap(A[c],A[d]); 
	     iswap(c, d, A); 
             b++; c--; d--;
          } else {
             if ( A[c] < p ) {
	     // if ( compar(A[c], p) < 0 ) {
                // swap(A[b],A[a]); swap(A[a],A[c]);
		iswap(a, b, A); iswap(a, c, A);   
                a++;
             } else
                // swap(A[b],A[c]);
		iswap(b, c, A);  
             b++; c--;
          }
        }
    } 
    a--; b--; c++; d++;
    // swap(A[left + 1],A[a]); swap(A[a],A[b]);
    iswap(left+1, a, A); iswap(a, b, A);
    a--;
    // swap(A[left],A[a]); swap(A[right],A[d]);
    iswap(left, a, A); iswap(right, d, A);
    // recursive calls & tail recursion
    if ( b - left <= right - b ) {
       partition3(A, left, a);
       partition3(A, a+1, b);
       if ( b - d <= right - d ) {
       	  partition3(A, b+1, d-1);
	  left = d; 
	  goto Again;
       } else {
	  partition3(A, d, right);
	  left = b+1; right = d-1;
	  goto Again;
       }
    } else {
       partition3(A, b+1, d-1);
       partition3(A, d, right);
       if ( a - left <= b - a ) {
          partition3(A, left, a);
	  left = a+1; right = b;
	  goto Again;
       }	  
       partition3(A, a+1, b);
       right = a;
       goto Again;
    }
} // end partition3

void showDistribution() {
  int siz = 40, m = 0, tweak = 5, i;
  int A[siz];
  stagger(A, siz, m, tweak);
  for (i = 0; i < siz; i++)
    printf("i %i %i\n", i, A[i]);
} // end showDistribution




void sortlinux(int *A, int N, int M)
{
  _quicksort(&A[N], M-N+1, sizeof(int), compareInt, NULL);
} // end sortlinux

void callLQ(int *A, int size, int (*compare1)() ) {
  _quicksort(A, size, sizeof(int), compare1, NULL);
} // end callLQ

// util functions for compareZeros00
// fill with random numbers according to percentage, otherwise 0
void fillRandom(int *A, int lng, int startv, int percentage) {
  if ( percentage < 0 || 100 < percentage ) percentage = 50;
  int i, v, r, v2;
  srand(startv);
  for ( i = 0; i < lng; i++) {
    if ( 0 == percentage ) A[i] = 0; else {
      v = rand();
      v2 = (v < 0) ? -v : v;
      r = v2%100;
      A[i] = (r <= percentage) ? v : 0; 
    }
  }
} // end fillRandom

// fill with random numbers according to percentage, otherwise with i
void fillRandom2(int *A, int lng, int startv, int percentage) {
  if ( percentage < 0 || 100 < percentage ) percentage = 50;
  int i, v, r, v2 ; 
  srand(startv);
  for ( i = 0; i < lng; i++) {
    if ( 0 == percentage ) A[i] = i; else {
      v = rand();
      v2 = (v < 0) ? -v : v;
      r = v2%100;
      A[i] = (r <= percentage) ? v : i; 
    }
    // A[i] = -A[i];
  }
} // end fillRandom2

// Generate distributions with a varying percentage of uniform random numbers
// and compare the running times of two algorithms
void compareZeros00(char *label, int siz, int seedLimit, 
		   void (*alg1)(), void (*alg2)(),
		   int (*compare1)() ) {
  siz = 1024 * 1024 * 16; seedLimit = 6;
  printf("%s on size: %d seedLimit: %d\n", label, siz, seedLimit);
  int alg1Time, alg2Time, T;
  int seed;
  int z;
  int limit = 1024 * 1024 * 16 + 1;
  int q;
  while (siz <= limit) {
    printf("%s %d %s %d %s", "siz: ", siz, " seedLimit: ", seedLimit, "\n");
    int *A = malloc (sizeof(int) * siz);
    // construct array
    // int i;
    // warm up the process
    for (seed = 0; seed < seedLimit; seed++) 
      fillarray(A, siz, seed);
    for (q = 0; q < 101; q = q + 20 ) { // percentages of random elements
      // for (q = 1; q < 3; q++ ) { // percentages of random elements
      // for (z = 0; z < 3; z++) { // repeat to check stability
      for (z = 0; z < 2; z++) { // repeat to check stability
	alg1Time = 0; alg2Time = 0;
	int TFill = clock();
	for (seed = 0; seed < seedLimit; seed++) 
	  fillarray(A, siz, seed);
	TFill = clock() - TFill;
	T = clock();
	for (seed = 0; seed < seedLimit; seed++) { 
	  fillRandom(A, siz, seed, q);      // zeros + random
	  // fillRandom2(A, siz, seed, q);  // ordered + random
	  (*alg1)(A, siz, compare1); 
	}
	alg1Time = clock() - T - TFill;
	T = clock();
	for (seed = 0; seed < seedLimit; seed++) { 
	  fillRandom(A, siz, seed, q);
	  // fillRandom2(A, siz, seed, q);
	  (*alg2)(A, 0, siz-1);
	}
	alg2Time = clock() - T - TFill;
	printf("%s %d %s %i", "siz: ", siz, " p: ", q);
	printf(" %s %d %s", "alg1Time: ", alg1Time, " ");
	printf("%s %d %s", "alg2Time: ", alg2Time, " ");
	float frac = 0;
	if ( alg1Time != 0 ) frac = alg2Time / ( 1.0 * alg1Time );
	printf("%s %f %s", "frac: ", frac, "\n");
      }
    }
    // free array
    free(A);
    siz = siz * 2;
    seedLimit = seedLimit / 2;
  }
} // end compareZeros00

void compare00LQAgainstFourSort() {
  compareZeros00("compare00LQAgainstFoursort", 
		 1024*1024, 16, callLQ, cut2, 
		 compareInt);
} // end compare00LQAgainstFourSort

void compare00BentleyAgainstFourSort() {
  compareZeros00("compare00BentleyAgainstFoursort", 
		 1024*1024, 16, callBentley, cut2, 
		 compareInt);
} // end compare00BentleyAgainstFourSort

/*
void compare00MergeAgainstFourSort() {
  compareZeros00("compare00MergeAgainstFoursort", 
		 1024*1024, 16, callMergesort, cut2, 
		 compareInt);
} // end compare00MergeAgainstFourSort
*/

void compare00DpqAgainstFourSort() {
  compareZeros00("compare00DpqAgainstFoursort", 
		 1024*1024, 16, callDpq, cut2, 
		 compareInt);
} // end compare00DpqgainstFourSort


void compare00DpqAgainstSixSort() {
  compareZeros00("compare00DpqAgainstSixsort", 
		 1024*1024, 16, callDpq, cut4, 
		 compareInt);
} // end compare00DpqgainstSixSort




// ------------------------- mergeSortDDC ('inplace') --------------------
// Looks like a version that does NOT require a 2nd array the size of 
// the input array

void mergeSortDDC();

void callMergeSortDDC(int *A, int size, int (*compare1)() ) {
  mergeSortDDC(A, 0, size-1);
} // end callMergeSortDDC

#define true 1

void merge2();
void mergeSortDDC(int *A, int N, int M) {
      // System.out.println("mergeSortDDC N: " + N + " M: " + M);
      // printf("mergeSortDDC N: %d M: %d\n", N, M);
	int L = M-N;
	if ( L <= 10 ) { insertionsort(A, N, M); return; }
	int k = L>>1; // L/2; 
	int pp = N+k; int q = pp+1;
	mergeSortDDC(A, N, pp); 
	// check(A, N, p, "N " + N + " p " + p);
	mergeSortDDC(A, q, M);
	// check(A, q, M, "q " + q + " M " + M);
	merge2(A, N, pp, q, M);
	// check(A, N, M, "N " + N + " M " + M);
	// check(A, N, M);
    } // end  mergeSortDDC

void merge0(int *A, int *B, int N, int pp, int M) {
        // System.out.println("merge0 N: " + N + " pp: " + pp + " M: " + M);
        // printf("merge0 N: %d pp: %d M: %d\n", N, pp, M);

	int L = M-N;
	// int k = L/2;
	int k = pp - N;
	int i = N; int j = N+k+1; 
	// int z = N;
	int z = 0;
	int leftDone = 0;
	int rightDone = 0;
	int ai = A[i]; int aj = A[j];
	while (true) {
	    while ( ai <= aj ) {
		B[z] = ai; z++;
		if ( i < N+k ) { i++; ai = A[i]; } else {
		    leftDone = true; break;
		}
	    }
	    if ( leftDone ) break;
	    // aj < ai
	    B[z] = aj; z++;
	    if ( M == j ) { rightDone = true; break; }
	    j++; aj = A[j];
	    while ( aj < ai ) {
		B[z] = aj; z++;
		if ( j < M ) { j++; aj = A[j]; } else {
		    rightDone = true; break;
		}
	    }
	    if ( rightDone ) break;
	    // ai <= aj
	    B[z] = ai; z++;
	    if ( i == N+k ) { leftDone = true; break; }
	    i++; ai = A[i];
	}
	// System.out.println("------");
	// for ( int p = i; p <= N+k; p++ ) System.out.println(p + " " + A[p]);
	int px;
	if ( leftDone ) {
	    // System.out.println("leftDone z: " + z);
	    for ( px = N; px < N+z; px++ ) A[px] = B[px-N];
	} else { // rightDone
	    // System.out.println("rightDone z: " + z + " i: " + i + " k: " + k);
	    // for ( int p = i; p <= N+k; p++ ) A[p + L - k] = A[p];
	    for ( px = N+k; i <= px; px-- ) A[px + L - k] = A[px];
	    for ( px = N; px < N+z; px++ ) A[px] = B[px-N];
	}
	// check(A, N, M, "merge0 N " + N + " M " + M);
    } // end merge0

#define  bufLng 1024
int buf [bufLng];

void shift();
int findMin();
int findMax();
void merge2(int *A, int N, int pp, int q, int M) {
	// left: [N, p];  right: [q, M]; q=p+1
	// System.out.println("merge2 N: " + N + " p&q: " + p + " " + q + " M: " + M);
        // printf("merge2 N: %d p&q: %d %d M: %d\n", N, pp, q, M);
  
	// printRange(A, N, 20, p, q, M, 39);
	if ( M-N <= bufLng ) {
	    merge0(A, buf, N, pp, M);
	    // check(A, N, M, "merge2 N " + N + " M " + M);
	    return;
	}
	// check(A, N, p, "merge N " + N + " p " + p);
	// check(A, q, M, "merge q " + q + " M " + M);
	// int Ni = N; int Mi = M;
	// printRange(A, N, 14, p, q, M, 48);
	/*
	if ( pp < N || pp+1 != q || M < q ) {
	  // System.out.println("*** Args error");
	  // System.exit(0);
	  exit(1);
	}
	*/
	// if ( p < N ) return;
	// if ( M < q ) return;
	if ( A[pp] <= A[q] ) {
	    // System.out.println("A[p] <= A[q]");
	    return;
	}
	if ( A[M] < A[N] ) {
	    // System.out.println("A[M] < A[N]");
	    shift(A, N, pp, q, M);
	    return;
	}
	if ( A[pp] <= A[M] ) M = findMin(A, q, M, A[pp]);
	if ( A[N] <= A[q] ) N = findMax(A, N, pp, A[q]);
	// printf("Ni %d N %d M %d Mi %d\n", Ni, N, M, Mi);
	/*
	int L = M-N;
	if ( L <= 10 ) { 
	    System.out.println("L <= 10");
	    insertionSort(A, N, M); return; 
	}
	*/
	// A[N] <= A[M]  .......
	int lngRight = M-q;
	int lngLeft = pp-N;
	if ( lngLeft <= lngRight ) {
	    int k = (q+M)/2;
	    int ak = A[k];
	    if ( A[pp] <= ak ) { // left side <= [k M]
	        // System.out.println("merge left side <= [k M]");
		merge2(A, N, pp, q, k);
		return;
	    }
	    if ( ak < A[N] ) { // [q k] < N p]
	        // System.out.println("merge [q k] < N p]");
		shift(A, N, pp, q, k);
		merge2(A, N + k-q+1, k, k+1, M);
		return;
	    }
	    // A[N] <= ak < A[p]
	    int z = findMax(A, N, pp, ak);
	    // System.out.println("mergez N: " + N + " M: " + M + 
	    //                    " z+1 " + (z+1) + " k " + k);
	    shift(A, z+1, pp, q, k);
	    // check(A, z+1, k, "merge2 after shift z+1 " + (z+1) + " k " + k);
	    merge2(A, N, z, z+1, z + k-q+1);
	    merge2(A, z+k-q+2, k, k+1, M);

	    // merge(A, N, p, q, k);
	    // merge(A, N, k, k+1, M);
	} else { 
	    int k = (N+pp)/2;
	    int ak = A[k];
	    if ( ak <= A[q] ) { // [N k] <= right side
	        // System.out.println("merge [N k] <= right side");
		merge2(A, k, pp, q, M);
		return;
	    }
	    if ( A[M] < ak  ) { //       [q M] < [k p]
	        // System.out.println("merge [q M] < [k p]");
		shift(A, k, pp, q, M);
		merge2(A, N, k-1, k, k+M-q);
		return;
	    }
	    int z = findMin(A, q, M, ak);
	    // System.out.println("mergez N: " + N + " M: " + M + 
	    //		          " z-1 " + (z-1) + " k " + k);
	    // shift(A, z+1, p, q, k);
	    shift(A, k, pp, q, z-1);
	    merge2(A, N, k-1, k, z-pp+k-2);
	    merge2(A, z-pp+k-1, z-1, z, M);

	    // merge(A, k+1, p, q, M);
	    // merge(A, N, k, k+1, M);
	}

	// check(A, Ni, Mi, "merge Ni " + Ni + " Mi " + Mi);
    } // end merge2

void swapm();
void shiftLeft2();
void shiftRight2();
void shift(int *A, int a, int b, int c, int d) {
        // System.out.println("shift a b c d: " + a + " " + b + " " + c + " " + d );
        // printf("shift a %d b %d c %d d %d\n", a, b, c, d);
	int lngRight = d-c;
	int lngLeft = b-a;
	if ( lngLeft == lngRight ) { // swap elements
	    swapm(A, a, b, c);
	    return;
	}
	if ( lngLeft < lngRight ) {
	    shiftLeft2(A, a, b, c, d);
	} else { 
	    shiftRight2(A, a, b, c, d);
	}
    } // end shift

void swapm(int  *A, int a, int b, int c) {
    // System.out.println("swap a b c: " + a + " " + b + " " + c + "              " + (b-a));
        int i, ai;
	for (i = 0; a+i <= b; i++) {
	    ai = A[a+i]; A[a+i] = A[c+i]; A[c+i] = ai;
	}
    } // end swap

void shiftLeft2(int *A, int a, int b, int c, int d) { // b-a<d-c & b+1=c
        // System.out.println("shiftLeft2 a b c d: " + a + " " + b + " " + c + " " + d);
	if ( b-a < bufLng ) {
	    // System.out.println("                                             " + (b-a));
	    int i;
	    for (i = a; i<=b; i++ ) buf[i-a] = A[i];
	    int lng = d-c + 1;
	    for (i = 0; i < lng; i++) A[a+i] = A[c+i];
	    lng = b-a + 1;
	    for (i = 0; i < lng; i++) A[i + d-b+a] = buf[i];
	    return;
	}
	int lng = b-a+1;
	int lng2 = d-c+1; // lng < lng2
	int a2 = a;
	while (true) {
	    swapm(A, a2, b, b+1);
	    lng2 = lng2 - lng;
	    if ( 0 == lng2 ) {
	        // System.out.println("shiftLeft2Y swap done");
		return;
	    }
	    a2 = a2+lng; b = b+lng;
	    if ( lng2 < lng ) break;
	}
	// System.out.println(a + " " + b + " " + (b+1) + " " + d);
	// merge(A, a, b, b+1, d);
	// System.out.println(a2 + " " + b + " " + (b+1) + " " + d);
	shiftRight2(A, a2, b, b+1, d);
    } // end shiftLeft2

void shiftRight2(int *A, int a, int b, int c, int d) { // b-a>d-c & b+1=c
  // System.out.println("shiftRight2 a b c d: " + a + " " + b + " " + c + " " + d );
	if ( d-c < bufLng ) {
	    // System.out.println("                                            " + (d-c));
	    int i;
	    for (i = c; i<=d; i++ ) buf[i-c] = A[i];
	    int lng = b-a + 1;
	    for (i = b; a <=i; i-- ) A[i + d-b] = A[i];
	    lng = d-c + 1;
	    for (i = 0; i < lng; i++) A[a + i] = buf[i];
	    return;
	}
	int lng = d-c+1;
	int lng2 = b-a+1;
	int d2 = d;
	while (true) {
	    swapm(A, c, d2, c-lng);
	    lng2 = lng2 - lng;
	    if ( 0 == lng2 ) {
	        // System.out.println("shiftRight2Y swap done");
		return;
	    }
	    c = c-lng; d2 = d2-lng;
	    if ( lng2 < lng ) break;
	}
	// System.out.println(a + " " + (c-1) + " " + c + " " + d);
	// merge(A, a, c-1, c, d);
	// System.out.println(a + " " + (c-1) + " " + c + " " + d2);
	shiftLeft2(A, a, c-1, c, d2);
    } // end shiftRight2
    
int findMin(int *A, int q, int M, int ap) {
        // int M0 = M;
	// A[q] < A[p]=ap <= A[M]
	// return smallest index z <= M so that ap <= A[z]
	while (true) {
	    int z = (q + M)/2;
	    if ( ap <= A[z] ) M = z; else q = z; 
	    // System.out.println("---- q & M " + q + " " + M);
	    if ( q+1 == M ) {
	      // if ( M < M0 ) System.out.println("--- M0 M " + M0 + " " + M);
	      // else System.out.println("||| M M0 " + M + " " + M0);
		return M;
	    }
	}
    } // end findMin
int findMax(int *A, int N, int pp, int aq) {
	// A[N] <= A[q]=aq < A[p]
	// return largest index z with N <= z so that A[z] <= aq
	// int N0 = N;
	while (true) {
	    int z = (N + pp)/2;
	    if ( A[z] <= aq ) N = z; else pp = z; 
	    // System.out.println("---- N & p " + N + " " + p);
	    if ( N+1 == pp ) {
	      // if ( N0 < N ) System.out.println("--- N0 N " + N0 + " " + N);
	      // else System.out.println("||| N0 N " + N0 + " " + N);
		return N;
	    }
	}
} // end findMax


