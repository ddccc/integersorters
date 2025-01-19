// File: c:/bsd/rigel/sort/integersorters/MergeSort.c
// Date: Fri Dec 27 06:51:44 2024
//      File: c:/bsd/rigel/sort/MergeSort.c
//      Date: Wed Apr 07 20:51:35 2021
// (C) OntoOO/ Dennis de Champeaux


static void mergeSortc();
void mergeSort(int *A, int lo, int hi) {
  // printf("mergeSort-- lo: %i hi %i\n ", lo, hi);
  if ( hi <= lo ) return;
  int lng = hi - lo + 1;
  // void *B[lng];
  // void *B = malloc(lng*sizeof(void));
  // struct intval *pi;
  // void *B = malloc(lng*sizeof(pi));
  void *B = malloc(lng*sizeof(A[0]));
  if ( NULL == B ) {
    cut2lr(A, lo, hi);
    return;
  }
  mergeSortc(A, B, lo, hi);
  free(B);
} // end mergeSort

static void merge(); void checkMS();
void mergeSortc(int *A, int *B, int lo, int hi) {
  // printf("mergeSort lo: %i hi %i\n ", lo, hi);
	int L = hi-lo;
	/*
      	if ( L <= 10 ) { 
	  insertionsort(A, lo, hi); 
	  return; 
	}
	*/
	//  /*
	if ( L < 250 ) {
	  int depthLimit = 2.9 * floor(log(L));
	  dflgm3(A, lo, hi, depthLimit);
	  // quicksort0(A, lo, hi, depthLimit);
	  return;
	}
	// */
	int k = L>>1; // L/2;
	mergeSortc(A, B, lo, lo+k); 
	mergeSortc(A, B, lo+k+1, hi);
	// if ( compare(A[lo+k], A[lo+k+1]) > 0 ) 
	if ( A[lo+k] > A[lo+k+1] ) 
	     merge(A, B, lo, hi);
	// checkMS(A, lo, hi);
    } // end mergeSortc

void checkMS(int *A, int lo, int hi) {
  int i;
  for ( i = lo+1; i <= hi; i++ )
    // if ( compare(A[i], A[i-1]) < 0 ) {
    if ( A[i]< A[i-1] ) {
      printf("mergesort check lo: %i hi: %i err i: %i\n", lo,hi,i);
      exit(0);
    }
} // end checkMS

void merge(int *A, int *B, int lo, int hi) {
  // printf("merge lo: %i hi %i\n ", lo, hi);
	int L = hi-lo;
	int k = L>>1; // L/2;
	int i = lo; int j = lo+k+1;  
	int z = 0; // index for B
	int leftDone = 0;
	int rightDone = 0;
	int ai = A[i]; int aj = A[j];
	while (1) {
	  while ( ai <= aj ) {
	  // while ( compare(ai, aj) <= 0 ) {
	    B[z] = ai; z++;
	    if ( i < lo+k ) { i++; ai = A[i]; } else {
	      leftDone = 1; break;
	    }
	  }
	  if ( leftDone ) break;
	  // aj < ai
	  B[z] = aj; z++;
	  if ( hi == j ) { rightDone = 1; break; }
	  j++; aj = A[j];
	  while ( aj < ai ) {
	    // while ( compare(aj, ai) < 0 ) {
	    B[z] = aj; z++;
	    if ( j < hi ) { j++; aj = A[j]; } else {
	      rightDone = 1; break;
	    }
	  }
	  if ( rightDone ) break;
	  // ai <= aj
	  B[z] = ai; z++;
	  if ( i == lo+k ) { leftDone = 1; break; }
	  i++; ai = A[i];
	}

	int q;
	if ( leftDone ) {
	  // printf("leftDone z: %i \n", z);
	  for ( q = lo; q < z+lo; q++ ) A[q] = B[q-lo];
	} else { // rightDone
	  int k2 = L-k;
	  for ( q = lo+k; i <= q; q-- ) A[q + k2] = A[q];
	  for ( q = lo; q < z+lo; q++ ) A[q] = B[q-lo];
	}

    } // end merge
