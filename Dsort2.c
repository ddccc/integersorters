// File: c:/bsd/rigel/sort/integersorters/Dsort2.c
// Date: Sat Dec 14 17:58:42 2024
// (C) OntoOO/ Dennisd de Champeaux

/*
This is an alternative to Dutch Flag dflgm in Dsort.c.
Amazingly short, but not faster.
The array layout is here asymmetric, hence the preference for dflgm. 
 */

#define iswap(p, q, A) { int t3t = A[p]; A[p] = A[q]; A[q] = t3t; }

void dflgm2(int *A, int lo, int hi, int pivotx, void (*cut)(), int depthLimit) {
  // printf("dflgm2 N %i M %i pivotx %i depthLimit %i\n", N,M,pivotx,depthLimit);
  /*
    Simple version of partitioning with: L/M/R
    L < pivot, M = pivot, R > pivot
    This is the integer version.  Code for the object/record version 
    has been commented out. 
         L    M                 R
      |*---)----)------------(----|
     lo    i    j            k    hi
   */
  register int i, j, k; // indices
  register int p3; // pivot

i = lo; j = i; k = hi; 
  p3 = A[pivotx];

    /*   L    M                 R
      |*---)----)------------(----|
     lo    i    j            k    hi
    */

  while ( j <= k ) {
    // int r = compareXY(A[j], p3);
    // if ( r < 0 ) {
    if ( A[j] < p3 ) {
      iswap(i, j, A);
      i++; j++;
    } else 
      // if ( 0 < r) {
      if ( p3 < A[j] ) {
	iswap(j, k, A);
	k--;
      } else j++;
  }
 
    if ( i - lo  < hi - k ) {
      if ( lo < i ) (*cut)(A, lo, i-1, depthLimit);
      if ( k < hi ) (*cut)(A, k+1, hi, depthLimit);
      return;
    }
    if ( k < hi ) (*cut)(A, k+1, hi, depthLimit);
    if ( lo < i ) (*cut)(A, lo, i-1, depthLimit);
  
} // end of dflgm2

#undef iswap
