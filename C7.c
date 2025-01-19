// c:/bsd/rigel/sort/integersorters/C7.c
// Date: Wed Apr 10 12:28:44 2019, Sat Dec 14 10:22:07 2024
// (C) OntoOO/ Dennis de Champeaux

#define iswap(p, q, A) { int t3t = A[p]; A[p] = A[q]; A[q] = t3t; }
#define delegate 1300

void cut7c(int *A, int lo, int hi, int depthLimit);

void cut7(int *A, int lo, int hi) {
  // printf("cut7 N %i lo %i \n", lo, hi);
  int L = hi - lo;
  if ( L <= 0 ) return;
  int depthLimit = 1 + 2.9 * floor(log(L));
  cut7c(A, lo, hi, depthLimit);
} // end cut7

void cut7c(int *A, int lo, int hi, int depthLimit) {
  int L;

 Start:
  //  printf("cut7c lo %i hi %i L %i\n", lo, hi, hi-lo);
  L = hi - lo + 1;
  if ( L <= 1 ) return; 
  if ( depthLimit <= 0 ) {
    heapc(A, lo, hi);
    return;
  }
  depthLimit--;

  if ( L < delegate ) {
    cut2lrc(A, lo, hi, depthLimit);
    return;
  }

  int k, lo1, hi1; // for sampling
  int maxlx, middlex, minrx;  
  // pivots for left/ middle / right regions
  register int maxl, middle, minr;   
  register int i, j, lw, up, z; // indices
  i = lo; j = hi;
  z = middlex = lo + (L>>1); // lo + L/2/
  //const int small = 900; 
{ // small <= L, use a variable number for sampling
    int probeLng = sqrt(L/5.6); 
    int halfSegmentLng = probeLng >> 1; // probeLng/2;
    int quartSegmentLng = probeLng >> 2; // probeLng/4;
    lo1 = middlex - halfSegmentLng; //  lo + (L>>1) - halfSegmentLng;
    hi1 = lo1 + probeLng - 1;
    maxlx = lo1 + quartSegmentLng;
    // int middlex = lo1 + halfSegmentLng;
    minrx = hi1 - quartSegmentLng;
    int offset = L/probeLng;  

    // assemble the mini array [lo1, hi1]
    for (k = 0; k < probeLng; k++) // iswap(lo1 + k, lo + k * offset, A);
    { int xx = lo1 + k, yy = lo + k * offset; iswap(xx, yy, A); }
    // sort this mini array to obtain good pivots
    cut7c(A, lo1, hi1, depthLimit);
    lw = maxlx; up = minrx;
 }
  // pivots
  maxl = A[maxlx]; middle = A[z]; minr = A[minrx];

  // check that segments can be properly initialized
  if 
    /*
    ( compare(maxl, middle) == 0 || 
       compare(middle, minr) == 0 || 
       compare(minr, A[hi1]) == 0 ||
       compare(maxl, A[lo1]) == 0 ) 
    */
    ( maxl == middle || middle == minr || minr == A[hi1] || maxl, A[lo1] )
    {
      // no good pivots available, thus escape
      dflgm(A, lo, hi, middlex, cut7c, depthLimit);
      return;
    }
    // Swap these segments to the corners
      for ( k = lo1; k <= maxlx; k++ ) {
      iswap(k, i, A); i++;
    }
    i--;
    lw = i+1;
    for ( k = maxlx+1; k <= middlex; k++ ) {
      iswap(k, lw, A); lw++;
    }
    lw--;
    for ( k = hi1; minrx <= k; k--) {
      iswap(k, j, A); j--;
    }
    j++;
    up = j-1;
    for ( k = minrx-1; middlex < k; k--) {
      iswap(k, up, A); up--;
    }
    up++;
    // } 

    int alw, aup; // values  

    // Here the general 1-gap layout
    /*  L     ML                        MR     R
     |-----]------]------------------[------[-----|
    lo     i     lw                  up     j     hi

     There are invariants to be maintained (which are >essential< 
     for machine assisted correctness proofs):
     maxl < middle < minr
       lo <= x <= i --> A[x] <= maxl
       i < x <= lw --> maxl < A[x] <= middle
       up <= x < j --> middle < A[x] < minr
       j <= x <= hi --> minr <= A[x]
   */
    // Ready to roll ... 

    while ( ++lw < up ) {
      alw = A[lw];
      // while ( compare(alw, middle) <= 0 ) {
      while ( alw <= maxl ) {
	// if ( compare(alw, maxl) <= 0 ) {
	if ( alw <= maxl ) {
	  A[lw] = A[++i]; A[i] = alw;
	}
	if ( up <= ++lw ) { lw = up-1; goto GapClosed; }
	alw = A[lw];
      }

      // alw -> right
      if ( --up <= lw ) { goto InsertALW; }
      aup = A[up];
      // while ( compare(middle, aup) < 0 ) {
      while ( middle < aup ) {
    	// if ( compare(minr, aup) <= 0 ) {
	if ( minr <= aup ) {
	  A[up] = A[--j]; A[j] = aup;
	}
	if ( --up <= lw )  { goto InsertALW; }
	aup = A[up];
      }

      // aup -> left
      // if ( compare(aup, maxl) <= 0 ) {
      if ( aup <= maxl ) {
	A[lw] = A[++i]; A[i] = aup;
      } else A[lw] = aup; 
      // if ( compare(minr, alw) <= 0 ) {
      if ( minr <= alw ) {
	A[up] = A[--j]; A[j] = alw;
      } else A[up] = alw;
     }

    // fall through
    lw--;
    goto GapClosed;

 InsertALW:
      // if ( compare(minr, alw) <= 0 ) {
      if ( minr <= alw ) {
      A[up] = A[--j]; A[j] = alw;
    } else A[up] = alw;
    lw--;

 GapClosed: ;

  // +++++++++++++++ gap closed+++++++++++++++++++
  /*      L     ML                        MR     R
       |-----]------]------------------[------[-----|
      lo     i     lw                  up     j     hi
  */
  // int k; // for testing

  /*
    for ( k = lo; k <= i; k++ ) if ( compare(maxl, A[k]) < 0 ) {
	printf("Err0 L k: %i\n",k);
	exit(0);
      }
    for ( k = i+1; k <= lw; k++ ) 
      if ( compare(A[k], maxl) <= 0 || compare(middle, A[k]) < 0) {
	printf("Err0 ML k: %i\n",k);
	printf("lo %i i %i lw %i up %i j %i hi %i\n",lo,i,lw,up,j,hi);
	// printf("---- callCnt %i\n", callCnt);
	exit(0);
      }
    for ( k = up; k < j; k++ ) 
      if ( compare(A[k], middle) <= 0 || compare(minr, A[k]) <= 0) {
	struct intval *pk, *pmr;
	// pk = A[326327]; pmr = minr;
	// printf("pk %i minr %i\n", pk->val, pmr->val);
	// printf("Err0 MR k: %i callCnt %i \n",k, callCnt);
	printf("Err0 MR k: %i\n",k);
	printf("Err0 lo %i i %i lw %i up %i j %i hi %i\n",lo,i,lw,up,j,hi);
	exit(0);
      }
    for ( k = j; k <= hi; k++ ) if ( compare(A[k], minr) < 0 ) {
	printf("Err0 R k: %i\n",k);
	exit(0);
      }

    // printf("Rcall lo %i i %i lw %i up %i j %i hi %i\n",lo,i,lw,up,j,hi);
    // */

  if ( lw-lo < hi-lw ) {
    cut7c(A, lo, i, depthLimit);
    cut7c(A, i+1, lw, depthLimit);
    if ( j-up < hi-j ) {
      cut7c(A, up, j-1, depthLimit);
      // cut7c(A, j, hi, depthLimit, compare);
      // return;
      lo = j;
      goto Start;
    }
    cut7c(A, j, hi, depthLimit);
    // cut7c(A, z+1, j-1, depthLimit, compare);
    // return;
    lo = up; hi = j-1;
    goto Start;
  }
  cut7c(A, up, j-1, depthLimit);
  cut7c(A, j, hi, depthLimit);
  if ( i-lo < lw-i ) {
    cut7c(A, lo, i, depthLimit);
    // cut7c(A, i+1, z, depthLimit, compare);
    // return;
    lo = i+1; hi = lw;
    goto Start;
  }
  cut7c(A, i+1, lw, depthLimit);
  // cut7c(A, lo, i, depthLimit, compare);
  hi = i;
  goto Start;
} // end 3-pivot module end cut7

#undef iswap
#undef delegate
