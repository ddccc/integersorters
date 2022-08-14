// File: c:/bsd/rigel/sort/DPQI.c
// Date: Sat Feb 14 07:31:00 2015
// (C) OntoOO/ Dennis de Champeaux

/* 
Comparing DPQ against our quicksort
*/


#include <stdio.h>
#include <stddef.h>
#include <math.h>
#include <stdlib.h>
// #include <pthread.h>
// #include <sys/time.h>

int cut2Limit = 127;
int cut3Limit = 250;
int probeParamCut4 = 1000000;
int cut4Limit = 375; 

#define errexit(code,str)                          \
  fprintf(stderr,"%s: %s\n",(str),strerror(code)); \
  exit(1);

void quicksort0();
void cut2();
void tps();
void cut4();
void dpq();
void cc();

main (int argc, char *argv[]) {
  printf("Running DPQI ...\n");
  cc("testing quicksort0", quicksort0);
  cc("testing cut2", cut2);
  cc("testing tps", tps);
  cc("testing cut4", cut4);
  cc("testing dpq", dpq);
} // end main


void fillarray(int *A, int lng, int startv) {
  int i;
  srand(startv);
  for ( i = 0; i < lng; i++) A[i] = rand(); 
} // end of fillarray

int clocktime;

void countclock(char* label, int siz, void (*alg1)(), int seed) {
  printf("%s on size: %d ", label, siz);
  int *A = malloc (sizeof(int) * siz);
  fillarray(A, siz, seed); // warm up
  int reps = 3;
  int TFill = clock();
  int i;
  for (i = 0; i < reps; i++) fillarray(A, siz, seed);
  TFill = clock() - TFill;
  int T = clock();
  for (i = 0; i < reps; i++) { 
    // fill its content
    fillarray(A, siz, seed);
    // sort it
    (*alg1)(A, 0, siz-1);
  }
  int algTime = (clock() - T - TFill)/reps;
  printf("time %d \n", algTime);
  clocktime += algTime;
  // free array
  free(A);
} // end countcomparisons

void cc(char* label, void (*alg1)()) {
  //  printf("cc %s\n", label);
  // int size = 1024*1024;
  int size = 1024*1024*16;
  // int repeat = 0;
  int repeat = 4;
  int reps = 3;
  int i;
  while ( repeat < 5) {
    clocktime = 0;
    for ( i = 0; i < reps; i++)
      countclock(label, size, alg1, i);
    printf("size %i clocktime %i\n\n",
	   size, clocktime/reps);
    size *= 2; repeat++;
  }
} // end cc

// insertionsort for small segments where the loop has been unrolled
void insertionSortLoopUnroll(int *A, int N, int M) {
  if ( M <= N ) return;
  int i, S, minimumPosition;
  int minimum, NEXT, AI;
  // find minimum  ...
  minimumPosition = N;
  minimum = A[N];
  for ( i = N+1; i <= M; i++ ) {
    AI = A[i];
    if ( AI < minimum ) {
      minimum = AI;
      minimumPosition = i;
    }
  }
  // ... and put it at loc N
  if ( N != minimumPosition ) {
    A[minimumPosition] = A[N];
    A[N] = minimum;
  }
  S = N+1;
  if ( M == S ) return;
  while ( S < M ) {
    S=S+1;
    NEXT=A[S];
    AI = A[S-1];
    if ( AI <= NEXT ) continue;
    A[S] = AI;
    AI = A[S-2];
    if ( AI <= NEXT ) { A[S-1] = NEXT; continue; }
    A[S-1] = AI;
    AI = A[S-3];
    if ( AI <= NEXT ) { A[S-2] = NEXT; continue; }
    A[S-2] = AI;
    AI = A[S-4];
    if ( AI <= NEXT ) { A[S-3] = NEXT; continue; }
    A[S-3] = AI;
    AI = A[S-5];
    if ( AI <= NEXT ) { A[S-4] = NEXT; continue; }
    A[S-4] = AI;
    AI = A[S-6];
    if ( AI <= NEXT ) { A[S-5] = NEXT; continue; }
    A[S-5] = AI;
    AI = A[S-7];
    if ( AI <= NEXT ) { A[S-6] = NEXT; continue; }
    A[S-6] = AI;
    AI = A[S-8];
    if ( AI <= NEXT ) { A[S-7] = NEXT; continue; }

    A[S-7] = AI;
    AI = A[S-9];
    if ( AI <= NEXT ) { A[S-8] = NEXT; continue; }
    A[S-8] = AI;
    AI = A[S-10];
    if ( AI <= NEXT ) { A[S-9] = NEXT; continue; }
    A[S-9] = AI;
    AI = A[S-11];
    if ( AI <= NEXT ) { A[S-10] = NEXT; continue; }
    A[S-10] = AI;
    AI = A[S-12];
    if ( AI <= NEXT ) { A[S-11] = NEXT; continue; }
    A[S-11] = AI;
    AI = A[S-13];
    if ( AI <= NEXT ) { A[S-12] = NEXT; continue; }
    A[S-12] = AI;
    AI = A[S-14];
    if ( AI <= NEXT ) { A[S-13] = NEXT; continue; }
    A[S-13] = AI;
    AI = A[S-15];
    if ( AI <= NEXT ) { A[S-14] = NEXT; continue; }
    A[S-14] = AI;
    AI = A[S-16];
    if ( AI <= NEXT ) { A[S-15] = NEXT; continue; }
    A[S-15] = AI;
    AI = A[S-17];
    if ( AI <= NEXT ) { A[S-16] = NEXT; continue; }
    A[S-16] = AI;
    AI = A[S-18];
    if ( AI <= NEXT ) { A[S-17] = NEXT; continue; }
    A[S-17] = AI;
    AI = A[S-19];
    if ( AI <= NEXT ) { A[S-18] = NEXT; continue; }
    A[S-18] = AI;
    AI = A[S-20];
    if ( AI <= NEXT ) { A[S-19] = NEXT; continue; }
    A[S-19] = AI;
    AI = A[S-21];
    if ( AI <= NEXT ) { A[S-20] = NEXT; continue; }

    printf("%s %d %s %d %s %d", "N: ", N, " M: ", M, " S: ", S);
    printf(" Error in insertionSortLoopUnroll\n");
  }
} // end insertionSortLoopUnroll

