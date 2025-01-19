// c:/bsd/rigel/sort/integersorters/D3sort.c
// Date: Thu Dec 12 17:30:16 2024
// (C) OntoOO/ Dennis de Champeaux

#define iswap(p, q, A) { int t3t = A[p]; A[p] = A[q]; A[q] = t3t; }

// calculate the median of 3
static int med(int *A, int a, int b, int c ) {
  return
    /*
    compareXY( A[a], A[b] ) < 0 ?
    ( compareXY( A[b], A[c] ) < 0 ? b : compareXY( A[a], A[c] ) < 0 ? c : a)
    : compareXY( A[b], A[c] ) > 0 ? b : compareXY( A[a], A[c] ) > 0 ? c : a;
    */
  A[a] < A[b] ?
    ( A[b] < A[c] ? b : A[a] < A[c] ? c : a ) :
    A[b] > A[c] ? b : A[a] > A[c] ? c : a;
} // end med

void dflgm3();
// dflgm0 is used as the driver of a sorter using dflgm3
void dflgm0(int *A, int lo, int hi) {
    // printf("dflgm0 %d %d %d\n", lo, hi, hi-lo);
  int L = hi - lo;
  if ( L <= 0 ) return;
  int depthLimit = 1 + 2.9 * floor(log(L));
  dflgm3(A, lo, hi, depthLimit);
}
void dflgm3(int *A, int lo, int hi, int depthLimit) {
    // Simple version of partitioning with: L/M/R
    // L < pivot, M = pivot, R > pivot
    // dflgm is the unique workhorse
  int L;
  // Printf("dflgm3 lo %i hi %i depthLimit %i\n", lo,hi,depthLimit);
  L = hi - lo +1;
  if ( L <= 1 ) return;
  if ( L < 70) { // param may depend on machine
    insertionsort(A, lo, hi);
    return;
  }
  if ( depthLimit <= 0 ) { // guarantee O(nlogn)
    heapc(A, lo, hi);
    return;
  }
  depthLimit--;
  // code from Bentley & McIlroy ?
    int p0 = lo + (L>>1); // lo + L/2;
    int px = p0;
    if ( 7 < L ) {
      int pn = lo;
      int pm = hi;
      if ( 40 < L ) {
	int d = (L-2)>>3; // L/8;
	pn = med(A, pn, pn + d, pn + 2 * d);
	p0 = med(A, p0 - d, p0, p0 + d);
	pm = med(A, pm - 2 * d, pm - d, pm);
      }
      p0 = med(A, pn, p0, pm);
    }
    if ( p0 != px ) iswap(p0, px, A);
    // invoke dflgm and recurse again with dflgm3 here
    dflgm(A, lo, hi, px, dflgm3, depthLimit);
    // dflgm2(A, lo, hi, px, dflgm3, depthLimit);
} // end dflgm3

#undef iswap
