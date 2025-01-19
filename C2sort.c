// c:/bsd/rigel/sort/integersorters/C2sort.c
// Date: Fri Jan 31 13:32:12 2014, 2017 Sun Mar 03 16:14:28 2019, Sat Jul 23 13:05:48 2022
// Mon Jan 04 10:43:49 2021, Sun May 30 14:48:25 2021 Thu Jul 14 16:05:59 2022
// Fri Dec 13 09:40:21 2024
// (C) OntoOO/ Dennis de Champeaux

#define iswap(p, q, A) { int t3t = A[p]; A[p] = A[q]; A[q] = t3t; }

// This version combines isort + dflgm + ( pivot sample + (fast loops | dflgm ) )


static const int dflgmLimit2 = 200; // 5626


void cut2c(); // is called also
// cut2 is used as a best in class quicksort implementation 
// with a defense against quadratic behavior due to duplicates
// cut2 is a support function to call up the workhorse cut2c
void cut2(int *A, int lo, int hi) { 
  // printf("cut2 %d %d %d\n", lo, hi, hi-lo);
  int L = hi - lo;
  if ( L <= 0 ) return;
  int depthLimit = 2.9 * floor(log(L));
  cut2c(A, lo, hi, depthLimit);
} // end cut2

void cut2c(int *A, int lo, int hi, int depthLimit) {
  // printf("Enter cut2c lo: %d hi: %d %d\n", lo, hi, depthLimit);
  // printf(" gap %d \n", hi-lo);

  while ( lo < hi ) {
    // printf("cut2c lo: %d hi %d  L %i\n", lo, hi, hi-lo);
    int L = hi - lo;
    if ( depthLimit <= 0 ) {
      heapc(A, lo, hi);
      return;
    }
    depthLimit--;

    if ( L <= dflgmLimit2 ) {
      dflgm3(A, lo, hi, depthLimit);
      return;
    }

    register int T; // pivot
    register int I = lo, J = hi; // indices
    int middlex = lo + (L>>1); // lo + L/2
    int middle;
    int k, lo1, hi1; // for sampling
    int probeLng = sqrt(L/7.0); if ( probeLng < 9 ) probeLng = 9;
    // int probeLng = 9;
    int halfSegmentLng = probeLng >> 1; // probeLng/2;
    lo1 = middlex - halfSegmentLng; //  lo + (L>>1) - halfSegmentLng;
    hi1 = lo1 + probeLng - 1;
    int offset = L/probeLng;  

    // assemble the mini array [lo1, hi1]
    for (k = 0; k < probeLng; k++) // iswap(lo1 + k, lo + k * offset, A);
      { int xx = lo1 + k, yy = lo + k * offset; iswap(xx, yy, A); }
    // sort this mini array to obtain good pivots
    // quicksort0c(A, lo1, hi1, depthLimit, compareXY);
    cut2c(A, lo1, hi1, depthLimit);
    T = middle = A[middlex];
    // if ( compareXY(A[hi1], middle) <= 0 ) {
    if ( A[hi1] <= middle ) {
      // give up because cannot find a good pivot
      // dflgm is a dutch flag type of algorithm
      dflgm(A, lo, hi, middlex, cut2c, depthLimit);
      return;
    }
    // swap the sorted segents to the corners
    for ( k = lo1; k <= middlex; k++ ) {
      iswap(k, I, A); I++;
    }
    I--;
    for ( k = hi1; middlex < k; k--) {
      iswap(k, J, A); J--;
    }
    J++;
 
    // get the ball rolling::
    register int AI, AJ; // array values
    // The left segment has elements <= T
    // The right segment has elements >= T
  Left:
    // quicksortdoing its thing 
    // while ( compareXY(A[++I], T) <= 0 ); 
    while ( A[++I] <= T );
    AI = A[I];
    // while ( compareXY(T, A[--J]) < 0 ); 
    while ( T < A[--J] );
    AJ = A[J];
    if ( I < J ) { // swap
      A[I] = AJ; A[J] = AI;
      goto Left;
    }
    // Tail iteration
    if ( (I - lo) < (hi - J) ) { // smallest one first
      cut2c(A, lo, J, depthLimit);
      lo = I; 
    } else {
      cut2c(A, I, hi, depthLimit);
      hi = J;
    }
  } // end while
} // (*  OF cut2; *) the brackets remind that this was once, 1985, Pascal code

#undef iswap