void handbookWithInsertionsortc();
void quicksort0(int *A, int N, int M) {
  int L = M - N;
  if ( L <= 0 ) return;
  // printf("quicksort0 %d %d \n", N, M);
  if ( L <= 10 ) { 
    insertionSortLoopUnroll(A, N, M);
    return;
  }
  int depthLimit = 2.5 * floor(log(L));
  handbookWithInsertionsortc(A, N, M, depthLimit);
} // end handbookWithInsertionsort

// calculate the median of 3
int med(int *A, int a, int b, int c) {
  return A[a] < A[b] ?
    (A[b] < A[c]  ? b : A[a] < A[c] ? c : a)
    : (A[b] > A[c] ? b : A[a] > A[c] ? c : a);
} // end med

void iswap(int p, int q, int *x) {
  int t = x[p];
  x[p] = x[q];
  x[q] = t;
} // end of iswap

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

// Quicksort  equipped with a defense against quadratic explosion;
// calling heapsort if depthlimit exhausted
void handbookWithInsertionsortc(int *A, int N, int M, int depthLimit) {
  // int k;
  register int i, j; // indices
  register int ai, aj; // values
  while ( M > N ) {
    // printf("HB enter N %i M %i depthLimit %i\n", N, M, depthLimit);
    int L = M - N;
    if ( L <= 10 ) {
      insertionSortLoopUnroll(A, N, M);
      return;
    }
    if ( depthLimit <= 0 ) {
      heapc(A, N, M);
      return;
    }
    depthLimit--;
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
    iswap(N, pm, A); // ... and is put in first position as required by handbook

    i = N;
    j = M;
    register int tempr = A[N];
    // Split array in two 
    /*
    while ( i<j ) {
      for ( ; A[j] > tempr; j-- );
      for ( A[i]=A[j]; i<j && A[i] <= tempr; i++ );
      A[j] = A[i];
    }
    */
    while ( i<j ) {
      // for ( ; A[j] > tempr; j-- );
      // for ( ; 0 < compar(A[j], tempr); j-- );
      aj = A[j];
      // while ( compar(tempr, aj) < 0 ) { j--; aj = A[j]; }
      while ( tempr < aj ) { j--; aj = A[j]; }
      if ( j <= i ) break;
      A[i] = aj; // fill hole !
      // for ( A[i]=A[j]; i<j && A[i] <= tempr; i++ );
      // for ( A[i] = A[j]; i<j && compar(A[i], tempr) <= 0; i++);
      i++; ai = A[i];
      // while ( i < j && compar(ai, tempr) <= 0 ) { i++; ai = A[i]; }
      while ( i < j && ai <= tempr) { i++; ai = A[i]; }

      // A[j] = A[i];
      A[j] = ai;
    }
    A[i] = tempr;

    // Sort recursively, the smallest first 
    if ( i-N < M-i ) { 
      handbookWithInsertionsortc(A, N, i-1, depthLimit);  
      N = i+1; 
    } else { 
      handbookWithInsertionsortc(A, i+1, M, depthLimit); 
      M = i-1; 
    }
  }
} // end handbookWithInsertionsort


