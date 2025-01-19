// File: c:/bsd/rigel/sort/integersorters/Isort.c
// Date: Thu Dec 12 14:13:43 2024
// (C) OntoOO/ Dennis de Champeaux


#define iswap(p, q, A) { int t3t = A[p]; A[p] = A[q]; A[q] = t3t; }

/*
// mimimal footprint version 
void insertionsort(int *A, int lo, int hi) {
  if ( hi <= lo ) return;
  int i, j;
  for ( i = lo+1; i <= hi; i++ )
    // for ( j = i; lo < j && compareXY(A[j-1], A[j]) > 0; j-- ) 
    for ( j = i; lo < j && A[j-1] > A[j]; j--)
      // iswap(j-1, j, A); 
       { int xx = j-1; iswap(xx, j, A); }
} // end insertionsort
// */


// /*
// Version that inserts two elements at the time
// Nigel version; clean up from DL from Y?
void insertionsort(int *A, int lo, int hi) {
// void insertionsort0(int *A, int lo, int hi) {
  if ( hi <= lo ) return;
  int size = hi-lo+1, i, k;

        if ((size & 1) == 0) {   // ensure even size
            int t = A[lo], u = A[lo+1];
            // if (compareXY(t, u) > 0) { A[lo] = u; A[lo+1] = t; }
	    if (t > u) { A[lo] = u; A[lo+1] = t; }
            i = lo+2;
        } else
            i = lo+1;

        for (; i < hi; ++i) {
            int fst = A[k = i], snd = A[++i];
            // if (compareXY(fst, snd) > 0) {
	    if (fst > snd) {
                for (; k > lo; --k) {
                    int x = A[k - 1];
                    // if (compareXY(fst, x) >= 0)
		    if ( fst >= x )
                        break;
                    A[k + 1] = x;
                }
                A[k + 1] = fst;
                for (; k > lo; --k) {
                    int x = A[k - 1];
                    // if (compareXY(snd, x) >= 0)
		    if ( snd >= x )
                        break;
                    A[k] = x;
                }
                A[k] = snd;
            } else {
                for (; k > lo; --k) {
                    int x = A[k - 1];
                    // if (compareXY(snd, x) >= 0)
		    if (snd >= x)
                        break;
                    A[k + 1] = x;
                }
                if (k != i)
                    A[k + 1] = snd;
                for (; k > lo; --k) {
                    int x = A[k - 1];
                    // if (compareXY(fst, x) >= 0)
		    if (fst >= x)
                        break;
                    A[k] = x;
                }
                if (k != i)
                    A[k] = fst;
	    }
	}
}  // end  insertionsort
// */


/*
// support member for the next one.
int binarySearch(int *A, int item, int low, int high) {
  if (high <= low)
    // return compareXY(item, A[low]) > 0 ? low+1 : low;
    return item > A[low] ? low+1 : low;
  int L = high - low;
  int mid = low + (L>>1); // low + L/2;
  if ( item == A[mid] ) return mid + 1;
  if ( item > A[mid] ) return binarySearch(A, item, mid + 1, high);
  return binarySearch(A, item, low, mid - 1);
  // int z = compareXY(item, A[mid]);
  // if ( 0 == z ) return mid + 1;
  // if ( 0 < z ) 
  //  return binarySearch(A, item, mid + 1, high, compareXY);
  // return binarySearch(A, item, low, mid - 1, compareXY);
} 
// */


/*
// *** Redefine a version above as insertionsort0 for this version
void insertionsort(int *A, int lo, int hi) {
  // This is a hybrid; 
  // it switches to the version above for sizes less than first.
  // Otherwise it uses binarySearch to find the location below which TWO
  // elements can be inserted.
  int first = lo + 9; 
  
  if ( hi <= first ) {
    // printf("Do insertionsort0\n");
    insertionsort0(A, lo, hi);
    return;
  }
  // printf("IN lo %i hi %i\n", lo, hi);
  int i = hi+1;
  while ( first <= i) i = i-2;
  // printf("i %i\n", i); 
  // sort the small head first
  insertionsort0(A, lo, i-1);
  // proceed by inserting pairs in [i:hi]
  //
  //  |------------------]-----------------|
  //  lo                  i               hi
  //
  int fst, snd;
  int loc, j, k;
  while ( i < hi ) {
    // printf("\n2i %i\n", i); 
    fst = A[i]; snd = A[i+1];
    // printf("fstX %i sndX %i\n", i, (i+1));
    // if ( compareXY(snd, fst) < 0 ) { // swap fst & snd
    if ( snd < fst ) {
      // void *t = fst; fst = snd; snd = t;
      int t = fst; fst = snd; snd = t;
    } 
    k = i-1;
    // printf("k %i\n", k);
    loc = binarySearch(A, snd, lo, k);
    // printf("A loc %i\n", loc);
  //
  //  |--------|------------]fs---------------|
  //  lo     loc            ki               hi
  //
    if ( loc < k ) {
      // printf("passing 1\n");
      for (j = k; loc <= k; k--) {
	// printf("Move %i to %i\n", k, k+2);
	A[k+2] = A[k];
      }
      // printf("Set %i\n", loc+1);
      A[loc+1] = snd;
      k = loc - 1;
      goto DoFst;
    }
    if ( loc == i ) {
      // printf("passing 4\n");
      A[i+1] = snd;
      goto DoFst;
    }
    // k == loc) 
    // printf("passing 00\n");
    A[i+1] = A[k]; A[i] = snd;
    k--;
  
  DoFst:
    // take care of fst
    // printf("passing 2 k %i\n", k); 
    // loc = binarySearch(A, fst, lo, k, compareXY);
    loc = binarySearch(A, fst, lo, k);
    // printf("B loc %i\n", loc);

    if ( loc < k ) {
      // printf("passing3\n");
      for (j = k; loc <= k; k--) A[k+1] = A[k];
      A[loc] = fst;
      goto Check;
    }  
    if ( loc == k ) {
      // printf("passing5 %i\n", loc);
      A[k+1] = A[k]; // A[k] = fst;
      // goto Check;
    } 
    // loc == i
    A[loc] = fst;

  Check:
    // printf("passing4\n");
    //
    // for ( j = lo+1; j <= i+1; j++ )
    //    if (compareXY(A[j], A[j-1]) < 0 ) {
    //    	printf("Err2 j %i\n", j); exit(0);
    // }
    // 
    // printf("passing5 sorted lo %i (i+1) %i\n", lo, i+1);
    i=i+2;
  }
} // end  insertionsort
// */

#undef iswap


