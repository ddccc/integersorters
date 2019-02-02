// File: c:/bsd/rigel/sort/compare2.c
// Date: Fri May 22 09:36:07 2009/ Sat Dec 28 12:23:32 2013/
// Fri Jul 07 13:40:56 2017 / Tue Jan 02 15:57:33 2018
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
 */


#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>

const int cut2fLimit = 2000;
const int cut2Limit = 10007;
const int cut3Limit = 250;
int probeParamCut3x = 1000000;
const int cut4Limit = 3000;

void fillarray();
void testQuicksort0();
void testMyQS();
void testCut2();
void testTps();
void testDflglTest();
void testDflgmTest();
void testDflgmTestX();
void testDflgmTestY();
void testDflgmTestZ();
void testDflgmTestW();
void testCut2x();
void testCut3x();
void testCut3y();
void testCut3z();
void testCut4();
void testCut4s();
void testCut4p();
void testHeapSort();
void testSortc();
void testDpqSort();
void testPart3();
void testMergeSort0();
void testMergeSortDDC();
void validateQuicksort0();
void validateMyQS();
void validateCut2();
void validateTps();
void validateDflglTest();
void validateDflgmTest();
void validateDflgmTestX();
void validateDflgmTestY();
void validateDflgmTestZ();
void validateDflgmTestW();
void validateCut2x();
void validateCut3x();
void validateCut3y();
void validateCut3z();
void validateCut4();
void validateCut4s();
void validateCut4p();
void validateHeapSort();
void validateSortc();
void validateSortcB();
void validateDpqSort();
void validatePart3();
void validateChenSort();
void validateMergeSort0();
void validateMergeSortDDC();
void exploreParams();
void timeTest();
void timeTest2();
// void compareQuicksort0Against2levelOutside();
// void compareQuicksort0AgainstQuicksort0WithInsertionsort();
// void compareQuicksort0OutsideInsertionsortAgainstQuicksort0WithInsertionsort();
// void compareQuicksort0WithInsertionsortAgainstCut2();
// void compareQuicksort0WithInsertionsortAgainstTps();
// void compareQuicksort0WithInsertionsortAgainstCut3x();
// void compareQuicksort0WithInsertionsortAgainstCut4();
// void compareQuicksort0WithInsertionsortAgainstDpqSort();
void compareDFLGMAgainstCut2(); 
void compareCut2AgainstTps(); 
void compareCut2AgainstCut2x(); 
void compareCut2AgainstCut2xB(); // using other distributions also
void compareDpqSortAgainstCut2xB(); 
void compareCut2AgainstCut3x();
void compareSortcAgainstCut2(); // sortc is build-in quicksort
void compareLQAgainstCut2(); // LQ is linux qsort
void compareSortcBUAgainstCut2(); // sortcB is B&M quicksort
void compareChenSortAgainstDdC2Sort(); // against cut2
void compareSortcBUAgainstChenSort(); // sortcB is B&M quicksort
void compareSortcAgainstCut3x();
void compareSortcAgainstCut4();
void compareCut3xAgainstTps(); 
void compareCut3xAgainstCut3y();
void compareCut3xAgainstCut4();
void compareCut2AgainstCut4();
void compareTPSAgainstCut4();
void compareCut4fAgainstCut4();
void compareCut4sAgainstCut4();
void compareAlgorithmsTime();
void compareCut2AgainstCut4p(); // real time test - not using clock
void compareCut4AgainstCut4p(); // real time test - not using clock
void compareDpqSortAgainstQuicksort0(); // 
void compareDpqSortAgainstDdC2Sort(); // cut2
void compareDpqSortAgainstDdC3Sort(); // cut3x
void compareDpqSortAgainstTPS();
void compareDpqSortAgainstTPS5();
void compareDpqSortAgainstTPS4();
void compareDpqSortAgainstTPS3();
void compareDpqSortAgainstTPS2();
void compareDpqSortAgainstCut4(); // cut4
void compareDpqSortAgainstCut4s(); //  cut4s
void compareMergeSort0AgainstDdC2Sort(); // cut2
void compareMergeSort0AgainstParMergeSort2(); // mergesort compares
void compareMergeSort0AgainstParMergeSort4(); // mergesort compares
void compareMergeSortDDCAgainstDdC2Sort(); // cut2
void compareDpqSortAgainstDdCSort2(); // using Bentley stuff <<<<<<<<<<<<<<<<<<<<<
void showSuffle2();
void compareSortcBAgainstCut4(); // using Bentley stuff
void compareSortcQAgainstCut2(); // !!!!native qsort/ LQ/ B&M <-> cut2 using Bentley stuff
void compareSortcBAgainstCut2(); // B&M <-> cut2 using Bentley stuff  <<<<<<<<<<
void compareChenSortAgainstCut2(); // using Bentley stuff
void musserTest();
void compare00LQAgainstFourSort();
void compare00BentleyAgainstFourSort();
void compare00ChenAgainstFourSort();
void compare00MergeAgainstFourSort();
void compare00DpqAgainstFourSort();
void compare00DpqAgainstTPS();
void compare00DpqAgainstSixSort();
void quicksort0();
void insertionsort();
int med();
void iswap();
void myqs();
void myqsc();
void cut2f();
void cut2fc();
void cut2();
void cut2c();
void tps();
void tpsc();
void dflglTest();
void dflgmTest();
void dflgmTest2();
void dflgmTestX();
void dflgmTestX2();
void dflgmTestY();
void dflgmTestY2();
void dflgmTestZ();
void dflgmTestZ2();
void dflgmTestW();
void dflgmTestW2();
void cut2x();
void cut2xc();
void cut2x2();
void cut3x();
void cut3xc();
void cut3y();
void cut3yc();
void cut3z();
void cut3zc();
void cut4();
void cut4c();
void cut4s();
void cut4sc();
void cut4f();
void cut4fc();
void callSixSort();
void heapc();
void sortc();
void dpqSort();
void mergeSort0();
void mergeSortDDC();
void sortcB();
void chenSort();
void parMergeSort2();
void parMergeSort4();
void callLQ();
void callBentley();
void callChensort();
void callMergesort();
void callDpq();
void part3();
void sortlinux();
void cut4f();
void tps5();
void tpsc5();
void tps4();
void tpsc4();
void tps3();
void tpsc3();
void tps2();
void tpsc2();
void dflgm();
void cut4p();
void cut4pc();
void showDistribution();

int main (int argc, char *argv[]) {
  printf("Running compare2 ...\n");
  // testQuicksort0();
  // testMyQS();
  // testQuicksort0WithInsertionsort();
  // testCut2();
  // testTps();
  // testDflglTest();
  // testDflgmTest();
  // testDflgmTestX();
  // testDflgmTestY();
  // testDflgmTestZ();
  // testDflgmTestW();
  // testCut2x(;)
  // testCut3x();
  // testCut3y();
  // testCut3z();
  // testCut4();
  // testCut4s();
  // testCut4p();
  // testHeapSort();
  // testSortc();
  // testDpqSort();
  // testPart3();
  // testMergeSort0();
  // testMergeSortDDC();
  // validateQuicksort0;
  // validateMyQS();
  // validateCut2();
  // validateTps();
  // validateDflglTest();
  // validateDflgmTest();
  // validateDflgmTestX();
  // validateDflgmTestY();
  // validateDflgmTestZ();
  // validateDflgmTestW(); 
  // validateCut2x();
  // validateCut3x();
  // validateCut3y();
  // validateCut3z();
  // validateCut4();
  // validateCut4s();
  // validateCut4p();
  // validateHeapSort();
  // validateSortc();
  // validateSortcB();
  // validateDpqSort();
  // validatePart3();
  // validateChenSort();
  // validateMergeSort0();
  // validateMergeSortDDC();
  // exploreParams();
  timeTest();
  // timeTest2();
  // compareDFLGMAgainstCut2(); 
  // compareCut2AgainstTps(); 
  // compareCut2AgainstCut2x(); 
  // compareCut2AgainstCut2xB(); // using other distributions also
  // compareDpqSortAgainstCut2xB(); 
  // compareCut2AgainstCut3x();
  // compareSortcAgainstCut2(); // sortc is build-in quicksort
  // compareLQAgainstCut2(); // LQ is linux qsort
  // compareSortcBUAgainstCut2(); // sortcB is B&M quicksort
  // compareChenSortAgainstDdC2Sort(); // against cut2
  // compareSortcBUAgainstChenSort(); // sortcB is B&M quicksort
  // compareSortcAgainstCut3x();
  // compareSortcAgainstCut4();
  // compareCut3xAgainstTps(); 
  // compareCut3xAgainstCut3y();
  // compareCut3xAgainstCut4();
  // compareCut2AgainstCut4();
  // compareTPSAgainstCut4();
  // compareCut4fAgainstCut4();
  // compareCut4sAgainstCut4();
  // compareCut2AgainstCut4p(); // real time test - not using clock
  // compareCut4AgainstCut4p(); // real time test - not using clock
  // compareDpqSortAgainstQuicksort0(); // 
  // compareDpqSortAgainstDdC2Sort(); // cut2
  // compareDpqSortAgainstDdC3Sort(); // cut3x
  // compareDpqSortAgainstTPS();
  // compareDpqSortAgainstTPS5();
  // compareDpqSortAgainstTPS4();
  // compareDpqSortAgainstTPS3();
  // compareDpqSortAgainstTPS2();
  // compareDpqSortAgainstCut4();  // cut4
  // compareDpqSortAgainstCut4s();  // cut4s
  // compareMergeSort0AgainstDdC2Sort(); // cut2
  // compareMergeSort0AgainstParMergeSort2(); // mergesort compares
  // compareMergeSort0AgainstParMergeSort4(); // mergesort compares
  // compareMergeSortDDCAgainstDdC2Sort(); // cut2
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

int findValue(int *A, int N, int M, int value) {
  int i;
  for (i = N; i <= M; i++) 
    if (A[i] == value) return i;
  return -1;
}

void check(int *A, int N, int M) 
{
  int i, x, y;
  for (i = N + 1; i <= M; i++) {
    x = A[i-1]; y = A[i];
    if (y < x) 
      printf("Error element i=%i less than preceding element: A[i]=%i\n",i,y);
  }
} // end check

int size2 = 1024*1024*16;
// int size2 = 1024 * 512; 



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

void testMyQS() {
  testAlgorithm("Running myqs ...", myqs);
}
void testQuicksort0() {
  testAlgorithm("Running testQuicksort0 ...", quicksort0);
}

void testInsertionsort() {
  testAlgorithm("Running testInsertionsortLong ...", insertionsort);
}

void testCut2() {
  testAlgorithm("Running testCut2 ...", cut2);
}

void testTps() {
  testAlgorithm("Running testTps ...", tps);
}

void testDflglTest() {
  testAlgorithm("Running testDflglTest ...", dflglTest);
}

void testDflgmTest() {
  testAlgorithm("Running testDflgmTest ...", dflgmTest);
}

void testDflgmTestX() {
  testAlgorithm("Running testDflgmTestX ...", dflgmTestX);
}

void testDflgmTestY() {
  testAlgorithm("Running testDflgmTestY ...", dflgmTestY);
}

void testDflgmTestZ() {
  testAlgorithm("Running testDflgmTestZ ...", dflgmTestZ);
}

void testDflgmTestW() {
  testAlgorithm("Running testDflgmTestW ...", dflgmTestW);
}

void testCut2x() {
  testAlgorithm("Running testCut2x ...", cut2x);
}

void testCut3x() {
  testAlgorithm("Running testCut3x ...", cut3x);
}

void testCut3y() {
  testAlgorithm("Running testCut3y ...", cut3y);
}

void testCut3z() {
  testAlgorithm("Running testCut3z ...", cut3z);
}

void testCut4() {
  testAlgorithm("Running testCut4 ...", cut4);
}

void testCut4s() {
  testAlgorithm("Running testCut4s ...", cut4s);
}

void testCut4p() {
  testAlgorithm("Running testCut4p ...", callSixSort);
}

void testHeapSort() {
  testAlgorithm("Running testHeapSort ...", heapc);
}

void testSortc() {
  testAlgorithm("Running testSortc ...", sortc);
}

void testDpqSort() {
  testAlgorithm("Running testDpqSort ...", dpqSort);
}

void testPart3() {
  testAlgorithm("Running testPart3 ...", part3);
}

void testMergeSort0() {
  testAlgorithm("Running testMergeSort0 ...", mergeSort0);
} // end testMergeSort0

void testMergeSortDDC() {
  testAlgorithm("Running testMergeSortDDC ...", mergeSortDDC);
} // end testMergeSortDDC

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

void validateMyQS() {
  validateAlgorithm("Running validate MyQs ...\n",
		    heapc, myqs);
}
void validateCut2() {
  validateAlgorithm("Running validate Cut2 ...\n",
		    quicksort0, cut2);
}

void validateTps() {
  validateAlgorithm("Running validate Tps ...\n",
		    quicksort0, tps);
}

void validateDflglTest() {
  validateAlgorithm("Running validate DflglTest ...\n",
		    quicksort0, dflglTest);
}

void validateDflgmTest() {
  validateAlgorithm("Running validate DflgmTest ...\n",
		    quicksort0, dflgmTest);
}

void validateDflgmTestX() {
  validateAlgorithm("Running validate DflgmTestX ...\n",
		    quicksort0, dflgmTestX);
}

void validateDflgmTestY() {
  validateAlgorithm("Running validate DflgmTestY ...\n",
		    quicksort0, dflgmTestY);
}

void validateDflgmTestZ() {
  validateAlgorithm("Running validate DflgmTestZ ...\n",
		    quicksort0, dflgmTestZ);
}

void validateDflgmTestW() {
  validateAlgorithm("Running validate DflgmTestW ...\n",
		    quicksort0, dflgmTestW);
}

void validateCut2x() {
  validateAlgorithm("Running validate Cut2x ...\n",
		    cut2, cut2x);
}

void validateCut3x() {
  validateAlgorithm("Running validate Cut3x ...\n",
		    cut2, cut3x);
}

void validateCut3y() {
  validateAlgorithm("Running validate Cut3y ...\n",
		    cut2, cut3y);
}

void validateCut3z() {
  validateAlgorithm("Running validate Cut3z ...\n",
		    cut2, cut3z);
}

void validateCut4() {
  validateAlgorithm("Running validate Cut4 ...\n",
		    cut2, cut4);
}

void validateCut4s() {
  validateAlgorithm("Running validate Cut4s ...\n",
		    cut2, cut4s);
}

void validateCut4p() {
  validateAlgorithm("Running validate Cut4p ...\n",
		    cut2, callSixSort);
}

void validateHeapSort() {
  validateAlgorithm("Running validate HeapSort ...\n",
		    cut2, heapc);
}

void validateSortc() {
  validateAlgorithm("Running validate Sortc ...\n",
		    cut2, sortc);
}

void validateSortcB() {
  validateAlgorithm("Running validate SortcB ...\n",
		    cut2, sortcB);
}
void validateDpqSort() {
  validateAlgorithm("Running validate DpqSort ...\n",
		    cut2, dpqSort);
}

void validatePart3() {
  validateAlgorithm("Running validate Part3 ...\n",
		    cut2, part3);
}

void validateChenSort() {
  validateAlgorithm("Running validate ChenSort ...\n",
		    cut2, chenSort);
} // end validateChenSort()

/*
void validateMergeSort0() {
  validateAlgorithm("Running validate MergeSort0 ...\n",
		    cut2, mergeSort0);
} // end validateMergeSort0()
*/

void validateMergeSortDDC() {
  validateAlgorithm("Running validate MergeSortDDC ...\n",
		    cut2, mergeSortDDC);
} // end validateMergeSortDDC()

/* Validating parMergeSort2 with two threads
void validateMergeSort0() {
  validateAlgorithm("Running validate parMergeSort2 ...\n",
		    cut2, parMergeSort2);
} // end validateMergeSort0()
*/
// /* Validating parMergeSort2 with four threads
void validateMergeSort0() {
  validateAlgorithm("Running validate parMergeSort4 ...\n",
		    cut2, parMergeSort4);
} // end validateMergeSort0()
// */

void exploreParams() {
  // run cut4 and test different parameters
  printf("exploreParams() cut3x probeParamCut3x   ........\n");
  int cut4Time, T;
  int seed;
  int seedLimit = 32;
  int z;
  // int siz = 1024 * 1024 * 16;
  int siz = 1024 * 1024;
  int *A = malloc (sizeof(int) * siz);
  printf("exploring::  cut3x probeParamCut3x\n");
  printf("%s %d %s", "siz: ", siz, "\n");

  // warm up the process
  for (seed = 0; seed < 1; seed++) 
    fillarray(A, siz, seed); 
  //  for (cut2Limit = 80; 
  //   cut2Limit <= 150; 
  //   cut2Limit = cut2Limit + 20) {
  //   printf("%s %d %s", "cut2Limit: ", cut2Limit, "cut4Times: ");
  //  for (cutSampleParam = 500; cutSampleParam <= 1500; cutSampleParam += 100) {
  //      printf("cutSampleParam: %d ", cutSampleParam);
  // for (probeParamCut3x = 500; probeParamCut3x <= 2000;  probeParamCut3x += 100) {
for (probeParamCut3x = 5000; probeParamCut3x <= 40000;  probeParamCut3x += 5000) {
    printf("probeParamCut3x: %d ", probeParamCut3x);
    int sumTimes = 0;
    for (z = 0; z < 1; z++) { // repeat to check stability
      cut4Time = 0;
      int TFill = clock();
      for (seed = 0; seed < seedLimit; seed++) 
	fillarray(A, siz, seed+10*z);
      TFill = clock() - TFill;

      T = clock();
      for (seed = 0; seed < seedLimit; seed++) { 
	fillarray(A, siz, seed+10*z);
	cut4(A, 0, siz-1); 
      }
      cut4Time = clock() - T - TFill;
      // printf("%d %s", cut4Time, " ");
      sumTimes = sumTimes + cut4Time;
    }
    printf("%s %d %s", "sumTimes: ", sumTimes, "\n");
  }

} // end exploreParams()

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
  int reps = 15;
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
      // quicksort0(A, 0, siz-1);
      // cut2(A, 0, siz-1);  
      // tps(A, 0, siz-1);
      // tps5(A, 0, siz-1);
      // tps4(A, 0, siz-1);
      // tps3(A, 0, siz-1);
      // tps2(A, 0, siz-1);
      // dpqSort(A, 0, siz-1);
      cut4(A, 0, siz-1);  
      // dflglTest(A, 0, siz-1);
      // dflgmTest(A, 0, siz-1);  
      // dflgmTestX(A, 0, siz-1);  
      // dflgmTestY(A, 0, siz-1);  
      // dflgmTestZ(A, 0, siz-1);
      // dflgmTestW(A, 0, siz-1);
      // sortlinux(A, 0, siz-1);    
      // sortcB(A, 0, siz-1);    
      // part3(A, 0, siz-1);    
      // chenSort(A, 0, siz-1); 
      // myqs(A, 0, siz-1); 
    }
    cut4Time = (clock() - T - TFill)/ seedLimit;
    printf("algorithm time: %d \n", cut4Time);
    sumTimes = sumTimes + cut4Time;
    printf("z %i average time: %d\n", z, sumTimes/(1+z));
  }
  printf("%s %d %s", "average time: ", (sumTimes/reps), "\n");
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

void compareDFLGMAgainstCut2() {
  compareAlgorithms("Entering compareDFLGMAgainstCut2 ........\n",
		    dflgmTest, cut2);
}

void compareCut2AgainstTps() {
  compareAlgorithms("Entering compareCut2AgainstTps ........\n",
		    cut2, tps);
}

void compareCut2AgainstCut2x() {
  compareAlgorithms("Entering compareCut2AgainstCut2x ........\n",
		    cut2, cut2x);
}

void compareCut2AgainstCut2xB() {
  printf("Entering compareCut2AgainstCut2x ........\n");
  int cut2Time, cut2xTime, T;
  int seed;
  int z, k;
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
      cut2Time = 0; cut2xTime = 0;
      int TFill = clock();
      for (seed = 0; seed < seedLimit; seed++) 
	// fillarray(A, siz, seed);
	// for (k = 0; k < siz; k++) A[k] = 0;
	for (k = 0; k < siz; k++) A[k] = k%2;
      TFill = clock() - TFill;
      T = clock();
      for (seed = 0; seed < seedLimit; seed++) { 
	// fillarray(A, siz, seed);
	// for (k = 0; k < siz; k++) A[k] = 0;
	for (k = 0; k < siz; k++) A[k] = k%2;
	cut2(A, 0, siz-1);
      }
      cut2Time = clock() - T - TFill;
      T = clock();
      for (seed = 0; seed < seedLimit; seed++) { 
	// fillarray(A, siz, seed);
	// for (k = 0; k < siz; k++) A[k] = 0;
	for (k = 0; k < siz; k++) A[k] = k%2;
	cut2x(A, 0, siz-1);  
      }
      cut2xTime = clock() - T - TFill;
      printf("%s %d %s", "siz: ", siz, " ");
      printf("%s %d %s", "cut2Time: ", cut2Time, " ");
      printf("%s %d %s", "cut2xTime: ", cut2xTime, " ");
      float frac = 0;
      if ( cut2Time != 0 ) frac = cut2xTime / ( 1.0 * cut2Time );
      printf("%s %f %s", "frac: ", frac, "\n");
    }
    free(A);
    siz = siz * 2;
    seedLimit = seedLimit / 2;
  }
} // end compareCut2AgainstCut2xB

void compareCut2AgainstCut3x() {
  compareAlgorithms("Entering compareCut2AgainstCut3x ........\n",
		    cut2, cut3x);
}

void compareSortcAgainstCut2() {
  printf("Entering compareSortcAgainstCut2 ........\n");
  int sortcTime, cut2Time, T;
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
      sortcTime = 0; cut2Time = 0;
      int TFill = clock();
      for (seed = 0; seed < seedLimit; seed++) 
	fillarray(A, siz, seed);
      TFill = clock() - TFill;
      T = clock();
      for (seed = 0; seed < seedLimit; seed++) { 
	fillarray(A, siz, seed);
	sortc(A, 0, siz-1);
      }
      sortcTime = clock() - T - TFill;
      T = clock();
      for (seed = 0; seed < seedLimit; seed++) { 
	fillarray(A, siz, seed);
	cut2(A, 0, siz-1);  
      }
      cut2Time = clock() - T - TFill;
      printf("%s %d %s", "siz: ", siz, " ");
      printf("%s %d %s", "sortcTime: ", sortcTime, " ");
      printf("%s %d %s", "cut2Time: ", cut2Time, " ");
      float frac = 0;
      if ( sortcTime != 0 ) frac = cut2Time / ( 1.0 * sortcTime );
      printf("%s %f %s", "frac: ", frac, "\n");
    }
    free(A);
    siz = siz * 2;
    seedLimit = seedLimit / 2;
  }
} // end compareSortcAgainstCut2

void compareSortcBUAgainstCut2() {
  printf("Entering compareSortcBUAgainstCut2 ........\n");
  int sortcBTime, cut2Time, T;
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
      sortcBTime = 0; cut2Time = 0;
      int TFill = clock();
      for (seed = 0; seed < seedLimit; seed++) 
	fillarray(A, siz, seed);
      TFill = clock() - TFill;
      T = clock();
      for (seed = 0; seed < seedLimit; seed++) { 
	fillarray(A, siz, seed);
	sortcB(A, 0, siz-1);
      }
      sortcBTime = clock() - T - TFill;
      T = clock();
      for (seed = 0; seed < seedLimit; seed++) { 
	fillarray(A, siz, seed);
	cut2(A, 0, siz-1);  
      }
      cut2Time = clock() - T - TFill;
      printf("%s %d %s", "siz: ", siz, " ");
      printf("%s %d %s", "sortcBTime: ", sortcBTime, " ");
      printf("%s %d %s", "cut2Time: ", cut2Time, " ");
      float frac = 0;
      if ( sortcBTime != 0 ) frac = cut2Time / ( 1.0 * sortcBTime );
      printf("%s %f %s", "frac: ", frac, "\n");
    }
    free(A);
    siz = siz * 2;
    seedLimit = seedLimit / 2;
  }
} // end compareSortcBUAgainstCut2

void compareLQAgainstCut2() {
  printf("Entering compareLQAgainstCut2 ........\n");
  int lqTime, cut2Time, T;
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
      lqTime = 0; cut2Time = 0;
      int TFill = clock();
      for (seed = 0; seed < seedLimit; seed++) 
	fillarray(A, siz, seed);
      TFill = clock() - TFill;
      T = clock();
      for (seed = 0; seed < seedLimit; seed++) { 
	fillarray(A, siz, seed);
	sortlinux(A, 0, siz-1);
      }
      lqTime = clock() - T - TFill;
      T = clock();
      for (seed = 0; seed < seedLimit; seed++) { 
	fillarray(A, siz, seed);
	cut2(A, 0, siz-1);  
      }
      cut2Time = clock() - T - TFill;
      printf("%s %d %s", "siz: ", siz, " ");
      printf("%s %d %s", "lqTime: ", lqTime, " ");
      printf("%s %d %s", "cut2Time: ", cut2Time, " ");
      float frac = 0;
      if ( lqTime != 0 ) frac = cut2Time / ( 1.0 * lqTime );
      printf("%s %f %s", "frac: ", frac, "\n");
    }
    free(A);
    siz = siz * 2;
    seedLimit = seedLimit / 2;
  }
} // end compareLQAgainstCut2

void compareSortcBUAgainstChenSort() {
  printf("Entering compareSortcBUAgainstChenSort ........\n");
  int sortcBTime, chenTime, T;
  int seed;
  int z;
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
      sortcBTime = 0; chenTime = 0;
      int TFill = clock();
      for (seed = 0; seed < seedLimit; seed++) 
	fillarray(A, siz, seed);
      TFill = clock() - TFill;
      T = clock();
      for (seed = 0; seed < seedLimit; seed++) { 
	fillarray(A, siz, seed);
	sortcB(A, 0, siz-1);
      }
      sortcBTime = clock() - T - TFill;
      T = clock();
      for (seed = 0; seed < seedLimit; seed++) { 
	fillarray(A, siz, seed);
	chenSort(A, 0, siz-1);  
      }
      chenTime = clock() - T - TFill;
      printf("%s %d %s", "siz: ", siz, " ");
      printf("%s %d %s", "sortcBTime: ", sortcBTime, " ");
      printf("%s %d %s", "chenTime: ", chenTime, " ");
      float frac = 0;
      if ( sortcBTime != 0 ) frac = chenTime / ( 1.0 * sortcBTime );
      printf("%s %f %s", "frac: ", frac, "\n");
    }
    free(A);
    siz = siz * 2;
    seedLimit = seedLimit / 2;
  }
} // end compareSortcBUAgainstChenSort


void compareSortcAgainstCut4() {
  printf("Entering compareSortcAgainstCut4 ........\n");
  int sortcTime, cut4Time, T;
  int seed;
  int z;
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
      sortcTime = 0; cut4Time = 0;
      int TFill = clock();
      for (seed = 0; seed < seedLimit; seed++) 
	fillarray(A, siz, seed);
      TFill = clock() - TFill;
      T = clock();
      for (seed = 0; seed < seedLimit; seed++) { 
	fillarray(A, siz, seed);
	sortc(A, 0, siz-1);
      }
      sortcTime = clock() - T - TFill;
      T = clock();
      for (seed = 0; seed < seedLimit; seed++) { 
	fillarray(A, siz, seed);
	cut4(A, 0, siz-1);  
      }
      cut4Time = clock() - T - TFill;
      printf("%s %d %s", "siz: ", siz, " ");
      printf("%s %d %s", "sortcTime: ", sortcTime, " ");
      printf("%s %d %s", "cut4Time: ", cut4Time, " ");
      float frac = 0;
      if ( sortcTime != 0 ) frac = cut4Time / ( 1.0 * sortcTime );
      printf("%s %f %s", "frac: ", frac, "\n");
    }
    free(A);
    siz = siz * 2;
    seedLimit = seedLimit / 2;
  }
} // end compareSortcAgainstCut4

void compareCut3xAgainstTps() {
  compareAlgorithms("Entering compareCut3xAgainstTps ........\n",
		    cut3x, tps);
}

void compareCut3xAgainstCut3y() {
  compareAlgorithms("Entering compareCut3xAgainstCut3y ........\n",
		    cut3x, cut3y);
}

void compareCut3xAgainstCut4() {
  compareAlgorithms("Entering compareCut3xAgainstCut4 ........\n",
		    cut3x, cut4);
}

void compareCut2AgainstCut4() {
  compareAlgorithms("Entering compareCut2AgainstCut4 ........\n",
		    cut2, cut4);
}
void compareTPSAgainstCut4() {
  compareAlgorithms("Entering compareTPSAgainstCut4 ........\n",
		    tps, cut4);
}

void compareCut4fAgainstCut4() { // temporary arrangement *********
  compareAlgorithms("Entering compareCut4fAgainstCut4 ........\n",
		    cut4f, cut4);
}

void compareCut4sAgainstCut4() { // temporary arrangement *********
  compareAlgorithms("Entering compareCut4sAgainstCut4 ........\n",
		    cut4s, cut4);
}

void compareCut2AgainstCut4p() {
  compareAlgorithmsTime("Entering compareCut2AgainstCut4p ........\n",
			cut2, callSixSort);
}

void compareCut4AgainstCut4p() {
  compareAlgorithmsTime("Entering compareCut4AgainstCut4p ........\n",
			cut4, callSixSort);
}

void compareDpqSortAgainstQuicksort0() {
  compareAlgorithms("Entering compareDpqSortAgainstQuicksort0 ........\n",
		    dpqSort, quicksort0);
}

void compareDpqSortAgainstDdC2Sort() {
  compareAlgorithms("Entering compareDpqSortAgainstCut2 ........\n",
		    dpqSort, cut2);
}

void compareDpqSortAgainstDdC3Sort() {
  compareAlgorithms("Entering compareDpqSortAgainstCut3x ........\n",
		    dpqSort, cut3x);
}

/*
void compareDpqSortAgainstDdC3Sort() {
  compareAlgorithms("Entering compareDpqSortAgainstCut3z ........\n",
		    dpqSort, cut3z);
}
*/

void compareDpqSortAgainstTPS() {
  compareAlgorithms("Entering compareDpqSortAgainstTPS ........\n",
		    dpqSort, tps);
}
void compareDpqSortAgainstTPS5() {
  compareAlgorithms("Entering compareDpqSortAgainstTPS5 ........\n",
		    dpqSort, tps5);
}
void compareDpqSortAgainstTPS4() {
  compareAlgorithms("Entering compareDpqSortAgainstTPS4 ........\n",
		    dpqSort, tps4);
}
void compareDpqSortAgainstTPS3() {
  compareAlgorithms("Entering compareDpqSortAgainstTPS3 ........\n",
		    dpqSort, tps3);
}
void compareDpqSortAgainstTPS2() {
  compareAlgorithms("Entering compareDpqSortAgainstTPS2 ........\n",
		    dpqSort, tps2);
}

void compareDpqSortAgainstCut4() {
  compareAlgorithms("Entering compareDpqSortAgainstCut4 ........\n",
		    dpqSort, cut4);
}

void compareDpqSortAgainstCut4s() {
  compareAlgorithms("Entering compareDpqSortAgainstCut4s ........\n",
		    dpqSort, cut4s);
}
void compareChenSortAgainstDdC2Sort() {
  compareAlgorithms("Entering compareChenSortAgainstCut2 ........\n",
		    chenSort, cut2);
} // compareChenSortAgainstDdC2Sort()

void compareMergeSort0AgainstDdC2Sort() {
  compareAlgorithms("Entering compareMergeSort0AgainstCut2 ........\n",
		    mergeSort0, cut2);
} // compareMergeSort0AgainstDdC2Sort()

void compareMergeSortDDCAgainstDdC2Sort() {
  compareAlgorithms("Entering compareMergeSortDDCAgainstCut2 ........\n",
		    mergeSortDDC, cut2);
} // compareMergeSortDDCAgainstDdC2Sort()

void compareAlgorithmsTime(char *label, void (*alg1)(), void (*alg2)() ) {
  printf(label);
  // int alg1Time, alg2Time, T;
  double alg1Time, alg2Time, T;
  int seed;
  int z;
  // int siz = 1024;
  // int seedLimit = 32 * 1024;
  int siz = 1024 * 1024;
  // int seedLimit = 32;
  int seedLimit = 128;
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
      // int TFill = clock();
      struct timeval tim;
      gettimeofday(&tim, NULL);
      double TFILL=tim.tv_sec+(tim.tv_usec/1000000.0);
      for (seed = 0; seed < seedLimit; seed++) 
	fillarray(A, siz, seed);
      // TFill = clock() - TFill;
      gettimeofday(&tim, NULL);
      TFILL=tim.tv_sec+(tim.tv_usec/1000000.0) - TFILL;
      // T = clock();
      gettimeofday(&tim, NULL);
      T=tim.tv_sec+(tim.tv_usec/1000000.0);
      for (seed = 0; seed < seedLimit; seed++) { 
	fillarray(A, siz, seed);
	(*alg1)(A, 0, siz-1); 
      }
      // alg1Time = clock() - T - TFill;
      gettimeofday(&tim, NULL);
      alg1Time=tim.tv_sec+(tim.tv_usec/1000000.0) - T - TFILL;
      // T = clock();
      gettimeofday(&tim, NULL);
      T=tim.tv_sec+(tim.tv_usec/1000000.0);
      for (seed = 0; seed < seedLimit; seed++) { 
	fillarray(A, siz, seed);
	(*alg2)(A, 0, siz-1);
      }
      // alg2Time = clock() - T - TFill;
      gettimeofday(&tim, NULL);
      alg2Time=tim.tv_sec+(tim.tv_usec/1000000.0) - T - TFILL;
      printf("%s %d %s", "siz: ", siz, " ");
      printf("%s %f %s", "alg1Time: ", alg1Time, " ");
      printf("%s %f %s", "alg2Time: ", alg2Time, " ");
      float frac = 0;
      if ( alg1Time != 0 ) frac = alg2Time / ( 1.0 * alg1Time );
      printf("%s %f %s", "frac: ", frac, "\n");
    }
    free(A);
    siz = siz * 2;
    seedLimit = seedLimit / 2;
  }
} // end compareAlgorithmsTime

void compareMergeSort0AgainstParMergeSort2() {
  compareAlgorithmsTime("Entering compareMergeSort0AgainstParMergeSort2 ........\n",
		    mergeSort0, parMergeSort2);
} // compareMergeSort0AgainstParMergeSort2()

void compareMergeSort0AgainstParMergeSort4() {
  compareAlgorithmsTime("Entering compareMergeSort0AgainstParMergeSort4 ........\n",
		    mergeSort0, parMergeSort4);
} // compareMergeSort0AgainstParMergeSort4()

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

void vswap(int *A, int N, int N3, int eq) {
  int t;
  while ( 0 < eq ) { eq--; t = A[N]; A[N++] = A[N3]; A[N3++] = t; }
}

const int small = 120;
void quicksort0c();
// Quicksort function for invoking quicksort0c.
void quicksort0(int *A, int N, int M) {
  //  printf("quicksort0 N %i M %i \n", N, M);
  int L = M - N;
  if ( L <= 0 ) return;
  if ( L < 7 ) { 
    insertionsort(A, N, M);
    return;
  }
  int depthLimit = 2.5 * floor(log(L));
  quicksort0c(A, N, M, depthLimit);
} // end quicksort0

void dflgm();
void insertionsort();
// Quicksort equipped with a defense against quadratic explosion;
// calling heapsort if depthlimit exhausted
void quicksort0c(int *A, int N, int M, int depthLimit) {
  // printf("Enter quicksort0c N: %d M: %d %d\n", N, M, depthLimit);
  // printf(" gap %d \n", M-N);
  while ( N < M ) {
    int L = 1+ M - N;
    if ( L < 7 ) {
      insertionsort(A, N, M);
      return;
    }
    if ( depthLimit <= 0 ) {
      heapc(A, N, M);
      return;
    }
    depthLimit--;

    // 7 <= L
    int p0 = N + (L>>1); // N + L/2;
    if ( 7 < L ) {
      int pn = N;
      int pm = M;
      if ( 40 < L ) {
	int d = (L-2)>>3; // L/8;
	pn = med(A, pn, pn + d, pn + 2 * d);
	p0 = med(A, p0 - d, p0, p0 + d);
	pm = med(A, pm - 2 * d, pm - d, pm);
      }
      p0 = med(A, pn, p0, pm);
    }

    /* optional check when inputs have many equal elements
       // if ( compareXY(A[N], A[M]) == 0 ) {
       if ( A[N] == A[M] ) {
          dflgm(A, N, M, p0, quicksort0c, depthLimit);
          return;
       } */

    // p0 is index to 'best' pivot ...    
    iswap(N, p0, A); // ... and is put in first position

    register int T = A[N]; // pivot
    register int I, J; // indices
    register int AI, AJ; // array values
    // int k;
    // int small = 120; 

    if ( L <= small ) {

      I = N+1; J = M; 
      int N2 = I, M2 = J, l, r, eql, eqr;
    Left2:
      // while ( I <= J && (r = compareXY(A[I], T)) <= 0 ) {
      while ( I <= J && A[I] <= T ) {
	  if ( A[I] == T ) { iswap(N2, I, A); N2++; }
	  I++;
      }
      // while ( I <= J && (r = compareXY(A[J], T)) >= 0 ) {
      while ( I <= J && A[J] >= T ) { 
	if ( A[J] == T ) { iswap(M2, J, A); M2--; }
	J--;
      }
      if ( I > J ) goto Skip2; 
      iswap(I, J, A);
      I++; J--;
      goto Left2;

  Skip2:
      l = N2-N; r = I-N2;
      eql = ( l < r ? l : r );
      vswap(A, N, I-eql, eql); 
      l = J+N-N2;
      if ( 0 < l-N )  { 
	// printf("Left recursion N %i l %i\n", N, l);
	quicksort0c(A, N, l, depthLimit);
      }
      l = M2-J; r = M-M2;
      eqr = ( l < r ? l : r );
      vswap(A, I, M-eqr+1, eqr);
      // right 'recursion' tail
      N = I + (M-M2);
      if ( N < M ) { continue; }
      return;
    }

    // 1st round of partitioning
	// The left segment has elements <= T
	// The right segment has elements > T
    /*
	  |----------]-------------[-----------|
	  N   <=T    I             J   >T      M   
    */
    J = M+1;
    // while ( compareXY(T, A[--J]) < 0 );
    while ( T < A[--J] );
    if ( N == J ) { // poor pivot  N < x -> T < A[x], suspect bad input
      int px =  N + (L>>1); // N + L/2;
      iswap(p0, px, A);
      dflgm(A, N, M, px, quicksort0c, depthLimit);
      return;
    }
    AJ = A[J]; // A[J] <= T

    // N < J <= M  
    I = N+1;
    if (J < M ) {
      // while ( compareXY(A[I], T) <= 0 ) I++;
      while ( A[I] <= T ) I++;
    }
    else { // J = M
      // if ( compareXY(T, A[M]) == 0 ) { // bail out
      if ( T == A[M] ) { // bail out
	int px =  N + (L>>1); // N + L/2;
	iswap(p0, px, A);
	dflgm(A, N, M, px, quicksort0c, depthLimit);
	return;
      }
      // define M2?
      // while ( I < J && compareXY(A[I], T) <= 0 ) { I++; }
      while ( I < J && A[I] <= T ) { I++; }
      if ( M == I ) { // all elements are <= T, suspect bad input
	int px =  N + (L>>1); // N + L/2;
	iswap(p0, px, A);
	dflgm(A, N, M, px, quicksort0c, depthLimit);
	return;
      }
    }

    if ( I == J ) {
      I++; 
      goto Skip;
    }
    if ( I < J ) { // swap
      A[J] = A[I]; A[I] = AJ;
      if ( I+1 == J ) {
	J--; I++;
	goto Skip;
      }
    } else { // J+1 = I }
      goto Skip; 
    }
    // fall through

  Left: 
    /*
	  |----------]-------------[-----------|
	  N   <=T    I             J   >T      M   
    */
    // while ( compareXY(A[++I],  T) <= 0 ); 
    while ( A[++I] <=  T ); 
    if ( J < I ) goto Skip;
    AI = A[I];
    // while ( compareXY(T, A[--J]) < 0 ); 
    while ( T < A[--J] ); 
    AJ = A[J];
    if ( I < J ) { // swap
      A[I] = AJ; A[J] = AI;
      goto Left;
    }
 Skip:
    // Tail iteration
    if ( (I - N) < (M - J) ) { // smallest one first
      quicksort0c(A, N, J, depthLimit);
      N = I; 
    } else {
      quicksort0c(A, I, M, depthLimit);
      M = J;
    }
  } // end of while loop
} // end of quicksort0c


/*
This code comes from:
JON L. BENTLEY & M. DOUGLAS McILROY / Engineering a Sort Function
SOFTWARE-PRACTICE AND EXPERIENCE, VOL. 23(11), 1249-1265 (NOVEMBER 1993)
http://www.cs.ubc.ca/local/reading/proceedings/spe91-95/spe/vol23/issue11/spe862jb.pdf
 */
void insertionsort(int *r, int lo, int up) {
  int pm, pl;
  for (pm = lo; pm <= up; pm++)
    for (pl = pm; pl > lo && r[pl-1] > r[pl]; pl--)
      iswap(pl, pl-1, r);
} // end of insertionsort 

void iswap(int p, int q, int *x) {
  int t = x[p];
  x[p] = x[q];
  x[q] = t;
} // end of iswap

// a very simple quicksort /// actually nearly identical now to cut2 
void myqs(int *A, int N, int M) {
  int L = M-N;
  if (L <= 0) return;
  int dp = 2.5 * floor(log(L));
  myqsc(A, N, M, dp);
}

void myqsc(int *A, int N, int M, int depthLimit) {
  int L;
 again:
  // printf("myqs N %i M %i\n", N, M);
  L = M - N;
  if ( L <= 10 ) { 
    insertionsort(A, N, M);
    return;
  }
  if ( depthLimit <= 0 ) {
    heapc(A, N, M);
    return;
  }
  depthLimit--;

  /*
    // 10 < L
    // grab median of 3 or 9 to get a good pivot
    int pm = (N+M)/2;
    int pl = N;
    int pn = M;
    if (L > 40) { // do median of 9
      int d = L/8;
      pl = med(A, pl, pl + d, pl + 2 * d);
      pm = med(A, pm - d, pm, pm + d);
      pn = med(A, pn - 2 * d, pn - d, pn);
    }
    pm = med(A, pl, pm, pn); // pm is index to 'best' pivot ...
  */
// Check for duplicates
        int sixth = (L + 1) / 6;
        int e1 = N  + sixth;
        int e5 = M - sixth;
	int e3 = N + L/2;// The midpoint
        // int e3 = (N+M) / 2; // The midpoint
        int e4 = e3 + sixth;
        int e2 = e3 - sixth;

        // Sort these elements using a 5-element sorting network
        int ae1 = A[e1], ae2 = A[e2], ae3 = A[e3], ae4 = A[e4], ae5 = A[e5];
	int t;

        if (ae1 > ae2) { t = ae1; ae1 = ae2; ae2 = t; }
        if (ae4 > ae5) { t = ae4; ae4 = ae5; ae5 = t; }
        if (ae1 > ae3) { t = ae1; ae1 = ae3; ae3 = t; }
        if (ae2 > ae3) { t = ae2; ae2 = ae3; ae3 = t; }
        if (ae1 > ae4) { t = ae1; ae1 = ae4; ae4 = t; }
        if (ae3 > ae4) { t = ae3; ae3 = ae4; ae4 = t; }
        if (ae2 > ae5) { t = ae2; ae2 = ae5; ae5 = t; }
        if (ae2 > ae3) { t = ae2; ae2 = ae3; ae3 = t; }
        if (ae4 > ae5) { t = ae4; ae4 = ae5; ae5 = t; }

        A[e1] = ae1; A[e2] = ae2; A[e3] = ae3; A[e4] = ae4; A[e5] = ae5;

	// Fix end points
	if ( ae1 < A[N] ) iswap(N, e1, A);
	if ( A[M] < ae5 ) iswap(M, e5, A);

	register int pv = ae3; // pivot 
	// check Left label invariant
	if ( pv <= A[N] || A[M] < pv ) {
	   // give up because cannot find a good pivot
	   // dflgm is a dutch flag type of algorithm
	  // void myqsc();
	  dflgm(A, N, M, e3, myqsc, depthLimit);
	  return;
	}

    int i = N; int j = M; // indices
	// The left segment has elements < T
	// The right segment has elements >= T
 loop:
    while ( pv < A[j] ) j--;
    while ( A[i] <= pv ) i++;
    if ( i < j ) { t = A[i]; A[i] = A[j]; A[j] = t; goto loop; }
    if ( i-N < M-i ) { // order the sub-segments
      myqs(A, N, j); // myqs(A, i, M); 
      N = i; goto again;
    } else {
      myqs(A, i, M); // myqs(A, N, j); 
      M = j; goto again;
    }
} // end myqs

// calculate the median of 3
int med(int *A, int a, int b, int c) {
  return A[a] < A[b] ?
    (A[b] < A[c]  ? b : A[a] < A[c] ? c : a)
    : (A[b] > A[c] ? b : A[a] > A[c] ? c : a);
} // end med

void cut2fc();
// cut2f is used as a best in class quicksort implementation 
// with a defense against quadratic behavior due to duplicates
// cut2f is a support function to call up the workhorse cut2fc
void cut2f(int *A, int N, int M) { 
  // printf("cut2f %d %d %d \n", N, M, M-N;
  int L = M - N;
  if ( L < cut2fLimit ) { 
    cut2(A, N, M);
    return;
  }
  int depthLimit = 2.5 * floor(log(L));
  cut2fc(A, N, M, depthLimit);
} // end cut2f

// Cut2fc does 2-partitioning with one pivot.
// Cut2fc invokes cut2c when trouble is encountered.
void cut2fc(int *A, int N, int M, int depthLimit) {
  int L;
  Start:
  // printf("cut2fc %d %d %d \n", N, M, M-N);
  if ( depthLimit <= 0 ) {
    heapc(A, N, M);
    return;
  }
  L = M - N +1;
  if ( L < cut2fLimit ) { 
    cut2c(A, N, M);
    return;
  }
  depthLimit--;

    int p0 = N + (L>>1); // N + L/2;
    int pn = N;
    int pm = M;
    int d = (L-2)>>3; // L/8;
    pn = med(A, pn, pn + d, pn + 2 * d);
    p0 = med(A, p0 - d, p0, p0 + d);
    pm = med(A, pm - 2 * d, pm - d, pm);
    p0 = med(A, pn, p0, pm);
    iswap(N, p0, A); // ... and is put in first position

  register int T = A[N];  // pivot
  register int I, J; // indices
  register int AI, AJ; // array values
  // int k; // tracing
    // 1st round of partitioning
	// The left segment has elements <= T
	// The right segment has elements > T
    /*
	  |----------]-------------[-----------|
	  N   <=T    I             J   >T      M   
    */

    J = M+1;
    // while ( compareXY(T, A[--J]) < 0 );
    while ( T < A[--J] < 0 );
    if ( N == J ) { 
      cut2c(A, N, M, depthLimit);
      return;
    }
    AJ = A[J]; // A[J] <= T
    I = N+1;
    if (J < M ) {
      // while ( compareXY(A[I], T) <= 0 ) I++;
      while ( A[I] <= T ) I++;
    }
    else { // J = M
      if ( T == A[M] ) { // bail out
	cut2c(A, N, M, depthLimit);
	return;
      }
      // while ( I < J && compareXY(A[I], T) <= 0 ) { I++; }
      while ( I < J && A[I] <= T ) { I++; }
      if ( M == I ) { // all elements are <= T, suspect bad input
	cut2c(A, N, M, depthLimit);
	return;
      }
    }

    if ( I < J ) { // swap
      A[J] = A[I]; A[I] = AJ;
      if ( I+1 == J ) { J--; I++; goto Skip; }
      goto Left;
    }
    if (I == J+1 ) goto Skip;
    // I = J
     I++; 
     goto Skip;


     // The left segment has elements < T
     // The right segment has elements >= T
     // Proceed with fast loops
  Left:
     // while ( compareXY(A[++I], T) <= 0 ); 
     while ( A[++I] <= T ); 
     if ( J < I ) goto Skip;
     AI = A[I];
     // while ( compareXY(T, A[--J]) < 0 ); 
     while ( T < A[--J] ); 
     AJ = A[J];
     if ( I < J ) { // swap
       A[I] = AJ; A[J] = AI;
       goto Left;
     }
 Skip:
     // Tail iteration
     if ( (I - N) < (M - J) ) { // smallest one first
       cut2fc(A, N, J, depthLimit);
       N = I; 
       goto Start;
     }
     cut2fc(A, I, M, depthLimit);
     M = J;
     goto Start;

} // end of cut2fc
// (*  OF cut2; *) the brackets remind that this was once Pascal code

void cut2(int *A, int N, int M) {
  int L = M-N;
  if (L <= 0) return;
  int dp = 2.5 * floor(log(L));
  cut2c(A, N, M, dp);
}

void cut2c(int *A, int N, int M, int depthLimit)
{
  register int I, J; // indices
  register int AI, AJ; // array values
  register int T; // pivot
  int L;

  Start:
  // printf("cut2 N %i M %i depthLimit %i\n", N, M, depthLimit);
  L = M - N;
  if ( L < cut2Limit ) { 
      quicksort0c(A, N, M, depthLimit);
    return;
  }
  if ( depthLimit <= 0 ) {
    heapc(A, N, M);
    return;
  }
  depthLimit--;

  // Check for duplicates
  int sixth = (L + 1) / 6;
  int e1 = N  + sixth;
  int e5 = M - sixth;
  int e3 = N + (L>>1); // N + L/2;// The midpoint
  // int e3 = (N+M) / 2; // The midpoint
  int e4 = e3 + sixth;
  int e2 = e3 - sixth;

  // Sort these elements using a 5-element sorting network
  int ae1 = A[e1], ae2 = A[e2], ae3 = A[e3], ae4 = A[e4], ae5 = A[e5];
  int t;
  
  if (ae1 > ae2) { t = ae1; ae1 = ae2; ae2 = t; }
  if (ae4 > ae5) { t = ae4; ae4 = ae5; ae5 = t; }
  if (ae1 > ae3) { t = ae1; ae1 = ae3; ae3 = t; }
  if (ae2 > ae3) { t = ae2; ae2 = ae3; ae3 = t; }
  if (ae1 > ae4) { t = ae1; ae1 = ae4; ae4 = t; }
  if (ae3 > ae4) { t = ae3; ae3 = ae4; ae4 = t; }
  if (ae2 > ae5) { t = ae2; ae2 = ae5; ae5 = t; }
  if (ae2 > ae3) { t = ae2; ae2 = ae3; ae3 = t; }
  if (ae4 > ae5) { t = ae4; ae4 = ae5; ae5 = t; }
  // ... and reassign
  A[e1] = ae1; A[e2] = ae2; A[e3] = ae3; A[e4] = ae4; A[e5] = ae5;

  // if ( compareXY(ae5, ae3) <= 0) {
  if ( ae5 <= ae3 ) {
    // give up because cannot find a good pivot
    // dflgm is a dutch flag type of algorithm
    void cut2c();
    dflgm(A, N, M, e3, cut2c, depthLimit);
    return;
  }

  iswap(e5, M, A); // right corner OK now
  // put pivot in the left corner
  iswap(N, e3, A);
  T = A[N];

  // initialize running indices
  I= N;
  J= M;

  // The left segment has elements <= T
  // The right segment has elements > T
 Left:
  while ( A[++I] <= T ); AI = A[I];
  while ( T < A[--J] ); AJ = A[J];
  if ( I < J ) {
    A[I] = AJ; A[J] = AI;
    goto Left;
  }

  if ( (I - N) < (M - J) ) { // smallest one first
    cut2c(A, N, J, depthLimit);
    N = I; 
    goto Start;
  }
  cut2c(A, I, M, depthLimit);
  M = J;
  goto Start;
} // (*  OF cut2; *) The brackets remind that this was 1985? Pascal code

// tps is the header function for the three partition sorter tpsc
void tpsc();
void tps(int *A, int N, int M) {
  int L = M - N;
  if ( L < cut3Limit ) { 
    cut2f(A, N, M);
    return;
  }
  int depthLimit = 2.5 * floor(log(L));
  tpsc(A, N, M, depthLimit);
} // end tps

void tpsc(int *A, int N, int M, int depthLimit) {  
  // int z; // for tracing
  register int i, j, up, lw; // indices
  register int ai, aj, am; // array values
  int pl, pr; // pivots

 // A 3d recursive call is avoided by jumping back to Start.  
 Start:

  if ( depthLimit <= 0 ) { // prevent quadradic explosion
    heapc(A, N, M);
    return;
  }
  int L = M - N;
  if ( L < cut3Limit ) {
    cut2fc(A, N, M, depthLimit);
    return;
  }
  depthLimit--;

        int sixth = (L + 1) / 6;
        int e1 = N  + sixth;
        int e5 = M - sixth;
	int e3 = N + (L>>1); // N + L/2;// The midpoint
        // int e3 = (N+M) / 2; // The midpoint
        int e4 = e3 + sixth;
        int e2 = e3 - sixth;
        // Sort these elements using a 5-element sorting network

        int ae1 = A[e1], ae2 = A[e2], ae3 = A[e3], ae4 = A[e4], ae5 = A[e5];
	int t;

        if (ae1 > ae2) { t = ae1; ae1 = ae2; ae2 = t; }
        if (ae4 > ae5) { t = ae4; ae4 = ae5; ae5 = t; }
        if (ae1 > ae3) { t = ae1; ae1 = ae3; ae3 = t; }
        if (ae2 > ae3) { t = ae2; ae2 = ae3; ae3 = t; }
        if (ae1 > ae4) { t = ae1; ae1 = ae4; ae4 = t; }
        if (ae3 > ae4) { t = ae3; ae3 = ae4; ae4 = t; }
        if (ae2 > ae5) { t = ae2; ae2 = ae5; ae5 = t; }
        if (ae2 > ae3) { t = ae2; ae2 = ae3; ae3 = t; }
        if (ae4 > ae5) { t = ae4; ae4 = ae5; ae5 = t; }
	// reassign
        A[e1] = ae1; A[e2] = ae2; A[e3] = ae3; A[e4] = ae4; A[e5] = ae5;

	if ( ae2 == ae3 || ae3 == ae4 ) {
	  // Give up, cannot find good pivots
	  dflgm(A, N, M, e3, tpsc, depthLimit);
	  return;
	}

	// Fix end points
	iswap(N, e2, A); iswap(M, e4, A);
	pl = A[N]; pr = A[M]; // they are there temporarily

	// initialize running indices
	i = N+1; j = M-1; 
	lw = e3-1; up = e3+1;

	while ( A[i] < pl ) i++;
	while ( pr < A[j] ) j--;

	/* 
	  |)----------(--)-------------(|
	 N i         lw  up            j M
	  N <= x < i -> A[x] < pl
	  lw < x < up -> pl <= A[x] <= pr
	  j < x <= M -> pr < A[x]
	*/
 again:
	// up <= j
	while ( i <= lw ) {
	  ai = A[i];
	  if ( ai < pl ) { i++; continue; }
	  if ( pr < ai) { // ai -> R
	    while ( pr < A[j] ) j--;
	    aj = A[j]; // aj <= pr
	    if ( aj < pl ) { // aj -> L
	      A[i++] = aj; A[j--] = ai; 
	      if ( j < up ) { j++;
		if ( lw < i ) { i--; goto done; }
		goto rightClosed;
	      }
	      continue;
	    }
	    // aj -> M
	    if ( j < up ) { // right gap closed
	      j++; goto rightClosedAIR;
	    } // up <= j
	    goto AIRAJM;
	  } 
	  // ai -> M
	  goto AIM;
	}
	// left gap closed
	i--;
	goto leftClosed;

 AIM:
	/* 
	  |)----------(--)-------------(|
	 N i         lw  up            j M
	 ai -> M
	*/
	if ( lw < i ) { i--; goto leftClosed; }
	// i <= lw
	am = A[lw];
	if ( am < pl ) { // am -> L
	  A[i++] = am; A[lw--] = ai; 
	  if ( lw < i ) { i--; goto leftClosed; }
	  goto again; 
	}
	if ( pr < am ) {
	repeatR:
	  // ai -> M and am -> R
	  aj = A[j]; 
	  if ( pr < aj ) { j--; // aj -> R
	    if ( j < up ) { // right closed
	      A[lw--] = A[j]; A[j] = am;
	      if ( i == lw ) { i--; goto done; }
	      goto rightClosedAIM;
	    } 
	    goto repeatR; 
	  }
	  if ( aj < pl ) { // aj -> L
	    A[i++] = aj; A[lw--] = ai; A[j--] = am; 
	    if ( j < up ) { j++; // right closed
	      if ( lw < i ) { // left closed
		i--; goto done;
	      }
	      goto rightClosed;
	    }
	    goto again;
	  }
	  // aj -> M
	  if ( j < up ) { // right closed
	    j++; 
	    goto rightClosedAIM;
	  }
	  A[lw--] = aj; A[j--] = am; 
	  if ( j < up ) { j++; // right closed
	    if ( lw < i ) { i--; goto done; }
	    goto rightClosed;
	  }
	  goto AIM; 
	}
	// am -> M
	if ( i == lw ) { i--; goto leftClosed; }
	lw--; 
	goto AIM; 

 AIRAJM:
	/* 
	  |)----------(--)-------------(|
	 N i         lw  up            j M
	 ai -> R aj -> M and up <= j
	*/
	am = A[lw];
	if ( am < pl ) { // am -> L
	  A[i++] = am; A[lw--] = aj; A[j--] = ai; 
	  if ( j < up ) { j++; // right closed
	    if ( lw < i ) { // left closed
	      i--; goto done;
	    }
	    goto rightClosed;
	  }
	  if ( lw < i ) { i--; goto leftClosed; }
	  goto again; 
	}
	if ( am <= pr ) { // am -> M
	  lw--; goto AIRAJM;
	}
	// am -> R
	if ( i == lw ) {
	  A[j--] = ai; A[i--] = aj; 
	  if ( j < up ) { j++; goto done; }
	  goto leftClosed;
	}
	// i < lw
	A[lw--] = aj; A[j--] = am; 
	if ( j < up ) { // right closed
	  j++; goto rightClosedAIR;
	}
	goto AIR;
	  
 AJM:
	/* 
	  |)----------(--)-------------(|
	 N i         lw  up            j M
	 aj -> M
	*/
	am = A[lw];
	if ( pr < am ) { // am -> R
	  A[lw--] = aj; A[j--] = am; 
	  if ( j < up ) { // right closed
	    j++; goto rightClosed;
	  }
	  goto again;
	}
	if ( pl <= am ) { // am -> M
	  lw--; 
	  if ( lw < i ) { // left closed
	    i--; goto repeatM3;
	  }
	  goto AJM;
	}
	// aj -> M and am -> L
      repeatL:
	ai = A[i];
	if ( ai < pl ) { i++; goto repeatL; }
	if ( pr < ai ) { // ai -> R
	  A[i++] = am; A[lw--] = aj; A[j--] = ai;
	  if ( j < up ) { j++; // right closed
	    if ( lw < i ) { i--; goto done; }
	    goto rightClosed;
	  }
	  if ( lw < i ) { i--; goto leftClosed; } 
	  goto again;
	}
	// ai -> M
	A[i++] = am; A[lw--] = ai; 
	if ( lw < i ) { i--; goto repeatM3; } 
	goto AJM;

 AIR:
	/* 
	  |)----------(--)-------------(|
	 N i         lw  up            j M
	 ai -> R
	*/
	aj = A[j];
	if ( pr < aj ) { j--; goto AIR; }
	if ( aj < pl ) {
	  A[i++] = aj; A[j--] = ai; 
	  if ( j < up ) { j++; // right closed
	    if ( lw < i ) { i--; goto done; }
	    goto rightClosed;
	  }
	  goto again;
	}
	// aj -> M
	if ( j < up ) { j++; goto rightClosedAIR; }
	goto AIRAJM;

 leftClosed: 
	  /* 
	  |--]------------)-------------(|
	 N   i            up            j M
	  */
	aj = A[j];
	if ( pr < aj ) { j--; goto leftClosed; }
	if ( aj < pl ) { // aj -> L
	repeatM:
	  am = A[up];
	  if ( pr < am ) { // am -> R
	    A[j--] = am; A[up++] = A[++i]; A[i] = aj; 
	    if ( j < up ) { j++; goto done; }
	    goto leftClosed;
	  }
	  if ( pl <= am ) { // am -> M
	    up++; goto repeatM;
	  }
	  // am -> L
	  if ( up == j ) {
	    A[j++] = A[++i]; A[i] = aj; 
	    goto done;
	  }
	  // up < j
	  A[up++] = A[++i]; A[i] = am;  
	  goto repeatM;
	}

	// aj -> M
	if ( j < up ) { j++; goto done; }
	// up <= j
      repeatM3:
	am = A[up];
	if ( pr < am ) { // am -> R
	  A[up++] = aj; A[j--] = am; 
	  if ( j < up ) { j++; goto done; }
	  goto leftClosed;
	}
	if ( am < pl ) { // am -> L
	  A[up++] = A[++i]; A[i] = am;
	  goto repeatM3;
	}
	// am -> M
	if ( j <= up ) { j++; goto done; }
	up++; goto repeatM3;
	
 rightClosedAIR:
	/* 
	   |-)----------(--[-------------|
	   N i         lw  j             M
	   ai -> R
	*/
	am = A[lw];
	if ( am < pl ) { // am -> L
	  A[i++] = am; A[lw--] = A[--j]; A[j] = ai;
	  if ( lw < i ) { i--; goto done; }
	  goto rightClosed; 
	}
	if ( am <= pr ) { lw--; goto rightClosedAIR; }
	// am -> R
	if ( i == lw ) {
	  A[i--] = A[--j]; A[j] = ai; 
	  goto done;
	}
	// i < lw
	A[lw--] = A[--j]; A[j] = am;
	goto rightClosedAIR;

 rightClosedAIM:
	/* 
	   |-)----------(--[-------------|
	   N i         lw  j             M
	   ai -> M
	*/
	am = A[lw];
	if ( am < pl ) { // am -> L
	  A[i++] = am; A[lw--] = ai;
	  if ( lw < i ) { i--; goto done; }
	  goto rightClosed;
	}
	if ( pr < am ) { // am -> R
	  A[lw--] = A[--j]; A[j] = am; 
	  goto rightClosedAIM;
	}
	// am -> M
	if ( i == lw ) { i--; goto done; }
	lw--; 
	goto rightClosedAIM;

 rightClosed:
	/* 
	   |-)----------(--[-------------|
	   N i         lw  j             M
	   i <= lw
	*/
	while ( A[i] < pl ) i++;
	ai = A[i];
	if ( pr < ai ) goto rightClosedAIR; 
	// ai -> M
	if ( lw < i ) { i--; goto done;	}
	goto rightClosedAIM;
 done:
	iswap(N, i--, A); iswap(M, j++, A); // put the pivots in place
    /*
      |---]---------[---------|
      N   i         j         M
    */


    /*
      for ( z = N; z <= i; z++ )
	if ( pl <= A[z] ) {
	  printf("doneL z %i\n", z);
	  printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
	  exit(0);
	}
      for ( z = i+1; z < j; z++ )
	if ( A[z] < pl || pr < A[z] ) {
	  printf("doneM z %i\n", z);
	  printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
	  exit(0);
	}
      for ( z = j; z <= M ; z++ )
	if ( A[z] <= pr ) {
	  printf("doneR z %i\n", z);
	  printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
	  exit(0);
	}
      */
	// tpsc(A, N, i, depthLimit);
	// tpsc(A, i+1, j-1, depthLimit);
	// tpsc(A, j, M, depthLimit);
      if ( i-N < j-i ) {
	tpsc(A, N, i, depthLimit);
	if ( j-i < M-j ) {
	   tpsc(A, i+1, j-1, depthLimit);
	   N = j; goto Start;
	   // (*cut)(A, j, M, depthLimit);
	   // return;
	}
	tpsc(A, j, M, depthLimit);
	N = i+1; M = j-1; goto Start;
	// (*cut)(A, i+1, j-1, depthLimit);
	// return;
      }
      tpsc(A, i+1, j-1, depthLimit);
      if ( i-N < M-j ) {
	tpsc(A, N, i, depthLimit);
	N = j; goto Start;
	// (*cut)(A, j, M, depthLimit);
	// return;
      }
      tpsc(A, j, M, depthLimit);
      M = i; goto Start;
      // (*cut)(A, N, i, depthLimit);
} // end tpsc

// tps5 another three partition function
// DEPRECATED
void tpsc5();
void tps5(int *A, int N, int M) {
  int L = M - N;
  if ( L < cut2Limit ) { 
    quicksort0(A, N, M);
    return;
  }
  int depthLimit = 2.5 * floor(log(L));
  tpsc5(A, N, M, depthLimit);
} // end tps5

void tpsc5(int *A, int N, int M, int depthLimit) 
{
  // register int k; // for-index
  // int z; // for tracing
  register int i, j, up, lw; // indices
  register int x, y; // array values
  register int pl, pr; // pivots

 // A 3d recursive call is avoided by jumping back to Start.  
 Start:
  //  printf("tspc5 N %d M %d depthLimit %d\n", N,M,depthLimit);
  if ( depthLimit <= 0 ) { // avoid quadradic explosion
    heapc(A, N, M);
    return;
  }
  int L = M - N;
  // if ( L < cut2Limit ) { // delegated small segments
  //   quicksort0c(A, N, M, depthLimit);
  if ( L < cut3Limit ) {
    cut2c(A, N, M, depthLimit);
    return;
  }
  depthLimit--;

        int sixth = (L + 1) / 6;
        int e1 = N  + sixth;
        int e5 = M - sixth;
	int e3 = N + (L>>1); // N + L/2;// The midpoint
        // int e3 = (N+M) / 2; // The midpoint
        int e4 = e3 + sixth;
        int e2 = e3 - sixth;
        // Sort these elements using a 5-element sorting network
        int ae1 = A[e1], ae2 = A[e2], ae3 = A[e3], ae4 = A[e4], ae5 = A[e5];
	int t;

        if (ae1 > ae2) { t = ae1; ae1 = ae2; ae2 = t; }
        if (ae4 > ae5) { t = ae4; ae4 = ae5; ae5 = t; }
        if (ae1 > ae3) { t = ae1; ae1 = ae3; ae3 = t; }
        if (ae2 > ae3) { t = ae2; ae2 = ae3; ae3 = t; }
        if (ae1 > ae4) { t = ae1; ae1 = ae4; ae4 = t; }
        if (ae3 > ae4) { t = ae3; ae3 = ae4; ae4 = t; }
        if (ae2 > ae5) { t = ae2; ae2 = ae5; ae5 = t; }
        if (ae2 > ae3) { t = ae2; ae2 = ae3; ae3 = t; }
        if (ae4 > ae5) { t = ae4; ae4 = ae5; ae5 = t; }
	// ... and reassign
        A[e1] = ae1; A[e2] = ae2; A[e3] = ae3; A[e4] = ae4; A[e5] = ae5;

	// Fix end points
	if ( ae1 < A[N] ) iswap(N, e1, A);
	if ( A[M] < ae5 ) iswap(M, e5, A);

	if ( ae2 == ae3 || ae3 == ae4 ) {
	  dflgm(A, N, M, e3, tpsc5, depthLimit);
	  return;
	}

	pl = A[e2]; pr = A[e4]; // left and right pivots
	if ( pl <= A[N] || A[M] <= pr ) {
	  dflgm(A, N, M, e3, tpsc5, depthLimit);
	  return;
	}

	// int z; // for tracing

	// initialize running indices
	i = N+1; j = M-1; lw = e3-1; up = e3+1;
	x = A[i]; A[i] = A[N]; i++; // create hole at A[N]

	  /* 
	  |)----------(--)-------------(|
	 N i         lw  up            j M
	  N+1 < x < i -> A[x] < pl
	  lw < x < up -> pl <= A[x] <= pr
	  j < x <= M -> pr < A[x]
	  */

  if ( x < pl ) goto L0;
  if ( pr < x ) goto R0;
  goto ML0;

 L0:  
   /*
      |---)-----(----)-------(----|
      N   i     lw  up       j    M
      x -> L
   */
  while ( A[i] < pl ) i++;
  if ( i <= lw ) {
    y = A[i]; A[i++] = x; x = y;
    if ( lw < i ) { i--;
      if ( pr < x ) goto L1R;
      goto L1M;
    }
    if ( pr < x ) goto R0;
    // x -> M
    goto ML0;
  }
  // lw < i
  i--;
  goto L1L;

 ML0:
   /*
      |---)-----(----)-------(----|
      N   i     lw  up       j    M
      x -> ML
   */
  while ( pl <= A[lw] && A[lw] <= pr ) lw--;
  if ( i <= lw ) {
    y = A[lw]; A[lw--] = x; x = y;
    if ( lw < i ) { i--;
      if ( x < pl ) goto L1L;
      goto L1R;
    }
    if ( x < pl ) goto L0;
    // pr < x
    goto R0;
  }
  // lw < i
  i--;
  goto L1M;

 R0:
   /*
      |---)-----(----)-------(----|
      N   i     lw  up       j    M
      x -> R
   */
  while ( pr < A[j] ) j--;
  if ( up <= j ) {
    y = A[j]; A[j--] = x; x = y;
    if ( j < up ) { j++;
      if ( x < pl ) goto R1L;
      goto R1M;
    }
    if ( x < pl ) goto L0;
    // x -> M
    goto MR0;
  }
  // j < up
  j++;
  goto R1R;
  
 MR0:
   /*
      |---)-----(----)-------(----|
      N   i     lw  up       j    M
      x -> MR
   */
  while ( pl <= A[up] && A[up] <= pr ) up++;
  if ( up <= j ) {
    y = A[up]; A[up++] = x; x = y;
    if ( j < up ) { j++;
      if ( pr < x ) goto R1R;
      goto R1L;
    }
    if ( pr < x ) goto R0;
    // x < pr
    goto L0;
  }
  // j < up
  j++;
  goto R1M;

 L1L: 
   /*
      |---]---------)-------(----|
      N   i        up       j    M
      x -> L
   */
  while ( pl <= A[up] && A[up] <= pr ) up++;
  if ( up <= j ) {
    y = A[up]; A[up++] = A[++i]; A[i] = x; x = y;
    if ( j < up ) {
      if ( x < pl ) goto L1Lx;
      // x -> R
      A[N] = A[i]; A[i--] = A[j]; A[j] = x; 
      goto done;
    }
    if ( x < pl ) { goto L1L; }
    // x -> R
    goto L1R;
    }
  // j < up
  L1Lx:
  j++; A[N] = x;
  goto done;

  L1R: 
   /*
      |---]---------)-------(----|
      N   i        up       j    M
      x -> R
   */
  while ( pr < A[j] ) j--;
  if ( up <= j ) {
    y = A[j]; A[j--] = x; x = y;
    if ( j < up ) {
      if ( x < pl ) { j++; A[N] = x; 
	goto done; }
      // x -> M
      j++; A[N] = A[i]; A[i--] = x; 
      goto done;
    }
    if ( x < pl ) goto L1L; 
    // x -> M
    goto L1M;
  }
  // j < up
  A[N] = A[i]; A[i] = A[j]; i--; A[j] = x; 
  goto done;

 L1M: 
   /*
      |---]---------)-------(----|
      N   i        up       j    M
      x -> M
   */
  while ( pl <= A[up] && A[up] <= pr ) up++;
  if ( up <= j ) {
    y = A[up]; A[up++] = x; x = y;
    if ( j < up ) { 
      if ( pr < x ) { 
	A[N] = A[i]; A[i--] = A[j]; A[j] = x; 
	goto done;
      }
      // x -> L
      j++; A[N] = x; 
      goto done;
    }
    if ( pr < x ) goto L1R;
    // x < pl
    goto L1L;
  }
  // j < up
  A[N] = A[i]; A[i] = x; i--; j++;
  goto done;

 R1R: 
   /*
      |---)---(-------------[----|
      N   i   lw            j    M
      x -> R
   */
  while ( pl <= A[lw] && A[lw] <= pr ) lw--;
  if ( i <= lw ) {
    y = A[lw]; A[lw--] = A[--j]; A[j] = x; x = y;
    if ( lw < i ) { i--;
      if ( x < pl ) { A[N] = x; goto done; }
      // x -> R
      A[N] = A[i]; A[i--] = A[--j]; A[j] = x; 
      goto done;
    }
    if ( x < pl ) goto R1L;
    // pr < x
    goto R1R;
  }
  // lw < i
  A[N] = A[--i]; A[i] = A[--j]; i--; A[j] = x; 
  goto done;

 R1L: 
   /*
      |---)---(-------------[----|
      N   i   lw            j    M
      x -> L
   */
  while ( A[i] < pl ) i++;
  if ( i <= lw ) {
    y = A[i]; A[i++] = x; x = y;
    if ( lw < i ) { i--;
      if ( pr < x ) { 
	 A[N] = A[i]; A[i--] = A[--j]; A[j] = x; 
	 goto done;
      }
      // x -> M
      A[N] = A[i]; A[i--] = x; goto done;
    }
    if ( pr < x ) goto R1R;
    // x -> M
    goto R1M;
  }
  // lw < i
  i--; A[N] = x; 
  goto done;

 R1M: 
   /*
      |---)---(-------------[----|
      N   i   lw            j    M
      x -> M
   */
  while ( pl <= A[lw] && A[lw] <= pr ) lw--;
  if ( i <= lw ) {
    y = A[lw]; A[lw--] = x; x = y;
    if ( lw < i ) { i--;
      if ( x < pl ) { A[N] = x; goto done; }
      // x -> R
      A[N] = A[i]; A[i--] = A[--j]; A[j] = x;
      goto done;
    }
    if ( x < pl ) goto R1L;
    // pr < x
    goto R1R;
  }
  // lw < i
  A[N] = A[--i]; A[i] = x; i--; // goto done;

 done:
    /*
      |---]---------[---------|
      N   i         j         M
    */
    /*
      for ( z = N; z <= i; z++ )
	if ( pl <= A[z] ) {
	  printf("doneL z %i\n", z);
	  printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
	  exit(0);
	}
      for ( z = i+1; z < j; z++ )
	if ( A[z] < pl || pr < A[z] ) {
	  printf("doneM z %i\n", z);
	  printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
	  exit(0);
	}
      for ( z = j; z <= M ; z++ )
	if ( A[z] <= pr ) {
	  printf("doneR z %i\n", z);
	  printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
	  exit(0);
	}
      */
	// tpsc5(A, N, i, depthLimit);
	// tpsc5(A, i+1, j-1, depthLimit);
	// tpsc5(A, j, M, depthLimit);
      if ( i-N < j-i ) {
	tpsc5(A, N, i, depthLimit);
	if ( j-i < M-j ) {
	   tpsc5(A, i+1, j-1, depthLimit);
	   N = j; goto Start;
	}
	tpsc5(A, j, M, depthLimit);
	N = i+1; M = j-1;
	goto Start;
      }
      tpsc5(A, i+1, j-1, depthLimit);
      if ( i-N < M-j ) {
	tpsc5(A, N, i, depthLimit);
	N = j; goto Start;
      }
      tpsc5(A, j, M, depthLimit);
      M = i; goto Start;
} // end tpsc5

// another three partition function
// very compact but not competitive on the machines tested
// DEPRECATED
void tpsc4();
void tps4(int *A, int N, int M) {
  int L = M - N;
  if ( L < cut2Limit ) { 
    quicksort0(A, N, M);
    return;
  }
  int depthLimit = 2.5 * floor(log(L));
  tpsc4(A, N, M, depthLimit);
} // end tps4

void tpsc4(int *A, int N, int M, int depthLimit) 
{
  register int k; // for-index
  // int z; // for tracing
  register int i, j; // indices
  register int aj, ak; // array values
  register int pl, pr; // pivots

 // A 3d recursive call is avoided by jumping back to Start.  
 Start:
  // printf("tspc4 N %d M %d depthLimit %d\n", N,M,depthLimit);

  if ( depthLimit <= 0 ) { // avoid quadradic explosion
    heapc(A, N, M);
    return;
  }
  int L = M - N;
  if ( L < cut3Limit ) {
    cut2c(A, N, M, depthLimit);
    return;
  }
  depthLimit--;

        int sixth = (L + 1) / 6;
        int e1 = N  + sixth;
        int e5 = M - sixth;
	int e3 = N + (L>>1); // N + L/2;// The midpoint
        int e4 = e3 + sixth;
        int e2 = e3 - sixth;


        // Sort these elements using a 5-element sorting network
	// /*
        int ae1 = A[e1], ae2 = A[e2], ae3 = A[e3], ae4 = A[e4], ae5 = A[e5];
	int t;

        if (ae1 > ae2) { t = ae1; ae1 = ae2; ae2 = t; }
        if (ae4 > ae5) { t = ae4; ae4 = ae5; ae5 = t; }
        if (ae1 > ae3) { t = ae1; ae1 = ae3; ae3 = t; }
        if (ae2 > ae3) { t = ae2; ae2 = ae3; ae3 = t; }
        if (ae1 > ae4) { t = ae1; ae1 = ae4; ae4 = t; }
        if (ae3 > ae4) { t = ae3; ae3 = ae4; ae4 = t; }
        if (ae2 > ae5) { t = ae2; ae2 = ae5; ae5 = t; }
        if (ae2 > ae3) { t = ae2; ae2 = ae3; ae3 = t; }
        if (ae4 > ae5) { t = ae4; ae4 = ae5; ae5 = t; }

        A[e1] = ae1; A[e2] = ae2; A[e3] = ae3; A[e4] = ae4; A[e5] = ae5;

	// Fix end points
	if ( A[e1] < A[N] ) iswap(N, e1, A);
	if ( A[M] < A[e5] ) iswap(M, e5, A);

	pl = A[e2]; pr = A[e4]; // left and right pivots
	if ( pl == pr ) {
	  dflgm(A, N, M, e3, tpsc4, depthLimit);
	  return;
	}

	// int z; // for tracing

	// initialize running indices
	k = i = N; j = M;

	A[e2] = A[N]; A[e4] = A[M]; // creating holes at the boundaries

	  /* 
	  |o-]-----]--------------------[-o|
	  N   i     k                  j   M
	  N < x < i -> A[x] < pl
	  i <= x < k -> pl <= A[x] <= pr
	  j < x < M -> pr < A[x]
	  */
	i = N;
	while ( A[++i] < pl );
	j = M;
	while ( pr < A[--j] );
	k = i;

 again:
	// printf("again N %i i %i k %i j %i M %i\n", N,i,k,j,M);
	while ( k <= j ) {
	  ak = A[k];
	  if ( ak < pl ) { 
	    // A[k] = A[i]; A[i] = ak; i++; k++; 
	    A[k++] = A[i]; A[i++] = ak; 
	    // printf("cont1 N %i i %i k %i j %i M %i\n", N,i,k,j,M);
	    continue;
	  } 
	  /*
	  if ( ak <= pr ) { // pl <= ak <= pr
	    k++;  continue;
	  }
	  */
	  if ( pr < ak ) {
	  // ak -> R
	  while ( k < j ) {
	    aj = A[j];
	    if ( pr < aj ) { j--; 
	      // printf("cont2 N %i i %i k %i j %i M %i\n", N,i,k,j,M);
	      continue; }
	    if ( pl <= aj ) { // aj -> M
	      // A[k] = aj; A[j] = ak; k++; j--;
	      A[k++] = aj; A[j--] = ak;
	      goto again;
	    }
	    // aj < pl
	    // A[k] = A[i]; A[j] = ak; A[i] = aj; i++; k++; j--;
	    A[k++] = A[i]; A[j--] = ak; A[i++] = aj;
	    goto again;
	  }
	  // j = k
	  i--;
	  goto done;
	  }
	  // pl <= ak <= pr
	  k++;  continue;
	}
	// j < k
	i--;
	j++;

 done:
    /*
      |o--]---------[--------o|
      N   i         j         M
    */
	A[N] = A[i]; A[i] = pl;
	A[M] = A[j]; A[j] = pr;
    /*
      |---[---------]---------|
      N   i         j         M
    */

    /*
      for ( z = N; z < i; z++ )
	if ( pl <= A[z] ) {
	  printf("doneL z %i\n", z);
	  printf("N %i i %i k %i j %i M %i\n", N,i,k,j,M);
	  exit(0);
	}
      for ( z = i; z <= j; z++ )
	if ( A[z] < pl || pr < A[z] ) {
	  printf("doneM z %i\n", z);
	  printf("N %i i %i k %i j %i M %i\n", N,i,k,j,M);
	  exit(0);
	}
      for ( z = j+1; z <= M ; z++ )
	if ( A[z] <= pr ) {
	  printf("doneR z %i\n", z);
	  printf("N %i i %i k %i j %i M %i\n", N,i,k,j,M);
	  exit(0);
	}
      */
      if ( i-N < j-i ) {
	tpsc4(A, N, i-1, depthLimit);
	if ( j-i < M-j ) {
	   tpsc4(A, i, j, depthLimit);
	   N = j+1; goto Start;
	}
	tpsc4(A, j+1, M, depthLimit);
	N = i; M = j;
	goto Start;
      }
      tpsc4(A, i, j, depthLimit);
      if ( i-N < M-j ) {
	tpsc4(A, N, i-1, depthLimit);
	N = j+1; goto Start;
      }
      tpsc4(A, j+1, M, depthLimit);
      M = i-1; goto Start;
} // end tpsc4

// yet another three partition sorter, very compact also
// DEPRECATED
void tpsc3();
void tps3(int *A, int N, int M) {
  int L = M - N;
  if ( L < cut3Limit ) { 
    cut2(A, N, M);
    // quicksort0(A, N, M);
    return;
  }
  int depthLimit = 2.5 * floor(log(L));
  tpsc3(A, N, M, depthLimit);
} // end tps3

void tpsc3(int *A, int N, int M, int depthLimit) 
{
  register int k; // for-index
  // int z;
  register int i, j; // indices
  register int aj, ak; // array values
  register int pl, pr; // pivots

 // A 3d recursive call is avoided by jumping back to Start ... but not done here  
 // Start:
  // printf("tspc N %d M %d depthLimit %d\n", N,M,depthLimit);

  if ( depthLimit <= 0 ) { // avoid quadradic explosion
    heapc(A, N, M);
    return;
  }
  int L = M - N;
  if ( L < cut3Limit ) {
    cut2c(A, N, M, depthLimit);
    return;
  }
  depthLimit--;

        int sixth = (L + 1) / 6;
        int e1 = N  + sixth;
        int e5 = M - sixth;
	int e3 = N + L/2;// The midpoint
        // int e3 = (N+M) / 2; // The midpoint
        int e4 = e3 + sixth;
        int e2 = e3 - sixth;
        // Sort these elements using a 5-element sorting network
        int ae1 = A[e1], ae2 = A[e2], ae3 = A[e3], ae4 = A[e4], ae5 = A[e5];
	int t;

        if (ae1 > ae2) { t = ae1; ae1 = ae2; ae2 = t; }
        if (ae4 > ae5) { t = ae4; ae4 = ae5; ae5 = t; }
        if (ae1 > ae3) { t = ae1; ae1 = ae3; ae3 = t; }
        if (ae2 > ae3) { t = ae2; ae2 = ae3; ae3 = t; }
        if (ae1 > ae4) { t = ae1; ae1 = ae4; ae4 = t; }
        if (ae3 > ae4) { t = ae3; ae3 = ae4; ae4 = t; }
        if (ae2 > ae5) { t = ae2; ae2 = ae5; ae5 = t; }
        if (ae2 > ae3) { t = ae2; ae2 = ae3; ae3 = t; }
        if (ae4 > ae5) { t = ae4; ae4 = ae5; ae5 = t; }

        A[e1] = ae1; A[e2] = ae2; A[e3] = ae3; A[e4] = ae4; A[e5] = ae5;

	// Fix end points
	if ( ae1 < A[N] ) iswap(N, e1, A);
	if ( A[M] < ae5 ) iswap(M, e5, A);

	pl = A[e2]; pr = A[e4];
	if ( pl == pr || pl <= A[N] || A[M] <= pr ) {
	  dflgm(A, N, M, e3, tpsc3, depthLimit);
	  return;
	}

	// p3 = A[e3]; // for dealing with the middle segment

	// A[N] < pl < A[e3] < pr < A[M]
	// Thus all three sections are NOT empty ! 
	/*
	// test to play safe --- cannot happen ...
	if ( pl < A[N] || A[M] < pr ) {
	  // give up because cannot find a good pivot
	  quicksort0c(A, N, M, depthLimit);
	  return;
	}
	*/
	// int z; // for tracing

	// initialize running indices
	k = i = N; j = M;
	  /* 
	  |-]-----]--------------------[-|
	  N i     k                    j M
	  N <= x <= i -> A[x] <= pl
	  i < x <= k -> pl < A[x] < pr
	  j <= x <= M -> pr <= A[x]
	  */
 again:
	// printf("again N %i i %i k %i j %i M %i\n", N,i,k,j,M);
	k++;
	while ( k < j ) {
	  ak = A[k];
	  if ( ak <= pl ) { 
	    i++; A[k] = A[i]; A[i] = ak; k++; 
	    // printf("cont1 N %i i %i k %i j %i M %i\n", N,i,k,j,M);
	    continue;
	  } 
	  if ( pr <= ak ) { // ak -> R
	    j--;
	    while ( k < j ) {
	      aj = A[j];
	      if ( pr <= aj ) { j--; 
		// printf("cont2 N %i i %i k %i j %i M %i\n", N,i,k,j,M);
		continue; }
	      if ( pl < aj ) {
		A[k] = aj; A[j] = ak;
		goto again;
	      }
	      // aj <= pl
	      i++; A[k] = A[i]; A[j] = ak; A[i] = aj;
	      goto again;
	    }
	    // j = k
	    A[j] = ak; k--;
	    goto done;
	  }
	  // pl < ak < pr
	  k++; 
	  // printf("cont3 N %i i %i k %i j %i M %i\n", N,i,k,j,M);
	  continue;
	}
	// j <= k

 done:
	if ( k == j ) k--;
    /*
      |---]---------[---------|
      N   i         j         M
    */
    /*

      for ( z = N; z <= i; z++ )
	if ( pl < A[z] ) {
	  printf("doneL z %i\n", z);
	  printf("N %i i %i k %i j %i M %i\n", N,i,k,j,M);
	  exit(0);
	}
      for ( z = i+1; z < j; z++ )
	if ( A[z] <= pl || pr <= A[z] ) {
	  printf("doneM z %i\n", z);
	  printf("N %i i %i k %i j %i M %i\n", N,i,k,j,M);
	  exit(0);
	}
      for ( z = j; z <= M ; z++ )
	if ( A[z] < pr ) {
	  printf("doneR z %i\n", z);
	  printf("N %i i %i k %i j %i M %i\n", N,i,k,j,M);
	  exit(0);
	}
      */
      tpsc3(A, N, i, depthLimit);
      tpsc3(A, i+1, j-1, depthLimit);
      tpsc3(A, j, M, depthLimit);
} // end tpsc3

// still another three partition sorter
// DEPRECATED
void tps2(int *A, int N, int M) {
  int L = M - N;
  if ( L < cut3Limit ) { 
    cut2(A, N, M);
    // quicksort0(A, N, M);
    return;
  }
  int depthLimit = 2.5 * floor(log(L));
  tpsc2(A, N, M, depthLimit);
} // end tps2

// Three partition sorting with 2 pivots
void tpsc2(int *A, int N, int M, int depthLimit) 
{
  register int k; // for-index
  // int z;
  register int i, j; // indices
  register int ai, aj; // array values
  register int pl, pr; // pivots

 // A 3d recursive call is avoided by jumping back to Start.  
 Start:
  // printf("tspc N %d M %d depthLimit %d\n", N,M,depthLimit);

  if ( depthLimit <= 0 ) { // avoid quadradic explosion
    heapc(A, N, M);
    return;
  }
  int L = M - N;
  if ( L < cut3Limit ) {
    cut2c(A, N, M, depthLimit);
    return;
  }
  depthLimit--;

  // Check for duplicates
        int sixth = (L + 1) / 6;
        int e1 = N  + sixth;
        int e5 = M - sixth;
	int e3 = N + (L>>1); // N + L/2;// The midpoint
        int e4 = e3 + sixth;
        int e2 = e3 - sixth;
        // Sort these elements using a 5-element sorting network
        int ae1 = A[e1], ae2 = A[e2], ae3 = A[e3], ae4 = A[e4], ae5 = A[e5];
	int t;

        if (ae1 > ae2) { t = ae1; ae1 = ae2; ae2 = t; }
        if (ae4 > ae5) { t = ae4; ae4 = ae5; ae5 = t; }
        if (ae1 > ae3) { t = ae1; ae1 = ae3; ae3 = t; }
        if (ae2 > ae3) { t = ae2; ae2 = ae3; ae3 = t; }
        if (ae1 > ae4) { t = ae1; ae1 = ae4; ae4 = t; }
        if (ae3 > ae4) { t = ae3; ae3 = ae4; ae4 = t; }
        if (ae2 > ae5) { t = ae2; ae2 = ae5; ae5 = t; }
        if (ae2 > ae3) { t = ae2; ae2 = ae3; ae3 = t; }
        if (ae4 > ae5) { t = ae4; ae4 = ae5; ae5 = t; }
        A[e1] = ae1; A[e2] = ae2; A[e3] = ae3; A[e4] = ae4; A[e5] = ae5;

	// Fix end points
	if ( ae1 < A[N] ) iswap(N, e1, A);
	if ( A[M] < ae5 ) iswap(M, e5, A);

	pl = A[e2]; pr = A[e4];
	if ( pl == pr || pl <= A[N] || A[M] <= pr ) {
	  dflgm(A, N, M, e3, tpsc2, depthLimit);
	  return;
	}


	// A[N] < pl < A[e3] < pr < A[M]
	// Thus all three sections are NOT empty ! 

	// initialize running indices
	k = i = N;
	j = M;

	/* 
	  |-]--------------------------[-|
	  N i                          j M

	   pl < pr
	   N <= x <= i  ==> A[x] < pl
	   j <= x <= M  ==> pr < A[x]
	*/

	while (1) {
	  /* 
	  |-]-----]--------------------[-|
	  N i     k                    j M
	  N <= x <= i -> A[x] <= pl
	  i < x <= k -> pl < A[x] < pr
	  j <= x <= M -> pr <= A[x]
	  */
	  k++;
	  ai = A[k];
	  if ( ai < pl ) { 
	    i++;
	    A[k] = A[i]; A[i] = ai;
	    continue;
	  }
	  if ( ai <= pr ) continue;
	  if ( k == j ) {
	    goto Finish;
	  }
	  break;
	}

 Right:
	/* 
	  |-]----]o--------------------[-|
	  N i     k                    j M

	   N <= x <= i  ==> A[x] < pl
	   j <= x <= M  ==> pr < A[x]
	   i < x < k    ==> pl <= A[x] <= pr
	   pr < ai = A[k]
	   k < j
	*/

	while (1) {
	  j--;
	  aj = A[j];
	  if ( aj < pl ) {
	    A[j] = ai; 
	    i++;
	    A[k] = A[i]; A[i] = aj;
	    break;
	  }
	  if ( aj <= pr ) {
	    A[k] = aj; A[j] = ai;
	    break;
	  }
	  // pl < aj
	  if ( k == j ) goto Finish;
	  // continue;	  
	}
 // Left:
	/* 
	  |-]----]--------------------[-|
	  N i    k                    j M

	   N <= x <= i  ==> A[x] < pl
	   j <= x <= M  ==> pr < A[x]
	   i < x <= k   ==> pl <= A[x] <= pr
	*/
	while (1) {
	  k++;
	  ai = A[k];
	  if ( ai < pl ) {
	    i++;
	    A[k] = A[i]; A[i] = ai;
	    continue;
	  }
	  if ( ai <= pr ) continue;
	  if ( k == j ) break;
	  goto Right;
	}
	// int z; // for tracing

 Finish:
	/*
	      |------]---------[----------------|
	      N      i         j                M

	      N < i < j < M
	      N < x <= i  ==> A[x] < pl
	      i < x < j  ==> pl <= A[x] <= pr
	      j <= x <= M ==> pr < A[x] 
	  */	
	/*
	for ( z = N; z <= i; z++ ) 
	  if ( pl <= A[z] ) printf("FinishL z: %i\n", z);

	for ( z = i+1; z < j; z++ ) 
	  if ( A[z] < pl ) printf("FinishM1 z: %i\n", z);
	for ( z = i+1; z < j; z++ ) 
	  if ( pr < A[z] ) printf("FinishM2 z: %i\n", z);

	for ( z = j; z <= M; z++ ) 
	  if ( A[z] <= pr ) printf("FinishR z: %i\n", z);
	*/

	// Recurse in order of segment size
	if ( i - N < j - i ) {
	  tpsc2(A, N, i, depthLimit);
	  if ( j - i < M - j ) {
	    tpsc2(A, i+1, j-1, depthLimit);
	    N = j;
	    goto Start;
	  }
	  tpsc2(A, j, M, depthLimit);
	  N = i+1; M = j-1;
	  goto Start;
	}
	if ( j - i < M - j ) {
	  tpsc2(A, i+1, j-1, depthLimit);
	  if ( i - N < M - j) {
	    tpsc2(A, N, i, depthLimit);
	    N = j;
	    goto Start;
	  }
	  tpsc2(A, j, M, depthLimit);
	  M = i;
	  goto Start;
	}
	tpsc2(A, j, M, depthLimit);
	tpsc2(A, i+1, j-1, depthLimit);
	M = i;
	goto Start;
} // end tpsc2

void dflglTest2();
void dflglTest(int *A, int N, int M) {
  int L = M - N; 
  int depthLimit = 2.5 * floor(log(L));
  dflglTest2(A, N, M, depthLimit);
}

void dflgl();
void dflglTest2(int *A, int N, int M, int depthLimit) {
  if ( depthLimit <= 0 ) { // avoid quadradic explosion
    heapc(A, N, M);
    return;
  }
  int L = M-N;
  if ( L <= 1 ) { 
     insertionsort(A, N, M);
     return;
  }
  dflgl(A, N, M, N+(L>>1), dflglTest2, depthLimit-1);
}

// Dutch flag type function with middle segment initialized left
// DEPRECATED
void dflgl(int *A, int N, int M, int pivotx, void (*cut)(), int depthLimit) {
  // printf("dflgl N %i M %i pivotx %i dl %i\n", N,M,pivotx,depthLimit);  
  /*
    Simple version of partitioning with: L/M/R
    L < pivot, M = piivot, R > pivot
   */
  register int k; // for-index
  // int z;
  register int i, j; // indices
  register int aj, ak; // array values
  register int p3; // pivot

    i = k = N; j = M; 
    /*
      |---|---|------|---------|
      N   i   k      j         M
      
      N <= i <= k < j <= M
      N <= x < i ==> A[x] < p3
      i <= x < k  ==> A[x] = p3
      j < x <= M & ==> p3 < A[x] 
    */
    p3 = A[pivotx];

 again:
    while ( k < j ) {
      ak = A[k];
      if ( ak == p3 ) { k++; continue; }
      if ( ak < p3 ) {
	A[k] = A[i]; A[i] = ak;
	i++; k++; continue;
      }
      // p3 < ak = A[k] and k < j
      while ( 1 ) {
	aj = A[j];
	if ( p3 < aj ) { 
	  j--; 
	  if ( k < j ) continue; else goto again;
	}
	if ( p3 == aj ) {
	  A[k] = aj; A[j] = ak; k++; j--;
	  goto again;
	}
	// aj < p3
	A[j] = ak; A[k] = A[i]; A[i] = aj;
	j--; k++; i++;
	goto again;
      }
    }
    // k = j
    if ( j < k ) { 
      j++; i--; 
      /*
      for ( z = j; z <= M ; z++ )
	if ( A[z] <= p3 ) {
	  printf("doneR G z %i\n", z);
	  printf("N %i i %i k %i j %i M %i\n", N,i,k,j,M);
	  exit(0);
	}
      */
      goto done; 
    }
    // j = k
    ak = A[k];
    if ( p3 == ak ) { j++; i--; 
      /*
      for ( z = j; z <= M ; z++ )
	if ( A[z] <= p3 ) {
	  printf("doneR D z %i\n", z);
	  printf("N %i i %i k %i j %i M %i\n", N,i,k,j,M);
	  exit(0);
	}
      */
      goto done; 
    }
    if ( p3 < ak ) { i--; 
      /*
      for ( z = j; z <= M ; z++ )
	if ( A[z] <= p3 ) {
	  printf("doneR E z %i\n", z);
	  printf("N %i i %i k %i j %i M %i\n", N,i,k,j,M);
	  exit(0);
	}
      */
      goto done; 
    }
    // ak < p3
    j++;
    A[k] = A[i]; A[i] = ak;
    /*
    for ( z = j; z <= M ; z++ )
      if ( A[z] <= p3 ) {
	printf("doneR F z %i\n", z);
	printf("N %i i %i k %i j %i M %i\n", N,i,k,j,M);
	exit(0);
      }
    */
    // goto done;

done: 
    /*
      |---]---------[---------|
      N   i         j         M
    */
    /*
      for ( z = N; z <= i; z++ )
	if ( p3 <= A[z] ) {
	  printf("doneL z %i\n", z);
	  exit(0);
	}
      for ( z = i+1; z < j; z++ )
	if ( p3 != A[z] ) {
	  printf("doneM z %i\n", z);
	  exit(0);
	}
      for ( z = j; z <= M ; z++ )
	if ( A[z] <= p3 ) {
	  printf("doneR z %i\n", z);
	  printf("N %i i %i k %i j %i M %i\n", N,i,k,j,M);
	  exit(0);
	}
    */
    if ( i - N  < M - j ) {
      (*cut)(A, N, i, depthLimit);
      (*cut)(A, j, M, depthLimit);
      return;
    }
    (*cut)(A, j, M, depthLimit);
    (*cut)(A, N, i, depthLimit);
} // end dflgl

void dflgmTest(int *A, int N, int M) {
  int L = M - N; 
  int depthLimit = 2.5 * floor(log(L));
  dflgmTest2(A, N, M, depthLimit);
} // end dflgmTest

void dflgmTest2(int *A, int N, int M, int depthLimit) {
  if ( depthLimit <= 0 ) { // avoid quadradic explosion
    heapc(A, N, M);
    return;
  }
  int L = M-N;
  if  (L <= 50 ) { 
    quicksort0c(A, N, M, depthLimit);
    return;
  }
  /*
  if ( L <= 10 ) { 
    insertionsort(A, N, M);
    return;
  }
  */
        int sixth = (M-N + 1) / 6;
        int e1 = N  + sixth;
        int e5 = M - sixth;
	int e3 = N + L/2;// The midpoint
        // int e3 = (N+M) / 2; // The midpoint
        int e4 = e3 + sixth;
        int e2 = e3 - sixth;

        // Sort these elements using a 5-element sorting network
        int ae1 = A[e1], ae2 = A[e2], ae3 = A[e3], ae4 = A[e4], ae5 = A[e5];
	int t;

        if (ae1 > ae2) { t = ae1; ae1 = ae2; ae2 = t; }
        if (ae4 > ae5) { t = ae4; ae4 = ae5; ae5 = t; }
        if (ae1 > ae3) { t = ae1; ae1 = ae3; ae3 = t; }
        if (ae2 > ae3) { t = ae2; ae2 = ae3; ae3 = t; }
        if (ae1 > ae4) { t = ae1; ae1 = ae4; ae4 = t; }
        if (ae3 > ae4) { t = ae3; ae3 = ae4; ae4 = t; }
        if (ae2 > ae5) { t = ae2; ae2 = ae5; ae5 = t; }
        if (ae2 > ae3) { t = ae2; ae2 = ae3; ae3 = t; }
        if (ae4 > ae5) { t = ae4; ae4 = ae5; ae5 = t; }
        A[e1] = ae1; A[e2] = ae2; A[e3] = ae3; A[e4] = ae4; A[e5] = ae5;

	// Fix end points
	if ( ae1 < A[N] ) iswap(N, e1, A);
	if ( A[M] < ae5 ) iswap(M, e5, A);
  dflgm(A, N, M, e3, dflgmTest2, depthLimit-1);
  // dflgm(A, N, M, N+(L>>1), dflgmTest2, depthLimit-1);
} // end dflgmTest2


void dflgm(int *A, int N, int M, int pivotx, void (*cut)(), int depthLimit) {
  // printf("dflgmY N %i M %i pivotx %i depthLimit %i\n", N,M,pivotx,depthLimit);
  /*
    Simple version of partitioning with: L/M/R
    L < pivot, M = pivot, R > pivot
    This is the integer version.  Code for the object/record version 
    has been commented out. 
    The 2nd best version is dflgmY
   */
  register int i, j, lw, up; // indices
  register int p3; // pivot
  register int x, y;
  // register int r; // comparison output 
  // int z; // for tracing

  i = N; j = M; lw = pivotx-1; up = pivotx+1;
  int N2 = N+1;
    /*
      |---)-----(----)-------(----|
      N   i     lw  up       j    M
      
      N <= i < lw < up < j <= M
      2 <= up - lw
      N <= x < i ==> A[x] < p3
      lw < x < up  ==> A[x] = p3
      j < x <= M & ==> p3 < A[x] 
    */
  p3 = A[pivotx]; // There IS a middle value somewhere:
  // store p3 temporarily; A[N] will be replaced at the end
  // x is the wack-a-mole item
  x = A[N]; A[N] = p3; i++; 
  // r = compareXY(x, p3);
  if ( x < p3 ) goto L0;
  // if ( r < 0 ) goto L0;
  if ( p3 < x ) goto R0;
  // if ( 0 < r ) goto R0;
  goto ML0;

 L0:  
   /*
      |---)-----(----)-------(----|
      N   i     lw  up       j    M
      x -> L 
   */
  // printf("L0  N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
  if ( lw < i ) { i--;
    goto L1L;
  }
  // i <= lw
  y = A[i];
  // r = compareXY(y, p3);
  // if ( r < 0 ) { i++;
  if ( y < p3 ) { i++;
    goto L0; 
  }
  // 0 <= r
  A[i++] = x; x = y;
  // if ( 0 < r ) {
  if ( p3 < y ) {
    goto R0;
  }
  // r = 0
  // goto ML0;

 ML0: 
   /*
      |---)-----(----)-------(----|
      N   i     lw  up       j    M
      x -> ML 
   */
  if ( lw < i ) { i--;
    goto L1M;
  }
  // i <= lw
  y = A[lw];
  // r = compareXY(y, p3);
  // if ( r == 0 ) { lw--;
  if ( y == p3 ) { lw--;
    goto ML0;
  }
  A[lw--] = x; x = y;
  // if ( 0 < r ) {
  if ( p3 < y ) {
    goto R0;
  }
  // r < 0
  goto L0;
  
 R0:
   /*
      |---)-----(----)-------(----|
      N   i     lw  up       j    M
      x -> R
   */
  if ( j < up ) { // A[j] = p3 !
    y = A[j]; A[j] = x; x = y;
    goto R1M;
  }
  // up <= j
  y = A[j]; 
  // r = compareXY(y, p3);
  // if ( 0 < r ) { j--;
  if ( p3 < y ) { j--;
    goto R0;
  }
  A[j--] = x; x = y;
  // if ( r == 0 ) {
  if ( y == p3 ) {
    goto MR0;
  }
  goto L0;
  
  MR0:
   /*
      |---)-----(----)-------(----|
      N   i     lw  up       j    M
      x -> MR
   */
  if ( j < up ) { j++;
    goto R1M;
  }
  // up <= j
  y = A[up];
  // r = compareXY(y, p3);
  // if ( r == 0 ) { up++;
  if ( y == p3 ) { up++;
    goto MR0;
  }
  A[up++] = x; x = y;
  // if ( r < 0 ) {
  if ( y < p3 ) {
    goto L0;
  }
  goto R0;

  L1L: 
   /*
      |---]---------)-------(----|
      N   i        up       j    M
      x -> L
   */
  // printf("L1L N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
  if ( j < up ) { j++; A[N] = x; 
    goto done; 
  }
  // up <= j
  y = A[up];
  // r = compareXY(y, p3);
  // if ( r == 0 ) { up++;
  if ( y == p3 ) { up++;
    goto L1L;
  }
  // if ( r < 0 ) { A[up++] = A[++i]; A[i] = y;
  if (  y < p3 ) { A[up++] = A[++i]; A[i] = y;
    goto L1L;
  }
  // 0 < r
  A[up++] = A[++i]; A[i] = x; x = y;
  // goto L1R;

  L1R: 
   /*
      |---]---------)-------(----|
      N   i        up       j    M
      x -> R
   */
  if ( j < up ) { 
    A[N] = A[i]; A[i--] = A[j]; A[j] = x;
    goto done; 
  }
  // up <= j
  y = A[j];
  // r = compareXY(y, p3);
  // if ( 0 < r ) { j--;
  if ( p3 < y ) { j--;
    goto L1R;
  }
  A[j--] = x; x = y;
  // if ( r == 0 ) { 
  if ( y == p3 ) { 
    goto L1M;
  }
  // r < 0
  goto L1L;

 L1M: 
   /*
      |---]---------)-------(----|
      N   i        up       j    M
      x -> M
   */
  if ( j < up ) { j++; A[N] = A[i]; A[i--] = x;
    goto done;
  }
  // up <= j
  y = A[up];
  // r = compareXY(y, p3);
  // if ( r == 0 ) { up++;
  if ( y == p3 ) { up++;
    goto L1M;
  }
  A[up++] = x; x = y;
  // if ( r < 0 ) {
  if ( y < p3 ) {
    goto L1L;
  }
  goto L1R;

 R1R: 
   /*
      |---)---(-------------[----|
      N   i   lw            j    M
      x -> R
   */
  if ( lw < i ) {
    if ( i == N2 ) { i = N-1; // left empty
      A[N] = A[--j]; A[j] = x;
      goto done;      
    }
    // N2 < i
    A[N] = A[--i]; A[i--] = A[--j]; A[j] = x;
    goto done;
  }
  // i <= lw
  y = A[lw];
  // r = compareXY(y, p3);
  // if ( r == 0 ) { lw--;
  if ( y == p3 ) { lw--;
    goto R1R;
  }
  A[lw--] = A[--j]; A[j] = x; x = y;
  // if ( r < 0 ) {
  if ( y < p3 ) {
    goto R1L;
  }
  goto R1R;

 R1L: 
   /*
      |---)---(-------------[----|
      N   i   lw            j    M
      x -> L
   */
  if ( lw < i ) {
    A[N] = x; i--;
    goto done;
  }
  // i <= lw
  y = A[i];
  // r = compareXY(y, p3);
  // if ( r < 0 ) { i++;
  if ( y < p3 ) { i++;
    goto R1L;
  }
  A[i++] = x; x = y;
  // if ( r == 0 ) {
  if ( y == p3 ) {
    goto R1M;
  }
  goto R1R;

 R1M: 
   /*
      |---)---(-------------[----|
      N   i   lw            j    M
      x -> M
   */
  if ( lw < i ) {
    if ( i == N2 ) { i = N-1; // left empty
      A[N] = x;
      goto done;      
    }
    // N2 < i
    A[N] = A[--i]; A[i--] = x;
    goto done;
  }
  // i <= lw
  y = A[lw];
  // r = compareXY(y, p3);
  // if ( r == 0 ) { lw--;
  if ( y == p3 ) { lw--;
    goto R1M;
  }
  A[lw--] = x; x = y;
  // if ( r < 0 ) {
  if ( y < p3 ) {
    goto R1L;
  }
  goto R1R;

 done: 
    /*
      |---]---------[---------|
      N   i         j         M
    */
    /*
      for ( z = N; z <= i; z++ )
	if ( p3 <= A[z] ) {
	// if ( compareXY(p3, A[z]) <= 0 ) {
	  printf("doneL z %i\n", z);
	  printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
	  exit(0);
	}
      for ( z = i+1; z < j; z++ )
	if ( p3 != A[z] ) {
	// if ( compareXY(p3, A[z]) != 0 ) {
	  printf("doneM z %i\n", z);
	  printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
	  exit(0);
	}
      for ( z = j; z <= M ; z++ )
	if ( A[z] <= p3 ) {
	// if ( compareXY(A[z], p3) <= 0 ) {
	  printf("doneR z %i\n", z);
	  printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
	  exit(0);
	}
      */
    if ( i - N  < M - j ) {
      (*cut)(A, N, i, depthLimit);
      (*cut)(A, j, M, depthLimit);
      return;
    }
    (*cut)(A, j, M, depthLimit);
    (*cut)(A, N, i, depthLimit);
} // end dflgm


// dflgmQ does 2 & 3 way swaps. 3.2% slower than dflgm on uniform distributions
void dflgmQ(int *A, int N, int M, int pivotx, void (*cut)(), int depthLimit) {
  // printf("dflgm N %i M %i pivotx %i depthLimit %i\n", N,M,pivotx,depthLimit);
  /*
    Simple version of partitioning with: L/M/R
    L < pivot, M = pivot, R > pivot
    This is the integer version.  Code for the object/record version 
    has been commented out. 
    The 2nd best version is dflgmY
   */
  register int i, j, lw, up; // indices
  register int p3; // pivot
  register int x, y, z;
  // register int r; // comparison output 
  // int z; // for tracing

  i = N; j = M; lw = pivotx-1; up = pivotx+1;
  // int N2 = N+1;
    /*
      |---)-----(----)-------(----|
      N   i     lw  up       j    M
      
      N <= i < lw < up < j <= M
      2 <= up - lw
      N <= x < i ==> A[x] < p3
      lw < x < up  ==> A[x] = p3
      j < x <= M & ==> p3 < A[x] 
    */
  p3 = A[pivotx]; // There IS a middle value somewhere:
  int left = 1;

  while (1) {
    x = A[i];
    if ( x < p3 ) { // x -> left
      i++;
      if ( lw < i ) {
	break; // left gap closed
      }
      continue;
    }
    if ( x == p3 ) { // x -> middle
    Mright:
      while ( up <= j && p3 == (y = A[up]) ) up++;
      if ( j < up ) { // right gap closed
	left = 0;
	break;
      }
      if ( y < p3 ) { // y -> left
	A[i++] = y; A[up++] = x;
	if ( j < up ) { // right gap closed
	  left = 0;
	  if ( lw < i ) { // left gap also closed
	    goto Finish;
	  }
	  break;
	}
	if ( lw < i ) break; // left gap closed
	continue;
      }
      // y -> right
      while ( p3 < (z = A[j]) ) j--;
      if ( j <= up ) { // right gap closed
	  left = 0;
	  break;
      }
      if ( p3 == z ) {
	A[up++] = z; A[j--] = y;
	if ( j < up ) { // right gap closed
	  left = 0;
	  break;
	}
	goto Mright;
      }
      // z -> left triple swap
      A[up++] = x; A[i++] = z; A[j--] = y;
      if ( j < up ) { // right gap closed
	left = 0;
	if ( lw < i ) { // left gap also closed
	  goto Finish;
	}
	break;
      }
      if ( lw < i ) { // left gap closed
	break;
      }
      continue;
    }
    // x -> right
  Mright2:
    while ( p3 < (z = A[j]) ) j--;
    if ( j < up ) { // right gap closed
      left = 0;
      break;
    }
    if ( z < p3 ) { // z -> L, swap with x
      A[i++] = z; A[j--] = x;
      if ( j < up ) { // right gap closed
	left = 0;
	if ( lw < i ) { // left gap closed also
	  goto Finish;
	}
	break;
      }
      if ( lw < i ) break; // left gap closed
      continue;
    }

    // z -> middle
    while ( up <= j && p3 == (y = A[up]) ) up++;
    if ( j <= up ) { // right gap closed
      left = 0;
      break;
    }
    if ( p3 < y ) { // y -> right swap with z
      A[up++] = z; 
      A[j--] = y;
      if ( j < up ) {
	left = 0;
	break;
      }
      goto Mright2;
    }

    // y < p3 y -> left
    A[i++] = y; A[up++] = z; A[j--] = x;
    if ( j < up ) { // right gap closed
      left = 0;
      if ( lw < i ) { // left gap closed also
	goto Finish;
      }
      break;
    }
    if ( lw < i ) { // left gap closed
      break;
    }
    continue;
  } 
  // end 2-gap phase

  int k;
  if ( left ) { // left closed
    // printf("left closed N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
    /*
      |--------(----)-----(------|
      N        lw  up     j      M
    */
    for (k = up; k <= j; k++) {
      if ( p3 == (y = A[k]) ) { // y -> middle
	up++;
	continue;
      }
      if ( y < p3 ) {  // y -> right
	A[k] = A[++lw]; A[lw] = y;
	up++;
	continue;
      }
      // y -> right
      while ( k < j && p3 < (z = A[j]) ) j--;
      if ( k == j ) break; // gap closed
      if ( p3 == z ) { // z -> middle
	A[k] = z; A[j--] = y;
	up++; 
	continue;
      }
      // z -> left
      A[up++] = A[++lw]; A[lw] = z; A[j--] = y;
    }
  } else { // right closed
    // printf("right closed N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
    /*
      |---)-----(----)-----------|
      N   i     lw  up           M
    */
    for ( k = lw; i <= k; k--) {
      if ( (y = A[k]) == p3 ) { // y -> middle
	lw--;
	continue;
      }
      if ( p3 < y ) { // y -> right 
	A[k] = A[--up]; A[up] = y;
	lw--;
	continue;
      }
      // y -> left
      while ( i < k && (x = A[i]) < p3 ) i++;
      if ( i == k ) break; // gap closed
      if ( p3 == x ) { // x -> middle
	A[k] = x; A[i++] = y;
	lw--;
	continue;
      }
      // x -> right
      A[lw--] = A[--up]; A[up] = x; A[i++] = y;
    }
  }

 Finish:
    /*
      |--------(----)-----------|
      N        lw  up           M
    */
  i = lw; j = up;
   /*
      |---]---------[---------|
      N   i         j         M
    */
  /* testing stuff
      for ( z = N; z <= i; z++ )
	if ( p3 <= A[z] ) {
	// if ( compareXY(p3, A[z]) <= 0 ) {
	  printf("doneL z %i\n", z);
	  printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
	  exit(0);
	}
      for ( z = i+1; z < j; z++ )
	if ( p3 != A[z] ) {
	// if ( compareXY(p3, A[z]) != 0 ) {
	  printf("doneM z %i\n", z);
	  printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
	  exit(0);
	}
      for ( z = j; z <= M ; z++ )
	if ( A[z] <= p3 ) {
	// if ( compareXY(A[z], p3) <= 0 ) {
	  printf("doneR z %i\n", z);
	  printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
	  exit(0);
	}
      */
    if ( i - N  < M - j ) {
      (*cut)(A, N, i, depthLimit);
      (*cut)(A, j, M, depthLimit);
      return;
    }
    (*cut)(A, j, M, depthLimit);
    (*cut)(A, N, i, depthLimit);

} // end dflgmQ


void dflgmTestX(int *A, int N, int M) {
  int L = M - N; 
  int depthLimit = 2.5 * floor(log(L));
  dflgmTestX2(A, N, M, depthLimit);
}

void dflgmX();
void dflgmTestX2(int *A, int N, int M, int depthLimit) {
  if ( depthLimit <= 0 ) { // avoid quadradic explosion
    heapc(A, N, M);
    return;
  }
  int L = M-N;
  if ( L <= 1 ) { 
     insertionsort(A, N, M);
     return;
  }
  dflgmX(A, N, M, N + (L>>1), dflgmTestX2, depthLimit--);
}

// Dutch flag type function that initializes the middle segment in the middle
// DEPRECATED
void dflgmX(int *A, int N, int M, int pivotx, void (*cut)(), int depthLimit) {
  // printf("dflgm N %i M %i pivotx %i dl %i\n", N,M,pivotx,depthLimit);  
  /*
    Simple version of partitioning with: L/M/R
    L < pivot, M = pivot, R > pivot
   */
  register int i, j, lw, up, k; // indices
  register int p3; // pivot
  register int ai, aj, alw, ak; // values

  // int z; // for tracing

  i = N; j = M; lw = pivotx-1; up = pivotx+1;
    /*
      |---)-----(----)-------(----|
      N   i     lw  up       j    M
      
      N <= i < lw < up < j <= M
      2 <= up - lw
      N <= x < i ==> A[x] < p3
      lw < x < up  ==> A[x] = p3
      j < x <= M & ==> p3 < A[x] 
    */
  p3 = A[pivotx]; // There IS a middle value somewhere:

  while ( N <= lw && A[lw] == p3 ) lw--;
  while ( up <= M && p3 == A[up] ) up++;
  if ( lw < N ) {
    i--;
    if ( M < up ) { j++; goto done; }
    goto xleftClosed0;
  }
  if ( M < up ) { j++; goto xrightClosed0; }

 x0:
    /*
      |---)-----(----)-------(----|
      N   i     lw  up       j    M
      N <= i <= lw up <= j <= M 
    */
  alw = A[lw];
  if ( alw < p3 ) goto xalwL; 
  if ( p3 < alw ) goto xalwR;
  // p3 = alw
  if ( i == lw ) {
    i--; goto xleftClosed0; 
  }
  // i < lw
  lw--;
  goto x0;

 xalwL:
    /*
      |---)-----(----)-------(----|
      N   i     lw  up       j    M
      i <= lw  up <= j alw -> L
    */
  while ( A[i] < p3 ) i++;
  if ( lw < i ) { i--; goto xleftClosed0; }
  // i < lw  (alw < p3 <= A[i])
  ai = A[i];
  if ( ai == p3 ) {
    A[i++] = alw; A[lw--] = ai;
    if ( i <= lw ) goto x0; // info loss
    i--; goto xleftClosed0;
  }
  // p3 < ai
  goto xalwLaiR;
  
 xalwR:
    /*
      |---)-----(----)-------(----|
      N   i     lw  up       j    M
      i <= lw  up <= j alw -> R
    */
  while ( p3 < A[j] ) j--;
  if ( j < up ) {
    A[lw] = A[j]; // OK for phase shift
    A[j] = alw;
    goto xrightClosed0;
  }
  // up <= j
  aj = A[j]; // aj <= p3
  if ( aj == p3 ) {
    A[lw--] = aj; A[j--] = alw;
    if ( j < up ) { 
      j++; // right closed
      if ( lw < i ) { i--; goto done; }
      // i <= lw
      goto xrightClosed0;      
    }
    // up <= j
    if ( lw < i ) { i--; goto xleftClosed0; }
    goto x0; // info loss
  }
  // aj < p3
  goto xalwRajL;

 xalwLaiR:
    /*
      |---)-----(----)-------(----|
      N   i     lw  up       j    M
      i < lw up <= j alw -> L ai -> R  
    */
  aj = A[j];
  if ( aj == p3 ) { // aj -> M
    A[j--] = ai; A[lw--] = aj; A[i++] = alw;
    if ( lw < i ) { 
      i--; // left closed
      if ( j < up ) { // right closed
	j++; goto done;
      }
      // up <= j
      goto xleftClosed0; // info loss
    }
    // i <= lw
    if ( j < up ) { // right closed
      j++; goto xrightClosed0;
    }
    // up <= j
    goto x1;
  }  
  if ( aj < p3 ) { // aj -> L
    A[j--] = ai; A[i++] = aj; // i <= lw
    if ( j < up ) { // right closing
      j++;
      // alw -> L is lost
      goto xrightClosed0;
    }
    // up <= j and i <= lw ! and alw -> L  
    goto xalwL;
  }
  // aj -> R
  j--;
  if ( up <= j ) goto xalwLaiR;
  // j < up   right closed
  A[i++] = alw; A[lw--] = A[j]; A[j] = ai; 
  if ( lw < i ) { i--; goto done; }
  goto xrightClosed0;

 xalwRajL:
  /*
    |---)-----(----)-------(----|
    N   i     lw  up       j    M
    i <= lw up <= j alw -> R aj -> L
  */
  ai = A[i];
  if ( ai == p3 ) { // ai -> M
    A[i++] = aj; A[lw--] = ai; A[j--] = alw;
    if ( lw < i ) { 
      i--; // left closed
      if ( j < up ) { // right closed
	j++; goto done;
      }
      // up <= j
      goto xleftClosed0; // info loss
    }
    // i <= lw
    if ( j < up ) { // right closed
      j++; goto xrightClosed0;
    }
    // up <= j
    goto x1;
  } 
  if ( p3 < ai ) { // ai -> R
    if ( i == lw ) { // left closing
      A[i] = aj; A[j--] = ai;
      if ( j < up ) { j++; goto done; }
      // up <= j
      goto xleftClosed0;
    }
    // i < lw
    A[i++] = aj; A[j--] = ai;
    if ( j < up ) { // right closed
      j++; goto xrightClosed0;
    }
    goto xalwR;
  }
  // ai -> L, thus i < lw
  i++;
  goto xalwRajL;

 x1:
    /*
      |---)-----(----)-------(----|
      N   i     lw  up       j    M
      N < i <= lw up <= j < M 
    */
  while ( A[lw] == p3 ) lw--;
  while ( A[up] == p3 ) up++;
  if ( lw < i ) { // left closed
    i--;
    if ( j < up ) { j++; goto done; }
    // up < j
    goto xleftClosed0;
  }
  // i <= lw
  if ( j < up  ) { j++; goto xrightClosed0; }
  // up <= j
  goto x0; // info loss

 xleftClosed0:
    /*
      |---]---------)-------(----|
      N   i        up       j    M
      up <= j 
    */
  for (k = up; k <= j; k++) {
    ak = A[k];
    if ( ak == p3 ) continue;
    if ( ak < p3 ) { A[k] = A[++i]; A[i] = ak; continue; }
    // p3 < ak
    while ( p3 < A[j]  ) j--;
    if ( j < k  ) { j = k; goto done; }
    // k < j
    aj = A[j]; // aj <= p3
    A[j--] = ak;
    if ( aj == p3 ) {
      A[k] = aj; continue;
    }
    // aj < p3
    A[k] = A[++i]; A[i] = aj; // continue
  }
  j++;
  goto done;

 xrightClosed0:
    /*
      |---)-----(-----------[----|
      N   i     lw          j    M
      i <= lw 
    */
   for (k = lw; i <= k; k--) {
    ak = A[k];
    if ( ak == p3 ) continue;
    if ( ak > p3 ) { A[k] = A[--j]; A[j] = ak; continue; }
    // ak < p3
    while ( A[i] < p3 ) i++;
    if ( k < i ) { i = k; goto done; }
    // i < k
    ai = A[i]; // p3 <= ai
    A[i++] = ak;
    if ( ai == p3 ) {
      A[k] = ai; continue;
    }
    // p3 < ai
    A[k] = A[--j]; A[j] = ai; // continue    
  }
  i--;
  // goto done;

 done: 
    /*
      |---]---------[---------|
      N   i         j         M
    */
  // printf("done dflgmX N %i i %i j %i M %i\n", N,i,j,M);

  /*
      for ( z = N; z <= i; z++ )
	if ( p3 <= A[z] ) {
	  printf("doneL z %i\n", z);
	  printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
	  exit(0);
	}
      for ( z = i+1; z < j; z++ )
	if ( p3 != A[z] ) {
	  printf("doneM z %i\n", z);
	  printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
	  exit(0);
	}
      for ( z = j; z <= M ; z++ )
	if ( A[z] <= p3 ) {
	  printf("doneR z %i\n", z);
	  printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
	  exit(0);
	}
      */
    if ( i - N  < M - j ) {
      (*cut)(A, N, i, depthLimit);
      (*cut)(A, j, M, depthLimit);
      return;
    }
    (*cut)(A, j, M, depthLimit);
    (*cut)(A, N, i, depthLimit);
} // end dflgmX

void dflgmTestY(int *A, int N, int M) {
  int L = M - N; 
  int depthLimit = 2.5 * floor(log(L));
  dflgmTestY2(A, N, M, depthLimit);
} // end dflgmY

void dflgmY();
void dflgmTestY2(int *A, int N, int M, int depthLimit) {
  if ( depthLimit <= 0 ) { // avoid quadradic explosion
    heapc(A, N, M);
    return;
  }
  /*
  if ( M-N <= 1 ) { 
     insertionsort(A, N, M);
     return;
  }
  */
  int L = M-N;
  if  (L <= 50 ) { 
    quicksort0c(A, N, M, depthLimit);
    return;
  }

        int sixth = (M-N + 1) / 6;
        int e1 = N  + sixth;
        int e5 = M - sixth;
	int e3 = N + (L>>1); // N + L/2;// The midpoint
        int e4 = e3 + sixth;
        int e2 = e3 - sixth;

        // Sort these elements using a 5-element sorting network
        int ae1 = A[e1], ae2 = A[e2], ae3 = A[e3], ae4 = A[e4], ae5 = A[e5];
	int t;

        if (ae1 > ae2) { t = ae1; ae1 = ae2; ae2 = t; }
        if (ae4 > ae5) { t = ae4; ae4 = ae5; ae5 = t; }
        if (ae1 > ae3) { t = ae1; ae1 = ae3; ae3 = t; }
        if (ae2 > ae3) { t = ae2; ae2 = ae3; ae3 = t; }
        if (ae1 > ae4) { t = ae1; ae1 = ae4; ae4 = t; }
        if (ae3 > ae4) { t = ae3; ae3 = ae4; ae4 = t; }
        if (ae2 > ae5) { t = ae2; ae2 = ae5; ae5 = t; }
        if (ae2 > ae3) { t = ae2; ae2 = ae3; ae3 = t; }
        if (ae4 > ae5) { t = ae4; ae4 = ae5; ae5 = t; }
        A[e1] = ae1; A[e2] = ae2; A[e3] = ae3; A[e4] = ae4; A[e5] = ae5;

	// Fix end points
	if ( ae1 < A[N] ) iswap(N, e1, A);
	if ( A[M] < ae5 ) iswap(M, e5, A);

	dflgmY(A, N, M, e3, dflgmTest2, depthLimit-1);
} // end dflgmTestY2


// Dutch flag type function that initializes the middle segment in the middle
// DEPRECATED
void dflgmY(int *A, int N, int M, int pivotx, void (*cut)(), int depthLimit) {
  // printf("dflgm N %i M %i pivotx %i dl %i\n", N,M,pivotx,depthLimit);  
  /*
    This the second version we have developed
    Simple version of partitioning with: L/M/R
    L < pivot, M = pivot, R > pivot
   */
  register int i, j, lw, up; // indices
  register int p3; // pivot
  register int ai, aj, am;
  // int z; // for tracing

  i = N; j = M; lw = pivotx-1; up = pivotx+1;
    /*
      |---)-----(----)-------(----|
      N   i     lw  up       j    M
      
      N <= i < lw < up < j <= M
      2 <= up - lw
      N <= x < i ==> A[x] < p3
      lw < x < up  ==> A[x] = p3
      j < x <= M & ==> p3 < A[x] 
    */
  p3 = A[pivotx]; // There IS a middle value somewhere:

 L0:
   /*
     |---)-----(----)-------(----|
     N   i     lw  up       j    M
   */

  while ( A[i] < p3 ) i++;
  if ( lw < i ) {
    i--;
    if ( N <= i ) goto leftClosed;
    i = N;
    goto leftEmpty;
  }
  ai = A[i]; // p3 <= ai
  if ( p3 < ai ) goto LaiR;
  goto LaiM;

 LaiR:  
   /*
      |---)-----(----)-------(----|
      N   i     lw  up       j    M
      ai -> R
   */
  while ( p3 < A[j] ) j--;
  aj = A[j]; // aj <= p3
  if ( aj < p3 ) { // aj -> L
    A[i] = aj; A[j] = ai; 
    goto L1; }
  // ai -> R  &  aj -> M
 LaiR2:
  if ( j <= up ) {
    if ( j == M ) goto emptyRightaiR;
    j++; goto rightClosedaiR;
  }
  // up < j
  am = A[up];
  if ( am < p3 ) { // am -> L
    A[i] = am; A[up++] = aj; A[j] = ai;
    goto L1; 
  }
  if ( p3 < am ) { // am -> R
    A[up++] = aj; A[j--] = am; 
    if ( j < up ) { j++; goto rightClosedaiR; }
    // up <= j
    goto LaiR; // info loss ...
  }
  // am -> M
  up++; 
  goto LaiR2; 

 LaiM:
   /*
      |---)-----(----)-------(----|
      N   i     lw  up       j    M
      ai -> M
   */
  while ( i < lw && A[lw] == p3 ) lw--;
  if ( N == lw ) goto leftEmpty; 
  // N < lw
  if ( lw <= i ) { i--; goto leftClosed; }
  // i < lw
  am = A[lw];
  if ( am < p3 ) { // am -> L
    A[i++] = am; A[lw--] = ai;
    if ( lw < i ) { i--; goto leftClosed; }
    goto L0; // info loss
  }
  // am -> R
  while ( p3 < A[j] ) j--;
  aj = A[j]; // aj <= p3
  if ( aj < p3 ) { // aj -> L
    A[i] = aj; A[lw--] = ai; A[j] = am;
    goto L1;
  }
  // aj -> M
  if ( j < up ) {
    A[lw--] = aj; A[j] = am;
    goto rightClosedaiM;
  }
  // up <= j
  A[lw--] = aj; A[j--] = am;
  if ( j < up ) {
    j++; // right side closed
    if ( i == lw ) { // left side closed
      i--; goto done;
    }
    goto rightClosedaiM;
  }
  // up <= j
  if ( i == lw ) {
    i--; goto leftClosed;
  }
  goto LaiM; // info loss
  
 leftEmpty:
   /*
      |-------------)-------(----|
    N=i             up      j    M
   */
  while ( up <= M && A[up] == p3 ) up++;
  if ( M < up ) return; // !!
  if ( j < up ) { j++; goto done; }
  // up <= j
  am = A[up];
  if ( am < p3) { // am -> L
    A[up++] = A[N]; A[N] = am; 
    if ( j < up ) { j++; goto done; }
    // up <= j
    goto leftClosed;
  }
  // am -> R
  while ( p3 < A[j] ) j--;
  if ( j < up ) { j++; goto done; }
  // up < j
  aj = A[j]; // aj <= p3
  if ( aj < p3 ) { // aj -> L
    A[j--] = am; A[up++] = A[N]; A[N] = aj;
    if ( j < up ) { j++; goto done; }
    goto leftClosed;
  }
  // aj -> M
  A[j--] = am; A[up++] = aj;
  if ( j < up ) { j++; goto done; }
  goto leftEmpty;
  
 emptyRightaiR:
   /*
      |---)-----(---------------|
      N   i     lw              j=M
      ai -> R
   */
  while ( A[lw] == p3 ) lw--;
  am = A[lw];
  if ( p3 < am ) { // am -> R
    if ( i == lw ) {
      A[i] = A[M]; A[M] = ai; i--; goto done;
    }
    // i < lw
    A[lw--] = A[M]; A[M] = am; goto rightClosedaiR;
  }
  // am -> L
  A[i++] = am; A[lw--] = A[M]; A[M] = ai;
  if ( lw < i ) { i--; goto done; }
  goto rightClosed;

 L1:
  /*
    |---]-----(----)-------[----|
    N   i     lw  up       j    M
  */
  while ( A[++i] < p3 );
  if ( lw < i ) { i--; j--; goto leftClosed; }
  // i <= lw
  ai = A[i];
  if ( p3 < ai ) { // ai -> R
  L1Repeat:
    while ( p3 < A[--j] );
    if ( j < up ) { j++; goto rightClosedaiR; }
    // up <= j
    aj = A[j];
    if ( aj < p3 ) { // aj -> L
      A[i] = aj; A[j] = ai; 
      goto L1;
    }
    // aj -> M
    while ( A[up] == p3 ) up++;
    if ( j < up ) { j++; goto rightClosedaiR; }
    // up <= j
    am = A[up];
    if ( am < p3 ) { // am -> L
      A[i] = am; A[up++] = aj; A[j] = ai;
      if ( j < up ) { i++; goto rightClosed; }
      goto L1;
    }
    // am -> R
    A[up++] = aj; A[j] = am;
    if ( j < up ) { goto rightClosedaiR; }
    // up <= j
    goto L1Repeat;
  }
  // ai -> M  
 L1Repeat2:
  while ( A[lw] == p3 ) lw--;
  if ( lw < i ) { i--; j--; goto leftClosed; }
  // i < lw
  am = A[lw]; 
  if ( am < p3 ) { // am -> L
    A[i] = am; A[lw--] = ai;
    if ( lw < i ) { i--; j--; goto leftClosed; } 
    goto L1;
  }
  // am -> R
  while ( p3 < A[--j] );
  if ( j < up ) {
    A[lw--] = A[j]; A[j] = am; goto rightClosedaiM;
  }
  // up <= j
  aj = A[j];
  if ( aj < p3 ) { // aj -> L
    A[i] = aj; A[lw--] = ai; A[j] = am;
    if ( lw < i ) { j--; goto leftClosed; }
    goto L1;
  }
  // aj -> M
  A[lw--] = aj; A[j] = am; 
  goto L1Repeat2;

 leftClosed: 
   /* 
      |---]----------)-------(----|
      N   i          up      j    M
   */
  while ( p3 < A[j] ) j--;
  if ( j < up ) { j++; goto done; }
  // up <= j
  aj = A[j]; // aj <= p3
  if ( aj < p3 ) { // aj -> L
    repeatM:
    while ( A[up] == p3 ) up++;
    am = A[up];
    if ( p3 < am ) { // am -> R
      A[j--] = am; A[up++] = A[++i]; A[i] = aj;
      if ( j < up ) { j++; goto done; }
      goto leftClosed;
    }
    // am -> L
    if ( up == j ) { A[j++] = A[++i]; A[i] = am; goto done; }  
    // up < j
    A[up++] = A[++i]; A[i] = am;
    goto repeatM;
  }
  // aj -> M
 repeatM2: 
  while ( up <= j && A[up] == p3 ) up++;
  if ( j <= up ) { j++; goto done; }
  // up < j
  am = A[up];
  if ( p3 < am ) { // am -> R
    A[j--] = am; A[up++] = aj;
    if ( j < up ) { j++; goto done; }
    goto leftClosed;
  }
  // am -> L
  A[up++] = A[++i]; A[i] = am; 
  goto repeatM2;

 rightClosed:
   /*
      |---)-----(-----------[----|
      N   i    lw           j    M
   */
  while ( A[i] < p3 ) i++;
  if ( lw < i ) { i--; goto done; }
  // i <= lw
  ai = A[i]; // p3 <= ai
  if ( p3 < ai ) { // ai -> R
    goto rightClosedaiR;
  }
  // ai -> M
  // goto rightClosedaiM;

 rightClosedaiM:
  while ( i <= lw && A[lw] == p3 ) lw--;
  if ( lw < i ) { i--; goto done; }
  // i <= lw
  am = A[lw];
  if ( p3 < am ) { // am -> R
    A[lw--] = A[--j]; A[j] = am; 
    if ( lw < i ) { i--; goto done; }
    goto rightClosedaiM;
  }
  // am -> L
  A[i++] = am; A[lw--] = ai;
  if ( lw < i ) { i--; goto done; }
  goto rightClosed;

 rightClosedaiR: 
   /*
      |---)-----(-----------[----|
      N   i    lw           j    M
      ai -> R
   */
  while ( A[lw] == p3 ) lw--;
  if ( i == lw ) {
    A[i--] = A[--j]; A[j] = ai; goto done;
  }
  // i < lw
  am = A[lw];
  if ( am < p3 ) { // am -> L
    A[i++] = am; A[lw--] = A[--j]; A[j] = ai;
    if ( lw < i ) { i--; goto done; }
    // i <= lw
    goto rightClosed;
  }
  // am -> R
  A[lw--] = A[--j]; A[j] = am; goto rightClosedaiR;
  
 done: 
    /*
      |---]---------[---------|
      N   i         j         M
    */
  // printf("done dflgmY N %i i %i j %i M %i\n", N,i,j,M);

    /*
      for ( z = N; z <= i; z++ )
	if ( p3 <= A[z] ) {
	  printf("doneL z %i\n", z);
	  printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
	  exit(0);
	}
      for ( z = i+1; z < j; z++ )
	if ( p3 != A[z] ) {
	  printf("doneM z %i\n", z);
	  printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
	  exit(0);
	}
      for ( z = j; z <= M ; z++ )
	if ( A[z] <= p3 ) {
	  printf("doneR z %i\n", z);
	  printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
	  exit(0);
	}
      */
    if ( i - N  < M - j ) {
      (*cut)(A, N, i, depthLimit);
      (*cut)(A, j, M, depthLimit);
      return;
    }
    (*cut)(A, j, M, depthLimit);
    (*cut)(A, N, i, depthLimit);
} // end dflgmY

void dflgmTestZ2();
void dflgmTestZ(int *A, int N, int M) {
  int L = M - N; 
  int depthLimit = 2.5 * floor(log(L));
  dflgmTestZ2(A, N, M, depthLimit);
}

void dflgmZ();
void dflgmTestZ2(int *A, int N, int M, int depthLimit) {
  if ( depthLimit <= 0 ) { // avoid quadradic explosion
    heapc(A, N, M);
    return;
  }
  int L = M-N;
  if ( L <= 1 ) { 
    insertionsort(A, N, M);
    return;
  }
  dflgmZ(A, N, M, N + (L>>1), dflgmTestZ2, depthLimit-1);
}

// another Dutch flag function that initializes the middle segment in the middle
// DEPRECATED
void dflgmZ(int *A, int N, int M, int pivotx, void (*cut)(), int depthLimit) {
  // printf("dflgmZ N %i M %i pivotx %i depthLimit %i\n", N,M,pivotx,depthLimit);
  /*
    Simple version of partitioning with: L/M/R
    L < pivot, M = pivot, R > pivot
   */
  register int i, j, lw, up, k, m; // indices
  register int p3; // pivot
  register int ai, aj, ak; // values
  // int z; // for tracing

  i = N; j = M; lw = pivotx-1; up = pivotx+1;
    /*
      |---)-----(----)-------(----|
      N   i     lw  up       j    M
      
      N <= i <= lw < up <= j <= M
      2 <= up - lw
      N <= x < i ==> A[x] < p3
      lw < x < up  ==> A[x] = p3
      j < x <= M & ==> p3 < A[x] 
    */
  p3 = A[pivotx]; // There IS a middle value somewhere:

    /*
      |---)-----(----)-------(----|
      N   i     lw  up       j    M
    */
  // printf("Starting::: N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
  for ( k = lw; i <= k; k-- ) {
    /*
    // printf("\nStart left k: %i \n", k);
    // printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
    */
    ak = A[k];
    if ( ak == p3 ) { lw--; continue; }
    if ( ak < p3 ) { // ak -> L
      for ( m = i; m <= k; m++ ) {
	ai = A[m];
	if ( ai < p3 ) { i++; continue; }
	if ( ai == p3 ) { A[k] = ai; lw--; A[m] = ak; i++; goto proceedK; }
	// p3 < ai -> R
	while ( p3 < A[j] ) j--;
	if ( j < up ) { j++; goto RightClosed; }
	aj = A[j];
	if ( p3 == aj ) {
	  A[k] = aj; lw--; A[m] = ak; i++; A[j--] = ai;
	  if ( j < up ) { j++; goto RightClosed; }
	  goto proceedK;
	}
	// aj < p3  aj -> L
	A[m] = aj; i++; A[j--] = ai;
	if ( j < up ) { // right closed
	  j++; goto RightClosed; 
	}
	// continue with next m
      }
      i--;
      // left closed
      // printf("left closed k: %i N %i i %i lw %i up %i j %i M %i\n", k,N,i,lw,up,j,M);
      goto LeftClosed;
    }
    // p3 < ak -> R 
    // printf(" p3 < ak -> R k: %i N %i i %i lw %i up %i j %i M %i\n", k,N,i,lw,up,j,M);

    for ( m = j; up <= m; m-- ) {
      aj = A[m];
      if ( p3 < aj ) { j--; continue; }
      if ( p3 == aj ) { A[m] = ak; j--; A[k] = aj; goto proceedK; }
      // aj -> L
      if ( i == k ) { // left closing
	A[i] = aj; A[m] = ak; j--;
	if ( j < up ) { j++; goto done; }
	goto LeftClosed;
      }
      // i < k
      while ( A[i] < p3) i++;
      if ( i == k ) { // left closing
	A[i] = aj; A[m] = ak; j--;
	if ( j < up ) { j++; goto done; }
	goto LeftClosed;
      }
      ai = A[i];
      if ( ai == p3 ) { // ai -> M
	A[i] = aj; i++; A[k] = ai; lw--; A[m] = ak; j--;
	if ( j < up ) { j++; // right closing
	  goto RightClosed; }
	goto proceedK;
      }
      // p3 < ai -> R 
      A[i] = aj; i++; A[m] = ai; j--;
      if ( lw < i ) { i--; // left closing 
	if ( j < up ) { j++; // right closing
	  goto done;
	}
	goto LeftClosed;
      }
      if ( j < up ) { j++; 
	goto RightClosed; 
      }
    }
    j++;
    goto RightClosed; 
   proceedK: 
    // printf("proceed k: %i N %i i %i lw %i up %i j %i M %i\n", k,N,i,lw,up,j,M);
    continue;
  }
  i--;
  // printf("falling through: N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
  goto LeftClosed;


 RightClosed:
  i--;
    /*
      |---]-----(-----------[----|
      N   i     lw          j    M
    */
  // printf("\nRightClosed: N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
  /*
  for ( z = N; z < i; z++ )
    if ( p3 <= A[z] ) {
      printf("RdoneL z %i\n", z);
      printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
      exit(0);
    }
  for ( z = lw+1; z < j; z++ )
    if ( p3 != A[z] ) {
      printf("RdoneM z %i\n", z);
      printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
      exit(0);
    }
  for ( z = j; z <= M ; z++ )
    if ( A[z] <= p3 ) {
      printf("RdoneR z %i\n", z);
      printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
      exit(0);
    }
  */
  // printf("\nRightClosed: N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
  for ( k = lw; i <= k; k-- ) { 
    ak = A[k];
    if ( p3 == ak ) { continue; }
    if ( p3 < ak ) {
      A[k] = A[--j]; A[j] = ak; continue;
    }
    // ak < p3
    while ( A[++i] < p3 );
    if ( k < i ) { i--; goto done; }
    // i < k 
    ai = A[i];
    if ( p3 == ai ) {
      A[i] = ak; A[k] = ai; continue;
    }
    // p3 < ai
    A[k] = A[--j]; A[j] = ai; A[i] = ak; 
  }
  goto done;

 LeftClosed:
  j++;
  /*
    |---]-----)-----------[----|
    N   i    up           j    M
  */
  // printf("\nLeftClosed: k: %i N %i i %i lw %i up %i j %i M %i\n", k,N,i,lw,up,j,M);
  /*
    for ( z = N; z <= i; z++ )
      if ( p3 <= A[z] ) {
	printf("LdoneL z %i\n", z);
	printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
	exit(0);
      }
    for ( z = i+1; z < up; z++ )
      if ( p3 != A[z] ) {
	printf("LdoneM z %i\n", z);
	printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
	exit(0);
      }
    for ( z = j; z <= M ; z++ )
      if ( A[z] <= p3 ) {
	printf("LdoneR z %i\n", z);
	printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
	exit(0);
      }
    // printf("\nLeftClosed: k: %i N %i i %i lw %i up %i j %i M %i\n", k,N,i,lw,up,j,M);
    */
  for ( k = up; k < j; k++ ) {
    ak = A[k];
    if ( p3 == ak ) { continue; }
    if ( ak < p3 ) {
      A[k] = A[++i]; A[i] = ak; continue;
    }
    // p3 < ak -> R
    while ( p3 < A[--j] );
    if ( j < k ) { j++; goto done; }
    // k < j
    aj = A[j];
    if ( p3 == aj ) {
      A[j] = ak; A[k] = aj; continue;
    }
    // aj < p3
    A[k] = A[++i]; A[i] = aj; A[j] = ak; 
  }
  
 done: 
    /*
      |---]---------[---------|
      N   i         j         M
    */
  // printf("done: N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
  /*
      for ( z = N; z <= i; z++ )
	if ( p3 <= A[z] ) {
	  printf("doneL z %i\n", z);
	  printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
	  exit(0);
	}
      for ( z = i+1; z < j; z++ )
	if ( p3 != A[z] ) {
	  printf("doneM z %i\n", z);
	  printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
	  exit(0);
	}
      for ( z = j; z <= M ; z++ )
	if ( A[z] <= p3 ) {
	  printf("doneR z %i\n", z);
	  printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
	  exit(0);
	}
  */
  if ( i - N  < M - j ) {
    (*cut)(A, N, i, depthLimit);
    (*cut)(A, j, M, depthLimit);
    return;
  }
  (*cut)(A, j, M, depthLimit);
  (*cut)(A, N, i, depthLimit);
  return;
} // end dflgmz

void dflgmTestW2();
void dflgmTestW(int *A, int N, int M) {
  int L = M - N; 
  int depthLimit = 2.5 * floor(log(L));
  dflgmTestW2(A, N, M, depthLimit);
}

void dflgmW();
// DEPRECATED
void dflgmTestW2(int *A, int N, int M, int depthLimit) {
  // printf("dflgmW2 N %i M %i depthLimit %i\n", N,M,depthLimit);
  if ( depthLimit <= 0 ) { // avoid quadradic explosion
    heapc(A, N, M);
    return;
  }
  int L = M-N;
  if ( L <= 10 ) { 
    insertionsort(A, N, M);
    return;
  }
  dflgmW(A, N, M, N + (L>>1), dflgmTestW2, depthLimit-1);
}

void dflgmW(int *A, int N, int M, int pivotx, void (*cut)(), int depthLimit) {
  // printf("dflgmW N %i M %i pivotx %i depthLimit %i\n", N,M,pivotx,depthLimit);
  /*
    Simple version of partitioning with: L/M/R
    L < pivot, M = pivot, R > pivot
   */
  register int i, j, lw, up; // indices
  // register void* p3; // pivot
  int p3;
  // register int r; // comparison output 
  // int z; // for tracing

  p3 = A[pivotx]; // There IS a middle value somewhere:
  lw = i = N; up = j = M; 
    /*
      |--)--)-------------(--(--|
      N lw  i             j  up M
      
      N <= lw <= i <= j <= up <= M
      N <= x < lw ==> A[x] = p3
      lw <= x < i ==> A[x] < p3
      j < x <= up  ==> p3 < A[x]
      up < x <= M  ==> p3 = A[x] 
    */
 again:
  // i <= j
  // while (  i <= j && (r = compareXY(p3, A[j])) <= 0 ) {
  while ( i <= j && p3 <= A[j] ) {
    // if ( 0 == r ) if ( up != j ) { iswap(up, j, A); up--; }
    if ( p3 == A[j] ) { 
      if ( up != j ) iswap(up, j, A);
      up--; 
    }
    j--;
  }
  // j < i or A[j] < p3
  // j < i -> ( j < x -> p3 <= A[x] )
  // i <= j -> A[j] < p3 
  if ( j <= i ) { i = j; j++; goto moves; }

  // while ( i < j & (  r = compareXY(A[i], p3)) <= 0 ) {
  while ( i < j && ( A[i] <= p3 ) ) {
    // if ( 0 == r ) { if ( lw != i ) { iswap(lw, i, A); lw++; }
    if ( A[i] == p3 ) { 
      if ( lw != i ) iswap(lw, i, A); 
      lw++; 
    }
    i++;
  }

  if ( i < j ) {
    iswap(i, j, A);
    i++; j--;
    if ( i <= j ) goto again;
    // j < i 
  }

  i = j; j++;
  int k, cnt;

 moves: 
  // pre tests::
  /*
  if ( i+1 != j ) {
    printf(" i+1 != j N %i lw %i i %i j %i up %i M %i\n", N, lw, i, j, up, M);
    exit(0);
  }
  for ( z = N; z <= i; z++ )
    if ( p3 < A[z] ) {
      // if ( compareXY(p3, A[z]) <= 0 ) {
      printf("pre doneL z %i\n", z);
      printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
      exit(0);
    }
  for ( z = j; z <= M ; z++ )
    if ( A[z] < p3 ) {
      // if ( compareXY(A[z], p3) <= 0 ) {
      printf("pre doneR z %i\n", z);
      printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
      exit(0);
    }
  */

  cnt = lw - N;
  if ( 0 < cnt ) { // move left side
    if ( lw - N <= i - lw ) 
      for ( k = lw - 1; N <= k; k-- ) { iswap(i, k, A); i--; }
    else { 
      int m = N;
      for ( k = lw; k <= i; k++ ) { iswap(k, m, A); m++; }
      i = i-cnt;
    }
  }

  cnt = M - up;
  if ( 0 < cnt  ) {// move right side
    if ( up - j <= M - up ) {
      int w = M;
      for ( k = up; j <= k; k-- ) { iswap(k, w, A); w--; }
      j = j+cnt;
    } else {
      for ( k = up+1; k <= M; k++ ) { iswap(k, j, A); j++; }
    }
  } 
  /* Test:
      for ( z = N; z <= i; z++ )
	if ( p3 <= A[z] ) {
	// if ( compareXY(p3, A[z]) <= 0 ) {
	  printf("doneL z %i\n", z);
	  printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
	  exit(0);
	}
      for ( z = i+1; z < j; z++ )
	if ( p3 != A[z] ) {
	  // if ( compareXY(p3, A[z]) != 0 ) {
	  printf("doneM z %i\n", z);
	  printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
	  exit(0);
	}
      for ( z = j; z <= M ; z++ )
	if ( A[z] <= p3 ) {
	  // if ( compareXY(A[z], p3) <= 0 ) {
	  printf("doneR z %i\n", z);
	  printf("N %i i %i lw %i up %i j %i M %i\n", N,i,lw,up,j,M);
	  exit(0);
	}
      */

    if ( i - N  < M - j ) {
      (*cut)(A, N, i, depthLimit);
      (*cut)(A, j, M, depthLimit);
      return;
    }
    (*cut)(A, j, M, depthLimit);
    (*cut)(A, N, i, depthLimit);
} // end dflgmW


/* deadx is NOT used by SixSort.  It can be employed for 
   a test to invoke cut2x2 (called cut3duplicates elsewhere)
   The function dflgm has replaced cut2x2/ cut3duplicates.
*/
// DEPRECATED
void cut2x(int *A, int N, int M) {
  int L = M - N;    
  int depthLimit = 2.5 * floor(log(L));
  cut2xc(A, N, M, depthLimit);
}

/* deadxc is also NOT used by SixSort.  It is employed for 
   a test to invoke cut3duplicates */
void cut2xc(int *A, int N, int M, int depthLimit) {
  if ( depthLimit <= 0 ) {
    heapc(A, N, M);
    return;
  }
  int L = M - N;  
  if ( L < cut2Limit ) { 
    quicksort0c(A, N, M, depthLimit);
    return;
  }
  depthLimit--;
    int pivotx;
    int midpointx = N + (L>>1); // N + L/2;
    if ( L <= 30 ) { // being defensive
      pivotx = med(A, N, midpointx, M);
    } else {
      int step = L * 0.1;
      int P = med(A, N +   step, N + 2*step, N + 3*step);
      int Q = med(A, midpointx - step, midpointx, midpointx + step);
      int R = med(A, M - 3*step, M - 2*step, M - step);
      pivotx = med(A, P, Q, R);
    }
    cut2x2(A, N, M, pivotx, cut2xc, depthLimit);
} // end cut2x

/* OBSOLETE version
   Three partition sorting with a single pivot p
   Left elements are less than the pivot
   Middle elements are equal to the pivot
   Right elements are greater than the pivot
   Only middle elements are guaranteed to be non-empty
   This module is invoked on 'pathological' inputs */
// DEPRECATED
void cut2x2(int *A, int N, int M, int pivotx, void (*cut)(), int depthLimit) {

  // printf("cut2x2 N: %d M: %d pivotx: %d\n", N, M, pivotx);

  // pivotx is pivot index inside [N,M] of pivot: p
  int p = A[pivotx];
  // indices 
  register int i, j, lw, up;
  // values
  register int x, y;
  int AM;

  // initialize end points
  x = A[N]; AM = A[M];
  A[N] = A[M] = p;

  /* We employ again whack-a-mole. We know in which partition element x 
     should be.  Find a close, undecided position where x should go.  
     Store its content in y.  Drop x (simplified).  Set x to y and repeat. 
   */
  // initialize running indices
  i = N;
  j = M;
  if ( x < p ) goto StartL;
  if ( p < x ) goto StartR;
  goto CreateMiddle;

 StartL:
  /*
    |o---]-----------------------------[---o|
    N    i                             j    M
    x -> L
   */
  // printf("StartL:       N %d i %d j %d M %d\n", N, i, j, M);
  i++;
  y = A[i];
  if ( y < p ) goto StartL;
  if ( p < y ) { 
    if ( i < j ) {
      A[i] = x;
      x = y;
      goto StartR;
    }
    // gap closed; i==j 
    /*
    |o-----------][----------------------o|
    N             i                       M
    x -> L        j
   */
    A[N] = x;
    // AM must be p
    A[M] = A[j];
    A[j] = AM;
    // ch2xM(p, 2162, A, i, j);
    goto Finish;
  }
  // y = p -> M
  if ( i < j ) {
    A[i] = x;
    x = y;
    goto CreateMiddle;
  }
  /* We have:
     x -> L
     y -> M
     i = j
     N+i <= k < i -> A[k] < p
     i = j <= k < M -> p < A[k]
     Therefore: 
     x must go to A[N] and 
     p = A[M], the only location where y can go.
     Hence: i = j = M 
     This cannot happen under normal use because there must be
     more than one element equal to p when used by the other 
     members of Par/SixSort

    |o------------------------------------]o|
    N                                      M=i=j
    x -> L
   */
  A[N] = x;
  // AM must be p hence the next assignment is defensive
  A[M] = AM;
  i--;
  goto FinishL;

 StartR:
  /*
    |o---]-----------------------------[---o|
    N    i                             j    M
    x -> R
   */
  // printf("StartR:       N %d i %d j %d M %d\n", N, i, j, M);
  j--;
  y = A[j];
  if ( p < y ) goto StartR;
  if ( y < p ) {
    if ( i < j ) {
      A[j] = x;
      x = y;
      goto StartL;
    }
    // gap closed; i==j
    /*
    |o-----------][----------------------o|
    N            i                        M
    x -> R       j
   */
    A[M] = x;
    // AM must be p
    A[N] = A[i]; A[i] = AM;
    // ch2xM(p, 2208, A, i, j);
    goto Finish;
  }
  // y = p -> M
  if ( i < j ) {
    A[j] = x;
    x = y;
    goto CreateMiddle;
  }
  /* We have:
     x -> R
     y -> M
     i = j
     N+i <= k <= i -> A[k] < p
     i = j < k < M -> p < A[k]
     Therefore: 
     x must go to A[M] and 
     p = A[N], the only location where y can go.
     Hence: i = j = N
     This cannot happen under normal use because there must be
     more than one element equal to p when used by the other 
     members of Par/SixSort.

    |o][-----------------------------------o|
    N=i=j                                   M
    x -> R
   */
  A[M] = x;
  // AM must be p
  A[N] = AM;
  j++;
  goto FinishR;

 CreateMiddle:
  /*
    |o---]-----------------------------[---o|
    N    i                             j    M
    x -> M
   */
  // printf("createMiddle: N %d i %d j %d M %d\n", N, i, j, M);

  if ( i+1 == j ) {
    /*
    |o--------------][---------------------o|
    N               ij                      M
    x -> M
    */
    if ( AM < p ) {
      A[N] = AM;
      A[M] = A[j]; A[j] = x;
      i++;
// ch2xM(p, 2247, A, i, j);
      goto Finish;
    } 
    if ( p < AM ) {
      A[M] = AM;
      A[N] = A[i]; A[i] = x;
      j--;
// ch2xM(p, 2254, A, i, j);
      goto Finish;
    }
    // p = AM
    A[N] = A[i]; A[i] = x; 
    A[M] = A[j]; A[j] = AM; 
// ch2xM(p, 2259, A, i, j);
    goto Finish;
  }

  lw = (i+j)/ 2;
  up = lw+1;
  y = A[lw];
  A[lw] = x;
  x = y;
  lw--;
  /*
    |o---]-----------+[-]+-------------[---o|
    N    i          lw   up            j    M
    x -> ?
   */
  if ( x < p ){
    goto LLgMgRL;
  }
  if ( p < x ) {
    goto LLgMgRR;
  }
  goto LLgMgRM1;

 LLgMgRL:
  /*
    |o---]-----------+[-]+-------------[---o|
    N    i          lw   up            j    M
    x -> L
   */
  i++;
  y = A[i];
  if ( y < p ) goto LLgMgRL;
  if ( p < y ) {
    A[i] = x;
    x = y;
    goto LLgMgRR;
  }
  // y -> M
  if ( i <= lw ) {
    A[i] = x;
    x = y;
    goto LLgMgRM1;
  }
  // left gap closed
  /*
    |o---------][------]+-------------[---o|
    N           i       up            j    M
    x -> L
   */
  goto LLMgRL;
  
 LLgMgRR:
  /*
    |o---]-----------+[-]+-------------[---o|
    N    i          lw   up            j    M
    x -> R
   */
  j--;
  y = A[j];
  if ( p < y ) goto LLgMgRR;
  if ( y < p ) {
    A[j] = x;
    x = y;
    goto LLgMgRL;
  }
  // y -> M
  if ( up <= j ) {
    A[j] = x;
    x = y;
    goto LLgMgRM2;
  }
  // right gap closed
  /*
    |o---]-----------+[---------][---------o|
    N    i          lw          j           M
    x -> R
   */
  goto LLgMRR;
  
 LLgMgRM1:
  /*
    |o---]-----------+[-]+-------------[---o|
    N    i          lw   up            j    M
    x -> M
   */
  y = A[lw];
  if ( p < y ) {
    A[lw] = x;
    x = y;
    lw--; 
    goto LLgMgRR;
  } 
  if ( y == p ) {
   if ( i < lw ) {
      lw--;
      goto LLgMgRM2;
    }
    goto LMgRM;
  }
  // y -> L
  if ( i < lw ) {
    A[lw] = x;
    x = y;
    lw--;
    goto LLgMgRL;
  }
  // left gap closed
  i++; 
  goto LLMgRM;
    
 LLgMgRM2:
  /*
    |o---]-----------+[-]+-------------[---o|
    N    i          lw   up            j    M
    x -> M
   */
  y = A[up];
  if ( y < p ) {
    A[up] = x;
    x = y;
    up++;
    goto LLgMgRL;
  }
  if ( y == p ) {
    if ( up < j ) {
      up++;
      goto LLgMgRM1;
    }
    /* We have here:
       p=x -> M
       N < k <= i -> A[k] < p
       lw < k < up -> p = A[k]
       j <= k < M -> p < A[k]
       up = j
       Hence the p=x-value can come only from the M-location.
       But that location is not accessible because the R-set 
       is not empty. Contradiction.  
     */
    goto LLgMM;
  } 
  // y -> R
  if ( up < j ) {
    A[up] = x;
    x = y;
    up++;
    goto LLgMgRR;
  }
  // right gap closed
  j--; 
  goto LLgMRM;

  // left gap closed
 LLMgRL:  
  /*
    |o---------][------]+-------------[---o|
    N           i       up            j    M
    x -> L
  */
// ch2xM(p, 2413, A, i, up-1);
  y = A[up];
  if ( y < p ) {
    A[up] = A[i]; A[i] = y;
    up++; i++;
    goto LLMgRL;
  }
  if ( y == p ) {
    if ( up == M ) {
   /*
    |o---------][-------------------]o|
    N           i                     M
    x -> L
   */
      A[N] = x;
      if ( p <= AM ) {
	A[M] = AM;
	// cut2x(A, N, i-1); return;
	i--;
	goto FinishL;
      }
      /*
      if ( p == AM ) {
	A[M] = AM;
	// cut2x(A, N, i-1); return;
	i--;
	goto FinishL;
      }
      */
      // AM < p
      A[M] = A[i]; A[i] = AM;
      // cut2x(A, N, i); return;
      goto FinishL;
    }
    up++;
    goto LLMgRL;
  }
  // y -> R
  if ( up < j ) {
    A[up] = A[i]; A[i] = x;
    x = y;
    i++; up++;
    goto LLMgRR;
  }
  // right gap closed also
  /*
    |o---------][------][-----------------o|
    N           i       j                  M
    x -> L
   */
  // up == j; A[j] == y 
  if ( p < AM ) {
    A[N] = x; A[M] = AM;
    j--;
// ch2xM(p, 2467, A, i, j);
    goto Finish;
  }
  if ( p == AM ) {
    A[N] = x; A[M] = y; A[j] = AM;
// ch2xM(p, 2472, A, i, j);
    goto Finish;
  }
  // AM -> L and x -> L
  A[M] = y; A[j] = A[i];
  if ( x <= AM ) { 
    A[N] = x; 
    A[i] = AM;
  } else {
    A[N] = AM; 
    A[i] = x;
  }
  i++;
// ch2xM(p, 2486, A, i, j);
  goto Finish;

 LLMgRM:
  /*
    |o---------][------]+-------------[---o|
    N           i       up            j    M
    x -> M
   */
  y = A[up];
  if ( y < p ) {
    A[up] = A[i]; A[i] = y; 
    i++; up++;
    goto LLMgRM;
  }
  if ( y == p ) {
    if ( up == M ) { // R empty 
      /*
    |o---------][-------------------]o|
    N           i                     M
    x -> M
      */
      if ( AM < p ) {
	A[N] = AM;
	A[M] = x;
	// cut2x(A, N, i-1); return;
	i--;
	goto FinishL;
      }
      /*
      if ( p < AM ) {
	A[M] = AM;
	i--;
	A[N] = A[i]; A[i] = x;
	i--;
	goto FinishL;
      }
      */
      A[M] = AM;
      i--;
      A[N] = A[i]; A[i] = x;
      // cut2x(A, N, i-1); return;
      i--;
      goto FinishL;
    }
    up++;
    goto LLMgRM;
  }
  // y -> R
  if ( up < j ) {
    A[up] = x;
    x = y;
    up++;
    goto LLMgRR;
  }
  // right gap closed also
  /*
    |o---------][------][-----------------o|
    N           i       j                  M
    x -> M
   */
  // up == j; A[j] == y 
  if ( p < AM ) {
    A[M] = AM;
    i--;
    A[N] = A[i]; A[i] = x;
    j--;
// ch2xM(p, 2554, A, i, j);
    goto Finish;
  } 
  if ( AM < p ) {
    A[N] = AM; A[M] = y; A[j] = x;
// ch2xM(p, 2559, A, i, j);
    goto Finish;
  }
  // AM -> M
  i--;
  A[N] = A[i]; A[i] = x;
  A[M] = y; A[j] = AM;
// ch2xM(p, 2566, A, i, j);
  goto Finish;

 LLMgRR:
  /*
    |o---------][------]+-------------[---o|
    N           i       up            j    M
    x -> R
  */
  j--;
  y = A[j];
  if ( p < y ) goto LLMgRR;
  if ( y < p ) {
    A[j] = x;
    x = y;
    goto LLMgRL;
  }
  // y -> M
  if ( up <= j ) {
    A[j] = x;
    x = y;
    goto LLMgRM;
  }
  // right gap closed also   y = A[j]
  /*
    |o---------][------][-----------------o|
    N           i      j                   M
    x -> R
   */
  A[M] = x;
  if ( AM < p ) {
    A[N] = AM; 
// ch2xM(p, 2598, A, i, j);
    goto Finish;
  }
  if ( p == AM ) {
    i--;
    A[N] = A[i]; A[i] = AM;
// ch2xM(p, 2604, A, i, j);
    goto Finish;
  }
  // AM -> R
  i--;
  A[N] = A[i]; A[i] = y; A[j] = AM;
  j--; 
// ch2xM(p, 2611, A, i, j);
  goto Finish;

  // right gap closed
 LLgMRR:
  /*
    |o---]-----------+[---------][---------o|
    N    i          lw          j           M
    x -> R
  */
  y = A[lw];
  if ( p < y ) {
    A[lw] = A[j]; A[j] = y;
    lw--; j--;
    goto LLgMRR;    
  }
  if ( y == p ) {
    if ( N == lw ) { // L is empty 
      /*
    |o[-----------------------][---------o|
    N                         j           M
    x -> R
      */
      A[M] = x;
      if ( AM <= p ) {
	A[N] = AM;
	// cut2x(A, j+1, M); return;
	j++;
	goto FinishR;
      }
      /*
      if ( AM == p ) {
	A[N] = AM;
	// cut2x(A, j+1, M); return;
	j++;
	goto FinishR;
      }
      */
      // p < AM
      A[N] = A[j]; A[j] = AM;
      // cut2x(A, j, M); return;
      goto FinishR;
    }
    lw--;
    goto LLgMRR; 
  }
  // y -> L
  if ( i < lw ) {
    A[lw] = A[j]; A[j] = x;
    x = y;
    lw--; j--;
    goto LLgMRL;
  }
  // left gap closed also
  /*
    |o---][---------------------][---------o|
    N    i                      j           M
    x -> R
  */
  // y == A[lw] == A[i]
  A[M] = x;
  if ( AM < p ) {
    A[N] = AM;
    i++;
// ch2xM(p, 2676, A, i, j);
    goto Finish;
  }
  if ( AM == p ) {
    A[N] = y; A[i] = AM;
// ch2xM(p, 2681, A, i, j);
    goto Finish;
  }
  // AM -> R
  A[N] = y; A[i] = A[j]; A[j] = AM;
  j--; 
// ch2xM(p, 2687, A, i, j);
  goto Finish;

 LLgMRM:
  /*
    |o---]-----------+[---------][---------o|
    N    i          lw          j           M
    x -> M
  */
  y = A[lw];
  if ( p < y ) {
    A[lw] = A[j]; A[j] = y;
    lw--; j--;
    goto LLgMRM;    
  }
  if ( y == p ) {
    if ( N == lw ) { // L empty 
      /*
    |o[-----------------------][---------o|
    N                         j           M
    x -> M
      */
      if ( AM <= p ) {
	A[N] = AM;
	j++;
	A[M] = A[j];
	A[j] = x;
	// cut2x(A, j+1, M); return;
	j++;
	goto FinishR; 
      }
      /*
      if ( AM == p ) {
	A[N] = AM;
	j++;
	A[M] = A[j];
	A[j] = x;
	// cut2x(A, j+1, M); return;
	j++;
	goto FinishR; 
      }
      */
      // p < AM
      A[M] = AM; A[N] = x;
      // cut2x(A, j+1, M); return;
      j++;
      goto FinishR; 
    }
    lw--;
    goto LLgMRM; 
  }
  // y -> L
  if ( i < lw ) {
    A[lw] = x;
    x = y;
    lw--;
    goto LLgMRL;
  }
  // left gap closed also
  /*
    |o---][---------------------][---------o|
    N    i                      j           M
    x -> M
  */
  // y == A[lw] == A[i]
  if ( p < AM ) {
    A[M] = AM; 
    A[N] = y; A[i] = x;
// ch2xM(p, 2756, A, i, j);
    goto Finish;
  }
  if ( AM < p ) {
    A[N] = AM; 
    j++;
    A[M] = A[j]; A[j] = x;
    i++;
// ch2xM(p, 2764, A, i, j);
    goto Finish;
  }
  // AM == p
  A[N] = y; A[i] = x;
  j++;
  A[M] = A[j]; A[j] = AM;
// ch2xM(p, 2771, A, i, j);
  goto Finish;

 LLgMRL:
  /*
    |o---]-----------+[---------][---------o|
    N    i          lw          j           M
    x -> L
   */
  i++;
  y = A[i];
  if ( y < p ) goto LLgMRL;
  if ( p < y ) {
    A[i] = x;
    x = y;
    goto LLgMRR;
  }
  // y -> M
  if ( i <= lw ) {
    A[i] = x;
    x = y;
    goto LLgMRM;  
  }
  // left gap closed also  y = A[i];
   /*
    |o---][----------------------][---------o|
    N     i                      j           M
    x -> L
   */
  A[N] = x;
  if ( p < AM ) { 
    A[M] = AM;
// ch2xM(p, 2803, A, i, j);
    goto Finish;
  }
  if ( p == AM ) {
    j++;
    A[M] = A[j]; A[j] = AM;
// ch2xM(p, 2809, A, i, j);
    goto Finish;
  }
  // AM -> L
  j++;
  A[M] = A[j]; A[j] = y; A[i] = AM;
  i++;
// ch2xM(p, 2816, A, i, j);
  goto Finish;

 LMgRM:
   /*
    |o[---------------]+-------------[---o|
    N                  up            j    M
    x -> M
   */
  y = A[up];
  if ( y < p ) { // N == i 
    i++;
    A[up] = A[i]; A[i] = y;
    i++; up++;
    goto LLMgRM;
  }
  if ( p == y ) {
    if ( up == M ) {
   /*
    |o[----------------------------]o|
    N                                M
    x -> M
   */
      if ( AM <= p ) {
	A[N] = AM; A[M] = x;
	goto Finish0;
      }
      /*
      if ( AM == p ) {
	A[N] = AM; A[M] = x;
	goto Finish0;
      }
      */
      // p < AM
      A[N] = x; A[M] = AM;
      goto Finish0;
    }
    up++;
    goto LMgRM;
  }
  // p < y
  if ( up < j ) {
    A[up] = x;
    x = y;
   /*
    |o[---------------]+-------------[---o|
    N                  up            j    M
    x -> R
   */
    goto LMgRR;
  }
  // right gap closed  y = A[up] = A[j]
  /*
    |o[-----------------------------][---o|
    N                                j    M
    x -> M
   */
  if ( AM <= p ) {
    A[N] = AM;
    A[M] = y; A[j] = x;
    // cut2x(A, j+1, M); return;
    j++;
    goto FinishR;
  }
  /*
  if ( AM < p ) {
    A[N] = AM;
    A[M] = A[j]; A[j] = x;
    // cut2x(A, j+1, M); return;
    j++;
    goto FinishR;
  }
  if ( AM == p ) {
    A[N] = AM;
    A[M] = A[j]; A[j] = x;
    // cut2x(A, j+1, M); return;
    j++;
    goto FinishR;
  }
  */
  // AM -> R
  A[N] = x; A[M] = AM;
  // cut2x(A, j, M); return;
  goto FinishR;

 LLgMM:
   /*
    |o---]-----------+[------------------]o|
    N    i          lw                     M=j
    x -> M
   */
  y = A[lw];
  if ( p < y ) {
    j--;
    A[lw] = A[j]; A[j] = y;
    lw--;
    j--;
    goto LLgMRM;
  }
  if ( p == y ) {
    if ( N == lw ) { // L empty also 
      /*
    |o[----------------------------]o|
    N                                M
    x -> M
      */
      if ( AM <= p ) {
	A[N] = AM; A[M] = x;
	goto Finish0;
      }
      /*
      if ( AM == p ) {
	A[N] = AM; A[M] = x;
	goto Finish0;
      }
      */
      // AM -> R
      A[N] = x; A[M] = AM;
      goto Finish0;
    }
    lw--;
    goto LLgMM;
  }
  // y -> L
  if ( i < lw ) {
    A[lw] = x;
    x = y;
    lw--;
    goto LLgML;
  }
  // left gap closed also  y = A[lw] = A[i] -> L
   /*
    |o---][-----------------------------]o|
    N    i                                M
    x -> M
   */
  if ( AM < p ) {
    A[N] = AM; A[M] = x;
    // cut2x(A, N, i); return;
    goto FinishL;
  }
  if ( AM == p ) {
    A[N] = y; A[i] = AM; A[M] = x;
    // cut2x(A, N, i); return;
    i--;
    goto FinishL;
  }
  // p < AM
  A[M] = AM;
  A[N] = y; A[i] = x;
  // cut2x(A, N, i-1); return;
  i--;
  goto FinishL;

 LLgML:
   /*
    |o---]-----------+[------------------]o|
    N    i          lw                     M=j
    x -> L
   */
  i++;
  y = A[i];
  if ( y < p ) goto LLgML;
  if ( p == y ) {
    if ( i <= lw ) {
      A[i] = x;
      x = y;
      goto  LLgMM;
    }
    // left gap closed  y = A[i]
   /*
    |o--------------][------------------]o|
    N                i                    M=j
    x -> L
   */
    A[N] = x;
    if ( p <= AM ) {
      A[M] = AM;
      // cut2x(A, N, i-1); return;
      i--;
      goto FinishL;
    }
    /*
    if ( p == AM ) {
      A[M] = AM;
      // cut2x(A, N, i-1); return;
      i--;
      goto FinishL;
    }
    */
    // AM -> L
    A[M] = y; A[i] = AM;
    // cut2x(A, N, i); return;
    goto FinishL;
  }
  // y -> R
  A[i] = x;
  x = y;
  goto LLgMR;

 LLgMR:
   /*
    |o---]-----------+[------------------]o|
    N    i          lw                     M=j
    x -> R
   */
  y = A[lw];
  if ( p < y ) {
    j--;
    A[lw] = A[j]; A[j] = y;
    j--; lw--;
    goto LLgMRR;
  }
  if ( p == y ) {
    if ( N == lw ) { // L is empty 
      /*
    |o[----------------------------]o|
    N                                M=j
    x -> R
      */
     if ( AM <= p ) { 
	A[N] = AM;
	A[M] = x;
	goto Finish0;
     }
     /*
     if ( AM == p ) { 
	A[N] = AM;
	A[M] = x;
	goto Finish0;
     }
     */
     // AM -> R
     j--;// = M-1
     A[N] = A[j];
     if ( x <= AM ) {
       A[j] = x; A[M] = AM;
     } else {
       A[j] = AM; A[M] = x;
     }
     goto Finish0;
    }
    lw--;
    goto LLgMR;
  }
  // y -> L
  if ( i < lw ) {
    j--;
    A[lw] = A[j]; A[j] = x;
    x = y;
    lw--; j--;
    goto LLgMRL;
  }
  // left gap closed y = A[lw] = A[i] -> L
 /*
    |o---][----------------------------]o|
    N    i                               M=j
    x -> R
   */
  if ( AM < p ) {
    A[N] = AM; A[M] = x;
    // cut2x(A, N, i); return;
    goto FinishL;
  }
  if ( p == AM ) {
    A[M] = x;
    A[N] = y; A[i] = AM;
    // cut2x(A, N, i-1); return;
    i--;
    goto FinishL;
  }
  // AM -> R
  j--;
  A[N] = y; A[i] = A[j];
  if ( x <= AM ) {
    A[j] = x; A[M] = AM;
  } else {
    A[j] = AM; A[M] = x;
  }
  // cut2x(A, N, i-1); return;
  i--;
  goto FinishL;

 LMgRR:
   /*
    |o[---------------]+-------------[---o|
    N                  up            j    M
    x -> R
   */
  j--;
  y = A[j];
  if ( p < y ) goto LMgRR;
  if ( y < p ) {
    A[j] = x;
    x = y;
   /*
    |o[---------------]+-------------[---o|
    N                  up            j    M
    x -> L
   */
    goto LMgRL;
  }
  // y -> M
  if ( up <= j ) {
    A[j] = x;
    x = y;
    goto LMgRM;
  }
  // right gap closed  y = A[j]
  /*
    |o[---------------][----------------o|
    N                 j                  M
    x -> R
   */
  A[M] = x;
  if ( AM <= p ) {
    A[N] = AM;
    // cut2x(A, j+1, M); return;
    j++;
    goto FinishR;
  }
  /*
  if ( AM == p ) {
    A[N] = AM;
    // cut2x(A, j+1, M); return;
    j++;
    goto FinishR;
  }
  */
  // AM -> R
  A[N] = A[j]; A[j] = AM;
  // cut2x(A, j, M); return;
  goto FinishR;

 LMgRL:
   /*
    |o[---------------]+-------------[---o|
    N                  up            j    M
    x -> L
   */
  y = A[up];
  if ( y < p ) { // create L
    i++;
    A[up] = A[i]; A[i] = y;
    i++; up++;
    goto LLMgRL;    
  }
  if ( p == y ) {
    if ( up == M ) { // R is empty 
      /*
    |o[----------------------------]o|
    N                                M
    x -> L
      */
      if ( p <= AM ) {
	A[N] = x; A[M] = AM;
	goto Finish0;
      }
      /*
      if ( AM == p ) {
	A[N] = x; A[M] = AM;
	goto Finish0;
      }
      */
      // AM -> L
      i++;
      A[M] = A[i];
      if ( x <= AM ) {
       A[N] = x; A[i] = AM;
      } else {
	A[N] = AM; A[i] = x;
      }
      goto Finish0;
    }
    up++;
    goto LMgRL;
  }
  // y -> R
  if ( up < j ) {
    i++;
    A[up] = A[i]; A[i] = x;
    x = y;
    i++; up++;
    goto LLMgRR;
  }
  // right gap closed   y = A[up] = A[j]
  /*
    |o[-----------------------------][---o|
    N                                j    M
    x -> L
   */
  if ( p < AM ) {
    A[N] = x; A[M] = AM;
    // cut2x(A, j, M); return;
    goto FinishR;
  }
  if ( p == AM ) {
    A[N] = x; 
    A[M] = A[j]; A[j] = AM;
    // cut2x(A, j+1, M); return;
    j++;
    goto FinishR;
  }
  // AM -> L
  i++;
  A[M] = A[j]; A[j] = A[i];
  if ( x <= AM ) {
    A[N] = x; A[i] = AM;
  } else {
    A[N] = AM; A[i] = x;
  }
  j--;
  goto FinishR;

  // int k; // for testing

 Finish0:
  return;

 FinishL:
  // cut4(A, N, i);
  (*cut)(A, N, i, depthLimit);
  return;

 FinishR:
  (*cut)(A, j, M, depthLimit);
  return;

 Finish:
  /* holes filled
    |o---------][------][-----------------o|
    N           i      j                   M
  */
  /*
  for (k = N; k < i; k++) 
    if ( p <= A[k] ) {
      printf("Error L at k: %d \n", k);
      exit(1);
    }
  for (k = i; k <= j; k++) 
    if ( p != A[k] ) {
      printf("Error M at k: %d \n", k);
      // printf("N: %d i: %d j: %d M: %d\n", N, i, j, M);
      exit(1);
    }
  for (k = j+1; k <= M; k++) 
    if ( A[k] <= p ) {
      printf("Error R at k: %d \n", k);
      printf("N: %d i: %d j: %d M: %d\n", N, i, j, M);
      exit(1);
    }
  */
  /*
  printf("next  N: %d M %d ", N, M);
  for (k = N; k <= M; k++) printf("%d ", A[k]);
  printf("\n");
  exit(1);
  */

  if ( i - N  < M - j ) {
    (*cut)(A, N, i-1, depthLimit);
    (*cut)(A, j+1, M, depthLimit);
  } else {
    (*cut)(A, j+1, M, depthLimit);
    (*cut)(A, N, i-1, depthLimit);
  }
} // end of cut2x2


/* OBSOLETE version
  Here the 2009 May version of the 3-partitioning algorithm.
  Small segments are delegated to: quicksort0 with insertionsort.
  Cut2 may be called in weird situations.
  The activated pivots calculator uses the quicksort0 sorter on a
  small, 0.1% sample in the middle.
  The code is tricky ... use extreme care to modify.
  The test to go left or right at the threeSegments label has been
  deactivated.  It slows down the performance.  Hence there is dead
  code that otherwise would work on the right gap.
  Using tps instead ...
 */
// DEPRECATED
void cut3x(int *A, int N, int M) {
  // printf("cut3x %d %d \n", N, M);
  int L = M - N;
  int depthLimit = 2.5 * floor(log(L));
  if ( L < cut3Limit ) {
    cut2c(A, N, M, depthLimit);
    return; 
  }
  cut3xc(A, N, M, depthLimit);
}

void cut3xc(int *A, int N, int M, int depthLimit) {
  // printf("cut3xc %d %d %d \n", N, M, depthLimit);
  int L = M - N;
  if ( L < cut3Limit ) {
    cut2c(A, N, M, depthLimit);
    return; 
  }
  
  /*   L             M             R
    |-----]------+[------]+-----[-----|
    N     i      lw      up     j     M
   */

  register int i, j, lw, up; // indices
  register int maxl, minr;   // pivots for left/ right boundaries
  /* There are invariants to be maintained (and >essential< 
     for machine assisted correctness proofs):
     If there are two gaps:
       N <= x <= i --> A[x] <= maxl
       lw < x < up  --> maxl < A[x] < minr
       j <= x <= M --> minr <= A[x]
     If the left gap has closed:
       N <= x < i --> A[x] <= maxl
       i <= x < up  --> maxl < A[x] < minr
       j <= x <= M --> minr <= A[x]
     If the right gap has closed:
       N <= x <= i --> A[x] <= maxl
       lw < x <= j  --> maxl < A[x] < minr
       j < x <= M --> minr <= A[x]
  */
  int ai, aj, alw, aup; // values
  /*
  int midpoint = (N+M)/2;
  int halfSampleLng = 10 + L/cutSampleParam;
  int oneThird = (2 * halfSampleLng)/3;
  int N1 = midpoint - halfSampleLng;
  int M1 = midpoint + halfSampleLng;
  // sort to find quality end points and pivots
  // quicksort0(A, N1, M1);
  cut3x(A, N1, M1); 
  if ( A[N1] < A[N] ) iswap(N, N1, A);
  if ( A[M] < A[M1] ) iswap(M, M1, A);
  maxl = A[N1+oneThird];
  minr = A[M1-oneThird];
  */

  int probeLng = L/ probeParamCut3x;
  if ( probeLng < 20 ) probeLng = 20; // quite short indeed
  int lastElement = N + probeLng - 1;
  int oneThird = probeLng/3;
  int maxlx = N + oneThird;
  int minrx = lastElement - oneThird;
  int offset = L/probeLng;
  int k;
  for (k = 0; k < probeLng; k++) iswap(N + k, N + k * offset, A);
  // quicksort0(A, N, lastElement);
  // quicksort0(A, N, lastElement); // protect quicksort0 against duplicates ...
  cut2c(A, N, lastElement, depthLimit);

  // int first = A[N];
  maxl = A[maxlx];
  minr = A[minrx];
  // int last = A[lastElement];

  /*
  // test here for duplicates and go to cut2x2 when dups found
  int dupx = -1; // candidate index to dups
  if ( first == last ) dupx = N; else
  if ( first == maxl ) dupx = N; else
  if ( last == minr ) dupx = lastElement;
  int cut3xc();
  if ( 0 <= dupx ) {
    cut2x2(A, N, M, dupx, cut3xc, depthLimit);
    return;
  }
  */

  if ( A[M] < A[lastElement] ) iswap(M, lastElement, A);
  // */

  // check to play safe
  if ( maxl < A[N] || A[M] < minr || maxl == minr ) { // give up
    cut2c(A, N, M, depthLimit);
    return;
  }

  // Ready to roll ...
  i = N;
  j = M;

 left:
         /*   L                           R
	   |-----]---------------------[-----|
	   N     i                     j     M
	 */
        i++;
	ai = A[i];
	if ( ai <= maxl ) goto left;
	if ( ai < minr ) goto createMiddleLeft; 

 right:
         /*   L                           R
	   |-----]o--------------------[-----|
	   N      i                    j     M
	   ai -> R
	 */
	j--;
	aj = A[j];
	if ( minr <= aj ) goto right;
	if ( maxl < aj ) goto createMiddleRight;
	// aj belongs in left
	if ( i < j ) { // swap them
	  A[i] = aj; A[j] = ai;
	  goto left;
	}
	// j = i-1
         /*   L                           R
	   |-----][--------------------------|
	   N     ji                          M
	 */
	if ( j - N < M - i ) {
	  cut3x(A, N, j);
	  cut3x(A, i, M);
	  return;
	}
	cut3x(A, i, M);
	cut3x(A, N, j);
	return;

 createMiddleLeft:
	//L200
         /*   L                           R
	   |-----]o--------------------[-----|
	   N      i                    j     M
	   ai -> M
	 */
	if ( i+1 == j ) { // very unusual case
	  if ( j - N < M - i ) {
	    cut3x(A, N, i-1);
	    cut3x(A, j, M);
	    return;
	  }
	  cut3x(A, j, M);
	  cut3x(A, N, i-1);
	  return;
	}
	// init non-empty middle segment
	lw = (i+j)/2;
	up = lw + 1;
	A[i] = A[lw]; A[lw] = ai;
	lw--;
	i--;
	goto threeSegments;
	
 createMiddleRight:
	//300
         /*   L                           R
	   |-----]o-------------------o[-----|
	   N      i                   j      M
	   ai -> R  aj -> M
	 */
	if ( i+1 == j ) { // again very unusual
	  A[i] = aj; A[j] = ai;
	  if ( j - N < M - i ) {
	    cut3x(A, N, i-1);
	    cut3x(A, j, M);
	    return;
	  }
	  cut3x(A, j, M);
	  cut3x(A, N, i-1);
	  return;
	}
	// init non-empty middle segment
	lw = (i+j)/2;
	up = lw + 1;
	A[i] = A[lw]; A[lw] = aj; A[j] = ai;
	i--;
	// goto threeSegments;

 threeSegments:
	//801
	/*   L             M             R
	  |-----]------+[------]+-----[-----|
	  N     i      lw      up     j     M
	*/

	// if ( lw - i < j - up ) goto threeSegmentsRight;
	i++;
	ai = A[i];
	if ( ai <= maxl ) goto threeSegments;
	if ( minr <= ai ) {
	  goto AItoR; 
	}
	// ai belongs in M
	if ( lw < i ) {
	  goto leftClosed;
	}
	goto AItoM;
	
// threeSegmentsRight: // used here
	/*   L             M             R
	  |-----]------+[------]+-----[-----|
	  N     i      lw      up     j     M
	*/
	/*
	j--;
	aj = A[j];
	if ( minr <= aj ) {
	  goto threeSegments;
	}
	if ( aj <= maxl ) goto AJtoL;
	// aj belongs in M
	if ( j < up ) goto rightClosed;
	goto AJtoM;
	*/

 AItoM: 
 //L201
         /*   L             M             R
	   |-----]o------+[---]+-------[-----|
	   N      i      lw    up      j     M
	   ai -> M
	 */

	alw = A[lw];
	if ( minr <= alw ) goto AItoMandALWtoR;
	if ( alw <= maxl ) {
	  if ( lw <= i ) goto leftClosed;
	  A[i] = alw; A[lw] = ai;
	  lw--;
	  goto threeSegments;
	}
	// alw belongs in M
	if ( lw <= i ) goto leftClosed;
	lw--;
	goto AItoM;

 AItoRandAJtoM:
	//301
         /*   L             M              R
	   |-----]o------+[---]+-------o[-----|
	   N      i      lw    up      j      M
	   ai -> R  aj -> M
	 */

	alw = A[lw];
	if ( alw <= maxl ) {
	  A[i] = alw; A[lw] = aj; A[j] = ai;
	  lw--;
	  goto threeSegments;
	}
	// alw not in L
	if ( alw < minr ) { // and not in R
	  lw--;
	  goto AItoRandAJtoM;
	}
	// alw belongs in R
	A[lw] = aj; A[j] = alw;
	if ( lw <= i ) {
	  goto leftClosed;
	}
	lw--;
	goto AItoR;

 AItoMandALWtoR: 
         /*   L             M             R
	   |-----]o------o[---]+-------[-----|
	   N      i      lw    up      j     M
	   ai -> M  alw -> R
	 */

	j--;
	aj = A[j];
	if ( minr <= aj ) goto AItoMandALWtoR;
	if ( aj <= maxl ) {
	  A[i] = aj; A[j] = alw; A[lw] = ai;
	  lw--;
	  goto threeSegments;
	}
	// aj belongs in M
	A[lw] = aj; A[j] = alw;
	if ( up <= j ) {
	  lw--;
	  goto AItoM;
	}
	// right is closed !
	lw--; j--;
	goto rightClosedAItoM;

 AItoR:
         /*   L             M              R
	   |-----]o------+[---]+--------[-----|
	   N      i      lw    up       j     M
	   ai -> R 
	 */

	j--;
	aj = A[j];
	if ( minr <= aj ) goto AItoR;
	if ( aj <= maxl ) {
	  A[i] = aj; A[j] = ai;
	  goto threeSegments;
	}
	// aj belongs to M
	if ( j < up ) goto rightClosedAItoR;
	goto AItoRandAJtoM;

 AJtoM:
	/*   L             M             R
	  |-----]------+[------]+----o[-----|
	  N     i      lw      up    j      M
	  aj -> M
	*/

	aup = A[up];
	if ( aup <= maxl ) {
	  goto AJtoMandAUPtoL; // like AItoMandALWtoR;
	}
	if ( minr <= aup ) { 
	  if ( j <= up ) goto rightClosed;	  
	  A[up] = aj; A[j] = aup;
	  up++;
	  goto threeSegments;
	}
	// aup belongs in M
	if ( j <= up ) goto rightClosed;
	up++;
	goto AJtoM;

 AJtoMandAUPtoL: 
	// like AItoMandALWtoR;
	/*   L             M             R
	  |-----]------+[------]o----o[-----|
	  N     i      lw      up    j      M
	  aj -> M  aup -> L
	*/

	i++;
	ai = A[i];
	if ( ai <= maxl ) goto AJtoMandAUPtoL;
	if ( minr <= ai ) {
	  A[j] = ai; A[i] = aup; A[up] = aj;
	  up++;
	  goto threeSegments;
	}
	// ai belongs in M
	A[up] = ai; A[i] = aup;
	if ( i <= up ) {
	  up++;
	  goto AJtoM;
	}
	// left is closed !
	up++; i++;
	goto leftClosedAJtoM;

 AJtoL:
	/*   L             M             R
	  |-----]------+[------]+----o[-----|
	  N     i      lw      up    j      M
	  aj -> L
	*/

	i++;
	ai = A[i];
	if ( ai <= maxl ) goto AJtoL;
	if ( minr <= ai ) {
	  A[i] = aj; A[j] = ai;
	  goto threeSegments;
	}
	// ai belongs to M
	if ( lw < i ) {
	  goto leftClosedAJtoL;
	}
	// fall through

 AItoMandAJtoL:
	/*   L             M             R
	  |-----]o-----+[------]+----o[-----|
	  N      i     lw      up    j      M
	  ai -> M   aj -> L
	*/

	aup = A[up];
	if ( minr <= aup ) {
	  A[i] = aj; A[up] = ai; A[j] = aup;
	  up++;
	  goto threeSegments;
	}
	// aup not in R
	if ( maxl < aup ) { // and not in L
	  up++;
	  goto AItoMandAJtoL;
	}
	// aup belongs in L
	A[i] = aup; A[up] = ai;
	if ( j <= up ) goto rightClosed;
	up++;
	goto AJtoL;

 leftClosed:
	//L701
         /*   L         M                 R
	   |-----][-----------]+-------[-----|
	   N      i           up       j     M
	 */

	j--;
	aj = A[j];
	if ( minr <= aj ) goto leftClosed;
	if ( aj <= maxl ) goto leftClosedAJtoL;
	// aj belongs in M
	if ( j < up ) goto finished;
	// fall through

leftClosedAJtoM:
	//711
         /*   L         M                 R
	   |-----][-----------]+------o[-----|
	   N      i           up      j      M
	   aj -> M
	 */

	aup = A[up];
	if ( aup <= maxl ) {
	  A[up] = A[i]; A[i] = aup;
	  i++; up++;
	  goto leftClosedAJtoM;
	}
	if ( aup < minr ) {
	  up++;
	  goto leftClosedAJtoM;
	}
	if ( up < j ) {
	  A[up] = aj; A[j] = aup;
	  goto leftClosed;
	}
	goto finished;

 leftClosedAJtoL:
	//702
         /*   L         M                 R
	   |-----][-----------]+------o[-----|
	   N      i           up      j      M
	   aj -> L
	 */

	aup = A[up];
	if ( minr <= aup ) {
	  A[up] = A[i]; A[i] = aj; A[j] = aup;
	  i++; up++;
	  goto leftClosed;
	}
	if ( maxl < aup ) {
	  up++;
	  goto leftClosedAJtoL;
	}
	A[up] = A[i]; A[i] = aup;
	i++;
	if ( j <= up ) { //  we have taken care of aj also !
	  goto finished;
	}
	up++;
	goto leftClosedAJtoL;

 rightClosed:
	// like 602
         /*   L                 M           R
	   |-----]-------+[------------][-----|
	   N     i       lw            j      M
	 */

	i++;
	ai = A[i];
	if ( ai <= maxl ) goto rightClosed;
	if ( minr <= ai ) goto rightClosedAItoR;
	// ai belongs in M
	if ( lw < i ) goto finished;
	// fall through

 rightClosedAItoM:
	// 621
         /*   L                 M           R
	   |-----]o------+[------------][-----|
	   N      i      lw            j      M
	   ai -> M
	 */

	alw = A[lw];
	if ( minr <= alw ) {
	  A[lw] = A[j]; A[j] = alw;
	  lw--; j--;
	  goto rightClosedAItoM;
	}
	if ( maxl < alw ) {
	  lw--;
	  goto rightClosedAItoM;
	}
	if ( i < lw ) {
	  A[lw] = ai; A[i] = alw;
	  goto rightClosed;
	}
	goto finished;

 rightClosedAItoR:
	//601
        /*   L                 M           R
	   |-----]o------+[------------][-----|
	   N      i      lw            j      M
	   ai -> R
	 */

	alw = A[lw];
	if ( alw <= maxl ) {
	  A[i] = alw; A[lw] = A[j]; A[j] = ai;
	  lw--; j--;
	  goto rightClosed;
	}
	if ( alw < minr ) {
	  lw--;
	  goto rightClosedAItoR;
	}
	A[lw] = A[j]; A[j] = alw;
	j--;
	if ( lw <= i ) { //  we have taken care of ai also !
	  goto finished;
	}
	lw--;
	goto rightClosedAItoR;

	int lleft, lmiddle, lright; // lengths of the segments

 finished:
	//714
        /*    L             M              R
	   |-----][--------------------][-----|
	   N      i                    j      M
	 */

	 lleft = i - N;
         lmiddle = j - i;
	 lright = M - j;
	 if ( lleft < lmiddle ) {      // L < M
	   if ( lleft < lright ) {     // L < R
	     cut3x(A, N, i-1);
	     if ( lmiddle < lright ) { // L < M < R
	       cut3x(A, i, j);
	       cut3x(A, j+1, M);
	     } else {                  // L < R < M     
	       cut3x(A, j+1, M);
	       cut3x(A, i, j);
	     }
	     return;
	   } else {                    // R < L < M 
	     cut3x(A, j+1, M);
	     cut3x(A, N, i-1);
	     cut3x(A, i, j);
	     return;
	   }
	 }                             // M < L
	 if ( lmiddle < lright ) {     // M < R
	   cut3x(A, i, j);
	   if ( lleft < lright) {
	     cut3x(A, N, i-1);
	     cut3x(A, j+1, M);
	   } else {
	     cut3x(A, j+1, M);
	     cut3x(A, N, i-1);
	   }
	   return;
	 }                             // R < M < L
	 cut3x(A, j+1, M);
	 cut3x(A, i, j);
	 cut3x(A, N, i-1);

} // end cut3xc


/* OBSOLETE version
  Here another version of the 3-partitioning algorithm.
  Small segments are delegated to: quicksort0 with insertionsort.
  Cut2 may be called in weird situations.
  The activated pivots calculator uses the quicksort0 sorter on a
  small, 0.1% sample in the middle.
  The code is tricky ... use extreme care to modify.
  Using tps instead ...
 */
// DEPRECATED
void cut3y(int *A, int N, int M) {
  // printf("cut3x %d %d \n", N, M);
  int L = M - N;
  int depthLimit = 2.5 * floor(log(L));
  if ( L < cut3Limit ) {
    cut2c(A, N, M, depthLimit); 
    return; 
  }
  cut3yc(A, N, M, depthLimit);
}

void cut3yc(int *A, int N, int M, int depthLimit) {
  // printf("cut3xc %d %d %d \n", N, M, depthLimit);
  int L = M - N;
  if ( L < cut3Limit ) {
    cut2c(A, N, M, depthLimit);
    return; 
  }  


  /*   L             M             R
    |--o--]-------[------]------[-----|
    N     i      lw      up     j     M
   */

  int i, j, lw, up, hole; // indices
  int maxl, minr;   // pivots for left/ right boundaries
  /* There are invariants to be maintained (and >essential< 
     for machine assisted correctness proofs):
     If there are two gaps:
       L             M             R
    |o----]-------[------]------[-----|
    N     i      lw      up     j     M

       N <= k <= i --> A[k] <= maxl except hole position
       lw <= k <= up  --> maxl < A[k] < minr
       j <= k <= M --> minr <= A[k]

     If the left gap has closed:
       L             M             R
    |o----][-------------]------[-----|
    N      i             up     j     M

       N <= k < i --> A[k] <= maxl
       i <= k <= up  --> maxl < A[k] < minr
       j <= k <= M --> minr <= A[k]

     If the right gap has closed:
       L             M             R
    |o----]-------[------------][-----|
    N     i      lw            j      M

       N <= k <= i --> A[k] <= maxl
       lw <= k <= j  --> maxl < A[k] < minr
       j < k <= M --> minr <= A[k]
  */
  int x, y; // values

  // initialize
  /*
  int midpoint = (N+M)/2;
  int halfSampleLng = 10 + L/cutSampleParam;
  int oneThird = (2 * halfSampleLng)/3;
  int N1 = midpoint - halfSampleLng;
  int M1 = midpoint + halfSampleLng;
  // sort to find quality end points and pivots
  quicksort0(A, N1, M1);
  if ( A[N1] < A[N] ) iswap(N, N1, A);
  if ( A[M] < A[M1] ) iswap(M, M1, A);
  maxl = A[N1+oneThird];
  minr = A[M1-oneThird];
  */

  int probeLng = L/ probeParamCut3x;
  if ( probeLng < 20 ) probeLng = 20; // quite short indeed
  int lastElement = N + probeLng - 1;
  int oneThird = probeLng/3;
  int maxlx = N + oneThird;
  int minrx = lastElement - oneThird;
  int offset = L/probeLng;
  int k;
  for (k = 0; k < probeLng; k++) iswap(N + k, N + k * offset, A);
  // quicksort0(A, N, lastElement);
  // quicksort0(A, N, lastElement); // protect quicksort0 against duplicates ...
  cut2c(A, N, lastElement, depthLimit);
  // int first = A[N];
  maxl = A[maxlx];
  minr = A[minrx];
  // int last = A[lastElement];

  /*
  // test here for duplicates and go to cut2x2 when dups found
  int dupx = -1; // candidate index to dups
  if ( first == last ) dupx = N; else
  if ( first == maxl ) dupx = N; else
  if ( last == minr ) dupx = lastElement;
  int cut3yc();
  if ( 0 <= dupx ) {
    cut2x2(A, N, M, dupx, cut3yc, depthLimit);
    return;
  }
  */ 
  if ( A[M] < A[lastElement] ) iswap(M, lastElement, A);

  // check to play safe
  if ( maxl < A[N] || A[M] < minr ) { // impossible, but give up
    cut2c(A, N, M, depthLimit);
    return;
  }

  // Ready to roll ...
  i = N;
  j = M;

 start:
         /*   L                           R
	   |-----]---------------------[-----|
	   N     i                     j     M
	 */
        i++;
	x = A[i];
	if ( x <= maxl ) goto start;
	A[i] = A[N];
	hole = N;
	if ( minr <= x ) goto right;
	goto createMiddle;

 left:
         /*   L                           R
	   |o----]---------------------[-----|
	   N     i                     j     M
	   x -> L
	 */
	i++;
	y = A[i];
	if ( y <= maxl ) goto left;
	if ( y < minr ) {
	  A[i] = x;
	  x = y;
	  goto createMiddle;
	}
	// y -> R but check whether gap closed
	if ( i < j ) {
	  A[i] = x;
	  x = y;
	  goto right;
	}
	// gap closed
	A[hole] = x;
	if ( i - N < M - j) {
	  cut3y(A, N, i-1);
	  cut3y(A, j, M);
	  return;
	}
	cut3y(A, j, M);
	cut3y(A, N, i-1);
	return;
	
 right:
         /*   L                           R
	   |o----]---------------------[-----|
	   N     i                     j     M
	   x -> R
	 */
	j--;
	y = A[j];
	if ( minr <= y ) goto right;
	if ( maxl < y ) {
	  A[j] = x;
	  x = y;
	  goto createMiddle;
	}
	// y -> L 
	if ( i == j ) {
	  A[hole] = A[i];
	  A[i] = x;
	  if ( i - N < M - j) {
	    cut3y(A, N, i-1);
	    cut3y(A, j, M);
	    return;
	  }
	  cut3y(A, j, M);
	  cut3y(A, N, i-1);
	  return;
	}
	// i < j
	A[j] = x;
	x = y;
	goto left;

 createMiddle:
         /*   L                           R
	   |o----]---------------------[-----|
	   N     i                     j     M
	   x -> M
	 */
	lw = up = (i+j)/2;
	y = A[lw];
	A[lw] = x;
	x = y;
	if ( minr <= x ) goto TLgMgRR;
	if ( maxl < x ) goto TLgMgRMright;
	// x -> L

 TLgMgRL:
         /*   L                           R
	   |o----]-------[----]--------[-----|
	   N     i       lw  up        j     M
	   x -> L
	 */
	i++;
	y = A[i];
	if ( y <= maxl ) goto TLgMgRL;
	if ( minr <= y ) {
	  A[i] = x;
	  x = y;
	  goto TLgMgRR;
	}
	// y -> M
	if ( i < lw ) {
	  A[i] = x;
	  x = y;
	  goto TLgMgRMright;
	}
	// left gap closed
	/*
              L             M             R
           |o----][-------------]------[-----|
           N      i             up     j     M
	*/
	A[hole] = x;
 TLMgR0:
	/*
              L             M             R
           |-----][-------------]------[-----|
           N      i             up     j     M
	*/
	j--;
	x = A[j];
	if ( minr <= x ) goto TLMgR0;
	if ( x <= maxl ) {
	  A[j] = A[M];
	  hole = M;
	  goto TLMgRL;
	}
	// x -> M
	if ( up < j ) {
	  A[j] = A[M];
	  hole = M;
	  goto TLMgRM;
	}
	// right gap closed also ...
	// printf("Finish 1970\n");
	goto Finish;

 TLgMgRR:
         /*   L                           R
	   |o----]-------[----]--------[-----|
	   N     i       lw  up        j     M
	   x -> R
	 */
	j--;
	y = A[j];
	if ( minr <= y ) goto TLgMgRR;
	if ( y <= maxl ) {
	  A[j] = x;
	  x = y;
	  goto TLgMgRL;
	}
	// y -> M
	if ( up < j ) {
	  A[j] = x;
	  x = y;
	  goto TLgMgRMright;
	}
	// right gap closed
         /*   L                           R
	   |o----]-------[----][-------------|
	   N     i       lw   j              M
	   x -> R
	 */
	goto TLgMRR;

	// dead code :::::
	// TLgMgRMleft:
         /*   L                           R
	   |--o--]-------[----]--------[-----|
	   N     i       lw  up        j     M
	   x -> M
	 */
	/*
	lw--;
	y = A[lw];
	if ( minr <= y ) {
	  A[lw] = x;
	  x = y;
	  goto TLgMgRR;
	}
	if ( maxl < y ) goto TLgMgRMleft;
	// y -> L
	if ( i < lw ) {
	  A[lw] = x;
	  x = y;
	  goto TLgMgRL;
	}
	// left gap closed
	A[hole] = y;
	goto TLMgR0;
	*/

 TLgMgRMright:
         /*   L                           R
	   |o----]-------[----]--------[-----|
	   N     i       lw  up        j     M
	   x -> M
	 */
	up++;
	y = A[up];
	if ( y <= maxl ) {
	  A[up] = x;
	  x = y;
	  goto TLgMgRL;
	}
	if ( y < minr ) goto TLgMgRMright;
	// y -> R
	if ( up < j ) {
	  A[up] = x;
	  x = y;
	  goto TLgMgRR;
	}
	// right gap closed
	j--;
         /*   L                           R
	   |o----]-------[------------][-----|
	   N     i       lw           j      M
	   x -> M
	 */
	goto TLgMRM;

 TLMgRL:
	/*    L             M             R
           |-----][-------------]------[----o|
           N      i             up     j     M
	   x -> L
	*/
	up++;
	y = A[up];
	if ( y <= maxl ) {
	  A[up] = A[i];
	  A[i] = y;
	  i++;
	  goto TLMgRL;
	}
	if ( y < minr ) goto TLMgRL;
	// y -> R
	if ( j <= up) {
	  A[hole] = A[j];
	  A[j] = A[i];
	  A[i] = x;
	  i++;
	  // printf("Finish 2078\n");
	  goto Finish;
	}
	// up < j
	A[up] = A[i];
	A[i] = x;
	i++;
	x = y;
	/*    L             M             R
           |-----][-------------]------[----o|
           N      i            up      j     M
	   x -> R
	*/
	goto TLMgRR;

 TLMgRM:
	/*    L             M             R
           |-----][-------------]------[---o|
           N      i            up      j    M
	   x -> M
	*/
	up++;
	y = A[up];
	if ( y <= maxl ) {
	  A[up] = A[i];
	  A[i] = y;
	  i++;
	  goto TLMgRM;
	}
	if ( y < minr ) goto TLMgRM;
	// y -> R
	if ( up < j ) {
	  A[up] = x;
	  x = y;
	  goto TLMgRR;
	}
	// right gap closed
	// x -> M & y -> R
	A[hole] = y;
	A[up] = x;
	// printf("Finish 2118\n"); 
	goto Finish;

 TLgMRR:
         /*   L                           R
	   |o----]-------[----][-------------|
	   N     i       lw   j              M
	   x -> R
	 */
	lw--;
	y = A[lw];
	if ( minr <= y ) {
	  A[lw] = A[j];
	  A[j] = y;
	  j--;
	  goto TLgMRR;
	}
	if ( maxl < y ) goto TLgMRR;
	// y -> L
	if ( i == lw ) {
	  A[hole] = A[i];
	  A[i] = A[j];
	  A[j] = x;
	  j--;
	  // printf("Finish 2142\n");
	  goto Finish;
	}
	// i < lw
	A[lw] = A[j];
	A[j] = x;
	j--;
	x = y;
         /*   L                           R
	   |o----]-------[------------][-----|
	   N     i       lw           j      M
	   x -> L
	 */
	goto TLgMRL;

 TLgMRM:
         /*   L                           R
	   |o----]-------[------------][-----|
	   N     i       lw           j      M
	   x -> M
	 */
	lw--;
	y = A[lw];
	if ( minr <= y ) {
	  A[lw] = A[j];
	  A[j] = y;
	  j--;
	  goto TLgMRM;
	}
	if ( maxl < y ) goto TLgMRM;
	// y -> L
	if ( i == lw ) {
	  A[hole] = A[i];
	  A[i] = x;
	  // printf("Finish 2176\n");
	  goto Finish;
	}
	// i < lw
	A[lw] = x;
	x = y;
	// goto TLgMRL; fall through

 TLgMRL:
         /*   L                           R
	   |o----]-------[------------][-----|
	   N     i       lw           j      M
	   x -> L
	 */
	i++;
	y = A[i];
	if ( y <= maxl ) goto TLgMRL;
	if ( minr <= y ) {
	  A[i] = x;
	  x = y;
	  goto TLgMRR;
	}
	// y -> M
	if ( i == lw ) {
	  A[hole] = x;
	  // printf("Finish 2201\n");
	  goto Finish;
	}
	A[i] = x;
	x = y;
	goto TLgMRM;

 TLMgRR:
	/*    L             M             R
           |-----][-------------]------[----o|
           N      i             up     j     M
	   x -> R
	*/
	j--;
	y = A[j];
	if ( minr <= y ) goto TLMgRR;
	if ( y <= maxl ) {
	  A[j] = x;
	  x = y;
	  goto TLMgRL;
	}
	// y -> M
	if ( up == j ) {
	  A[hole] = x;
	  // printf("Finish 2225\n");
	  goto Finish;
	}
	// up < j
	A[j] = x;
	x = y;
	goto TLMgRM;

	int lleft, lmiddle, lright;
	// int k;
 Finish:
	/*
              L             M             R
           |-----][------------------][-----|
           N      i                  j      M
	*/
	/*
	for  (k = N; k < i; k++) 
	  if ( maxl < A[k] ) {
	    printf("%s %d %s %d %s", "L error N: ", N, " M: ", M, "\n");
	    printf("%s %d %s %d %s", "L error N: ", N, " i: ", i, "\n");
	    printf("%s %d %s %d %s", "L error k: ", k, " k: ", k, "\n");
	    printf("%s %d %s %d %s", "L error maxl: ", maxl, " minr: ", minr, "\n");
	  }
	for  (k = i; k <= j; k++) 
	  if ( A[k] <= maxl || minr <= A[k] ) {
	    printf("%s %d %s %d %s", "M error N: ", N, " M: ", M, "\n");
	    printf("%s %d %s %d %s", "M error i: ", i, " j: ", j, "\n");
	    printf("%s %d %s %d %s", "M error k: ", k, " k: ", k, "\n");
	    printf("%s %d %s %d %s", "M error maxl: ", maxl, " minr: ", minr, "\n");
	  }
	for  (k = j+1; k <= M; k++) 
	  if ( A[k] < minr ) {
	    printf("%s %d %s %d %s", "R error N: ", N, " M: ", M, "\n");
	    printf("%s %d %s %d %s", "R error j: ", j, " M: ", M, "\n");
	    printf("%s %d %s %d %s", "R error k: ", k, " k: ", k, "\n");
	    printf("%s %d %s %d %s", "R error maxl: ", maxl, " minr: ", minr, "\n");
	  }
	*/

	 lleft = i - N;
         lmiddle = j - i;
	 lright = M - j;
	 if ( lleft < lmiddle ) {      // L < M
	   if ( lleft < lright ) {     // L < R
	     cut3y(A, N, i-1);
	     if ( lmiddle < lright ) { // L < M < R
	       cut3y(A, i, j);
	       cut3y(A, j+1, M);
	     } else {                  // L < R < M     
	       cut3y(A, j+1, M);
	       cut3y(A, i, j);
	     }
	     return;
	   } else {                    // R < L < M 
	     cut3y(A, j+1, M);
	     cut3y(A, N, i-1);
	     cut3y(A, i, j);
	     return;
	   }
	 }                             // M < L
	 if ( lmiddle < lright ) {     // M < R
	   cut3y(A, i, j);
	   if ( lleft < lright) {
	     cut3y(A, N, i-1);
	     cut3y(A, j+1, M);
	   } else {
	     cut3y(A, j+1, M);
	     cut3y(A, N, i-1);
	   }
	   return;
	 }                             // R < M < L
	 cut3y(A, j+1, M);
	 cut3y(A, i, j);
	 cut3y(A, N, i-1);

} // end cut3y

// Yet another three partition sorter 
// DEPRECATED
void cut3z(int *A, int N, int M) {
  // printf("cut3z %d %d \n", N, M);
  int L = M - N;
  int depthLimit = 2.5 * floor(log(L));
  if ( L < cut3Limit ) {
    cut2c(A, N, M, depthLimit); 
    return; 
  }
  cut3zc(A, N, M, depthLimit);
}

#define Mcheck(STR) \
   for (k = lw+1; k < up; k++) \
      if ( A[k] <= maxl || minr <= A[k]) { \
	printf(STR); \
	printf("@ k %d i %d j %d \n", k,i,j); \
	exit(0); \
      } \

#define McheckL(STR) \
   for (k = i+1; k < up; k++) \
      if ( A[k] <= maxl || minr <= A[k]) { \
	printf(STR); \
	printf(" @ k %d i %d j %d \n", k,i,j); \
	exit(0); \
      } \

#define McheckR(STR) \
   for (k = lw+1; k < j; k++) \
      if ( A[k] <= maxl || minr <= A[k]) { \
	printf(STR); \
	printf(" @ k %d i %d j %d \n", k,i,j); \
	exit(0); \
      } \

#define Lcheck(STR) \
  for (k = N; k <= i; k++) \
    if ( maxl < A[k] ) { \
      printf(STR); \
      printf(" @ N %d k %d i %d lw %d up %d j %d M %d\n", N,k,i,lw,up,j,M); \
      exit(0); \
    } \

#define Rcheck(STR) \
  for (k = j; k <= M; k++) \
    if ( A[k] < minr ) { \
      printf(STR); \
      printf(" @ k %d\n", k); \
      exit(0); \
    } \

void cut3zc(int *A, int N, int M, int depthLimit) {
  // printf("cut3zc %d %d %d \n", N, M, depthLimit);
  if ( depthLimit <= 0 ) {
    heapc(A, N, M);
    return;
  }
  depthLimit--;

  int L = M - N;
  if ( L < cut3Limit ) {
    cut2c(A, N, M, depthLimit);
    return; 
  }  

   /*   L            M            R
    |-----]------+[----]+------[-----|
    N     i     lw      up     j     M
   */

  int i, j, lw, up; // indices
  int maxl, minr;   // pivots for left/ right boundaries

  /* There are invariants to be maintained (and >essential< 
     for machine assisted correctness proofs):
     If there is one gap:
        L     ML             MR     R
    |-----]-------]------[------[-----|
    N     i      lw      up     j     M

       N <= k <= i --> A[k] <= maxl 
       lw < k < lw --> maxl < A[k] < minr
       j <= k <= M --> minr <= A[k]
  */
  int x, l, r, m; // values
  // int k; // for loop index as needed

  
  // Check for duplicates
        int sixth = (L + 1) / 6;
        int e1 = N  + sixth;
        int e5 = M - sixth;
	int e3 = N + (L>>1); // N + L/2;// The midpoint
        int e4 = e3 + sixth;
        int e2 = e3 - sixth;

        // Sort these elements using a 5-element sorting network
        int ae1 = A[e1], ae2 = A[e2], ae3 = A[e3], ae4 = A[e4], ae5 = A[e5];
	int t;

        if (ae1 > ae2) { t = ae1; ae1 = ae2; ae2 = t; }
        if (ae4 > ae5) { t = ae4; ae4 = ae5; ae5 = t; }
        if (ae1 > ae3) { t = ae1; ae1 = ae3; ae3 = t; }
        if (ae2 > ae3) { t = ae2; ae2 = ae3; ae3 = t; }
        if (ae1 > ae4) { t = ae1; ae1 = ae4; ae4 = t; }
        if (ae3 > ae4) { t = ae3; ae3 = ae4; ae4 = t; }
        if (ae2 > ae5) { t = ae2; ae2 = ae5; ae5 = t; }
        if (ae2 > ae3) { t = ae2; ae2 = ae3; ae3 = t; }
        if (ae4 > ae5) { t = ae4; ae4 = ae5; ae5 = t; }

        A[e1] = ae1; A[e2] = ae2; A[e3] = ae3; A[e4] = ae4; A[e5] = ae5;
	// Fix end points
	if ( ae1 < A[N] ) iswap(N, e1, A);
	if ( A[M] < ae5 ) iswap(M, e5, A);

	/*
	int duplicate = -1;
	if ( ae1 == ae5 ) { duplicate = e1; } else
	if ( ae1 == ae4 ) { duplicate = e1; } else
	if ( ae2 == ae5 ) { duplicate = e2; } else
	if ( ae1 == ae3 ) { duplicate = e1; } else
	if ( ae2 == ae4 ) { duplicate = e2; } else
	if ( ae3 == ae5 ) { duplicate = e3; } else
	if ( ae1 == ae2 ) { duplicate = e1; } else
	if ( ae2 == ae3 ) { duplicate = e2; } else
	if ( ae3 == ae4 ) { duplicate = e3; } else
	if ( ae4 == ae5 ) { duplicate = e4; };

	if ( 0 <= duplicate ) {
	  // int depthLimit = 2.5 * floor(log(L));
	  int cut2c();
	  cut2x2(A, N, M, duplicate, cut2c, depthLimit);
	  return;
	}
	*/

	// pivots
	maxl = ae2;
	minr = ae4;

  // Ready to roll ...
  i = N;
  j = M;

 left:
   /*  L                          R
    |----]---------------------[-----|
    N    i                     j     M
   */
  i++;
  x = A[i];
  if ( x <= maxl ) goto left;
  if ( minr <= x ) {
    r = x;
    goto right;
  }
  m = x;
  goto createMiddle;

 right:
   /*  L                          R
    |----]o--------------------[-----|
    N     i                    j     M
    r
   */
  j--;
  x = A[j];
  if ( minr <= x ) 
    // there is a middle value somewhere thus:
    goto right;
  if ( x <= maxl ) {
    A[i] = x; A[j] = r;
    goto left;
  }
  m = x; A[j] = r; A[i] = m;
  // goto createMiddle

 createMiddle:
   /*  L                          R
    |----]o--------------------[-----|
    N     i                    j     M
    m
   */
  if ( i+1 == j ) {
    i--;
    if ( i-N < M-j ) {
      cut3zc(A, N, i, depthLimit);
      cut3zc(A, j, M, depthLimit);
      return;
    }
    cut3zc(A, j, M, depthLimit);
    cut3zc(A, N, i, depthLimit);
    return;
  }
  lw = (i+j)/2; up = lw+1;
  x = A[lw];
   /*  L                          R
    |----]o----+[o]+-----------[-----|
    N     i     lw up          j     M
    m
   */
  if ( x <= maxl ) { // x -> L
    A[i] = x; A[lw] = m; lw--;
    goto SLgMgR0;
  }
  if ( minr <= x ) { // x -> R
    A[lw] = m; lw--;
    r = x; A[i] = r;
    goto SLgMgRr;
  }
  // x -> M  and i < lw !
  lw--;
  if ( i == lw ) { // left gap closed
    i--;
    // Mcheck("createMiddle")
    goto SLMgR0;
  }
  // i < lw
  goto SLgMgRm;
  
 SLgMgR0:
   /*  L        M                R
    |----]----+[-]+-----------[-----|
    N    i   lw   up          j     M
   */
  // Lcheck("SLgMgR0")
  // Rcheck("SLgMgR0x")
  // Mcheck("SLgMgR0y")
  i++;
  x = A[i];
  if ( x <= maxl ) // x -> L
    goto SLgMgR0;
  if ( minr <= x ) { // x -> R
    r = x;
    goto SLgMgRr;
  }
  // x -> M
  if ( lw < i) { 
    i--;
    // Mcheck("SLgMgR0z")
    goto SLMgR0;
  }
  // i <= lw
  m = x;
  goto SLgMgRm;

 SLgMgRr:
  /*  L        M                R
    |---]o---+[-]+-----------[-----|
    N    i  lw   up          j     M
    r
   */
  // Rcheck("SLgMgRr:")
  // Mcheck("SLgMgRr:2")
  j--;
  x = A[j];
  if ( minr <= x ) // x -> R
    goto SLgMgRr;
  if ( x <= maxl ) { // x -> L
    A[i] = x; A[j] = r;
    // Rcheck("SLgMgRr3")
    goto SLgMgR0;
  }
  // x -> M
  if ( j < up ) {
    j++;
    // Rcheck("SLgMgRr4")
    goto SLgMRr;
  }
  // up <= j
  A[j] = r;
  A[i] = m = x;
  // goto SLgMgRm;

 SLgMgRm:
  /*  L        M                R
    |---]o---+[-]+-----------[-----|
    N    i  lw   up          j     M
    m
   */
  // Mcheck("SLgMgRm:")
  x = A[up];
  if ( x <= maxl ) { // x -> L
    A[i] = x; A[up] = m; up++;
    // Rcheck("SLgMgRm1")
    goto SLgMgR0;
  }
  if ( x < minr ) { // x -> M
    up++;
    goto SLgMgRm;
  }
  // x -> R
  if ( up < j ) {
    A[up] = m; up++;
    A[i] = r = x;
    goto SLgMgRr;
  }
  // up = j / right gap closed
  goto SLgMRm;

 SLMgR0:
   /*  L        M                R
    |---------][-]+-----------[-----|
    N         i   up          j     M
   */
  // Lcheck("SLMgR0:")
  // McheckL("SLMgR0:2")
  // Rcheck("SLMgR0:3")
  j--;
  x = A[j];
  if ( minr <= x ) { // x -> R
    // McheckL("SLMgR04")
    goto SLMgR0;
  }
  if ( x <= maxl ) { // x -> L
    l = x;
    // McheckL("SLMgR05")
    goto SLMgRl;
  }
  // x -> M
  if ( j < up ) {
    j++;
    // McheckL("SLMgR06")
    goto Finish;
  }
  // up <= j
  m = x;
  goto SLMgRm;

 SLgMRr:
  /*  L              M          R
    |---]o---+[-------------][-----|
    N    i  lw               j     M
    r
   */
  // Rcheck("SLgMRr:")
  x = A[lw];
  if ( x <= maxl ) { // x -> L
    j--;
    A[lw] = A[j]; A[j] = r;
    A[i] = x; 
    lw--;
    // Lcheck("SLgMRr2")
    // Rcheck("SLgMRr3")
    goto SLgMR0;    
  } 
  if ( minr <= x ) { // x -> R
    j--;
    if ( i == lw ) {
      A[i] = A[j]; A[j] = r;
      i--;
      goto Finish;
    }
    A[lw] = A[j]; A[j] = x; 
    lw--;
    goto SLgMRr;
  }
  // x -> M
  lw--;
  goto SLgMRr;
 
 SLgMR0:
  /*  L              M          R
    |---]----+[-------------][-----|
    N   i   lw               j     M
   */
  // Lcheck("SLgMR0")
  // Rcheck("SLgMR0A")
  // McheckR("SLgMR0B")
  i++;
  x = A[i];
  if ( x <= maxl ) { // x -> L
    // Lcheck("SLgMR0C")
    goto SLgMR0;
  }
  if ( minr <= x ) { // x -> R
    r = x;
    // Lcheck("SLgMR0D")
    goto SLgMRr;
  }
  // x -> M
  if ( i <= lw ) {
    m = x;
    goto SLgMRm;
  }
  // left gap closed
  i--;
  goto Finish;
  
 SLgMRm:
  /*  L              M          R
    |---]o---+[-------------][-----|
    N    i  lw               j     M
    m
   */
  /*
  for (k = N; k < i; k++) 
    if ( maxl < A[k] ) {
      printf("Er SLgMRmx @ k %d\n", k);
      exit(0);
    }
  */
  x = A[lw];
  if ( x <= maxl ) { // x -> L
    A[i] = x;
    A[lw] = m; lw--;
    // Lcheck("SLgMRm2")
    // Rcheck("SLgMRm3")
    goto SLgMR0;
  }
  if ( minr <= x ) { // x -> R
    j--; A[lw] = A[j];
    A[j] = x; lw--;
    goto SLgMRm;
  }
  // x -> M
  if ( i < lw ) {
    lw--;
    goto SLgMRm;
  }
  // i == lw
  i--;
  goto Finish;

 SLMgRl:
   /*  L        M                R
    |---------][-]+-----------o[-----|
    N         i   up          j      M
    l
   */
  // McheckL("SLMgRl:")
  x = A[up];
  if ( minr <= x ) { // x -> R
    i++; A[up] = A[i]; up++;
    A[i] = l; A[j] = x;
    // McheckL("SLMgRl2")
    goto SLMgR0;
  }
  if ( maxl < x ) { // x -> M
    up++;
    // McheckL("SLMgRl3")
    goto SLMgRl;
  }
  // x -> L
  if ( up < j ) {
    i++; A[up] = A[i]; up++;
    A[i] = x;
    // McheckL("SLMgRl4")
    goto SLMgRl;
  }
  // up == j
  i++; A[up] = A[i];
  A[i] = l;
  j++;
  goto Finish;

SLMgRm:
   /*  L        M                R
    |---------][-]+-----------o[-----|
    N         i   up          j      M
    m
   */
  x = A[up];
  if ( x <= maxl ) { // x -> L
    i++;
    A[up] = A[i]; A[i] = x; up++;
    goto SLMgRm;
  }
  if ( minr <= x ) { // x -> R
    A[j] = x; A[up] = m; up++;
    // McheckL("SLMgRm2")
    goto SLMgR0;
  }
  // x -> M
  if ( up < j ) {
    up++;
    goto SLMgRm;
  }
  j++;
 
  // Rcheck("SLMgRm3")
  // goto Finish;

 Finish:
   /*  L              M        R
    |---------][-----------][-----|
    N         i             j     M
   */
  // printf("Finish N %d i %d j %d M %d depthLimit %d\n", N,i,j,M,depthLimit);
  
  // Lcheck("FinishL)
  /*
  for (k = i+1; k < j; k++) 
    if ( A[k] <= maxl || minr <= A[k]) {
      printf("ErM @ k %d\n", k);
      exit(0);
    }
  */
  // Rcheck("FinishR)
  if (i-N < j-i) {
    cut3zc(A, N, i, depthLimit);
    if (i-j < M-j) {
      cut3zc(A, i+1, j-1, depthLimit);
      cut3zc(A, j, M, depthLimit);
      return;
    }
    cut3zc(A, j, M, depthLimit);
    cut3zc(A, i+1, j-1, depthLimit);
    return;
  }
  cut3zc(A, i+1, j-1, depthLimit);
  if (i-N <  M-j) {
    cut3zc(A, N, i, depthLimit);
    cut3zc(A, j, M, depthLimit);
    return;
  }
  cut3zc(A, j, M, depthLimit);
  cut3zc(A, N, i, depthLimit);
} // end cut3zc

// A four partition sorter - very compact 
// Twice movements of middle segments elements slow things down.
// DEPRECATED
void cut4s(int *A, int N, int M)
{
  // printf("cut4s %d %d \n", N, M);
  int L = M - N; 
  if ( L < cut4Limit ) {
    cut2(A, N, M);
    return; 
  }
  int depthLimit = 2.5 * floor(log(L));
  cut4sc(A, N, M, depthLimit);
} // end cut4s

void cut4sc(int *A, int N, int M, int depthLimit) 
{
  // printf("cut4sc %d %d %d \n", N, M, depthLimit);
  if ( depthLimit <= 0 ) {
    heapc(A, N, M);
    return;
  }
  int L = M - N; 

  if ( L < cut4Limit ) {
    cut2c(A, N, M, depthLimit);
    return; 
  }
  depthLimit--;

  int maxl, middle, minr;   // pivots for left/ middle / right regions

  int probeLng = L/ probeParamCut3x;
  if ( probeLng < 20 ) probeLng = 20; // quite short indeed
  int halfSegmentLng = probeLng/2;
  int N1 = N + L/2 - halfSegmentLng;
  int M1 = N1 + probeLng - 1;
  int quartSegmentLng = probeLng/4;
  int maxlx = N1 + quartSegmentLng;
  int middlex = N1 + halfSegmentLng;
  int minrx = M1 - quartSegmentLng;
  int offset = L/probeLng;

  register int k;
  for (k = 0; k < probeLng; k++) iswap(N1 + k, N + k * offset, A);
  cut2(A, N1, M1);
  
  // int first = A[N1];
  maxl = A[maxlx];
  middle = A[middlex];
  minr = A[minrx];
  // int last = A[M1];

  /*
  // test here for duplicates and go to cut2x2 when dups found
  int dupx = -1; // candidate index to dups
  if ( first == last ) dupx = N1; else
  if ( middle == first || middle == last ||
       middle == maxl || middle == minr ) dupx = middlex; else
  if ( first == maxl ) dupx = N1; else
  if ( last == minr ) dupx = M1;
  if ( 0 <= dupx ) {
    int cut4sc();
    dflgm(A, N, M, dupx, cut4sc, depthLimit);
    return;
  }
  */
  if ( A[N1] < A[N] ) iswap(N1, N, A);
  if ( A[M] < A[M1] ) iswap(M1, M, A);

  // check to play safe
  if ( maxl < A[N] || A[M] < minr ||
       middle <= maxl || minr <= middle ) {
    cut2c(A, N, M, depthLimit);
    return;
  }

   /*   L     ML                      MR     R
    |-----]------]------------------[-----[-----|
    N     i       k                up     j     M
   */

  register int i, j, up; // indices
  
  /* There are invariants to be maintained (which are >essential< 
     for machine assisted correctness proofs):
     maxl < middle < minr
     If there are two gaps:
       N <= x < i  --> A[x] <= maxl
       i <= x <    --> maxl < A[x] <= middle
       up < x <= j --> middle < A[x] < minr
       j < x <= M  --> minr <= A[x]
  */
  register int ak, x; // values
  
  // int z; // for tracing
  // Ready to roll ...
  i = N; j = M; up = j-1;
  // printf("%i  %i  %i\n--\n", maxl, middle, minr);

  // for ( z = N; z < N+8; z++) printf("%i ", A[z]); printf("\n----\n");

  for ( k = i+1; k < up; k++ ) {
    ak = A[k];
    if ( ak <= middle ) {
      if ( maxl < ak ) continue;      
      A[k] = A[++i]; A[i] = ak; continue;
    }
    // middle < ak -> rightside
  repeatRight:
    x = A[up];
    if ( x <= middle ) {
      if ( x <= maxl ) {
	A[k] = A[++i]; A[i] = x;
	if ( ak < minr ) { A[up--] = ak; continue; }
	// minr <= ak
	A[up--] = A[--j]; A[j] = ak; continue;	
      }
      // maxl < x 
      A[k] = x;
      if ( ak < minr ) { A[up--] = ak; continue; }
      // minr <= ak
      A[up--] = A[--j]; A[j] = ak; continue;	
    }
    // middle < x
    if ( minr <= x ) {
      A[up--] = A[--j]; A[j] = x;
    } else up--;
    if ( k < up ) goto repeatRight;
    // printf("break %i   %i\n", k, up);
    break;
  }
  // printf("%i   %i\n", k, up);
  if ( k == up ) {
    ak = A[k];
    if ( ak <= middle ) {
      if ( ak <= maxl ) {
	A[k] = A[++i]; A[i] = ak;
      }
    } else { // middle < ak
      if ( minr <= ak ) {
	A[k] = A[--j]; A[j] = ak;
	k--;
      } else k--;
    }
  } else k--;

   /*   L     ML                      MR     R
    |-----]-------------][----------------[-----|
    N     i             k                 j     M
   */

  // printf("exit   N %i i %i k %i up %i j %i M %i\n", N,i,k,up,j,M);
  /*
  for ( z = N; z <= i; z++ ) 
    if ( maxl < A[z] ) {
      printf("** Er L @ z %i   %i\n", z, A[z]);
      printf("N %i i %i k %i up %i j %i M %i\n", N,i,k,up,j,M);
      exit(0);
    }
 for ( z = i+1; z <= k; z++ ) 
    if ( A[z] <= maxl || middle < A[z] ) {
      printf("** Er ML @ z %i\n", z);
      printf("N %i i %i k %i up %i j %i M %i\n", N,i,k,up,j,M);
      exit(0);
    }

 for ( z = k+1; z < j; z++ ) 
    if ( A[z] <= middle || minr <= A[z] ) {
      printf("** Er MR @ z %i\n", z);
      printf("N %i i %i k %i up %i j %i M %i\n", N,i,k,up,j,M);
      exit(0);
    }

 for ( z = j; z <= M; z++ ) 
    if ( A[z] < minr ) {
      printf("** Er R @ z %i\n", z);
      printf("N %i i %i k %i up %i j %i M %i\n", N,i,k,up,j,M);
      exit(0);
    }
  */
  if ( k-N < M-k ) {
    if ( i-N < k-i ) {
      cut4sc(A, N, i, depthLimit);
      cut4sc(A, i+1, k, depthLimit);
    } else {
      cut4sc(A, i+1, k, depthLimit);
      cut4sc(A, N, i, depthLimit);
    }
    if ( j-k < M-j ) {
      cut4sc(A, k+1, j-1, depthLimit);
      cut4sc(A, j, M, depthLimit);
    } else {
      cut4sc(A, j, M, depthLimit);
      cut4sc(A, k+1, j-1, depthLimit);
    }
  } else {
    if ( j-k < M-j ) {
      cut4sc(A, k+1, j-1, depthLimit);
      cut4sc(A, j, M, depthLimit);
    } else {
      cut4sc(A, j, M, depthLimit);
      cut4sc(A, k+1, j-1, depthLimit);
    }
    if ( i-N < k-i ) {
      cut4sc(A, N, i, depthLimit);
      cut4sc(A, i+1, k, depthLimit);
    } else {
      cut4sc(A, i+1, k, depthLimit);
      cut4sc(A, N, i, depthLimit);
    }
  }
  // Finished
} // end cut4sc

// Another 4-partition sorter, still not as good as cut4 ...
// DEPRECATED

void cut4f(int *A, int N, int M)
{
  // printf("cut4f %d %d \n", N, M);
  int L = M - N; 
  if ( L < cut4Limit ) {
    cut2(A, N, M);
    return; 
  }
  int depthLimit = 2.5 * floor(log(L));
  cut4fc(A, N, M, depthLimit);
} // end cut4f

void cut4fc(int *A, int N, int M, int depthLimit) 
{
  // printf("cut4fc %d %d %d \n", N, M, depthLimit);
  if ( depthLimit <= 0 ) {
    heapc(A, N, M);
    return;
  }
  int L = M - N; 

  if ( L < cut4Limit ) {
    cut2c(A, N, M, depthLimit);
    return; 
  }
  depthLimit--;

  int maxl, middle, minr;   // pivots for left/ middle / right regions

  int probeLng = L/ probeParamCut3x;
  if ( probeLng < 20 ) probeLng = 20; // quite short indeed
  int halfSegmentLng = probeLng/2;
  int N1 = N + L/2 - halfSegmentLng;
  int M1 = N1 + probeLng - 1;
  int quartSegmentLng = probeLng/4;
  int maxlx = N1 + quartSegmentLng;
  int middlex = N1 + halfSegmentLng;
  int minrx = M1 - quartSegmentLng;
  int offset = L/probeLng;

  int k;
  for (k = 0; k < probeLng; k++) iswap(N1 + k, N + k * offset, A);
  cut2(A, N1, M1);

  // Fix end points
  if ( A[N1] < A[N] ) iswap(N1, N, A);
  if ( A[M] < A[M1] ) iswap(M1, M, A);

  // int first = A[N1];
  maxl = A[maxlx];
  middle = A[middlex];
  minr = A[minrx];
  // int last = A[M1];

  // check to play safe
  if ( maxl < A[N] || A[M] < minr ||
       middle <= maxl || minr <= middle ) 
    { cut2c(A, N, M, depthLimit);
      return;
    }

   /*   L             ML         MR             R
    |-----]------+[---------]---------]+-----[-----|
    N     i      lw         z         up     j     M
   */

  register int i, j, lw, up, z; // indices
  
  /* There are invariants to be maintained (which are >essential< 
     for machine assisted correctness proofs):
     maxl < middle < minr
     If there are two gaps:
       N <= x <= i --> A[x] <= maxl
       lw < x <= z  --> maxl < A[x] <= middle
       z < x < up  --> middle < A[x] < minr
       j <= x <= M --> minr <= A[x]
     If the left gap has closed:
       N <= x < i --> A[x] <= maxl
       i <= x <= z --> maxl < A[x] <= middle
       z <  x < up  --> middle < A[x] < minr
       j <= x <= M --> minr <= A[x]
     If the right gap has closed:
       N <= x <= i --> A[x] <= maxl
       lw < x <= z  --> maxl < A[x] <= middle
       z < x <= j  --> middle < A[x] < minr
       j < x <= M --> minr <= A[x]
  */

  register int ai, al, au, aj;
  
  // Ready to roll ...
  i = N; j = M; z = middlex; lw = z-1; up = z+1;
  // MR is empty at the start

 left:

   /*   L             ML         MR             R
    |-----]------+[---------]---------]+-----[-----|
    N     i      lw         z         up     j     M
   */

  if ( lw < i ) {

    goto Kmg;
  }

  ai = A[++i];
  if ( ai <= middle ) {
    if ( ai <= maxl ) { goto left; }
    goto KgmgaiML;
  }
  // middle < ai
  if ( ai < minr ) { goto KgmgaiMR; }
  goto KgmgaiR;

 KgmgaiML:
  if ( lw < i ) { i--; goto Kmg; }
  // i <= lw
  al = A[lw];
  if ( al <= middle ) {
    if ( al <= maxl ) { A[i] = al; A[lw--] = ai; goto left; }
    if ( i == lw ) { i--; goto Kmg; }
    lw--; goto KgmgaiML;
  }
  // middle < al
  if ( al < minr ) { goto KgmgaiMLalMR; }
  goto KgmgaiMLalR;

KgmgaiMR:
  if ( j < up ) { i--; goto Kgm; }
  // up <= j
  au = A[up];
  if ( au <= middle ) {
    if ( au <= maxl ) { A[i] = au; A[up++] = ai; goto left; }
  goto KgmgaiMRauML;
  }
  // middle < au
  if ( au < minr ) { up++; goto KgmgaiMR; }
  if ( up == j ) { i--; goto Kgm; }
  goto KgmgaiMRauR;

KgmgaiR:
   /*   L             ML         MR             R
    |-----]o------+[---------]---------]+-----[-----|
    N      i      lw         z         up     j     M
    ai -> R
   */
  if ( j < up ) { i--; goto Kgm; } 
  aj = A[--j];
  if ( aj <= middle ) {
    if ( aj <= maxl ) { A[i] = aj; A[j] = ai; goto left; }
    goto KgmgaiRajML;
  }
  // middle < aj
  // up <= j
  if ( aj < minr ) {
    if ( up == j ) { j++; i--; goto Kgm; }
    goto KgmgaiRajMR;
  }
  goto KgmgaiR;

KgmgaiMLalMR:
   /*   L             ML         MR             R
    |-----]o------o[---------]---------]+-----[-----|
    N      i      lw         z         up     j     M
    ai -> ML al -> MR
   */
  if ( j < up ) { i--; goto Kgm; }
  au = A[up];
  if ( au <= middle ) {
    if ( au <= maxl ) { A[i] = au; A[lw--] = ai; A[up++] = al; goto left; }
    A[lw--] = au; A[up++] = al; goto KgmgaiML;
  }
  // middle < au 
  // up <= j
  if ( au < minr ) { up++; goto KgmgaiMLalMR; }
  if ( up == j ) { i--; goto Kgm; }
  goto KgmgaiMLalMRauR;

KgmgaiMLalR:
   /*   L             ML         MR             R
    |-----]o------o[---------]---------]+-----[-----|
    N      i      lw         z         up     j     M
    ai -> ML al -> R
   */
  if ( j < up ) { i--; goto Kgm; }
  aj = A[--j];
  if ( aj <= middle ) {
    if ( aj <= maxl ) {
      A[i] = aj; A[lw--] = ai; A[j] = al; goto left;
    }
    A[lw--] = aj; A[j] = al; goto KgmgaiML;
  }
  // middle < aj
  // up <= j
  if ( minr <= aj ) goto KgmgaiMLalR;
  if ( j < up ) { i--; j++; goto Kgm; }
  goto KgmgaiMLalRajMR;

 KgmgaiMRauML:
   /*   L             ML         MR             R
    |-----]o------+[---------]---------]o-----[-----|
    N      i      lw         z         up     j     M
    ai -> MR au -> ML
   */
  al = A[lw]; 
  if ( al <= middle ) {
    if ( al <= maxl ) { A[i] = al; A[lw--] = au; A[up++] = ai; goto left; }
    lw--; 
goto KgmgaiMRauML;
  }
  // middle < al
  if ( al < minr ) {
    if ( i == lw ) {
      A[i--] = au; A[up++] = ai; goto Kmg;
    }
    // i < lw;
    A[lw--] = au; A[up++] = al; goto KgmgaiMR;
  }
  goto KgmgaiMRalRauML;

 KgmgaiMRauR:
   /*   L             ML         MR             R
    |-----]o------+[---------]---------]o-----[-----|
    N      i      lw         z         up     j     M
    ai -> MR au -> R
   */
  if ( j <= up ) { i--; goto Kgm; }
  // up < j
  aj = A[--j];
  if ( aj <= middle ) {
    if ( aj <= maxl ) {
      A[i] = aj; A[up++] = ai; A[j] = au; goto left;
    }
    goto KgmgaiMRauRajML;    
  }
  // middle < aj
  if ( aj < minr ) {
    A[up++] = aj; A[j] = au; goto KgmgaiMR;
  }
  if ( up == j ) { i--; goto Kgm; }
  goto KgmgaiMRauR;

 KgmgaiRajML:
   /*   L             ML         MR             R
    |-----]o------+[---------]---------]+-----o[-----|
    N      i      lw         z         up      j     M
    ai -> R aj -> ML
   */
  al = A[lw];
  if ( al <= middle ) {
    if ( al <= maxl ) {
      A[i] = al; A[lw--] = aj; A[j] = ai; goto left;
    }
    lw--; goto KgmgaiRajML;
  }
  // middle < al
  if ( al < minr ) { goto KgmgaiRajMLalMR;
  }
  // al -> R also
  if ( i == lw ) { A[j] = ai; A[i--] = aj; goto Kmg; }
  A[lw--] = aj; A[j] = al;
goto KgmgaiR;
  
 KgmgaiRajMR:
  /*   L             ML         MR             R
    |-----]o------+[---------]---------]+-----o[-----|
    N      i      lw         z         up     j      M
    ai -> R aj -> MR
  */
  if ( j < up ) { i--; j++; goto Kgm; }
  au = A[up];
  if ( au <= middle) {
    if ( au <= maxl ) { A[i] = au; A[up++] = aj; A[j] = ai; goto left; }
    goto KgmgaiRajMRauML;
  }
  // middle < au
  if ( au < minr ) {
    if ( up == j ) { i--; j++; goto Kgm; }
    up++; goto KgmgaiRajMR;
  }
  A[j] = au; A[up++] = aj; 
  goto KgmgaiR;
 
 KgmgaiMLalMRauR:
  /*   L             ML         MR               R
    |-----]o------o[---------]---------]o-----[-----|
    N      i      lw         z         up     j     M
    ai -> ML al -> MR au -> R
  */
  if ( j <= up ) { i--; goto Kgm; }
  // up < j
  aj = A[--j];
  // up <= j
  if ( aj <= middle ) {
    if ( aj <= maxl ) { // 4-swap
      A[i] = aj; A[lw--] = ai; A[up++] = al; A[j] = au; goto left;
    }
    A[lw--] = aj; A[up++] = al; A[j] = au; goto KgmgaiML;
  }
  // middle < aj
  if ( aj < minr  ) { A[up++] = aj; A[j] = au; goto KgmgaiMLalMR; }
  if ( up == j ) { i--; goto Kgm; }
  goto KgmgaiMLalMRauR;
  
 KgmgaiMLalRajMR:
  /*   L             ML         MR               R
    |-----]o------o[---------]---------]-----o[-----|
    N      i      lw         z         up    j      M
    ai -> ML al -> R aj -> MR
  */
  if ( j < up ) { i--; j++; goto Kgm; }
  au = A[up];
  if ( au <= middle ) {
    if ( au <= maxl ) {
      A[i] = au; A[lw--] = ai; A[up++] = aj; A[j] = al; 
      goto left;
    }
    A[lw--] = au; A[up++] = aj; A[j] = al; goto KgmgaiML;
  }
  // middle < au
  if ( au < minr ) {
    if ( up == j ) { i--; j++; goto Kgm; }
    up++; goto KgmgaiMLalRajMR;
  }
  A[up++] = aj; A[j] = au; goto KgmgaiMLalR;
  
 KgmgaiMRalRauML:
  /*   L             ML         MR               R
    |-----]o------o[---------]---------]o-----[-----|
    N      i      lw         z         up     j     M
    ai -> MR al -> R au -> ML
  */
  aj = A[--j];
  if ( aj <= middle ) {
    if ( aj <= maxl ) { // aj -> L
      A[i] = aj; A[lw--] = au; A[up++] = ai; A[j] = al; 
      goto left;
    }
    // aj -> ML
    if ( up == j ) { A[lw--] = au; A[j] = al; i--; goto Kgm; }
    A[lw--] = aj; A[j] = al; 
    goto KgmgaiMRauML;
  }
  // middle < aj
  if ( aj < minr ) {
    A[lw--] = au; A[up++] = aj; A[j] = al; goto KgmgaiMR;
  }
  goto KgmgaiMRalRauML;
  
 KgmgaiMRauRajML:
  /*   L             ML         MR               R
    |-----]o-------[---------]---------]o----o[-----|
    N      i      lw         z         up    j      M
    ai -> MR au -> R aj -> ML
  */
  al = A[lw];
  if ( al <= middle ) {
    if ( al <= maxl ) {
      A[i] = al; A[lw--] = aj; A[up++] = ai; A[j] = au; goto left;
    }
    lw--; goto KgmgaiMRauRajML;
  }
  // middle < al
  if ( al < minr ) {
    if ( i == lw ) {
      A[i--] = aj; A[up++] = al; A[j] = au; goto Kmg;
    }
    A[lw--] = aj; A[up++] = al; A[j] = au; goto KgmgaiMR;
  }
  A[lw--] = aj; A[j] = al; goto KgmgaiMRauR;
  
 KgmgaiRajMLalMR:
  /*   L             ML         MR               R
    |-----]o------o[---------]---------]+----o[-----|
    N      i      lw         z         up    j      M
    ai -> R aj -> ML al -> MR
  */
  au = A[up];
  if ( au <= middle ) {
    if ( au <= maxl ) {
      A[i] = au; A[lw--] = aj; A[up++] = al; A[j] = ai; goto left;
    }
    // au -> ML
    if ( up == j ) { A[lw--] = au; A[j++] = al; i--; goto Kgm; }
    A[lw--] = au; A[up++] = al; goto KgmgaiRajML;
  }
  //  middle < au
  if ( au < minr ) {    
    up++; 
goto KgmgaiRajMLalMR;
  }
  A[lw--] = aj; A[up++] = al; A[j] = au;
  goto KgmgaiR;
  
 KgmgaiRajMRauML:
  /*   L             ML         MR               R
    |-----]o------+[---------]---------]o----o[-----|
    N      i      lw         z         up    j      M
    ai -> R aj -> MR au -> ML
  */
  al = A[lw];
  if ( al <= middle ) {
    if ( al <= maxl ) {
      A[i] = al; A[lw--] = au; A[up++] = aj; A[j] = ai; goto left;
    }
    lw--; goto KgmgaiRajMRauML;
  }
  // middle < al
  if ( al < minr ) {
    A[lw--] = au; A[up++] = al; goto KgmgaiRajMR;
  }
  // al -> R
  if ( i == lw ) { A[i--] = au; A[up++] = aj; A[j] = al; goto Kmg; }
  A[lw--] = au; A[up++] = aj; A[j] = al; 
goto KgmgaiR;
  
  // right:
   /*   L             ML         MR             R
    |-----]------+[---------]---------]+-----[-----|
    N     i      lw         z         up     j     M
   */
  // -------------------------------------------------------
  
 Kmg:
   /*   L          ML         MR             R
    |-----]--------------]---------]+-----[-----|
    N     i              z         up     j     M
   */
  if ( j <= up ) { goto Finish; }
  au = A[up];
  if ( au <= middle ) {
    if ( au <= maxl ) { A[up++] = A[++z]; A[z] = A[++i]; A[i] = au; goto Kmg; }
    A[up++] = A[++z]; A[z] = au; goto Kmg;
  }
  // middle < au
  if ( au < minr ) { up++; goto Kmg; }
  // goto KmgauR;
  
 KmgauR:
   /*   L          ML         MR             R
    |-----]--------------]---------]o-----[-----|
    N     i              z         up     j     M
    au -> R up < j
   */
  aj = A[--j];
  if ( minr <= aj ) {
    if ( up == j ) { goto Finish; }
    goto KmgauR;
  }
  // goto KmgauRX;
// KmgauRX:
  // aj < minr
  if ( middle < aj ) { A[up++] = aj; A[j] = au; goto Kmg; }
  // aj -> left
  if ( aj <= maxl ) {
    A[up++] = A[++z]; A[z] = A[++i]; A[i] = aj; A[j] = au; goto Kmg;
  }
  // aj -> ML
  A[up++] = A[++z]; A[z] = aj; A[j] = au; goto Kmg;    

 Kgm:
   /*   L             ML         MR            R
    |-----]------+[---------]--------------[-----|
    N     i      lw         z              j     M
   */
  if ( lw <= i ) { goto Finish; }
  al = A[lw];
  if ( al <= middle ) {
    if ( al <= maxl ) goto KgmalL;
    lw--; goto Kgm;
  }
  // middle < al
  if ( al < minr ) { A[lw--] = A[z]; A[z--] = al;   goto Kgm; }
  // al -> R
  A[lw--] = A[z]; A[z--] = A[--j]; A[j] = al; goto Kgm;

KgmalL:
   /*   L             ML         MR            R
    |-----]------o[---------]--------------[-----|
    N     i      lw         z              j     M
    al -> L i < lw
   */
  ai = A[++i]; 
  if ( ai <= maxl ) {
    if ( i < lw ) goto KgmalL;
    goto Finish;
  }
  // goto KgmalLX;
//KgmalLX:
  // al -> L   maxl < ai
  A[i] = al;
  if ( ai <= middle ) { // ai -> ML
    A[lw--] = ai; goto Kgm;
  }
  // middle < ai
  if ( ai < minr ) { A[lw--] = A[z]; A[z--] = ai; goto Kgm; }
  A[lw--] = A[z]; A[z--] = A[--j]; A[j] = ai; goto Kgm;


 Finish:
	// printf("cut4 exit Finish N: %d M: %d\n", N, M);
         /*   L        ML         MR         R
	   |-----][----------|-----------][-----|
	   N     i           z            j     M
	 */
/*
  for (k = N; k <= i; k++) 
    if ( maxl < A[k] ) {
      printf("ER L  at k: %i\n", k);
     printf("N %i i %i lw %i z %i up %i j %i M %i\n", N,i,lw,z,up,j,M);
      exit(0);
    }
  for (k = i+1; k <= z; k++) 
    if ( A[k] <= maxl || middle < A[k] ) {
      printf("ER ML at k: %i\n", k);
      printf("N %i i %i lw %i z %i up %i j %i M %i\n", N,i,lw,z,up,j,M);
      exit(0);
    }
  for (k = z+1; k < j; k++) 
    if ( A[k] <= middle || minr <= A[k] ) {
      printf("ER MR at k: %i\n", k);
      printf("N %i i %i lw %i z %i up %i j %i M %i\n", N,i,lw,z,up,j,M);
      exit(0);
    }
  for (k = j; k <= M; k++) 
    if ( A[k] < minr ) {
      printf("ER R  at k: %i\n", k);
      printf("N %i i %i lw %i z %i up %i j %i M %i\n", N,i,lw,z,up,j,M);
      exit(0);
    }
  // */

         /*   L        ML         MR         R
	   |-----][----------|-----------][-----|
	   N     i           z            j     M
	 */
	if ( z-N < M-z ) {
	  cut4fc(A, N, i, depthLimit);
	  cut4fc(A, i+1, z, depthLimit);
	  if ( j-z < M-j ) {
	    cut4fc(A, z+1, j-1, depthLimit);
	    cut4fc(A, j, M, depthLimit);
	    return;
	  }
	  cut4fc(A, j, M, depthLimit);
	  cut4fc(A, z+1, j-1, depthLimit);
	  return;
	}
	// M-z <= z-N
	cut4fc(A, z+1, j-1, depthLimit);
	cut4fc(A, j, M, depthLimit);
	if ( i-N < z-i ) {
	  cut4fc(A, N, i, depthLimit);
	  cut4fc(A, i+1, z, depthLimit);
	  return;
	}
	cut4fc(A, i+1, z, depthLimit);
	cut4fc(A, N, i, depthLimit);
} // end cut4fc

/*
  Here the 2009 August version 
        with extensive later changes
  of the 4-partitioning algorithm.
  Small segments are delegated to: cut2
  The activated pivots calculator uses the cut2 sorter on a
  small sample in the middle.
  The code is tricky ... use extreme care to modify.
  Tue Jan 02 15:29:56 2018  Has been fixed recently
 */

void cut4(int *A, int N, int M)
{
  // printf("cut4 %d %d \n", N, M);
  int L = M - N; 
  if ( L < cut4Limit ) {
    cut2f(A, N, M);
    return; 
  }
  int depthLimit = 2.5 * floor(log(L));
  cut4c(A, N, M, depthLimit);
} // end cut4

void cut4c(int *A, int N, int M, int depthLimit) 
{
 Start:
  // printf("cut4c %d %d %d \n", N, M, depthLimit);
  if ( depthLimit <= 0 ) {
    heapc(A, N, M);
    return;
  }
  int L = M - N; 

  if ( L < cut4Limit ) {
    cut2fc(A, N, M, depthLimit);
    return; 
  }
  depthLimit--;

  int maxl, middle, minr;   // pivots for left/ middle / right regions

  int probeLng = L/ probeParamCut3x;
  if ( probeLng < 20 ) probeLng = 20; // quite short indeed
  int halfSegmentLng = probeLng >> 1; // probeLng/2;
  int N1 = N + (L>>1) - halfSegmentLng;
  int M1 = N1 + probeLng - 1;
  int quartSegmentLng = probeLng >> 2; // probeLng/4;
  int maxlx = N1 + quartSegmentLng;
  int middlex = N1 + halfSegmentLng;
  int minrx = M1 - quartSegmentLng;
  int mrx = middlex + (quartSegmentLng>>1);
  int offset = L/probeLng;

  int k;

  for (k = 0; k < probeLng; k++) iswap(N1 + k, N + k * offset, A);
  quicksort0(A, N1, M1);
  // cut2(A, N1, M1);

  /*
  if ( compareXY(A[maxlx], A[middlex]) == 0 || 
       compareXY(A[middlex], A[mrx]) == 0 || 
       compareXY(A[mrx], A[minrx]) == 0 ) {
    // no good pivots available, thus escape
    dflgm(A, N, M, middlex, cut4c, depthLimit, compareXY);
    return;
  }
  */

  if ( A[maxlx] == A[middlex] || 
       A[middlex] == A[mrx] || 
       A[mrx] == A[minrx] ) {
    // no good pivots available, thus escape
    dflgm(A, N, M, middlex, cut4c, depthLimit);
    return;
  }

  int x, y; // values  
  int hole;

  iswap(N+1, maxlx, A); maxl = A[N+1]; // left pivot
  iswap(M, minrx, A); minr = A[M]; // right pivot
  x = A[N]; // 1st roving variable element
  middle = A[N] = A[middlex]; // middle pivot
  iswap(middlex+1, mrx, A); // init MR

  register int i, j, lw, up, z; // indices

  // Here the general layout
   /*   L             ML         MR             R
    |-----]------+[---------]---------]+-----[-----|
    N     i      lw         z         up     j     M
   */

  /* There are invariants to be maintained (which are >essential< 
     for machine assisted correctness proofs):
     maxl < middle < minr
     If there are two gaps:
       N < x <= i --> A[x] <= maxl
       lw < x <= z  --> maxl < A[x] <= middle
       z < x < up  --> middle < A[x] < minr
       j <= x <= M --> minr <= A[x]
     If the left gap has closed:
       N < x < i --> A[x] <= maxl
       i <= x <= z --> maxl < A[x] <= middle
       z <  x < up  --> middle < A[x] < minr
       j <= x <= M --> minr <= A[x]
     If the right gap has closed:
       N < x <= i --> A[x] <= maxl
       lw < x <= z  --> maxl < A[x] <= middle
       z < x <= j  --> middle < A[x] < minr
       j < x <= M --> minr <= A[x]
  */

  /* We employ again whack-a-mole. We know in which partition element x 
     should be.  Find a close, undecided position where x should go.  
     Store its content in y.  Drop x (simplified).  Set x to y and repeat. 
   */
  // Ready to roll ...
  i = N+1; j = M; z = middlex; lw = z-1; up = z+2; hole = N;

   /*   L             ML         MR             R
    o-----]------+[---------]---------]+-----[-----|
    N     i      lw         z         up     j     M
   */

  if ( x <= middle ) {
    if ( x <= maxl ) goto TLgMLMRgRL;
    goto TLgMLMRgRML;
  }
  // middle < x
  if ( x < minr ) goto TLgMLMRgRMR;
    goto TLgMLMRgRR;

 TLgMLMRgRL:
         /*   L                              R
	   |o----]---------+[-|-]+--------[-----|
	   N     i        lw     up       j     M
           x -> L             z
	 */
	i++;
	y = A[i];
	if ( y <= middle ) {
	  if ( y <= maxl ) goto TLgMLMRgRL;
	  // y -> ML
	  if ( lw < i ) {
	 /*   L                              R
	   |o----][-----------|-]+--------[-----|
	   N      i           z  up       j     M
           x -> L             
	 */
	    goto TLMLMRgRL;
	  }
	  A[i] = x;
	  x = y;
	  goto TLgMLMRgRML;
	}
	// middle < y
	A[i] = x;
	x = y;
	if ( minr <= y ) { 
	  goto TLgMLMRgRR;
	}
	goto TLgMLMRgRMR;

 TLgMLMRgRML:
         /*   L                              R
	   |o----]---------+[-|-]+--------[-----|
	   N     i        lw     up       j     M
           x -> ML            z
	 */
	y = A[lw];
	if ( y <= middle ) {
	  if ( y <= maxl ) {
	    if ( i < lw ) {
	      A[lw] = x;
	      x = y;
	      lw--;
	      goto TLgMLMRgRL;
	    }
	    // left gap closed
	    i++;
         /*   L                              R
	   |o----][-----------|-]+--------[-----|
	   N      i              up       j     M
           x -> ML            z
	 */
	    goto TLMLMRgRML;
	  }
	  // y -> ML
	  lw--;
	  goto TLgMLMRgRML;
	}
	// middle < y
	A[lw] = x;
	x = y;
	lw--;
	if ( minr <= y ) { 
	  goto TLgMLMRgRR;
	}
	goto TLgMLMRgRMR;

 TLgMLMRgRR:
         /*   L                              R
	   |o----]---------+[-|-]+--------[-----|
	   N     i        lw     up       j     M
           x -> R             z
	 */
	j--;
	y = A[j];
	if ( y <= middle ) {
	  A[j] = x;
	  x = y;
	  if ( y <= maxl ) { 
	    goto TLgMLMRgRL;
	  }
	  goto TLgMLMRgRML;
	}
	// middle < y
	if ( minr <= y ) {
	  goto TLgMLMRgRR;
	}
	// y -> MR
	if ( up <= j ) {
	  A[j] = x;
	  x = y;
	  goto TLgMLMRgRMR;
	}
	// right gap closed
         /*   L                           R
	   |o----]---------+[-|-------][-----|
	   N     i        lw          j      M
           x -> R             z
	 */
	goto TLgMLMRRR;

 TLgMLMRgRMR:
         /*   L                              R
	   |o----]---------+[-|-]+--------[-----|
	   N     i        lw     up       j     M
           x -> MR            z
	 */
	y = A[up];
	if ( y <= middle ) {
	  A[up] = x;
	  x = y;
	  up++;
	  if ( y <= maxl ) { 
	    goto TLgMLMRgRL;
	  }	  
	  goto TLgMLMRgRML;
	}
	// middle < y
	if ( y < minr ) {
	  up++;
	  goto TLgMLMRgRMR;
	}
	// y -> R
	if ( up < j ) {
	  A[up] = x;
	  x = y;
	  up++;
	  goto TLgMLMRgRR;
	}
	// right gap closed
	j--;
         /*   L                           R
	   |o----]---------+[-|-------][-----|
	   N     i        lw          j      M
           x -> MR            z
	 */
	goto TLgMLMRRMR;


	// right gap closed
 TLgMLMRRR:
         /*   L                           R
	   |o----]---------+[-|-------][-----|
	   N     i        lw          j      M
           x -> R             z
	 */
	y = A[lw];
	if ( y <= middle ) {
	  if ( y <= maxl ) {
	    if ( i < lw ) {  // shift ML&MR to the left
	      A[lw] = A[z]; A[z] = A[j]; A[j] = x;
	      x = y;
	      lw--; z--; j--; 
	      goto TLgMLMRRL;
	    }
	    // left gap also closed; shift ML&MR to the left
	    A[hole] = A[i]; A[i] = A[z]; A[z] = A[j]; A[j] = x;
	    i--; z--; 
	    goto Finish4;
	  }
	  // y -> ML
	  lw--;
	  goto TLgMLMRRR;
	}
	// middle < y
	if ( minr <= y ) { // shift ML&MR to the left
	  A[lw] = A[z]; A[z] = A[j]; A[j] = y;
	  lw--; z--; j--; 
	  goto TLgMLMRRR;
	}
	// y -> MR; shift ML to the left
	A[lw] = A[z]; A[z] = y;
	lw--; z--; 
	goto TLgMLMRRR;

 TLgMLMRRMR:
         /*   L                           R
	   |o----]---------+[-|-------][-----|
	   N     i        lw          j      M
           x -> MR            z
	 */
	y = A[lw];
	if ( y <= middle ) {
	  if ( y <= maxl ) {
	    if ( i < lw ) {
	      A[lw] = A[z]; A[z] = x;
	      x = y;
	      z--; lw--;
	      goto TLgMLMRRL;
	    }
	    // left gap also closed
	    A[hole] = y; A[lw] = A[z]; A[z] = x;
	    i--; z--; j++;
	    goto Finish4;
	  }
	  // y -> ML
	  lw--;
	  goto TLgMLMRRMR;
	}
	// middle < y
	if ( minr <= y ) {
	  A[lw] = A[z]; A[z] = A[j]; A[j] = y;
	  z--; j--; lw--;
	  goto TLgMLMRRMR;
	}
	// y -> MR
	A[lw] = A[z]; A[z] = y;
	z--; lw--;
	goto TLgMLMRRMR;

 TLgMLMRRML:
        /*   L                         R
	   |o----]------+[--|------][-----|
	   N     i     lw          j      M
           x -> ML          z
	 */
	y = A[lw];
	if ( y <= middle ) {
	  if ( y <= maxl ) {
	    if ( i < lw ) {
	      A[lw] = x;
	      x = y;
	      lw--;
	      goto TLgMLMRRL;
	    }
	    // left gap also closed
	    A[hole] = y; // y = A[i] = A[lw]
	    A[lw] = x;
	    i--; j++;
	    goto Finish4;
	  }
	  // y -> ML
	  lw--;
	  goto TLgMLMRRML;
	}
	// middle < y
	if ( minr <= y ) {
	  A[lw] = A[z]; A[z] = A[j]; A[j] = y;
	  z--; j--; lw--;
	  goto TLgMLMRRML;
	}
	// y -> MR
	A[lw] = A[z]; A[z] = y;
	z--; lw--;
	goto TLgMLMRRML;

 TLgMLMRRL:
         /*   L                           R
	   |o----]---------+[-|-------][-----|
	   N     i        lw          j      M
           x -> L             z
	 */
	i++;
	y = A[i];
	if ( y <= middle) {
	  if ( y <= maxl ) goto TLgMLMRRL;
	  if ( i <= lw ) {
	    A[i] = x;
	    x = y;
	    goto TLgMLMRRML;
	  }
	  // left gap closed
	  A[hole] = x;
	  i--; j++;
	  goto Finish4;
	}
	// middle < y
	A[i] = x;
	x = y;
	if ( minr <= y ) goto TLgMLMRRR;
	// y -> MR
	goto TLgMLMRRMR;

	// left gap closed
 TLMLMRgRL:
        /*   L                               R
	   |o----][-----------|-]+--------[-----|
	   N      i           z  up       j     M
           x -> L             
	 */
	y = A[up];
	if ( y <= middle ) {
	  if ( y <= maxl ) { // y -> L
	    z++;
	    A[up] = A[z]; A[z] = A[i]; A[i] = y;
	    i++; up++;
	    goto TLMLMRgRL;
	  }
	  // y -> ML
	  z++;
	  A[up] = A[z]; A[z] = y;
	  up++;
	  goto TLMLMRgRL;
	}
	// middle < y
	if ( y < minr ) {
	  up++;
	  goto TLMLMRgRL;
	}
	// y -> R
	if ( up < j ) {
	  z++;
	  A[up] = A[z]; A[z] = A[i]; A[i] = x;
	  x = y;
	  i++; up++;
	  goto TLMLMRgRR;
	}
	// right gap closed
	A[hole] = x; 
	i--;
	goto Finish4;

 TLMLMRgRML:
         /*   L                              R
	   |o----][-----------|-]+--------[-----|
	   N      i              up       j     M
           x -> ML            z
	 */
	y = A[up];
	if ( y <= middle ) {
	  if ( y <= maxl ) {
	    z++;
	    A[up] = A[z]; A[z] = A[i]; A[i] = y;
	    i++; up++;
	    goto TLMLMRgRML;
	  }
	  // y -> ML
	  z++;
	  A[up] = A[z]; A[z] = y;
	  up++;
	  goto TLMLMRgRML;
	}
	// middle < y
	if ( y < minr ) {
	  up++;
	  goto TLMLMRgRML;
	}
	// y --> R
	if ( up < j ) {
	    z++;
	    A[up] = A[z]; A[z] = x;
	    x = y;
	    up++;
	    goto TLMLMRgRR;
	}
	// right gap closed
	i--;
	A[hole] = A[i]; A[i] = x;
	i--;
	goto Finish4;
	
 TLMLMRgRR:
         /*   L                              R
	   |o----][-----------|-]+--------[-----|
	   N      i              up       j     M
           x -> R             z
	 */
	j--;
	y = A[j];
	if ( y <= middle ) {
	  A[j] = x;
	  x = y;
	  if ( y <= maxl ) {
	    goto TLMLMRgRL;
	  }
	  // y -> ML
	  goto TLMLMRgRML;
	}
	// middle < y
	if ( minr <= y ) goto TLMLMRgRR;
	// y -> MR
	if ( up <= j ) {
	  A[j] = x;
	  x = y;
         /*   L                              R
	   |o----][-----------|-]+--------[-----|
	   N      i              up       j     M
           x -> MR            z
	 */
	  goto TLMLMRgRMR;
	}
	// right gap closed
	i--;
	A[hole] = A[i]; A[i] = A[z]; A[z] = A[j]; A[j] = x;
	z--; i--;
	goto Finish4;

 TLMLMRgRMR:
         /*   L                              R
	   |o----][-----------|-]+--------[-----|
	   N      i              up       j     M
           x -> MR            z
	 */
	y = A[up];
	if ( y <= middle ) {
	  if ( y <= maxl ) {
	    z++;
	    A[up] = A[z]; A[z] = A[i]; A[i] = y;
	    i++; up++;
	    goto TLMLMRgRMR;
	  }
	  // y -> ML
	  z++;
	  A[up] = A[z]; A[z] = y;
	  up++;
	  goto TLMLMRgRMR;
	}
	// middle < y
	if ( y < minr ) {
	  up++;
	  goto TLMLMRgRMR;
	}
	// y --> R
	if ( up < j ) {
	  A[up] = x;
	  x = y;
	  up++;
	  goto TLMLMRgRR;
	}
	// right gap closed
	i--;
	A[hole] = A[i]; A[i] = A[z]; A[z] = x;
	i--; z--;
	goto Finish4;


	// Finish0:
	// printf("cut4 exit Finish0 N: %d M: %d\n", N, M);
	// return;

	// +++++++++++++++both gaps closed+++++++++++++++++++
	// int lleft, lmiddle, lright;
	// int k; // for testing

 Finish4:
	// printf("cut4 exit Finish4 N: %d M: %d\n", N, M);
	// printf("cut4 exit Finish4 i %i z %i j %i \n", i, z, j);
         /*   L        ML         MR            R
	   |-----][----------|-----------][-----|
	   N     i           z            j     M
	 */
	/*
	for  (k = N; k <= i; k++) 
	  if ( maxl < A[k] ) {
	     printf("L error k: %i\n", k); 
             printf("N: %d i %i lw %i z %i up %i j %i M: %d\n", N,i,lw,z,up,j,M);
	    exit(1);
	  }
	for  (k = i+1; k <= z; k++) 
	  if ( A[k] <= maxl || middle < A[k] ) {
	     printf("ML error k: %i\n", k); 
             printf("N: %d i %i lw %i z %i up %i j %i M: %d\n", N,i,lw,z,up,j,M);
	    exit(1);
	  }
	for  (k = z+1; k < j; k++) 
	  if ( A[k] <= middle || minr <= A[k] ) {
	     printf("MR error k: %i\n", k); 
             printf("N: %d i %i lw %i z %i up %i j %i M: %d\n", N,i,lw,z,up,j,M);
	    exit(1);
	  }
	for  (k = j; k <= M; k++) 
	  if ( A[k] < minr ) {
	     printf("R error k: %i\n", k); 
             printf("N: %d i %i lw %i z %i up %i j %i M: %d\n", N,i,lw,z,up,j,M);
	    exit(1);
	  }
	*/

         /*   L        ML         MR         R
	   |-----][----------|-----------][-----|
	   N     i           z            j     M
	 */
	if ( z-N < M-z ) {
	  cut4c(A, N, i, depthLimit);
	  cut4c(A, i+1, z, depthLimit);
	  if ( j-z < M-j ) {
	    cut4c(A, z+1, j-1, depthLimit);
	    // cut4c(A, j, M, depthLimit);
	    // return;
	    N = j;
	    goto Start;
	  }
	  cut4c(A, j, M, depthLimit);
	  // cut4c(A, z+1, j-1, depthLimit);
	  // return;
	  N = z+1; M = j-1;
	  goto Start;
	}
	// M-z <= z-N
	cut4c(A, z+1, j-1, depthLimit);
	cut4c(A, j, M, depthLimit);
	if ( i-N < z-i ) {
	  cut4c(A, N, i, depthLimit);
	  // cut4c(A, i+1, z, depthLimit);
	  // return;
	  N = i+1; M = z;
	  goto Start;
	}
	cut4c(A, i+1, z, depthLimit);
	// cut4c(A, N, i, depthLimit);
	M = i;
	goto Start;
} // end cut4c

void heapSort();
void heapc(int *A, int N, int M) {
  if ( M <= N ) return;
  heapSort(&A[N], M-N+1);
} // end heapc

void heapify();
void siftDown();
void heapSort(int *a, int count) {
  // input:  an unordered array a of length count
  // first place a in max-heap order
  heapify(a, count);
  int end = count-1; // in languages with zero-based arrays the children are 2*i+1 and 2*i+2
  while ( end > 0 ) {
    // (swap the root(maximum value) of the heap with the last element of the heap)
    // swap(a[end], a[0]);
    iswap(end, 0, a);
    // (decrease the size of the heap by one so that the previous max value will
    // stay in its proper placement) 
    end = end - 1;
    // (put the heap back in max-heap order)
    siftDown(a, 0, end);
  }
} // end heapSort
         
void heapify(int *a, int count) {
  // (start is assigned the index in a of the last parent node)
  int start = (count - 2) / 2;
  // while ( start >= 0 do
  while ( 0 <= start ) {
    // (sift down the node at index start to the proper place such that all nodes below
    // the start index are in heap order)
    siftDown(a, start, count-1);
    start = start - 1;
  } // (after sifting down the root all nodes/elements are in heap order)
} // end heapify
 
void siftDown(int *a, int start, int end) {
  // input:  end represents the limit of how far down the heap to sift.
  int root = start;
  int child, swapi;
  // while root * 2 + 1 <= end do          (While the root has at least one child)
  while ( root * 2 + 1 <= end ) {
    child = root * 2 + 1; // (root*2 + 1 points to the left child)
    swapi = root; //(keeps track of child to swap with)
    // (check if root is smaller than left child)
    if ( a[swapi] < a[child] ) 
      swapi = child;
    // (check if right child exists, and if it's bigger than what we're currently swapping with)
    if ( child+1 <= end && a[swapi] < a[child+1] )
      swapi = child + 1;
    // (check if we need to swap at all)
    if ( swapi != root ) {
      // swap(a[root], a[swapi]);
      iswap(root, swapi, a);
      root = swapi; // (repeat to continue sifting down the child now)
    } else return;
  }
} // end siftDown

// -------------- Bentley & McIlroy's stuff ------------------------ 

#include <stdlib.h>

#ifndef __GNUC__
#define inline
#endif

// The next two lines are not accept by some gcc compilers.
// static inline char	*med33 _PARAMS((char *, char *, char *, int (*)()));
// static inline void	 swapfunc _PARAMS((char *, char *, int, int));

#define min(a, b)	(a) < (b) ? a : b

/*
 * Qsort routine from Bentley & McIlroy's "Engineering a Sort Function".
 */
#define swapcode(TYPE, parmi, parmj, n) { 		\
	long i = (n) / sizeof (TYPE); 			\
	register TYPE *pi = (TYPE *) (parmi); 		\
	register TYPE *pj = (TYPE *) (parmj); 		\
	do { 						\
		register TYPE	t = *pi;		\
		*pi++ = *pj;				\
		*pj++ = t;				\
        } while (--i > 0);				\
}

#define SWAPINIT(a, es) swaptype = ((char *)a - (char *)0) % sizeof(long) || \
	es % sizeof(long) ? 2 : es == sizeof(long)? 0 : 1;

static inline void swapfunc(a, b, n, swaptype)
     char *a;
     char *b;
     int n;
     int swaptype;
{
	if (swaptype <= 1) 
		swapcode(long, a, b, n)
	else
		swapcode(char, a, b, n)
}

#define swap(a, b)					\
	if (swaptype == 0) {				\
		long t = *(long *)(a);			\
		*(long *)(a) = *(long *)(b);		\
		*(long *)(b) = t;			\
	} else						\
		swapfunc(a, b, es, swaptype)

#define vecswap(a, b, n) 	if ((n) > 0) swapfunc(a, b, n, swaptype)

static inline char * med33(a, b, c, cmp)
     char *a;
     char *b;
     char *c;
     int (*cmp)(); 
{
	return cmp(a, b) < 0 ?
	       (cmp(b, c) < 0 ? b : (cmp(a, c) < 0 ? c : a ))
              :(cmp(b, c) > 0 ? b : (cmp(a, c) < 0 ? a : c ));
}

void bentley(a, n, es, cmp)
     void *a;
     size_t n;
     size_t es;
     int (*cmp)();
{
  char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
  int d, r, swaptype; 
  // The use of swap_cnt and the 2nd invocation of insertionsort has been removed
  // int swap_cnt; 

loop:	SWAPINIT(a, es);
  // swap_cnt = 0;
	if (n < 7) {
		for (pm = (char *) a + es; pm < (char *) a + n * es; pm += es)
			for (pl = pm; pl > (char *) a && cmp(pl - es, pl) > 0;
			     pl -= es)
				swap(pl, pl - es);
		return;
	}
	pm = (char *) a + (n / 2) * es;
	if (n > 7) {
		pl = a;
		pn = (char *) a + (n - 1) * es;
		if (n > 40) {
			d = (n / 8) * es;
			pl = med33(pl, pl + d, pl + 2 * d, cmp);
			pm = med33(pm - d, pm, pm + d, cmp);
			pn = med33(pn - 2 * d, pn - d, pn, cmp);
		}
		pm = med33(pl, pm, pn, cmp);
	}

	swap(a, pm);
	pa = pb = (char *) a + es;
	// pa = pb = (char *) a;
	pc = pd = (char *) a + (n - 1) * es;
	for (;;) {
		while (pb <= pc && (r = cmp(pb, a)) <= 0) {
			if (r == 0) {
			        // swap_cnt = 1;
				swap(pa, pb);
				pa += es;
			}
			pb += es;
		}
		while (pb <= pc && (r = cmp(pc, a)) >= 0) {
			if (r == 0) {
			        // swap_cnt = 1;
				swap(pc, pd);
				pd -= es;
			}
			pc -= es;
		}
		if (pb > pc) break;
		swap(pb, pc);
		// swap_cnt = 1;
		pb += es;
		pc -= es;
	}
	/*
	if (swap_cnt == 0) {  // Switch to insertion sort 
		for (pm = (char *) a + es; pm < (char *) a + n * es; pm += es)
			for (pl = pm; pl > (char *) a && cmp(pl - es, pl) > 0; 
			     pl -= es)
				swap(pl, pl - es);
		return;
	}
	*/

	pn = (char *) a + n * es;
	r = min(pa - (char *)a, pb - pa);
	vecswap(a, pb - r, r);
	r = min(pd - pc, pn - pd - es);
	vecswap(pb, pn - r, r);

	/* Ordering on the recursive calls has been added to obtain at most 
	   log2(N) stack space 
	if ((r = pb - pa) > es)
		bentley(a, r / es, es, cmp);
	if ((r = pd - pc) > es) { 
	        // Iterate rather than recurse to save stack space 
		a = pn - r;
		n = r / es;
		goto loop;
	}
	*/

    int left =  pb - pa;
    int right = pd - pc;
    if ( left <= right ) {
       if ( left > es ) bentley(a, left / es, es, cmp);
       if ( right > es ) {
	   // Iterate rather than recurse to save stack space 
	   a = pn - right;
	   n = right / es;
	   goto loop;
       }
    } else {
       if ( right > es ) bentley(pn-right, right / es, es, cmp);
       if ( left > es ) {
	   // Iterate rather than recurse to save stack space 
	   // a = pn - left;
	   n = left / es;
	   goto loop;
       }
    }
} // end of bentley


void sortcB(int *A, int N, int M)
{
  int compareInt();
  bentley(&A[N], M-N+1, sizeof(int), compareInt);
} // end sortcB

void callBentley(int *A, int size, int (*compare1)() ) {
  bentley(A, size, sizeof(int), compare1);
} // end callBentley

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


void compareDpqSortAgainstDdCSort2() {
  // This code can be used also easily for cut2 & cut3x, tps, cut4
  printf("Entering compareDpqSortAgainstCut4 Sawtooth ........\n");
  // printf("Entering compareDpqSortAgainstCut2 Sawtooth ........\n");
  // printf("Entering compareDpqSortAgainstCut3x Sawtooth ........\n");
  // printf("Entering compareDpqSortAgainstTPS Sawtooth ........\n");
  int dpqSortTime, cut4Time, T;
  int seed = 666;
  int z;
  // int siz = 1024;
  int siz = 1024*1024;
  // int limit = 1024 * 1024 * 16 + 1;
  // int seedLimit = 32 * 1024;
  int limit = siz + 1;
  // int seedLimit = 32;
  int seedLimit = 4;
  // int seedLimit = 1;
  float frac;

  while (siz <= limit) {
    printf("%s %d %s %d %s", "siz: ", siz, " seedLimit: ", seedLimit, "\n");
    // int A[siz];
    int *A = malloc (sizeof(int) * siz);
    // warm up the process
    fillarray(A, siz, seed);
    int TFill, m, tweak;
    int dpqCnt, cut4Cnt, dpqCntx, cut4Cntx;
    int sumDpq, sumCut4, sumDpqx, sumCut4x;
    // for (z = 0; z < 3; z++) { // repeat to check stability
    for (z = 0; z < 1; z++) { // repeat to check stability
      dpqCnt = cut4Cnt = sumDpq = sumCut4 = 0;
      dpqCntx = cut4Cntx = sumDpqx = sumCut4x = 0;
      for (m = 1; m < 2 * siz; m = m * 2) {
      	for (tweak = 0; tweak <= 5; tweak++ ) {
	  if ( 4 == tweak ) continue;
	  dpqSortTime = 0; cut4Time = 0;
	  TFill = clock();
	  for (seed = 0; seed < seedLimit; seed++) 
	    // sawtooth(A, siz, m, tweak);
	    // rand2(A, siz, m, tweak, seed);
	    // plateau(A, siz, m, tweak);
	    // shuffle(A, siz, m, tweak, seed);
	    // stagger(A, siz, m, tweak);
	    slopes(A, siz, m, tweak);

	  TFill = clock() - TFill;
	  T = clock();
	  for (seed = 0; seed < seedLimit; seed++) { 
	    // sawtooth(A, siz, m, tweak);
	    // rand2(A, siz, m, tweak, seed);
	    // plateau(A, siz, m, tweak);
	    // shuffle(A, siz, m, tweak, seed);
	    // stagger(A, siz, m, tweak);
	    slopes(A, siz, m, tweak);

	    dpqSort(A, 0, siz-1);
	    // cut4(A, 0, siz-1);   
	  }
	  dpqSortTime = dpqSortTime + clock() - T - TFill;
	  sumDpq += dpqSortTime;
	  // if ( 4 != tweak ) sumDpqx += dpqSortTime;
	  // if ( 4 != tweak && 1 != tweak) sumDpqx += dpqSortTime;

	  T = clock();
	  for (seed = 0; seed < seedLimit; seed++) { 
	    // sawtooth(A, siz, m, tweak);
	    // rand2(A, siz, m, tweak, seed);
	    // plateau(A, siz, m, tweak);
	    // shuffle(A, siz, m, tweak, seed);
	    // stagger(A, siz, m, tweak);
	    slopes(A, siz, m, tweak);

	    // cut4(A, 0, siz-1);  
	    // cut2(A, 0, siz-1);  
	    // cut3x(A, 0, siz-1);  
	    tps(A, 0, siz-1);  
	    // tps4(A, 0, siz-1);  
	  }
	  cut4Time = cut4Time + clock() - T - TFill;
	  sumCut4 += cut4Time;
	  // if ( 4 != tweak && 1 != tweak ) sumCut4x += cut4Time;
	  // if ( 4 != tweak ) sumCut4x += cut4Time;
	  printf("Sawtooth m: %d tweak: %d ", m, tweak);
	  printf("dpqSortTime: %d ", dpqSortTime);
	  printf("Cut2Time: %d ", cut4Time);
	  frac = 0;
	  if ( dpqSortTime != 0 ) frac = cut4Time / ( 1.0 * dpqSortTime );
	  printf("frac: %f \n", frac);
	  if ( dpqSortTime < cut4Time ) dpqCnt++;
	  else cut4Cnt++;
	  /*
	  if ( 4 != tweak && 1 != tweak ) {
	    // if ( 4 != tweak ) {
	    if ( dpqSortTime < cut4Time ) dpqCntx++;
	    else cut4Cntx++;
	  }
	  // */
	} // end tweak
	frac = 0;
	if ( sumDpq != 0 ) frac = sumCut4 / ( 1.0 * sumDpq );
	printf("sumDpq:    %i sumCut4:  %i frac: %f\n", 
	       sumDpq, sumCut4, frac);
	if ( sumDpqx != 0 ) frac = sumCut4x / ( 1.0 * sumDpqx );
	printf("sumDpqx:   %i sumCut4:  %i frac: %f\n\n", 
	       sumDpqx, sumCut4x, frac);
      }
      frac = 0;
      if ( sumDpq != 0 ) frac = sumCut4 / ( 1.0 * sumDpq );
      printf("Measurements:\n");
      printf("sumDpq:    %i sumCut4:  %i frac: %f\n", 
	     sumDpq, sumCut4, frac);
      printf("sumDpq:    %i sumCut4:  %i", sumDpq, sumCut4);
      printf(" dpqCnt:  %i cut4Cnt:  %i\n", dpqCnt, cut4Cnt);
      // /*
      if ( sumDpqx != 0 ) frac = sumCut4x / ( 1.0 * sumDpqx );
      printf("sumDpqx:  %i sumCut4:  %i frac: %f\n", 
	     sumDpqx, sumCut4x, frac);
      printf("sumDpqx:  %i sumCut4x: %i", sumDpqx, sumCut4x);
      printf(" dpqCntx: %i cut4Cntx: %i\n", dpqCntx, cut4Cntx);
      // */
    }

    free(A);
    siz = siz * 2;
    seedLimit = seedLimit / 2;
  }
} // end compareDpqSortAgainstDdCSort2


void compareSortcBAgainstCut4() {
  // printf("Entering compareSortcBAgainstCut4 Sawtooth ........\n");
  // printf("Entering compareSortcBAgainstCut4 Rand2 ........\n");
  // printf("Entering compareSortcBAgainstCut4 Plateau ........\n");
  // printf("Entering compareSortcBAgainstCut4 Shuffle ........\n");
  printf("Entering compareSortcBAgainstCut4 Stagger ........\n");
  int sortcBTime, cut4Time, T;
  int seed = 666;
  int z;
  // int siz = 1024;
  int siz = 1024*1024;
  // int limit = 1024 * 1024 * 16 + 1;
  // int seedLimit = 32 * 1024;
  int limit = siz + 1;
  // int seedLimit = 32;
  int seedLimit = 1;
  float frac;
  while (siz <= limit) {
    printf("%s %d %s %d %s", "siz: ", siz, " seedLimit: ", seedLimit, "\n");
    // int A[siz];
    int *A = malloc (sizeof(int) * siz);
    // warm up the process
    fillarray(A, siz, seed);
    int TFill, m, tweak;
    int sortcBCnt, cut4Cnt;
    // int sortcBCntx, cut4Cntx;
    int sumQsortB, sumCut4;
    // int sumQsortBx, sumCut4x;
    // for (z = 0; z < 3; z++) { // repeat to check stability
    for (z = 0; z < 1; z++) { // repeat to check stability
      sortcBCnt = cut4Cnt = sumQsortB = sumCut4 = 0;
      // sortcBCntx = cut4Cntx = sumQsortBx = sumCut4x = 0;
      for (m = 1; m < 2 * siz; m = m * 2) {
      // m = 1024 * 1024; {
      	for (tweak = 0; tweak <= 5; tweak++ ) { 
	  if ( 4 == tweak ) continue;  // due to bias of sorted arrays
	  sortcBTime = 0; cut4Time = 0;
	  TFill = clock();
	  for (seed = 0; seed < seedLimit; seed++) 
	    // sawtooth(A, siz, m, tweak);
	    // rand2(A, siz, m, tweak, seed);
	    // plateau(A, siz, m, tweak);
	    // shuffle(A, siz, m, tweak, seed);
	    stagger(A, siz, m, tweak);
	  TFill = clock() - TFill;
	  T = clock();
	  for (seed = 0; seed < seedLimit; seed++) { 
	    // sawtooth(A, siz, m, tweak);
	    // rand2(A, siz, m, tweak, seed);
	    // plateau(A, siz, m, tweak);
	    // shuffle(A, siz, m, tweak, seed);
	    stagger(A, siz, m, tweak);
	    sortcB(A, 0, siz-1); // Bentley
	    // part3(A, 0, siz-1);
	  }
	  sortcBTime = sortcBTime + clock() - T - TFill;
	  sumQsortB += sortcBTime;
	  // if ( 4 != tweak ) sumQsortBx += sortcBTime;
	  T = clock();
	  for (seed = 0; seed < seedLimit; seed++) { 
	    // sawtooth(A, siz, m, tweak);
	    // rand2(A, siz, m, tweak, seed);
	    // plateau(A, siz, m, tweak);
	    // shuffle(A, siz, m, tweak, seed);
	    stagger(A, siz, m, tweak);
	    // cut2(A, 0, siz-1); 
	    cut4(A, 0, siz-1); 
	  }
	  cut4Time = cut4Time + clock() - T - TFill;
	  sumCut4 += cut4Time;
	  // if ( 4 != tweak ) sumCut4x += cut4Time;
	  printf("Stagger size: %d m: %d tweak: %d ", siz, m, tweak);
	  printf("sortcBTime: %d ", sortcBTime);
	  printf("Cut4Time: %d ", cut4Time);
	  frac = 0;
	  if ( sortcBTime != 0 ) frac = cut4Time / ( 1.0 * sortcBTime );
	  printf("frac: %f \n", frac);
	  if ( sortcBTime < cut4Time ) sortcBCnt++;
	  else cut4Cnt++;
	}
	printf("sumQsortB:   %i sumCut4:  %i frac: %f", 
	       sumQsortB, sumCut4, (sumCut4/(1.0 * sumQsortB)));
	printf(" sortcBCnt:  %i cut4Cnt:  %i\n", sortcBCnt, cut4Cnt);
      }
      frac = 0;
      if ( sumQsortB != 0 ) frac = sumCut4 / ( 1.0 * sumQsortB );
      printf("Measurements:\n");
      printf("sumQsortB:   %i sumCut4:  %i frac: %f", 
	     sumQsortB, sumCut4, (sumCut4/(1.0 * sumQsortB)));
      printf(" sortcBCnt:  %i cut4Cnt:  %i\n", sortcBCnt, cut4Cnt);
      // printf("sumQsortBx:  %i sumCut4x: %i", sumQsortBx, sumCut4x);
      // printf(" sortcBCntx: %i cut4Cntx: %i\n", sortcBCntx, 
    }
    free(A);
    siz = siz * 2;
    seedLimit = seedLimit / 2;
  }
} // end compareQsortBAgainstCut4

void showDistribution() {
  int siz = 40, m = 0, tweak = 5, i;
  int A[siz];
  stagger(A, siz, m, tweak);
  for (i = 0; i < siz; i++)
    printf("i %i %i\n", i, A[i]);
} // end showDistribution

void compareSortcQAgainstCut2() { // native qsort or linux qsort <-> cut2 using Bentley stuff
  printf("Entering compareSortcQAgainstCut2 Sawtooth ........\n");
  // printf("Entering compareSortcQAgainstCut2 Rand2 ........\n");
  // printf("Entering compareSortcQAgainstCut2 Plateau ........\n");
  // printf("Entering compareSortcQAgainstCut2 Shuffle ........\n");
  // printf("Entering compareSortcQAgainstCut2 Stagger ........\n");
  int sortcBTime, cut2Time, T;
  int seed = 666;
  int z;
  // int siz = 1024;
  // int seedLimit = 32 * 1024;
  int siz = 1024*1024;
  // int siz = 1024;
  // int siz = 512;
  // int siz = 256;
  // int seedLimit = 32;
  int seedLimit = 2;
  // int seedLimit = 1;
  // int limit = 1024 * 1024 * 16 + 1;
  int limit = siz + 1;
  float frac;
  while (siz <= limit) {
    printf("%s %d %s %d %s", "siz: ", siz, " seedLimit: ", seedLimit, "\n");
    // int A[siz];
    int *A = malloc (sizeof(int) * siz);
    int *B = malloc (sizeof(int) * siz);
    // warm up the process
    fillarray(A, siz, seed);
    fillarray(B, siz, seed);
    int TFill, m, tweak;
    int sortcBCnt, cut2Cnt;
    // int sortcBCntx, cut2Cntx;
    int sumQsortB, sumCut2;
    // int sumQsortBx, sumCut2x;
    // for (z = 0; z < 3; z++) { // repeat to check stability
    for (z = 0; z < 1; z++) { // repeat to check stability
      sortcBCnt = cut2Cnt = sumQsortB = sumCut2 = 0;
      // sortcBCntx = cut2Cntx = sumQsortBx = sumCut2x = 0;
      for (m = 1; m < 2 * siz; m = m * 2) {
      	for (tweak = 0; tweak <= 5; tweak++ ) {
	  if ( 4==tweak) continue; // due to bias for sorted
	  sortcBTime = 0; cut2Time = 0;
	  TFill = clock();
	  for (seed = 0; seed < seedLimit; seed++) 
	    sawtooth(A, siz, m, tweak);
	    // rand2(A, siz, m, tweak, seed);
	    // plateau(A, siz, m, tweak); // do not use
	    // shuffle(A, siz, m, tweak, seed); // do not use
	    // stagger(A, siz, m, tweak);
	    // slopes(A, siz, m, tweak);
	  TFill = clock() - TFill;
	  T = clock();
	  for (seed = 0; seed < seedLimit; seed++) { 
	    sawtooth(A, siz, m, tweak);
	    // rand2(A, siz, m, tweak, seed);
	    // plateau(A, siz, m, tweak);
	    // shuffle(A, siz, m, tweak, seed);
	    // stagger(A, siz, m, tweak);
	    // slopes(A, siz, m, tweak);

	    // sortc(A, 0, siz-1);
	    // sortlinux(A, 0, siz-1);  // retrieved from Linux OK
	    // sortcB(A, 0, siz-1); // B&M
	    // chenSort(A, 0, siz-1); 
	    dpqSort(A, 0, siz-1);
	  }
	  sortcBTime = sortcBTime + clock() - T - TFill;
	  sumQsortB += sortcBTime;
	  // if ( 4 != tweak ) sumQsortBx += sortcBTime;

	  T = clock();
	  for (seed = 0; seed < seedLimit; seed++) { 
	    sawtooth(B, siz, m, tweak);
	    // rand2(B, siz, m, tweak, seed);
	    // plateau(B, siz, m, tweak);
	    // shuffle(B, siz, m, tweak, seed);
	    // stagger(B, siz, m, tweak);
	    // slopes(B, siz, m, tweak);

	    // heapc(B, 0, siz-1);
	    // sortlinux(B, 0, siz-1);  // retrieved from Linux
	    // cut2(B, 0, siz-1);
	    // quicksort0(B, 0, siz-1);
	    // myqs(B, 0, siz-1);
	    tps(B, 0, siz-1);
	    // cut4(B, 0, siz-1);
	  }
	  cut2Time = cut2Time + clock() - T - TFill;
	  sumCut2 += cut2Time;
	  int k;
	  for (k = 0; k < siz; k++) 
	    if ( A[k] != B[k] ) {
	      printf("*** Error k %i A[k] %i B[k] %i\n", k, A[k], B[k]);
	      exit(0);
	    }

	  // if ( 4 != tweak ) sumCut2x += cut2Time;
	  printf("Size: %d m: %d tweak: %d ", siz, m, tweak);
	  printf("sortcBTime: %d ", sortcBTime);
	  printf("Cut2Time: %d ", cut2Time);
	  frac = 0;
	  if ( sortcBTime != 0 ) frac = cut2Time / ( 1.0 * sortcBTime );
	  printf("frac: %f \n", frac);
	  if ( sortcBTime < cut2Time ) sortcBCnt++;
	  else cut2Cnt++;
	}
	printf("sumQsortB:   %i sumCut2:  %i frac: %f", 
	       sumQsortB, sumCut2, (sumCut2/(1.0 * sumQsortB)));
	printf(" sortcBCnt:  %i cut2Cnt:  %i\n", sortcBCnt, cut2Cnt);
      }
      frac = 0;
      if ( sumQsortB != 0 ) frac = sumCut2 / ( 1.0 * sumQsortB );
      printf("Measurements:\n");
      printf("sumQsortB:   %i sumCut2:  %i frac: %f", 
	     sumQsortB, sumCut2, (sumCut2/(1.0 * sumQsortB)));
      printf(" sortcBCnt:  %i cut2Cnt:  %i\n", sortcBCnt, cut2Cnt);
      // printf("sumQsortBx:  %i sumCut2x: %i", sumQsortBx, sumCut2x);
      // printf(" sortcBCntx: %i cut2Cntx: %i\n", sortcBCntx, cut2Cntx);

    }
    free(A); free(B);
    siz = siz * 2;
    seedLimit = seedLimit / 2;
  }
} // end compareQsortBAgainstCut2

void compareSortcBAgainstCut2() {
  // printf("Entering compareSortcB2AgainstCut2 Sawtooth ........\n");
  // printf("Entering compareSortcB2AgainstCut2 Rand2 ........\n");
  // printf("Entering compareSortcB2AgainstCut2 Plateau ........\n");
  // printf("Entering compareSortcB2AgainstCut2 Shuffle ........\n");
  printf("Entering compareSortcB2AgainstCut2 Stagger ........\n");
  int sortcBTime, cut2Time, T;
  int seed = 666;
  int z;
  // int siz = 1024;
  int siz = 1024*1024;
  // int limit = 1024 * 1024 * 16 + 1;
  // int seedLimit = 32 * 1024;
  int limit = siz + 1;
  int seedLimit = 32;
  float frac;
  while (siz <= limit) {
    printf("%s %d %s %d %s", "siz: ", siz, " seedLimit: ", seedLimit, "\n");
    // int A[siz];
    int *A = malloc (sizeof(int) * siz);
    // warm up the process
    fillarray(A, siz, seed);
    int TFill, m, tweak;
    int sortcBCnt, cut2Cnt; 
    // int sortcBCntx, cut2Cntx;
    int sumQsortB, sumCut2;
    // int sumQsortBx, sumCut2x;
    // for (z = 0; z < 3; z++) { // repeat to check stability
    for (z = 0; z < 1; z++) { // repeat to check stability
      sortcBCnt = cut2Cnt = sumQsortB = sumCut2 = 0;
      // sortcBCntx = cut2Cntx = sumQsortBx = sumCut2x = 0;
      for (m = 1; m < 2 * siz; m = m * 2) {
	// m = 1024 * 1024; {
      	for (tweak = 0; tweak <= 5; tweak++ ) {
	  // tweak = 1; {
	  sortcBTime = 0; cut2Time = 0;
	  TFill = clock();
	  for (seed = 0; seed < seedLimit; seed++) 
	    // sawtooth(A, siz, m, tweak);
	    // rand2(A, siz, m, tweak, seed);
	    // plateau(A, siz, m, tweak);
	    // shuffle(A, siz, m, tweak, seed);
	    stagger(A, siz, m, tweak);
	  TFill = clock() - TFill;
	  T = clock();
	  for (seed = 0; seed < seedLimit; seed++) { 
	    // sawtooth(A, siz, m, tweak);
	    // rand2(A, siz, m, tweak, seed);
	    // plateau(A, siz, m, tweak);
	    // shuffle(A, siz, m, tweak, seed);
	    stagger(A, siz, m, tweak);
	    sortcB(A, 0, siz-1);  
	  }
	  sortcBTime = sortcBTime + clock() - T - TFill;
	  sumQsortB += sortcBTime;
	  // if ( 4 != tweak ) sumQsortBx += sortcBTime;
	  T = clock();
	  for (seed = 0; seed < seedLimit; seed++) { 
	    // sawtooth(A, siz, m, tweak);
	    // rand2(A, siz, m, tweak, seed);
	    // plateau(A, siz, m, tweak);
	    // shuffle(A, siz, m, tweak, seed);
	    stagger(A, siz, m, tweak);
	    cut2(A, 0, siz-1);
	  }
	  cut2Time = cut2Time + clock() - T - TFill;
	  sumCut2 += cut2Time;
	  // if ( 4 != tweak ) sumCut2x += cut2Time;
	  printf("Size: %d m: %d tweak: %d ", siz, m, tweak);
	  printf("sortcBTime: %d ", sortcBTime);
	  printf("Cut2Time: %d ", cut2Time);
	  frac = 0;
	  if ( sortcBTime != 0 ) frac = cut2Time / ( 1.0 * sortcBTime );
	  printf("frac: %f \n", frac);
	  if ( sortcBTime < cut2Time ) sortcBCnt++;
	  else cut2Cnt++;
	}
	printf("sumQsortB:   %i sumCut2:  %i frac: %f", 
	       sumQsortB, sumCut2, (sumCut2/(1.0 * sumQsortB)));
	printf(" sortcBCnt:  %i cut2Cnt:  %i\n", sortcBCnt, cut2Cnt);
      }
      frac = 0;
      if ( sumQsortB != 0 ) frac = sumCut2 / ( 1.0 * sumQsortB );
      printf("Measurements:\n");
      printf("sumQsortB:   %i sumCut2:  %i frac: %f", 
	     sumQsortB, sumCut2, (sumCut2/(1.0 * sumQsortB)));
      printf(" sortcBCnt:  %i cut2Cnt:  %i\n", sortcBCnt, cut2Cnt);
      // printf("sumQsortBx:  %i sumCut2x: %i", sumQsortBx, sumCut2x);
      // printf(" sortcBCntx: %i cut2Cntx: %i\n", sortcBCntx, cut2Cntx);
    }
    free(A);
    siz = siz * 2;
    seedLimit = seedLimit / 2;
  }
} // end compareQsortB2AgainstCut2


void compareChenSortAgainstCut2() { 
  printf("Entering compareChenSortAgainstCut2 Sawtooth ........\n");
  // printf("Entering compareChenSortAgainstCut2 Rand2 ........\n");
  // printf("Entering compareChenSortAgainstCut2 Plateau ........\n");
  // printf("Entering compareChenSortAgainstCut2 Shuffle ........\n");
  // printf("Entering compareChenSortAgainstCut2 Stagger ........\n");
  int chenSortTime, cut2Time, T;
  int seed = 666;
  int z;
  // int siz = 1024;
  int siz = 1024*1024;
  // int limit = 1024 * 1024 * 16 + 1;
  // int seedLimit = 32 * 1024;
  int limit = siz + 1;
  int seedLimit = 32;
  float frac;
  while (siz <= limit) {
    printf("%s %d %s %d %s", "siz: ", siz, " seedLimit: ", seedLimit, "\n");
    // int A[siz];
    int *A = malloc (sizeof(int) * siz);
    // warm up the process
    fillarray(A, siz, seed);
    int TFill, m, tweak;
    int chenSortCnt, cut2Cnt;
    // chenSortCntx, cut2Cntx;
    int sumQsortB, sumCut2;
    // int  sumQsortBx, sumCut2x;
    // for (z = 0; z < 3; z++) { // repeat to check stability
    for (z = 0; z < 1; z++) { // repeat to check stability
      chenSortCnt = cut2Cnt = sumQsortB = sumCut2 = 0;
      // chenSortCntx = cut2Cntx = sumQsortBx = sumCut2x = 0;
      for (m = 1; m < 2 * siz; m = m * 2) {
	// m = 1024 * 1024; {
      	for (tweak = 0; tweak <= 5; tweak++ ) {
	  chenSortTime = 0; cut2Time = 0;
	  TFill = clock();
	  for (seed = 0; seed < seedLimit; seed++) 
	    sawtooth(A, siz, m, tweak);
	    // rand2(A, siz, m, tweak, seed);
	    // plateau(A, siz, m, tweak);
	    // shuffle(A, siz, m, tweak, seed);
	    // stagger(A, siz, m, tweak);
	  TFill = clock() - TFill;
	  // printf("Time measurement finished\n");;
	  T = clock();
	  for (seed = 0; seed < seedLimit; seed++) { 
	    sawtooth(A, siz, m, tweak);
	    // rand2(A, siz, m, tweak, seed);
	    // plateau(A, siz, m, tweak);
	    // shuffle(A, siz, m, tweak, seed);
	    // stagger(A, siz, m, tweak);
	    chenSort(A, 0, siz-1);  
	  }
	  chenSortTime = chenSortTime + clock() - T - TFill;
	  sumQsortB += chenSortTime;
	  // printf("ChenSort finished\n");;
	  // if ( 4 != tweak ) sumQsortBx += chenSortTime;
	  T = clock();
	  for (seed = 0; seed < seedLimit; seed++) { 
	    sawtooth(A, siz, m, tweak);
	    // rand2(A, siz, m, tweak, seed);
	    // plateau(A, siz, m, tweak);
	    // shuffle(A, siz, m, tweak, seed);
	    // stagger(A, siz, m, tweak);
	    cut2(A, 0, siz-1);
	  }
	  cut2Time = cut2Time + clock() - T - TFill;
	  sumCut2 += cut2Time;
	  // if ( 4 != tweak ) sumCut2x += cut2Time;
	  printf("Size: %d m: %d tweak: %d ", siz, m, tweak);
	  printf("chenSortTime: %d ", chenSortTime);
	  printf("Cut2Time: %d ", cut2Time);
	  frac = 0;
	  if ( chenSortTime != 0 ) frac = cut2Time / ( 1.0 * chenSortTime );
	  printf("frac: %f \n", frac);
	  if ( chenSortTime < cut2Time ) chenSortCnt++;
	  else cut2Cnt++;
	}
	printf("sumQsortB:   %i sumCut2:  %i frac: %f", 
	       sumQsortB, sumCut2, (sumCut2/(1.0 * sumQsortB)));
	printf(" chenSortCnt:  %i cut2Cnt:  %i\n", chenSortCnt, cut2Cnt);
      }
      frac = 0;
      if ( sumQsortB != 0 ) frac = sumCut2 / ( 1.0 * sumQsortB );
      printf("Measurements:\n");
      printf("sumQsortB:   %i sumCut2:  %i frac: %f", 
	     sumQsortB, sumCut2, (sumCut2/(1.0 * sumQsortB)));
      printf(" chenSortCnt:  %i cut2Cnt:  %i\n", chenSortCnt, cut2Cnt);
      // printf("sumQsortBx:  %i sumCut2x: %i", sumQsortBx, sumCut2x);
      // printf(" chenSortCntx: %i cut2Cntx: %i\n", chenSortCntx, cut2Cntx);
    }
    free(A);
    siz = siz * 2;
    seedLimit = seedLimit / 2;
  }
} // end compareChenSortAgainstCut2()



/*   This is the C code for Adaptive Symmetry Partition Sort       */     
/*   which sorts the specified sequence into ascending order.      */
/*   This algorithm is Rem-adaptive and offers better performance  */ 
/*   than Psort                                                    */
/*   Date : 2007/01/24, version 1.0                                */
/*   Copyright (C) 2007 Jingchao Chen                              */
/*   Any comment is welcome. For any question, email to            */
/*   chen-jc@dhu.edu.cn or chenjingchao@yahoo.com                  */
// From: http://www.gtoal.com/languages/bcpl/BCPL/bcplprogs/sort/AdoSymmetryPSort.cpp

// #include <conio.h>
#include <stdlib.h>
#include <stdio.h>
// #include <dos.h>
#define SIZE_n 45
#define ItemType int

int  n=SIZE_n;
ItemType key[SIZE_n];
/*
// compare two member
int cmp(const void * a,const void *b)
{
     return (*(int *)a - *(int *)b);
}
*/

void Adp_SymPSort(void *a, int n, int es, int (*cmp)(const void *,const void *));
void chenSort(int *A, int N, int M) {
  // Adp_SymPSort(&A[N], M-N+1, sizeof(int), cmp);
  Adp_SymPSort(&A[N], M-N+1, sizeof(int), compareInt);
}

void callChensort(int *A, int size, int (*compare1)() ) {
  Adp_SymPSort(A, size, sizeof(int), compare1);
} // end callChensort



// void check(const void * a);
/*
main(void)
{   int  i;

    printf("\n Adaptive Symmetry Partition Sort \n",n);
    srand(2007);
    for(i=0;i<n;i++) key[i]=rand()%n; // generate a random integer in [0,32767],
   	Adp_SymPSort((char *)key,n,sizeof(ItemType),cmp);
    check(key); //verify output;
}
*/
 /*
void check(const void * a)
{    int i;
     for (i=0; i< n-1; i++){
          if (*((ItemType *)a+i)>*((ItemType *)a+i+1))
          {
                printf( "\nThe sequence is not ordered");
                return;
          }
     }
     printf( "\nThe sequence is correctly sorted");
}
 */
// the code for the sorting algorithm begins  
  /*
#define swapvector(TYPE,pi,pj,n)            \
	do {                                    \
       TYPE t= *(TYPE *) (pi);              \
       *(TYPE *) (pi) = *(TYPE *) (pj);     \
       *(TYPE *) (pj) = t;                  \
       pi+=sizeof(TYPE); pj+=sizeof(TYPE);  \
       n-=sizeof(TYPE);                     \
  	} while (n > 0);
 
void swapfunc(char *a, char *b, int n, int swaptype)
{    if (swaptype <=1 ) swapvector(long,a,b,n)
     else swapvector(char,a,b,n)
}

#define swap(a,b)                          \
    if (swaptype == 0) {                   \
       long t = * (long *) (a);            \
       * (long *) (a) = * (long *) (b);    \
       * (long *) (b) = t;                 \
	}                                      \
    else swapfunc(a,b,es,swaptype)

#define SWAPINIT(a,es) swaptype =                           \
 (a - (char *) 0) % sizeof(long) || es % sizeof(long) ? 2 : \
 es == sizeof(long) ? 0 : 1
  */


#define p 16
#define beta1 256
#define beta2 512
// Symmetry Partition Sort
void SymPartitionSort(char *a, int s, int n, int es, int (*cmp)(const void *,const void *))
{   char *pm,*pb,*pc,*pi,*pj;
    int i,v,vL,m,left,right,swaptype,sp,eq,ineq,rc;
    left=right=0;
    SWAPINIT(a,es);
    while(1){
        if(n < 8){ //Insertion sort on small arrays
             for (s=1; s < n; s++)
                for (pb = a+s*es; cmp(pb-es,pb) > 0; ) {
                        swap(pb,pb-es); pb-=es; 
                        if(pb <= a) break;
                }
             return;
        }
        m= s<0 ? -s:s;
        if(m <= 2){//First,middle,last items are ordered and placed 1st,2nd and last
            v = beta2 > n ? n : 63;
            pc=a+(v-1)*es;
            pm=a+es; 
            swap(pm,a+(v/2)*es);
            if(cmp(a, pm) > 0) {swap(a,pm);}
                if((cmp(pm, pc) > 0)) {
                      swap(pm,pc);
                      if((cmp(a, pm) > 0)) {swap(a,pm);}
                }
                left=right=1; pc-=es;
            }
            else{
               v=m > n/beta1 ? n : p*m-1;
               if(s < 0) {  //Move sorted items to left end
                      if(v<n) {left=m; s=-s;}
                      else    {left=(m+1)/2; right=m/2;} 
                      swapfunc(a, a+(n-m)*es, left*es, swaptype);
                      left--;
               }
               if(s>0){
                      pb=a+m*es; pc=a+v*es;  
                      if(v < n){ //Extract sampling items 
                          sp=(n/v)*es; pj=pb; pi=pb;  
                          for(; pi < pc; pi+=es, pj+=sp) swap(pi,pj);
                      }
                      i=right=m/2; //Right move sorted items
                      do{ pb-=es; pc-=es; swap(pb,pc); i--;} while (i);
                      left=(m-1)/2; 
               }
               pm=a+left*es; pc=pm+(v-m)*es;
            }
//Fat partition begins
        pb=pi=pm+es;  
        do {
            while ( (rc=cmp(pb,pm)) < 0 ) pb+=es;
            if(pb >= pc) break;
            if(rc==0){
				if(pi!=pb) {swap(pb,pi);}
                 pi+=es; pb+=es;
                 continue;
            }
            while ((rc=cmp(pc,pm)) > 0 ) pc-=es;
            if(pb >= pc) break;
            swap(pb,pc);
            if(rc==0){
				if(pi!=pb) { swap(pb,pi);}
                pi+=es; 
            }
            pb+=es; pc-=es;
        } while (pb <= pc);
//Move equal-key items
        eq=pi-pm, ineq=pb-pi;
        if( ineq < eq) pi=pm+ineq;
        pc=pb;
        while (pm < pi ) { pc-=es; swap(pc,pm); pm+=es;} 
//Fat partition ends
            vL=(pb-a)/es; 
            if(right < v-vL) SymPartitionSort(pb, -right, v-vL, es, cmp);
            vL=vL-eq/es; 
            if(v < n){
                if(left < vL) SymPartitionSort(a, left,vL,es,cmp);
                s=v;  //Remove tail recursion
            }
            else{
                if(left >= vL) return;
                s=left; n=vL; //Remove tail recursion
            }
    }
}

// Adaptive Symmetry Partition Sort
void Adp_SymPSort(void *a, int n, int es, int (*cmp)(const void *,const void *))
{   char *pb,*pc,*pi,*pj;
    int swaptype,i,j,ne,rc,D_inv,left,m,Rev=0;
  
    SWAPINIT((char *)a,es);
//Find 1st run
    ne = n * es;
    for (i=es; i < ne; i+=es){
         if((rc=cmp((char *)a+i-es, (char *)a+i)) != 0 ){
             if(Rev==0) Rev= rc < 0 ? 1 : -1;//Rev=1: increasing, -1: decreasing
             else if(rc*Rev > 0) break;
         }
    }
    D_inv= Rev*(i/es);   //D_inv: difference of inversions & orders
    for(j=i+es; j < ne; j+=(97*es)){
         if((rc=cmp((char *)a+j-es, (char *)a+j)) < 0) D_inv++;
         if(rc>0) D_inv--;
    }
    pb=(char *)a+i-es;
    if(abs(D_inv) > n/512 ) {     
         if(Rev*D_inv < 0) {pb=(char *)a; Rev=-Rev;}  //If 1st run is reverse, re-find it
            pc=(char *)a+n*es; pj=pb;
            while(1){
                pj=pj+10*es; pi=pj-es;
                if(pj >= pc) break;
                while (pj < pc && Rev*cmp(pj-es, pj) <=0) pj+=es; //Find next run foreward
                while (pi > pb && Rev*cmp(pi-es, pi) <=0) pi-=es; //Find next run backward
                if(pj-pi < 4*es) continue;
                if(pb!=a) { //Find knots in 1st and 2nd run 
                      j=((pj-pi)/es)/2;
                      m=((pb-(char *)a)/es)/4;
                      if (j > m ) j=m;
                      for(i=0; i<j; i++) if(Rev*cmp(pb-i*es,pi+i*es) <= 0) break;
                      if(i>=j) continue;
                      pb=pb+(1-i)*es; pi=pi+i*es;
                }
                // Merge two runs by moving 2nd knot to 1st knot 
                if(pi!=pb) while(pi < pj ) { swap(pb,pi); pb+=es; pi+=es;}
                else pb=pj;
                pb-=es;
            }
    }   
    left=(pb-(char *)a)/es+1;
    if(Rev==-1){ //if the longest run reverse, reverse it
        pc=(char *)a;
        while(pc < pb ) {swap(pc,pb); pc+=es; pb-=es; }
    }
    if(left < n) SymPartitionSort((char *)a, left, n, es, cmp);
}

// Linux stuff 

/* Copyright (C) 1991,1992,1996,1997,1999,2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Written by Douglas C. Schmidt (schmidt@ics.uci.edu).

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

/* If you consider tuning this algorithm, you should consult first:
   Engineering a sort function; Jon Bentley and M. Douglas McIlroy;
   Software - Practice and Experience; Vol. 23 (11), 1249-1265, 1993.  */

#include <alloca.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

/* Byte-wise swap two items of size SIZE. */
#define SWAP(a, b, size)						      \
  do									      \
    {									      \
      register size_t __size = (size);					      \
      register char *__a = (a), *__b = (b);				      \
      do								      \
	{								      \
	  char __tmp = *__a;						      \
	  *__a++ = *__b;						      \
	  *__b++ = __tmp;						      \
	} while (--__size > 0);						      \
    } while (0)

/* Discontinue quicksort algorithm when partition gets below this size.
   This particular magic number was chosen to work best on a Sun 4/260. */
#define MAX_THRESH 4

/* Stack node declarations used to store unfulfilled partition obligations. */
typedef struct
  {
    char *lo;
    char *hi;
  } stack_node;

/* The next 4 #defines implement a very fast in-line stack abstraction. */
/* The stack needs log (total_elements) entries (we could even subtract
   log(MAX_THRESH)).  Since total_elements has type size_t, we get as
   upper bound for log (total_elements):
   bits per byte (CHAR_BIT) * sizeof(size_t).  */
#define STACK_SIZE	(CHAR_BIT * sizeof(size_t))
#define PUSH(low, high)	((void) ((top->lo = (low)), (top->hi = (high)), ++top))
#define	POP(low, high)	((void) (--top, (low = top->lo), (high = top->hi)))
#define	STACK_NOT_EMPTY	(stack < top)


/* Order size using quicksort.  This implementation incorporates
   four optimizations discussed in Sedgewick:

   1. Non-recursive, using an explicit stack of pointer that store the
      next array partition to sort.  To save time, this maximum amount
      of space required to store an array of SIZE_MAX is allocated on the
      stack.  Assuming a 32-bit (64 bit) integer for size_t, this needs
      only 32 * sizeof(stack_node) == 256 bytes (for 64 bit: 1024 bytes).
      Pretty cheap, actually.

   2. Chose the pivot element using a median-of-three decision tree.
      This reduces the probability of selecting a bad pivot value and
      eliminates certain extraneous comparisons.

   3. Only quicksorts TOTAL_ELEMS / MAX_THRESH partitions, leaving
      insertion sort to order the MAX_THRESH items within each partition.
      This is a big win, since insertion sort is faster for small, mostly
      sorted array segments.

   4. The larger of the two sub-partitions is always pushed onto the
      stack first, with the algorithm then concentrating on the
      smaller partition.  This *guarantees* no more than log (total_elems)
      stack size is needed (actually O(1) in this case)!  */


void
_quicksort (void *const pbase, size_t total_elems, size_t size,
	    //	    __compar_d_fn_t cmp, void *arg)
	    int (*cmp)(), void *arg)
{
  register char *base_ptr = (char *) pbase;

  const size_t max_thresh = MAX_THRESH * size;

  if (total_elems == 0)
    /* Avoid lossage with unsigned arithmetic below.  */
    return;

  if (total_elems > MAX_THRESH)
    {
      char *lo = base_ptr;
      char *hi = &lo[size * (total_elems - 1)];
      stack_node stack[STACK_SIZE];
      stack_node *top = stack;

      PUSH (NULL, NULL);

      while (STACK_NOT_EMPTY)
        {
          char *left_ptr;
          char *right_ptr;

	  /* Select median value from among LO, MID, and HI. Rearrange
	     LO and HI so the three values are sorted. This lowers the
	     probability of picking a pathological pivot value and
	     skips a comparison for both the LEFT_PTR and RIGHT_PTR in
	     the while loops. */

	  char *mid = lo + size * ((hi - lo) / size >> 1);

	  if ((*cmp) ((void *) mid, (void *) lo, arg) < 0)
	    SWAP (mid, lo, size);
	  if ((*cmp) ((void *) hi, (void *) mid, arg) < 0)
	    SWAP (mid, hi, size);
	  else
	    goto jump_over;
	  if ((*cmp) ((void *) mid, (void *) lo, arg) < 0)
	    SWAP (mid, lo, size);
	jump_over:;

	  left_ptr  = lo + size;
	  right_ptr = hi - size;

	  /* Here's the famous ``collapse the walls'' section of quicksort.
	     Gotta like those tight inner loops!  They are the main reason
	     that this algorithm runs much faster than others. */
	  do
	    {
	      while ((*cmp) ((void *) left_ptr, (void *) mid, arg) < 0)
		left_ptr += size;

	      while ((*cmp) ((void *) mid, (void *) right_ptr, arg) < 0)
		right_ptr -= size;

	      if (left_ptr < right_ptr)
		{
		  SWAP (left_ptr, right_ptr, size);
		  if (mid == left_ptr)
		    mid = right_ptr;
		  else if (mid == right_ptr)
		    mid = left_ptr;
		  left_ptr += size;
		  right_ptr -= size;
		}
	      else if (left_ptr == right_ptr)
		{
		  left_ptr += size;
		  right_ptr -= size;
		  break;
		}
	    }
	  while (left_ptr <= right_ptr);

          /* Set up pointers for next iteration.  First determine whether
             left and right partitions are below the threshold size.  If so,
             ignore one or both.  Otherwise, push the larger partition's
             bounds on the stack and continue sorting the smaller one. */

          if ((size_t) (right_ptr - lo) <= max_thresh)
            {
              if ((size_t) (hi - left_ptr) <= max_thresh)
		/* Ignore both small partitions. */
                POP (lo, hi);
              else
		/* Ignore small left partition. */
                lo = left_ptr;
            }
          else if ((size_t) (hi - left_ptr) <= max_thresh)
	    /* Ignore small right partition. */
            hi = right_ptr;
          else if ((right_ptr - lo) > (hi - left_ptr))
            {
	      /* Push larger left partition indices. */
              PUSH (lo, right_ptr);
              lo = left_ptr;
            }
          else
            {
	      /* Push larger right partition indices. */
              PUSH (left_ptr, hi);
              hi = right_ptr;
            }
        }
    }

  /* Once the BASE_PTR array is partially sorted by quicksort the rest
     is completely sorted using insertion sort, since this is efficient
     for partitions below MAX_THRESH size. BASE_PTR points to the beginning
     of the array to sort, and END_PTR points at the very last element in
     the array (*not* one beyond it!). */

  // #define min(x, y) ((x) < (y) ? (x) : (y))

  {
    char *const end_ptr = &base_ptr[size * (total_elems - 1)];
    char *tmp_ptr = base_ptr;
    char *thresh = min(end_ptr, base_ptr + max_thresh);
    register char *run_ptr;

    /* Find smallest element in first threshold and place it at the
       array's beginning.  This is the smallest array element,
       and the operation speeds up insertion sort's inner loop. */

    for (run_ptr = tmp_ptr + size; run_ptr <= thresh; run_ptr += size)
      if ((*cmp) ((void *) run_ptr, (void *) tmp_ptr, arg) < 0)
        tmp_ptr = run_ptr;

    if (tmp_ptr != base_ptr)
      SWAP (tmp_ptr, base_ptr, size);

    /* Insertion sort, running from left-hand-side up to right-hand-side.  */

    run_ptr = base_ptr + size;
    while ((run_ptr += size) <= end_ptr)
      {
	tmp_ptr = run_ptr - size;
	while ((*cmp) ((void *) run_ptr, (void *) tmp_ptr, arg) < 0)
	  tmp_ptr -= size;

	tmp_ptr += size;
        if (tmp_ptr != run_ptr)
          {
            char *trav;

	    trav = run_ptr + size;
	    while (--trav >= run_ptr)
              {
                char c = *trav;
                char *hi, *lo;

                for (hi = lo = trav; (lo -= size) >= tmp_ptr; hi = lo)
                  *hi = *lo;
                *hi = c;
              }
          }
      }
  }
} // end _quicksort

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

void compare00ChenAgainstFourSort() {
  compareZeros00("compare00ChenAgainstFoursort", 
		 1024*1024, 16, callChensort, cut2, 
		 compareInt);
} // end compare00ChenAgainstFourSort

void compare00MergeAgainstFourSort() {
  compareZeros00("compare00MergeAgainstFoursort", 
		 1024*1024, 16, callMergesort, cut2, 
		 compareInt);
} // end compare00MergeAgainstFourSort

void compare00DpqAgainstFourSort() {
  compareZeros00("compare00DpqAgainstFoursort", 
		 1024*1024, 16, callDpq, cut2, 
		 compareInt);
} // end compare00DpqgainstFourSort

void compare00DpqAgainstTPS() {
  compareZeros00("compare00DpqAgainstTPS", 
		 1024*1024, 16, callDpq, tps, 
		 compareInt);
} // end compare00DpqgainstFourSort

void compare00DpqAgainstSixSort() {
  compareZeros00("compare00DpqAgainstSixsort", 
		 1024*1024, 16, callDpq, cut4, 
		 compareInt);
} // end compare00DpqgainstSixSort



// ------------------------- mergeSort0 (copying) --------------------

void mergeSort0();
void callMergesort(int *A, int size, int (*compare1)() ) {
  mergeSort0(A, 0, size-1);
} // end callMergesort

void mergeSort();
void mergeSort0(int *A, int N, int M) {
  int lng = M - N + 1;
  int *B = malloc (sizeof(int) * lng);
  mergeSort(A, B, N, M);
  free(B);
} // end mergeSort0

void merge();
void insertionSort();
void mergeSort(int *A, int *B, int N, int M) {
	// System.out.println("mergeSort N: " + N + " M: " + M);
	int L = M-N;
	if ( L <= 10 ) { insertionSort(A, N, M); return; }
	int k = L>>1; // L/2;
	mergeSort(A, B, N, N+k); mergeSort(A, B, N+k+1, M);
	merge(A, B, N, M);
	// check(A, N, M);
    } // end  mergeSort

void merge(int *A, int  *B, int N, int M) {
	// System.out.println("merge N: " + N + " M: " + M);
	int L = M-N;
	int k = L>>1; // L/2;
	int i = N; int j = N+k+1; int z = N;
	int leftDone = 0;
	int rightDone = 0;
	int ai = A[i]; int aj = A[j];
	while (1) {
	    while ( ai <= aj ) {
		B[z] = ai; z++;
		if ( i < N+k ) { i++; ai = A[i]; } else {
		    leftDone = 1; break;
		}
	    }
	    if ( leftDone ) break;
	    // aj < ai
	    B[z] = aj; z++;
	    if ( M == j ) { rightDone = 1; break; }
	    j++; aj = A[j];
	    while ( aj < ai ) {
		B[z] = aj; z++;
		if ( j < M ) { j++; aj = A[j]; } else {
		    rightDone = 1; break;
		}
	    }
	    if ( rightDone ) break;
	    // ai <= aj
	    B[z] = ai; z++;
	    if ( i == N+k ) { leftDone = 1; break; }
	    i++; ai = A[i];
	}
	// System.out.println("------");
	// for ( int p = i; p <= N+k; p++ ) System.out.println(p + " " + A[p]);
	int q;
	if ( leftDone ) {
	    // System.out.println("leftDone z: " + z);
	    for ( q = N; q < z; q++ ) A[q] = B[q];
	} else { // rightDone
	    // System.out.println("rightDone z: " + z + " i: " + i + " k: " + k);
	    for ( q = N+k; i <= q; q-- ) A[q + L - k] = A[q];
	    for ( q = N; q < z; q++ ) A[q] = B[q];
	}
    } // end merge

void insertionSort(int *A, int N, int M) {
	if ( M <= N ) return;
	int k, i;
	for ( k = N+1; k <= M; k++ ) {
	    int ak = A[k];
	    int notInserted = 1;
	    for ( i = k-1; N <= i; i-- ) {
		int ai = A[i];
		if ( ai <= ak ) {
		    A[i+1] = ak; 
		    notInserted = 0;
		    break;
		}
		A[i+1] = ai;
	    }
	    if ( notInserted ) A[N] = ak;
	}
	// for (int i = N; i <= M; i++) System.out.println(i + " " + A[i]);
    } // end insertionSort


struct args {
  int *A;
  int *B;
  int N;
  int M;
};

void *sortThread2(void *ar0) {
  struct args *ar = (struct args *) ar0;
  mergeSort(ar->A, ar->B, ar->N, ar->M);
  return NULL;
} // end sortThread

#define errexit(code,str)                          \
  fprintf(stderr,"%s: %s\n",(str),strerror(code)); \
  exit(1);

void parMSort2(int *A, int *B, int N, int M) {
  int L = M - N;
  int k = N + (L>>1); // N + L/2; // (N + M)/ 2;
  struct args leftArgs = {A, B, N, k};
  struct args *lA = &leftArgs;
  struct args rightArgs = {A, B, k+1, M};
  struct args *rA = &rightArgs;
  pthread_t threadLeft, threadRight;
  int iret1, iret2;
  if ( (iret1=pthread_create(&threadLeft, NULL, 
			     sortThread2, lA)) ) {
      errexit(iret1,"parMSort2()/pthread_create");
  }
  if ( (iret2=pthread_create(&threadRight, NULL, 
			     sortThread2, rA)) ) {
      errexit(iret2,"parMSort2()/pthread_create");
  }
  if ( (iret1=pthread_join(threadLeft, NULL)) ) {
      errexit(iret1,"parMSort2()/pthread_join");
  }
  if ( (iret2=pthread_join(threadRight, NULL)) ) {
      errexit(iret2,"parMSort2()/pthread_join");
  }
  merge(A, B, N, M);
} // end parMSort2

void parMSort2();
void parMergeSort2(int *A, int N, int M) {
  int lng = M - N + 1;
  if ( lng < 1000 ) { mergeSort0(A, N, M); return; }
  int *B = malloc (sizeof(int) * lng);
  parMSort2(A, B, N, M);
  free(B);
} // end parmergeSort2

void *sortThread4(void *ar0) {
  struct args *ar = (struct args *) ar0;
  parMSort2(ar->A, ar->B, ar->N, ar->M);
  return NULL;
} // end sortThread

void parMSort4(int *A, int *B, int N, int M) {
  int L = M - N;
  int k = N + (L>>1); // N + L/2; // (N + M)/ 2;
  struct args leftArgs = {A, B, N, k};
  struct args *lA = &leftArgs;
  struct args rightArgs = {A, B, k+1, M};
  struct args *rA = &rightArgs;
  pthread_t threadLeft, threadRight;
  int iret1, iret2;
  if ( (iret1=pthread_create(&threadLeft, NULL, 
			     sortThread4, lA)) ) {
      errexit(iret1,"parMSort4()/pthread_create");
  }
  if ( (iret2=pthread_create(&threadRight, NULL, 
			     sortThread4, rA)) ) {
      errexit(iret2,"parMSort4()/pthread_create");
  }
  if ( (iret1=pthread_join(threadLeft, NULL)) ) {
      errexit(iret1,"parMSort4()/pthread_join");
  }  if ( (iret2=pthread_join(threadRight, NULL)) ) {
      errexit(iret2,"parMSort4()/pthread_join");
  }
  merge(A, B, N, M);
} // end parMSort4

void parMergeSort4(int *A, int N, int M) {
  int lng = M - N + 1;
  if ( lng < 10000 ) { mergeSort0(A, N, M); return; }
  int *B = malloc (sizeof(int) * lng);
  parMSort4(A, B, N, M);
  free(B);
} // end parmergeSort4


// ------------------------- mergeSortDDC ('inplace') --------------------
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
	if ( L <= 10 ) { insertionSort(A, N, M); return; }
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


// Here the pthread infrastructure for cut4p
/* Infrastructure for the parallel equipment:
  Addded the array field A
  To obtain the int n field from X: ((struct task *) X)->n
  To obtain the int m field from X: ((struct task *) X)->m
  To obtain the task next field from X: ((struct task *) X)->next
  */

/* The version for objects/referenced items has also infrastructure 
   for doing the first partitioning in parallel with two threads. */

void *myMallocSS(char* location, int size) {
  void *pp = malloc(size);
  if ( NULL == pp ) {
    fprintf(stderr, "malloc fails for: %s\n", location);
    // printf("Cannot allocate memory in: %s\n", location);
    exit(1);
  }
  return pp;
} // end of myMalloc

struct task {
  int *A;
  int n;
  int m;
  int dl;
  struct task *next;
};
int *getA(struct task *t) { return ((struct task *) t)->A; }
int getN(struct task *t) { return ((struct task *) t)->n; }
int getM(struct task *t) { return ((struct task *) t)->m; }
int getDL(struct task *t) { return ((struct task *) t)->dl; }
struct task *getNext(struct task *t) { return ((struct task *) t)->next; }

void setA(struct task *t, int *A) { ((struct task *) t)->A = A; }
void setN(struct task *t, int n) { ((struct task *) t)->n = n; }
void setM(struct task *t, int m) { ((struct task *) t)->m = m; }
void setDL(struct task *t, int dl) { ((struct task *) t)->dl = dl; }
void setNext(struct task *t, struct task* tn) { 
  ((struct task *) t)->next = tn; }
struct task *newTask(int *A, int N, int M, int depthLimit) {
  struct task *t = (struct task *) 
    myMallocSS("ParFourSort/ newTask()", sizeof (struct task));
  setA(t, A);
  setN(t, N); setM(t, M); setDL(t, depthLimit); setNext(t, NULL);
  return t;
} // end newTask

struct stack {
  struct task *first;
  int size;
};
struct task *getFirst(struct stack *ll) { 
  return ((struct stack *) ll)->first; }
int getSize(struct stack *ll) { 
  return ((struct stack *) ll)->size; }
void setFirst(struct stack *ll, struct task *t) { 
  ((struct stack *) ll)->first = t; }
void setSize(struct stack *ll, int s) { 
  ((struct stack *) ll)->size = s; }
void incrementSize(struct stack *ll) { 
  setSize(ll, getSize(ll) + 1); }
void decrementSize(struct stack *ll) { 
  setSize(ll, getSize(ll) - 1); }
struct stack *newStack() {
  struct stack *ll = (struct stack *)
    myMallocSS("ParFourSort/ newStack()", sizeof (struct stack));
  setFirst(ll, NULL); setSize(ll, 0);
  return ll;
} // end newStack
int isEmpty(struct stack *ll) { 
  return ( NULL == getFirst(ll) ); 
} // end isEmpty
struct task *pop(struct stack *ll) {
  struct task *t = getFirst(ll);
  if ( NULL == t ) return NULL;
  setFirst(ll, getNext(t));
  decrementSize(ll);
  return t;
} // end pop
void push(struct stack *ll, struct task *t) {
  if ( !isEmpty(ll) ) setNext(t, getFirst(ll)); 
  setFirst(ll, t);
  incrementSize(ll);
} // end push

struct stack *ll;
// int *A;
int sleepingThreads = 0;
int NUMTHREADS;

pthread_mutex_t condition_mutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition_cond2  = PTHREAD_COND_INITIALIZER;


void addTaskSynchronized(struct stack *ll, struct task *t) {
  /*
  printf("Thread#: %ld ", pthread_self());
  printf("addTask N: %d M: %d sleepingThreads: %d\n", 
	 getN(t), getM(t), sleepingThreads);
  */
  pthread_mutex_lock( &condition_mutex2 );
  push(ll, t);
  pthread_cond_signal( &condition_cond2 );
  pthread_mutex_unlock( &condition_mutex2 );
} // end addTaskSynchronized



// threads execute sortThread
void *sortThread(void *X) { // X-argument is NOT used
  // int taskCnt = 0;
  // printf("Thread number: %ld #sleepers %d\n", 
  //         pthread_self(), sleepingThreads);
  struct task *t = NULL;
  for (;;) {
    pthread_mutex_lock( &condition_mutex2 );
       sleepingThreads++;
       while ( NULL == ( t = pop(ll) ) && 
	       sleepingThreads < NUMTHREADS ) {
	 pthread_cond_wait( &condition_cond2, &condition_mutex2 );
       }
       if ( NULL != t ) sleepingThreads--;
    pthread_mutex_unlock( &condition_mutex2 );
    if ( NULL == t ) { 
      pthread_mutex_lock( &condition_mutex2 );
      pthread_cond_signal( &condition_cond2 );
      pthread_mutex_unlock( &condition_mutex2 );
      break;
    }
    // printf("sortThread %d\n", taskCnt);
    int *A = getA(t);
    int n = getN(t);
    int m = getM(t);
    int depthLimit = getDL(t);
    free(t);
    // taskCnt++;
    cut4pc(A, n, m, depthLimit);
  }

  //  printf("Exit of Thread number: %ld taskCnt: %d\n", pthread_self(), taskCnt);
  return NULL;
} // end sortThread

void sixsort();
void callSixSort(int *A, int N, int M) {
  // printf("callSixSort %d %d \n", N, M);
  sixsort(A+N, M-N+1, 2);
}

int cut4S2Limit = 2000;
void sixsort(int *A, int size, int numberOfThreads) {
  // printf("SixSort %d %d \n", size,  numberOfThreads);
  /*
  // Set host & licence expiration date
  char* myHost = "ddc2";
  int year = 2010;
  int month = 11; // December = 11
  int day = 31;
  // show allowed host and licence expiration date
  if ( 0 == A ) {
    printf("Licence expires: %d / %d / %d\n", year, month+1, day);
    printf("Host: %s\n", myHost);
    exit(0);
  }
  // Check whether licence expired
  time_t now = time(NULL);
  struct tm *laterx = localtime(&now);
  laterx->tm_year = year-1900; // = 2010
  laterx->tm_mon = month;
  laterx->tm_mday = day;
  time_t endTime = mktime(laterx);
  if ( endTime < now ) {
     printf(expiration);
     exit(1);
  }
  // Check whether this host is allowed
  int nameLng = 100;
  char hostName[nameLng];
  int out = gethostname(hostName, nameLng);
  if ( 0 != out ) {
    printf("*** Error: cannot get: Host name\n");
    exit(1);
  }
  out = strcmp(myHost, hostName);
  if ( 0 != out ) {
    printf("*** Error: not allowed host\n");
    exit(1);
  }
  */
  // Proceed !
  if ( size <= cut4S2Limit || numberOfThreads <= 0) {
    cut2(A, 0, size-1);
    return;
  }
  sleepingThreads = 0;
  NUMTHREADS = numberOfThreads;
  // printf("Entering sixsort %d \n", size);
  ll = newStack();
  int depthLimit = 2.5 * floor(log(size));
  struct task *t = newTask(A, 0, size-1, depthLimit);
  addTaskSynchronized(ll, t);

  pthread_t thread_id[NUMTHREADS];
  int i;
  int errcode;
  for ( i = 0; i < NUMTHREADS; i++ ) {
    if ( (errcode=pthread_create(&thread_id[i], NULL, 
				 sortThread, (void*) A)) ) {
      errexit(errcode,"ParSort/ sortArray()/pthread_create");
    }
  }
  // printf("Waiting sixsort\n");
  for ( i = 0; i < NUMTHREADS; i++ ) {
    if ( (errcode=pthread_join(thread_id[i], NULL) )) {
      errexit(errcode,"ParSort/ sortArray()/pthread_join");
    }
  }
  free(ll);
  // printf("Exiting sixsort\n");
} // end sixsort


// Here the parallel version of cut4
void cut4p(int *A, int N, int M)
{
  // printf("cut4p %d %d \n", N, M);
  int L = M - N; 
  if ( L < cut4Limit ) {
    cut2f(A, N, M);
    return; 
  }
  int depthLimit = 2.5 * floor(log(L));
  cut4pc(A, N, M, depthLimit);
} // end cut4p

void cut4pc(int *A, int N, int M, int depthLimit) 
{
 Start:
  // printf("cut4pc %d %d %d \n", N, M, depthLimit);
  if ( depthLimit <= 0 ) {
    heapc(A, N, M);
    return;
  }
  int L = M - N; 

  if ( L < cut4Limit ) {
    cut2fc(A, N, M, depthLimit);
    return; 
  }
  depthLimit--;

  int maxl, middle, minr;   // pivots for left/ middle / right regions

  int probeLng = L/ probeParamCut3x;
  if ( probeLng < 20 ) probeLng = 20; // quite short indeed
  int halfSegmentLng = probeLng >> 1; // probeLng/2;
  int N1 = N + (L>>1) - halfSegmentLng;
  int M1 = N1 + probeLng - 1;
  int quartSegmentLng = probeLng >> 2; // probeLng/4;
  int maxlx = N1 + quartSegmentLng;
  int middlex = N1 + halfSegmentLng;
  int minrx = M1 - quartSegmentLng;
  int mrx = middlex + (quartSegmentLng>>1);
  int offset = L/probeLng;

  int k;

  for (k = 0; k < probeLng; k++) iswap(N1 + k, N + k * offset, A);
  //  cut2(A, N1, M1);
  quicksort0(A, N1, M1);

  if ( A[maxlx] == A[middlex] || 
       A[middlex] == A[mrx] == 0 || 
       A[mrx] == A[minrx] == 0 ) {
    // no good pivots available, thus escape
    dflgm(A, N, M, middlex, cut4c, depthLimit);
    return;
  }

  int x, y; // values  
  int hole;

  iswap(N+1, maxlx, A); maxl = A[N+1]; // left pivot
  iswap(M, minrx, A); minr = A[M]; // right pivot
  x = A[N]; // 1st roving variable element
  middle = A[N] = A[middlex]; // middle pivot
  iswap(middlex+1, mrx, A); // init MR

  register int i, j, lw, up, z; // indices

  // Here the general layout

   /*   L             ML         MR             R
    |-----]------+[---------]---------]+-----[-----|
    N     i      lw         z         up     j     M
   */
  
  /* There are invariants to be maintained (which are >essential< 
     for machine assisted correctness proofs):
     maxl < middle < minr
     If there are two gaps:
       N < x <= i --> A[x] <= maxl
       lw < x <= z  --> maxl < A[x] <= middle
       z < x < up  --> middle < A[x] < minr
       j <= x <= M --> minr <= A[x]
     If the left gap has closed:
       N < x < i --> A[x] <= maxl
       i <= x <= z --> maxl < A[x] <= middle
       z <  x < up  --> middle < A[x] < minr
       j <= x <= M --> minr <= A[x]
     If the right gap has closed:
       N < x <= i --> A[x] <= maxl
       lw < x <= z  --> maxl < A[x] <= middle
       z < x <= j  --> middle < A[x] < minr
       j < x <= M --> minr <= A[x]
  */

  /* We employ again whack-a-mole. We know in which partition element x 
     should be.  Find a close, undecided position where x should go.  
     Store its content in y.  Drop x (simplified).  Set x to y and repeat. 
   */
  // Ready to roll ...
  i = N+1; j = M; z = middlex; lw = z-1; up = z+2; hole = N;

   /*   L             ML         MR             R
    o-----]------+[---------]---------]+-----[-----|
    N     i      lw         z         up     j     M
   */

  if ( x <= middle ) {
    if ( x <= maxl ) goto TLgMLMRgRL;
    goto TLgMLMRgRML;
  }
  // middle < x
  if ( x < minr ) goto TLgMLMRgRMR;
    goto TLgMLMRgRR;

 TLgMLMRgRL:
         /*   L                              R
	   |o----]---------+[-|-]+--------[-----|
	   N     i        lw     up       j     M
           x -> L             z
	 */
	i++;
	y = A[i];
	if ( y <= middle ) {
	  if ( y <= maxl ) goto TLgMLMRgRL;
	  // y -> ML
	  if ( lw < i ) {
	 /*   L                              R
	   |o----][-----------|-]+--------[-----|
	   N      i           z  up       j     M
           x -> L             
	 */
	    goto TLMLMRgRL;
	  }
	  A[i] = x;
	  x = y;
	  goto TLgMLMRgRML;
	}
	// middle < y
	A[i] = x;
	x = y;
	if ( minr <= y ) { 
	  goto TLgMLMRgRR;
	}
	goto TLgMLMRgRMR;

 TLgMLMRgRML:
         /*   L                              R
	   |o----]---------+[-|-]+--------[-----|
	   N     i        lw     up       j     M
           x -> ML            z
	 */
	y = A[lw];
	if ( y <= middle ) {
	  if ( y <= maxl ) {
	    if ( i < lw ) {
	      A[lw] = x;
	      x = y;
	      lw--;
	      goto TLgMLMRgRL;
	    }
	    // left gap closed
	    i++;
         /*   L                              R
	   |o----][-----------|-]+--------[-----|
	   N      i              up       j     M
           x -> ML            z
	 */
	    goto TLMLMRgRML;
	  }
	  // y -> ML
	  lw--;
	  goto TLgMLMRgRML;
	}
	// middle < y
	A[lw] = x;
	x = y;
	lw--;
	if ( minr <= y ) { 
	  goto TLgMLMRgRR;
	}
	goto TLgMLMRgRMR;

 TLgMLMRgRR:
         /*   L                              R
	   |o----]---------+[-|-]+--------[-----|
	   N     i        lw     up       j     M
           x -> R             z
	 */
	j--;
	y = A[j];
	if ( y <= middle ) {
	  A[j] = x;
	  x = y;
	  if ( y <= maxl ) { 
	    goto TLgMLMRgRL;
	  }
	  goto TLgMLMRgRML;
	}
	// middle < y
	if ( minr <= y ) {
	  goto TLgMLMRgRR;
	}
	// y -> MR
	if ( up <= j ) {
	  A[j] = x;
	  x = y;
	  goto TLgMLMRgRMR;
	}
	// right gap closed
         /*   L                           R
	   |o----]---------+[-|-------][-----|
	   N     i        lw          j      M
           x -> R             z
	 */
	goto TLgMLMRRR;

 TLgMLMRgRMR:
         /*   L                              R
	   |o----]---------+[-|-]+--------[-----|
	   N     i        lw     up       j     M
           x -> MR            z
	 */
	y = A[up];
	if ( y <= middle ) {
	  A[up] = x;
	  x = y;
	  up++;
	  if ( y <= maxl ) { 
	    goto TLgMLMRgRL;
	  }	  
	  goto TLgMLMRgRML;
	}
	// middle < y
	if ( y < minr ) {
	  up++;
	  goto TLgMLMRgRMR;
	}
	// y -> R
	if ( up < j ) {
	  A[up] = x;
	  x = y;
	  up++;
	  goto TLgMLMRgRR;
	}
	// right gap closed
	j--;
         /*   L                           R
	   |o----]---------+[-|-------][-----|
	   N     i        lw          j      M
           x -> MR            z
	 */
	goto TLgMLMRRMR;


	// right gap closed
 TLgMLMRRR:
         /*   L                           R
	   |o----]---------+[-|-------][-----|
	   N     i        lw          j      M
           x -> R             z
	 */
	y = A[lw];
	if ( y <= middle ) {
	  if ( y <= maxl ) {
	    if ( i < lw ) {  // shift ML&MR to the left
	      A[lw] = A[z]; A[z] = A[j]; A[j] = x;
	      x = y;
	      lw--; z--; j--; 
	      goto TLgMLMRRL;
	    }
	    // left gap also closed; shift ML&MR to the left
	    A[hole] = A[i]; A[i] = A[z]; A[z] = A[j]; A[j] = x;
	    i--; z--; 
	    goto Finish4;
	  }
	  // y -> ML
	  lw--;
	  goto TLgMLMRRR;
	}
	// middle < y
	if ( minr <= y ) { // shift ML&MR to the left
	  A[lw] = A[z]; A[z] = A[j]; A[j] = y;
	  lw--; z--; j--; 
	  goto TLgMLMRRR;
	}
	// y -> MR; shift ML to the left
	A[lw] = A[z]; A[z] = y;
	lw--; z--; 
	goto TLgMLMRRR;

 TLgMLMRRMR:
         /*   L                           R
	   |o----]---------+[-|-------][-----|
	   N     i        lw          j      M
           x -> MR            z
	 */
	y = A[lw];
	if ( y <= middle ) {
	  if ( y <= maxl ) {
	    if ( i < lw ) {
	      A[lw] = A[z]; A[z] = x;
	      x = y;
	      z--; lw--;
	      goto TLgMLMRRL;
	    }
	    // left gap also closed
	    A[hole] = y; A[lw] = A[z]; A[z] = x;
	    i--; z--; j++;
	    goto Finish4;
	  }
	  // y -> ML
	  lw--;
	  goto TLgMLMRRMR;
	}
	// middle < y
	if ( minr <= y ) {
	  A[lw] = A[z]; A[z] = A[j]; A[j] = y;
	  z--; j--; lw--;
	  goto TLgMLMRRMR;
	}
	// y -> MR
	A[lw] = A[z]; A[z] = y;
	z--; lw--;
	goto TLgMLMRRMR;

 TLgMLMRRML:
        /*   L                         R
	   |o----]------+[--|------][-----|
	   N     i     lw          j      M
           x -> ML          z
	 */
	y = A[lw];
	if ( y <= middle ) {
	  if ( y <= maxl ) {
	    if ( i < lw ) {
	      A[lw] = x;
	      x = y;
	      lw--;
	      goto TLgMLMRRL;
	    }
	    // left gap also closed
	    A[hole] = y; // y = A[i] = A[lw]
	    A[lw] = x;
	    i--; j++;
	    goto Finish4;
	  }
	  // y -> ML
	  lw--;
	  goto TLgMLMRRML;
	}
	// middle < y
	if ( minr <= y ) {
	  A[lw] = A[z]; A[z] = A[j]; A[j] = y;
	  z--; j--; lw--;
	  goto TLgMLMRRML;
	}
	// y -> MR
	A[lw] = A[z]; A[z] = y;
	z--; lw--;
	goto TLgMLMRRML;

 TLgMLMRRL:
         /*   L                           R
	   |o----]---------+[-|-------][-----|
	   N     i        lw          j      M
           x -> L             z
	 */
	i++;
	y = A[i];
	if ( y <= middle) {
	  if ( y <= maxl ) goto TLgMLMRRL;
	  if ( i <= lw ) {
	    A[i] = x;
	    x = y;
	    goto TLgMLMRRML;
	  }
	  // left gap closed
	  A[hole] = x;
	  i--; j++;
	  goto Finish4;
	}
	// middle < y
	A[i] = x;
	x = y;
	if ( minr <= y ) goto TLgMLMRRR;
	// y -> MR
	goto TLgMLMRRMR;

	// left gap closed
 TLMLMRgRL:
        /*   L                               R
	   |o----][-----------|-]+--------[-----|
	   N      i           z  up       j     M
           x -> L             
	 */
	y = A[up];
	if ( y <= middle ) {
	  if ( y <= maxl ) { // y -> L
	    z++;
	    A[up] = A[z]; A[z] = A[i]; A[i] = y;
	    i++; up++;
	    goto TLMLMRgRL;
	  }
	  // y -> ML
	  z++;
	  A[up] = A[z]; A[z] = y;
	  up++;
	  goto TLMLMRgRL;
	}
	// middle < y
	if ( y < minr ) {
	  up++;
	  goto TLMLMRgRL;
	}
	// y -> R
	if ( up < j ) {
	  z++;
	  A[up] = A[z]; A[z] = A[i]; A[i] = x;
	  x = y;
	  i++; up++;
	  goto TLMLMRgRR;
	}
	// right gap closed
	A[hole] = x; 
	i--;
	goto Finish4;

 TLMLMRgRML:
         /*   L                              R
	   |o----][-----------|-]+--------[-----|
	   N      i              up       j     M
           x -> ML            z
	 */
	y = A[up];
	if ( y <= middle ) {
	  if ( y <= maxl ) {
	    z++;
	    A[up] = A[z]; A[z] = A[i]; A[i] = y;
	    i++; up++;
	    goto TLMLMRgRML;
	  }
	  // y -> ML
	  z++;
	  A[up] = A[z]; A[z] = y;
	  up++;
	  goto TLMLMRgRML;
	}
	// middle < y
	if ( y < minr ) {
	  up++;
	  goto TLMLMRgRML;
	}
	// y --> R
	if ( up < j ) {
	    z++;
	    A[up] = A[z]; A[z] = x;
	    x = y;
	    up++;
	    goto TLMLMRgRR;
	}
	// right gap closed
	i--;
	A[hole] = A[i]; A[i] = x;
	i--;
	goto Finish4;
	
 TLMLMRgRR:
         /*   L                              R
	   |o----][-----------|-]+--------[-----|
	   N      i              up       j     M
           x -> R             z
	 */
	j--;
	y = A[j];
	if ( y <= middle ) {
	  A[j] = x;
	  x = y;
	  if ( y <= maxl ) {
	    goto TLMLMRgRL;
	  }
	  // y -> ML
	  goto TLMLMRgRML;
	}
	// middle < y
	if ( minr <= y ) goto TLMLMRgRR;
	// y -> MR
	if ( up <= j ) {
	  A[j] = x;
	  x = y;
         /*   L                              R
	   |o----][-----------|-]+--------[-----|
	   N      i              up       j     M
           x -> MR            z
	 */
	  goto TLMLMRgRMR;
	}
	// right gap closed
	i--;
	A[hole] = A[i]; A[i] = A[z]; A[z] = A[j]; A[j] = x;
	z--; i--;
	goto Finish4;

 TLMLMRgRMR:
         /*   L                              R
	   |o----][-----------|-]+--------[-----|
	   N      i              up       j     M
           x -> MR            z
	 */
	y = A[up];
	if ( y <= middle ) {
	  if ( y <= maxl ) {
	    z++;
	    A[up] = A[z]; A[z] = A[i]; A[i] = y;
	    i++; up++;
	    goto TLMLMRgRMR;
	  }
	  // y -> ML
	  z++;
	  A[up] = A[z]; A[z] = y;
	  up++;
	  goto TLMLMRgRMR;
	}
	// middle < y
	if ( y < minr ) {
	  up++;
	  goto TLMLMRgRMR;
	}
	// y --> R
	if ( up < j ) {
	  A[up] = x;
	  x = y;
	  up++;
	  goto TLMLMRgRR;
	}
	// right gap closed
	i--;
	A[hole] = A[i]; A[i] = A[z]; A[z] = x;
	i--; z--;
	goto Finish4;


	// +++++++++++++++both gaps closed+++++++++++++++++++
	// int lleft, lmiddle, lright;
	// int k; // for testing

 Finish4:
	// printf("cut4p exit Finish4 N: %d M: %d\n", N, M);
         /*   L        ML         MR            R
	   |-----][----------|-----------][-----|
	   N     i           z            j     M
	 */
	/*
	for  (k = N; k <= i; k++) 
	  if ( maxl < A[k] ) {
	     printf("L error k: %i\n", k); 
             printf("N: %d i %i lw %i z %i up %i j %i M: %d\n", N,i,lw,z,up,j,M);
	    exit(1);
	  }
	for  (k = i+1; k <= z; k++) 
	  if ( A[k] <= maxl || middle < A[k] ) {
	     printf("ML error k: %i\n", k); 
             printf("N: %d i %i lw %i z %i up %i j %i M: %d\n", N,i,lw,z,up,j,M);
	    exit(1);
	  }
	for  (k = z+1; k < j; k++) 
	  if ( A[k] <= middle || minr <= A[k] ) {
	     printf("MR error k: %i\n", k); 
             printf("N: %d i %i lw %i z %i up %i j %i M: %d\n", N,i,lw,z,up,j,M);
	    exit(1);
	  }
	for  (k = j; k <= M; k++) 
	  if ( A[k] < minr ) {
	     printf("R error k: %i\n", k); 
             printf("N: %d i %i lw %i z %i up %i j %i M: %d\n", N,i,lw,z,up,j,M);
	    exit(1);
	  }
	*/

         /*   L        ML         MR         R
	   |-----][----------|-----------][-----|
	   N     i           z            j     M
	 */
	if ( z-N < M-z ) {
	  /*
	  cut4pc(A, N, i, depthLimit);
	  cut4pc(A, i+1, z, depthLimit);
	  if ( j-z < M-j ) {
	    cut4pc(A, z+1, j-1, depthLimit);
	    cut4pc(A, j, M, depthLimit);
	    return;
	  }
	  cut4pc(A, j, M, depthLimit);
	  cut4pc(A, z+1, j-1, depthLimit);
	  */
	  if ( j-z < M-j ) {
	    // cut4(A, z+1, j-1);
	    // cut4(A, j, M);
	    addTaskSynchronized(ll, newTask(A, j, M, depthLimit));
	    addTaskSynchronized(ll, newTask(A, z+1, j-1, depthLimit));
	  } else {
	    addTaskSynchronized(ll, newTask(A, z+1, j-1, depthLimit));
	    addTaskSynchronized(ll, newTask(A, j, M, depthLimit));
	  }
	  addTaskSynchronized(ll, newTask(A, i+1, z, depthLimit));
	  // cut4pc(A, N, i, depthLimit);
	  // return;
	  M = i;
	  goto Start;
	}
	// M-z <= z-N
	/*
	cut4pc(A, z+1, j-1, depthLimit);
	cut4pc(A, j, M, depthLimit);
	if ( i-N < z-i ) {
	  cut4pc(A, N, i, depthLimit);
	  cut4pc(A, i+1, z, depthLimit);
	  return;
	}
	cut4pc(A, i+1, z, depthLimit);
	cut4pc(A, N, i, depthLimit);
	*/
	if ( i-N < z-i ) {
	  // cut4(A, N, i);
	  // cut4(A, i+1, z);
	  addTaskSynchronized(ll, newTask(A, i+1, z, depthLimit));
	  addTaskSynchronized(ll, newTask(A, N, i, depthLimit));
	} else{
	  addTaskSynchronized(ll, newTask(A, N, i, depthLimit));
	  addTaskSynchronized(ll, newTask(A, i+1, z, depthLimit));
	}
	addTaskSynchronized(ll, newTask(A, j, M, depthLimit));
	// cut4pc(A, z+1, j-1, depthLimit);
	N = z+1; M = j-1;
	goto Start;
} // end cut4pc