// Dutch flag type function that initializes the middle segment in the middle
void dflgm(int *A, int N, int M, int pivotx, void (*cut)(), int depthLimit) {
  // printf("dflgm N %i M %i pivotx %i dl %i\n", N,M,pivotx,depthLimit);  
  /*
    This version is the fastest among 5-6 versions we have tried.
    Still there is info loss going on.  Hence improvements are plausible.
   */
  /*
    This dutch-flag-type sorter can be used as-is when called by something like:
<<<
void dflgmTest(int *A, int N, int M) {
  int L = M - N; 
  int depthLimit = 2.5 * floor(log(L));
  dflgmTest2(A, N, M, depthLimit);
}

void dflgmTest2(int *A, int N, int M, int depthLimit) {
  if ( depthLimit <= 0 ) { // avoid quadradic explosion
    heapc(A, N, M);
    return;
  }
  if ( M-N <= 1 ) { 
     insertionSortLoopUnroll(A, N, M);
     return;
  }
  dflgm(A, N, M, (N+M)/2, dflgmTest2, depthLimit-1);
}
>>>
  /*
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
      
      N <= i < lw <= up < j <= M
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
  // printf("done dflgm N %i i %i j %i M %i\n", N,i,j,M);

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
} // end dflgm

void cut2c();
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
      handbookWithInsertionsortc(A, N, M, depthLimit);
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
        int e3 = (N+M) / 2; // The midpoint
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

	T = ae3; // pivot

	 // check Left label invariant
	 if ( T <= A[N] || A[M] < T ) {
	   // give up because cannot find a good pivot
	   // dflgm is a dutch flag type of algorithm
	   dflgm(A, N, M, e3, cut2c, depthLimit);
	   return;
	 }

         // initialize running indices
	 I= N;
	 J= M;


	 // The left segment has elements < T
	 // The right segment has elements >= T
  Left:
	 // I = I + 1;
	 // AI = A[++I];
	 // if (AI < T) goto Left;
	 while ( A[++I] < T ); AI = A[I];
 // Right:
	 // J = J - 1;
	 // AJ = A[--J];
	 // if ( T <= AJ ) goto Right;
	 while ( T <= A[--J] ); AJ = A[J];
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
    cut2(A, N, M);
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
    cut2c(A, N, M, depthLimit);
    return;
  }
  depthLimit--;

        int sixth = (L + 1) / 6;
        int e1 = N  + sixth;
        int e5 = M - sixth;
        int e3 = (N+M) / 2; // The midpoint
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

	// Fix end points
	if ( A[e1] < A[N] ) iswap(N, e1, A);
	if ( A[M] < A[e5] ) iswap(M, e5, A);

	if ( ae2 == ae3 || ae3 == ae4 ) {
	  dflgm(A, N, M, e3, tpsc, depthLimit);
	  // dflgmX(A, N, M, e3, tpsc, depthLimit);
	  // dflgl(A, N, M, e3, tpsc, depthLimit);
	  // dflgmY(A, N, M, e3, tpsc, depthLimit);
	  return;
	}


	/* 
	// Check for duplicates
	// This can be improved using different dutch flag versions? 
	int duplicate = -1;
	if ( ae1 == ae5 ) { duplicate = e3; } else
	if ( ae1 == ae4 ) { duplicate = e3; } else
	if ( ae2 == ae5 ) { duplicate = e3; } else
	if ( ae1 == ae3 ) { duplicate = e3; } else
	if ( ae2 == ae4 ) { duplicate = e3; } else
	if ( ae3 == ae5 ) { duplicate = e3; } else
	if ( ae1 == ae2 ) { duplicate = e2; } else
	if ( ae2 == ae3 ) { duplicate = e3; } else
	if ( ae3 == ae4 ) { duplicate = e3; } else
	if ( ae4 == ae5 ) { duplicate = e4; };
	// */
	// Fix end points
	// if ( ae1 < A[N] ) iswap(N, e1, A);
	// if ( A[M] < ae5 ) iswap(M, e5, A);

	/*
	if ( 0 <= duplicate ) { // delegate when duplicate found
	  int tpsc();
	  dflgm(A, N, M, duplicate, tpsc, depthLimit);
	  return;
	}
	*/

	pl = A[e2]; pr = A[e4];

	/*
	if ( pl == pr ) { // they are different due to the tests above
	  int tpsc();
	  dflgm(A, N, M, e3, tpsc, depthLimit);
	  return;
	}
	*/

	if ( pl <= A[N] || A[M] <= pr ) {
	  // ascertain that the corners are not empty
	  dflgm(A, N, M, e3, tpsc, depthLimit);
	  // dflgmX(A, N, M, e3, tpsc, depthLimit);
	  // dflgl(A, N, M, e3, tpsc, depthLimit);
	  // dflgmY(A, N, M, e3, tpsc, depthLimit);
	  return;
	}

	// initialize running indices
	i = N+1; j = M-1; 
	lw = e3; 
	// iswap(e3, lw, A);
	up = lw+1; lw--;

	while ( A[i] < pl ) i++;
	while ( pr < A[j] ) j--;

	/* 
	  |)----------(--)-------------(|
	 N i         lw  up            j M
	  N <= x < i -> A[x] < pl
	  lw < x < lup -> pl <= A[x] <= pr
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
	repeatM2:
	  // i <= lw
	  am = A[lw];
	  if ( am < pl ) { // am -> L
	    A[i++] = am; A[lw--] = ai; 
	    if ( lw < i ) { i--; goto leftClosed; }
	    continue;
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
	      continue;
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
	    goto repeatM2;
	  }
	  // am -> M
	  if ( i == lw ) { i--; goto leftClosed; }
	  lw--; goto repeatM2;
	}
	// left gap closed
	i--;
	goto leftClosed;

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

void cut4c();
void cut4(int *A, int N, int M)
{
  // printf("cut4 %d %d \n", N, M);
  int L = M - N; 
  if ( L < cut4Limit ) {
    cut2(A, N, M);
    return; 
  }
  int depthLimit = 2.5 * floor(log(L));
  cut4c(A, N, M, depthLimit);
} // end cut4

void cut4c(int *A, int N, int M, int depthLimit) 
{
  // printf("cut4c %d %d %d \n", N, M, depthLimit);
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

  int probeLng = L/ probeParamCut4;
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
  // handbookWithInsertionsort(A, N1, M1);
  cut2(A, N1, M1);

  // Fix end points
  if ( A[N1] < A[N] ) iswap(N1, N, A);
  if ( A[M] < A[M1] ) iswap(M1, M, A);

  // int first = A[N1];
  maxl = A[maxlx];
  middle = A[middlex];
  minr = A[minrx];
  // int last = A[M1];

  /*
  // test here for duplicates and go to dflgm when dups found
  int dupx = -1; // candidate index to dups
  if ( first == last ) dupx = N1; else
  if ( middle == first || middle == last ||
       middle == maxl || middle == minr ) dupx = middlex; else
  if ( first == maxl ) dupx = N1; else
  if ( last == minr ) dupx = M1;
  if ( 0 <= dupx ) {
    if ( maxl != minr && A[N] < maxl && minr < A[M] ) {
      int cut4c();
      dflgm(A, N, M, dupx, cut4c, depthLimit)
      return;
    }
  }
  */
  /* no gain from this change ....
  int dupx = -1; // candidate index to dups
  // if (first == maxl || maxl == middle || middle == minr || minr == last ) {
  if ( maxl == middle || middle == minr ) dupx = middlex;
  // else if ( first == maxl ) dupx = N1; else
  // if ( minr == last ) dupx = M1;
  if ( -1 != dupx ) {
    int cut4c();
    dflgm(A, N, M, dupx, cut4c, depthLimit);
    return;
  }
  */
  // check to play safe
  if ( maxl < A[N] || A[M] < minr ||
       middle <= maxl || minr <= middle ) {
    // cut2c(A, N, M, depthLimit);
    dflgm(A, N, M, middlex, cut4c, depthLimit);
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
  register int x, y; // values
  
  int hole;

  /* We employ again whack-a-mole. We know in which partition element x 
     should be.  Find a close, undecided position where x should go.  
     Store its content in y.  Drop x (simplified).  Set x to y and repeat. 
   */
  // Ready to roll ...
  i = N; j = M; z = middlex; lw = z-1; up = z+1; hole = N;

   /*   L             ML         MR             R
    |-----]------+[---------]---------]+-----[-----|
    N     i      lw         z         up     j     M
    MR is the only segment that is empty
   */

  x = A[N]; A[N] = maxl;
  if ( x <= middle ) {
    if ( x <= maxl ) goto TLgMLgRL;
    goto TLgMLgRML;
  }
  // middle < x
  if ( x < minr ) goto TLgMLgRMR;
    goto TLgMLgRR;

	// MR empty
 TLgMLgRL:
         /*   L                            R
	   |o----]---------+[-]+--------[-----|
	   N     i        lw   up       j     M
           x -> L             z
	 */
	i++;
	y = A[i];
	if ( y <= middle ) {
	  if ( y <= maxl ) { 
	    goto TLgMLgRL;
	  }
	  // y -> ML 
	  if ( i <= lw ) {
	    A[i] = x;
	    x = y;
	    goto TLgMLgRML;
	  }
         /*   L                           R
	   |o----][----------]+--------[-----|
	   N      i           up       j     M
           x -> L            z
	 */
	  goto TLMLgRL;
	} 
	// middle < y
	A[i] = x;
	x = y;
	if ( minr <= y ) { 
	  goto TLgMLgRR;
	}
	goto TLgMLgRMR;

	// MR empty
 TLgMLgRML:
         /*   L                            R
	   |o----]---------+[-]+--------[-----|
	   N     i        lw   up       j     M
           x -> ML            z
	 */
	y = A[lw];
	if ( y <= middle ) {
	  if ( y <= maxl ) {
	    if ( i < lw ) {
	      A[lw] = x;
	      x = y;
	      lw--;
	      goto TLgMLgRL;
	    }
	    // left gap closed
	    i++;
         /*   L                           R
	   |o----][----------]+--------[-----|
	   N      i           up       j     M
           x -> ML           z
	 */
	    goto TLMLgRML;
	  }
	  // y -> ML
	  lw--;
	  goto TLgMLgRML;
	}
	// middle < y
	A[lw] = x;
	x = y;
	lw--;
	if ( minr <= y ) {
	  goto TLgMLgRR;
	}
	// y -> MR
	goto TLgMLgRMR;

	// MR is empty
 TLgMLgRMR:
         /*   L                            R
	   |o----]---------+[-]+--------[-----|
	   N     i        lw   up       j     M
           x -> MR            z
	 */
	y = A[up];
	if ( y <= middle ) {
	  A[up] = x;
	  x = y;
	  up++;
	  if ( y <= maxl ) {
         /*   L                              R
	   |o----]---------+[-|-]+--------[-----|
	   N     i         lw   up        j     M
           x -> L             z
	 */
	    goto TLgMLMRgRL;
	  }
	  // y -> ML
         /*   L                              R
	   |o----]---------+[-|-]+--------[-----|
	   N     i         lw   up        j     M
           x -> ML            z
	 */
	  goto TLgMLMRgRML;
	}
	// middle < y
	if ( y < minr ) { 
	  up++;
	  goto TLgMLMRgRMR;
	}
	// y -> R
	if ( j <= up ) { 
	  j--; z = j;
	  // right gap closed, while MR is empty ...
         /*   L                       R
	   |o----]---------+[-][-------------|
	   N     i         lw j              M
           x -> MR            z
	 */
	  goto TLgMLRMR;
	}
	// up < j
	A[up] = x;
	x = y;
	up++;
         /*   L                              R
	   |o----]---------+[-|-]+--------[-----|
	   N     i         lw   up        j     M
           x -> R             z
	 */
	goto TLgMLMRgRR;

	// MR empty  z+1=up
 TLgMLgRR: 
         /*   L                            R
	   |o----]---------+[-]+--------[-----|
	   N     i        lw   up       j     M
           x -> R             z
	 */
	j--;
	y = A[j];
	if ( y <= middle ) {
	  if ( y <= maxl ) {
	    A[j] = x;
	    x = y;
	    goto TLgMLgRL;
	  }
	  // y -> ML
	  if ( up <= j ) {
	    A[j] = x;
	    x = y;
	    goto TLgMLgRML;
	  }
	  // right gap closed, empty MR	  
	 /*   L                           R
	   |o----]---------+[-][--------------|
	   N     i        lw  j               M
           x -> R             z
	 */
	  goto TLgMLRR;
	}
	// middle < y
	if ( minr <= y ) {
	  goto TLgMLgRR;
	}
	// y -> MR
	A[j] = x;
	x = y;
         /*   L                           R
	   |o----]---------+[-]+--------[-----|
	   N     i        lw   up       j     M
           x -> MR            z
	 */
	goto TLgMLgRMR;

	// empty ML
 // TLgMRgRL:
         /*   L                            R
	   |o----]---------+[-]+--------[-----|
	   N     i        lw   up       j     M
           x -> L          z
	 */
	/*
	i++;
	y = A[i];
	if ( y <= middle ) {
	  if ( y <= maxl ) goto TLgMRgRL;
	  // y -> ML
	  A[i] = x;
	  x = y;
	  goto TLgMRgRML;
	}
	// middle < y
	if ( minr <= y ) {
	  A[i] = x;
	  x = y;
	  goto TLgMRgRR;
	}
	// y -> MR
	if ( i <= lw ) {
	  A[i] = x;
	  x = y;
	  goto TLgMRgRMR;
	}
	// left gap closed, empty ML
	*/
	 /*   L                           R
	   |o-------------][-]+--------[-----|
	   N               i  up       j     M
           x -> L         z
	 */
	// goto TLMRgRL; 

	// empty ML
 // TLgMRgRML:
         /*   L                           R
	   |o----]---------+[-]+--------[-----|
	   N     i         lw  up       j     M
           x -> ML         z
	 */
	/*
	y = A[lw];
	if ( y <= middle ) {
	  if ( y <= maxl ) {
	    if ( i < lw ) { // create ML!
	      A[lw] = x;
	      x = y;
	      lw--;
	      goto TLgMLMRgRL;
	    }
	    // i == lw, left gap closed and empty ML
	    i++; 
              L              MR           R
	   |o--------------][-]+--------[-----|
	   N                i  up       j     M
           x -> ML         z
	 
	    goto TLMRgRML;
	  }
	  // y -> ML
	  lw--;
	  goto TLgMLMRgRML;
	}
	// middle < y
	// create ML !
	A[lw] = x;
	x = y;
	lw--;
	if ( minr <= y ) { 
	  goto TLgMLMRgRR;
	}
	goto TLgMLMRgRMR; 
	*/
	// empty ML
 // TLgMRgRMR:
         /*   L                           R
	   |o----]---------+[-]+--------[-----|
	   N     i        lw   up       j     M
           x -> MR         z
	 */
	/*
	y = A[up];
	if ( y <= middle ) {
	  if ( y <= maxl ) {
	    A[up] = x;
 	    x = y;
	    up++;
              L                           R
	   |o----]---------+[-]+--------[-----|
	   N     i         lw up        j     M
           x -> L          z
	 
	    goto TLgMRgRL;
	  }
	  // y -> ML
	    A[up] = x;
 	    x = y;
	    up++;
              L                           R
	   |o----]---------+[-]+--------[-----|
	   N     i         lw up        j     M
           x -> ML         z
	 
	    goto TLgMRgRML;
	}
	// middle < y
	if ( y < minr ) { 
	  up++; 
	  goto TLgMRgRMR; 
	}
	// y -> R ?
	if ( up < j ) {
	  A[up] = x;
	  x = y;
	  goto TLgMRgRR;
	}
	// right gap closed with empty ML
	j--;
            L                           R
	   |o----]---------+[---------][-----|
	   N     i         lw         j      M
           x -> MR         z
	 
	goto TLgMRRMR;
	*/
	// empty ML
 // TLgMRgRR: 
         /*   L                            R
	   |o----]---------+[-]+--------[-----|
	   N     i         lw  up       j     M
           x -> R          z
	 */
	/*
	j--;
	y = A[j];
	if ( y <= middle ) {
	  if ( y <= maxl ) {
	    A[j] = x;
	    x = y;
	    goto TLgMRgRL;
	  }
	  // y -> ML
	    L                            R
	   |o----]---------+[-]+--------[-----|
	   N     i         lw  up       j     M
           x -> ML         z
	 
	  A[j] = x;
	  x = y;
	  goto TLgMRgRML;
	}
	// middle < y
	if ( minr <= y ) goto TLgMRgRR;
	// y -> MR
	if ( up <= j ) {
	  A[j] = x;
	  x = y;
	  goto TLgMRgRMR;
	}
	// right gap closed, empty ML
            L                           R
	   |o----]---------+[---------][-----|
	   N     i        lw          j      M
           x -> R          z
	 
	goto TLgMRRR;
	*/

	// empty MR
 TLMLgRML: 
         /*   L                           R
	   |o----][----------]+--------[-----|
	   N      i           up       j     M
           x -> ML           z
	 */
	y = A[up];
	if ( y <= middle ) {
	  if ( y <= maxl ) {
	    A[up] = A[i];
	    A[i] = y;
	    i++;
	    z = up; up++; 
	    goto TLMLgRML;
	  }
	  // y -> ML
	  z = up; up++; 
	  goto TLMLgRML;	  
	}
	// middle < y
	if ( y < minr ) {
	  A[up] = x;
	  x = y;
	  z = up; up++; 
         /*   L                           R
	   |o----][----------]+--------[-----|
	   N      i           up       j     M
           x -> MR           z
	 */
	  goto TLMLgRMR;
	}
	// y -> R
	if ( up < j ) {
	  A[up] = x;
	  x = y;
	  z = up; up++; 
	  goto TLMLgRR;
	}
	// right gap closed with only 3 segments !
         /*   L                           R
	   |o----][----------][-------------|
	   N      i           j             M
           x -> ML           z
	 */
	j--;
	i--;
	A[hole] = A[i];
	A[i] = x;
        /*   L                           R
	   |-----][----------][-------------|
	   N      i          j              M
           x -> ML           z
	 */
	goto Finish3;

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

	// right gap closed, empty MR
 TLgMLRMR: 
         /*   L                       R
	   |o----]---------+[-][-------------|
	   N     i        lw  j              M
           x -> MR            z
	 */
	y = A[lw];
	if ( y <= middle ) {
	  if ( maxl < y ) {
	    lw--;
	    goto TLgMLRMR;
	  }
	  // y -> L
	  if ( i < lw ) {
	    A[lw] = A[j];
	    A[j] = x;
	    z--;
	    x = y;
	    lw--;
         /*   L                           R
	   |o----]---------+[-|-------][-----|
	   N     i         lw         j      M
           x -> L             z
	 */
	    goto TLgMLMRRL;
	  }
	  // left gap closed
         /*   L       ML            R
	   |o----][----------][-------------|
	   N     i           j              M
           x -> MR           z
	 */
	  // create MR with 1 element ...
	  A[hole] = A[i];
	  A[i] = A[j];
	  A[j] = x;
	  z--;
	  i--;
	  j++;
	  goto Finish4;
	}
	// middle < y / postpone x
	// Shift ML to the left and add y to R or create MR
	A[lw] = A[j];
	A[j] = y;
	lw--;
	if ( minr <= y ) { // y -> R
	  j--;
	  z = j;
	  goto TLgMLRMR;
	}
	// y -> MR
	z--;
	goto TLgMLMRRMR;

	  // right gap closed, empty MR
 TLgMLRR:
	 /*   L                       R
	   |o----]---------+[-][--------------|
	   N     i        lw  j               M
           x -> R             z
	 */
	y = A[lw];
	if ( y <= middle ) {
	  if ( y <= maxl ) {
	    if ( i < lw ) {
	      A[lw] = A[j];
	      A[j] = x;
	      j--;
	      z = j;
	      x = y;
	      lw--;
	 /*   L                       R
	   |o----]---------+[-][--------------|
	   N     i         lw j               M
           x -> L             z
	 */
	      goto TLgMLRL;
	    }
	    // left gap closed also & i=lw & y = A[i]
	    A[hole] = y; // A[hole] = A[i];
	    A[i] = A[j];
	    A[j] = x;
	    j--;
	    goto Finish3;
	  }
	  // y -> ML
	  lw--;
	  goto TLgMLRR;
	}
	// middle < y / postpone x
	// Shift ML to the left and add y to R or create MR
	A[lw] = A[j];
	A[j] = y;
	lw--;
	if ( minr <= y ) { // y -> R
	   j--;
	   z = j;
	   goto TLgMLRR;
	}
	// y -> MR
	z--;
	goto TLgMLMRRR;

	// right gap closed, empty MR
 TLgMLRL:
	 /*   L                        R
	   |o----]---------+[-][--------------|
	   N     i        lw  j               M
           x -> L             z
	 */
	i++;
	y = A[i];
	if ( y <= middle ) {
	  if ( y <= maxl ) goto TLgMLRL;
	  if ( i <= lw ) {
	    A[i] = x;
	    x = y;
	 /*   L                        R
	   |o----]---------+[-][--------------|
	   N     i        lw  j               M
           x -> ML            z
	 */
	    goto TLgMLRML;
	  }
	  // left gap closed
	  A[hole] = x;
	  goto Finish3;
	}
	// middle < y
	A[i] = x;
	x = y;
	if ( minr <= y ) goto TLgMLRR;
	goto TLgMLRMR;

	// right gap closed with empty ML
// TLgMRRR:
         /*   L                           R
	   |o----]---------+[---------][-----|
	   N     i        lw          j      M
           x -> R          z
	 */
	/*
	y = A[lw];
	if ( y <= middle ) {
	  if ( y <= maxl ) { // y -> L
	    if ( i < lw ) {
	      A[lw] = A[j];
	      A[j] = x;
	      j--;
	      x = y;
	      lw--; z = lw;
              L                           R
	   |o----]---------+[---------][-----|
	   N     i        lw          j     M
           x -> L          z
	 
	      goto TLgMRRL;
	    }
	    // left gap closed
	    A[hole] = A[i];
	    A[i] = A[j];
	    A[j] = x;
	    j--;
	    goto Finish3;
	  }
	  // y -> ML !
	  lw--;
              L                           R
	   |o----]---------+[--|------][-----|
	   N     i        lw          j     M
           x -> R              z
	 
	  goto TLgMLMRRR;
	}
	// middle < y; shift MR to the left
	if ( minr <= y ) { // y -> R
	  A[lw] = A[j];
	  A[j] = y;
	  j--; lw--; z = lw;
	  goto TLgMRRR;
	}
	// y -> MR
	lw--; z = lw;
	goto TLgMRRR;
	*/

	// right gap closed with empty ML
// TLgMRRL:
         /*   L                           R
	   |o----]---------+[---------][-----|
	   N     i        lw          j      M
           x -> L          z
	 */
	/*
	i++;
	y = A[i];
	if ( y <= middle ) {
	  if ( y <= maxl ) goto TLgMRRL;
	  // y -> ML!
	  A[i] = x;
	  x = y;
              L                           R
	   |o----]---------+[---------][-----|
	   N     i        lw          j      M
           x -> ML         z
	 
	  goto TLgMRRML;
	}
	// middle < y
	if ( minr <= y ) {
	  A[i] = x;
	  x = y;
	  goto TLgMRRR;
	}
	// y -> MR
	if ( i <= lw ) {
	  A[i] = x;
	  x = y;
              L                           R
	   |o----]---------+[---------][-----|
	   N     i        lw          j      M
           x -> MR         z
	 
	  goto TLgMRRMR;
	}
	// left gap closed
	A[hole] = x;
	goto Finish3;
	*/

	// right gap closed, empty ML
 // TLgMRRML:
         /*   L                           R
	   |o----]---------+[---------][-----|
	   N     i         lw         j      M
           x -> ML         z
	 */
	// going to create ML
	/*
	y = A[lw];
	if ( y <= middle ) {
	  if ( y <= maxl ) {
	    if ( i < lw ) {
	      A[lw] = x;
	      x = y;
	      lw--;
	      goto TLgMLMRRL;
	    }
	    // left gap closed also
	    // create ML with one element in it ...
	    A[hole] = y; // = A[i]
	    A[i] = x;
	    i--;
	    j++;
	    goto Finish4;
	  }
	  // y -> ML
	  // create ML
	  lw--;
              L                        R
	   |o----]------+[--|------][-----|
	   N     i      lw         j      M
           x -> ML          z
	 
	  goto TLgMLMRRML;
	}
	// middle < y
	if ( minr <= y ) {
	  A[lw] = A[j];
	  A[j] = y;
	  j--; lw--; z = lw;
	  goto TLgMRRML;
	}
	// y -> MR
	lw--; z = lw;
	goto TLgMRRML;
	*/

	// right gap closed, empty ML
 // TLgMRRMR:
         /*   L                           R
	   |o----]---------+[---------][-----|
	   N     i        lw          j      M
           x -> MR         z
	 */
	/*
	y = A[lw];
	if ( y <= middle ) {
	  if ( y <= maxl ) {
	    if ( i < lw ) {
	      A[lw] = x;
	      x = y;
	      lw--; z = lw;
	      goto TLgMRRL;
	    }
	    // left gap closed also
	    A[hole] = y; // = A[i]
	    A[i] = x;
	    goto Finish3;
	  }
	  // y -> ML; create ML
	  lw--;
	  goto TLgMLMRRMR;
	}
	// middle < y
	z--;
	if ( minr <= y ) { // y -> R
	  A[lw] = A[j];
	  A[j] = y;
	  j--; lw = z;
	  goto TLgMRRMR;
	}
	// y -> MR
	lw = z;
	goto TLgMRRMR;
	*/ 

	// right gap closed, empty MR
 TLgMLRML: 
	 /*   L                        R
	   |o----]---------+[-][--------------|
	   N     i        lw  j               M
           x -> ML            z
	 */
	y = A[lw];
	if ( y <= middle ) {
	  if ( y <= maxl ) {
	    if ( i < lw ) {
	      A[lw] = x;
	      x = y;
	      lw--;
	      goto TLgMLRL;
	    }
	    // left gap also closed
	    A[hole] = A[i];
	    A[i] = x;
	    goto Finish3;
	  }
	  // y -> ML
	  lw--;
	  goto TLgMLRML;
	}
	// middle < y
	if ( minr <= y ) { // shift ML to the left
	  A[lw] = A[j];
	  A[j] = y;
	  j--;
	  z = j;
	  lw--;
	  goto TLgMLRML;
	}
	// y -> MR !
	A[lw] = A[j];
	A[j] = y;
	z--;
	lw--;
        /*   L                            R
	   |o----]---------+[-|-------][-----|
	   N     i        lw          j      M
           x -> ML            z
	 */
	goto TLgMLMRRML;

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

	// left gap closed, empty ML
// TLMRgRML:
         /*   L              MR            R
	   |o--------------][-]+--------[-----|
	   N                i  up       j     M
           x -> ML         z
	 */
	/*
	y = A[up];
	if ( y <= middle ) {
	  if ( y <= maxl ) {
	    A[up] = A[i]; A[i] = y;
	    z = i; i++; up++;
	    goto TLMRgRML;
	  }
	  // y -> ML !
	  A[up] = A[i]; A[i] = y;
	  z = i; up++;
	  goto TLMLMRgRML;
	}
	// middle < y
	if ( minr <= y ) {
	  if ( up < j ) { // y -> R; create ML 
	    A[up] = A[i]; A[i] = x;
	    x = y;
	    z = i; up++;
              L                              R
	   |o----][-----------|-]+--------[-----|
	   N      i              up       j     M
           x -> R             z
	 
	    goto TLMLMRgRR;
	  }
	  // right gap closed; create 1-element ML
	  A[hole] = A[z]; A[z] = x;
	  i = z-1;
	  goto Finish4;
	}
	// y -> MR
	up++;
	goto TLMRgRML;
	*/

	// left gap closed, empty ML
 // TLMRgRL:
	 /*   L             MR            R
	   |o-------------][-]+--------[-----|
	   N               i  up       j     M
           x -> L         z
	 */
	/*
	y = A[up];
	if ( y <= middle ) {
	  if ( y <= maxl ) {
	    A[up] = A[i]; A[i] = y;
	    z = i; i++; up++;
	    goto TLMRgRL;
	  }
	  // y -> ML, create ML
	  A[up] = A[i]; A[i] = y;
	  z = i; up++;
	  goto TLMLMRgRL;
	}
	// middle < y
	if ( y < minr ) {
	  up++;
	  goto TLMRgRL;
	}
	// y -> R
	if ( up < j ) {
	  A[up] = A[i]; A[i] = x;
	  x = y;
	  z = i; i++; up++;
	      L             MR            R
	   |o-------------][-]+--------[-----|
	   N               i  up       j     M
           x -> R         z
	 
	  goto TLMRgRR;
	}
	// right gap closed;
	A[hole] = x;
	j--;
	goto Finish3;
	*/ 

	// left gap closed, empty ML
 //  TLMRgRR:
	 /*   L             MR            R
	   |o-------------][-]+--------[-----|
	   N               i  up       j     M
           x -> R         z
	 */
	/*
	j--;
	y = A[j];
	if ( y <= middle ) {
	  A[j] = x;
	  x = y;
	  if ( y <= maxl ) goto TLMRgRL;
	  // y -> ML !
	  goto TLMRgRML;
	}
	// middle < y
	if ( minr <= y ) goto TLMRgRR;
	// y -> MR
	if ( up <= j ) {
	  A[j] = x;
	  x = y;
	      L             MR            R
	   |o-------------][-]+--------[-----|
	   N               i  up       j     M
           x -> MR        z
	 
	  goto TLMRgRMR;
	}
	// right gap closed also, with empty ML
	A[hole] = A[z]; A[z] = y; A[j] = x;
	i = z;  j--;
	goto Finish3;
	*/ 

	// left gap closed, empty ML
// TLMRgRMR:
	 /*   L             MR            R
	   |o-------------][-]+--------[-----|
	   N               i  up       j     M
           x -> MR        z
	 */
	/*
	y = A[up];
	if ( y <= middle ) {
	  if ( y <= maxl ) {
	    A[up] = A[i]; A[i] = y;
	    z = i; i++; up++;
	    goto TLMRgRMR;
	  }
	  // y -> ML, create ML
	  A[up] = A[i]; A[i] = y;
	  z = i; up++;
	  goto TLMLMRgRMR;
	}
	// middle < y
	if ( y < minr ) {
	  up++;
	  goto TLMRgRMR;
	}
	// y --> R
	if ( up < j ) {
	  A[up] = x;
	  x = y;
	  up++;
	  goto TLMRgRR;
	}
	// right gap closed, empty ML
	A[hole] = A[z]; A[z] = x;
	i = z;  j--;
	goto Finish3;
	*/

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

	// left gap closed, empty MR
 TLMLgRL:
	 /*    L                          R
	   |o----][----------]+--------[-----|
	   N      i           up       j     M
           x -> L            z
	 */
	y = A[up];
	if ( y <= middle ) {
	  if ( y <= maxl ) {
	    A[up] = A[i];
	    A[i] = y;
	    i++; z = up; up++; 
	    goto TLMLgRL;
	  }
	  // y -> ML
	  z = up; up++; 
	  goto TLMLgRL;
	}
	// middle < y
	if ( y < minr ) { // y -> MR ! create MR
	  up++;
	  goto TLMLMRgRL;
	}
	// y --> R
	if ( up < j ) {
	  A[up] = A[i]; A[i] = x;
	  x = y;
	  i++; z = up; up++; 
	  goto TLMLgRR;
	}
	// right gap closed
	A[hole] = x;
	j--;
	goto Finish3;

	// left gap closed, empty MR
 TLMLgRMR:
         /*   L                           R
	   |o----][----------]+--------[-----|
	   N      i           up       j     M
           x -> MR           z
	 */
	y = A[up];
	if ( y <= middle ) {
	  if ( y <= maxl ) {
	    A[up] = A[i]; A[i] = y;
	    i++; z = up; up++; 
	    goto TLMLgRMR;
	  }
	  // y -> ML
	  z = up; up++; 
	  goto TLMLgRMR;
	}
	// middle < y
	if ( y < minr ) { // create MR !
	  up++;
	  goto TLMLMRgRMR;
	}
	// y --> R
	if ( up < j ) { // create MR !
	  A[up] = x;
	  x = y;
	  up++;
	  goto TLMLMRgRR;
	}
	// right gap closed // create MR, single element
	i--;
	A[hole] = A[i]; A[i] = A[z]; A[z] = x;
	i--; z--;
	goto Finish4;

	// left gap closed, empty MR
 TLMLgRR:
         /*   L                           R
	   |o----][----------]+--------[-----|
	   N      i           up       j     M
           x -> R            z
	 */
	j--;
	y = A[j];
	if ( y <= middle ) {
	  A[j] = x;
	  x = y;
	  if ( y <= maxl ) goto TLMLgRL;
	  // y -> ML
	  goto TLMLgRML;
	}
	// middle < y
	if ( minr <= y ) goto TLMLgRR;
	// y --> MR !
	if ( up < j ) {
	  A[j] = x;
	  x = y;
	  goto TLMLgRMR;
	}
	// both gaps closed, create MR, single element
	i--;
	A[hole] = A[i]; A[i] = A[z]; A[z] = y; A[j] = x;
	i--; z--;
	goto Finish4;

	// Finish0:
	// printf("cut4 exit Finish0 N: %d M: %d\n", N, M);
	// return;

	// +++++++++++++++both gaps closed+++++++++++++++++++
	int lleft, lmiddle, lright;
	// int k; // for testing
 Finish3:
	// printf("cut4 exit Finish3 N: %d M: %d\n", N, M);

         /*   L            ML/MR          R
	   |-----][-------------------][-----|
	   N      i                   j      M
	 */
	/*
	for  (k = N; k < i; k++) 
	  if ( maxl < A[k] ) {
	     printf("L error k: %i\n", k); 
             printf("N: %d i %i lw %i z %i up %i j %i M: %d\n", N,i,lw,z,up,j,M);
	  }
	for  (k = i; k <= j; k++) 
	  if ( A[k] <= maxl || minr <= A[k] ) {
	     printf("ML-MR error k: %i\n", k); 
             printf("N: %d i %i lw %i z %i up %i j %i M: %d\n", N,i,lw,z,up,j,M);
	  }
	for  (k = j+1; k <= M; k++) 
	  if ( A[k] < minr ) {
	     printf("MR error k: %i\n", k); 
             printf("N: %d i %i lw %i z %i up %i j %i M: %d\n", N,i,lw,z,up,j,M);
	  }
	*/

	lleft = i - N;
	lmiddle = j - i;
	lright = M - j;
	if ( lleft < lmiddle ) {      // L < M
	  if ( lleft < lright ) {     // L < R
	    cut4c(A, N, i-1, depthLimit);
	    if ( lmiddle < lright ) { // L < M < R
	      cut4c(A, i, j, depthLimit);
	      cut4c(A, j+1, M, depthLimit);
	    } else {                  // L < R < M     
	      cut4c(A, j+1, M, depthLimit);
	      cut4c(A, i, j, depthLimit);
	    }
	    return;
	  } else {                    // R < L < M 
	    cut4c(A, j+1, M, depthLimit);
	    cut4c(A, N, i-1, depthLimit);
	    cut4c(A, i, j, depthLimit);
	    return;
	  }
	}                             // M < L
	if ( lmiddle < lright ) {     // M < R
	  cut4c(A, i, j, depthLimit);
	  if ( lleft < lright) {
	     cut4c(A, N, i-1, depthLimit);
	     cut4c(A, j+1, M, depthLimit);
	  } else {
	    cut4c(A, j+1, M, depthLimit);
	    cut4c(A, N, i-1, depthLimit);
	  }
	  return;
	}                             // R < M < L
	cut4c(A, j+1, M, depthLimit);
	cut4c(A, i, j, depthLimit);
	cut4c(A, N, i-1, depthLimit);
	return;

 Finish4:
	// printf("cut4 exit Finish4 N: %d M: %d\n", N, M);
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
	    cut4c(A, j, M, depthLimit);
	    return;
	  }
	  cut4c(A, j, M, depthLimit);
	  cut4c(A, z+1, j-1, depthLimit);
	  return;
	}
	// M-z <= z-N
	cut4c(A, z+1, j-1, depthLimit);
	cut4c(A, j, M, depthLimit);
	if ( i-N < z-i ) {
	  cut4c(A, N, i, depthLimit);
	  cut4c(A, i+1, z, depthLimit);
	  return;
	}
	cut4c(A, i+1, z, depthLimit);
	cut4c(A, N, i, depthLimit);
} // end cut4c


void dpqSort();
void dpq(int *a, int left, int right) {
  dpqSort(a, left, right);
} // end dpq


int INSERTION_SORT_THRESHOLD = 32;

    /**
     * Sorts the specified range of the array into ascending order. This
     * method differs from the public {@code sort} method in that the
     * {@code right} index is inclusive, and it does no range checking on
     * {@code left} or {@code right}.
     *
     * @param a the array to be sorted
     * @param left the index of the first element, inclusive, to be sorted
     * @param right the index of the last element, inclusive, to be sorted
     */
    // private static void doSort(int[] a, int left, int right) {
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
	int e3 = (left + right)/2;
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
	    dpqSort(a, less, great);
	    return;
	
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
			    a[less] = pivot1;
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
                        a[k] = pivot;
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



