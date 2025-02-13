// c:/bsd/rigel/sort/integersorters/Qusort.c
// Date: Fri Jan 31 13:32:12 2014/ Tue May 19 15:02:00 2015, 2017
// Fri Nov 27 20:13:42 2020, Fri Dec 13 10:10:41 2024
// (C) OntoOO/ Dennis de Champeaux

#define iswap(p, q, A) { int t3t = A[p]; A[p] = A[q]; A[q] = t3t; }

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

#undef iswap
