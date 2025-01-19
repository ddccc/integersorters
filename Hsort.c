// c:/bsd/rigel/sort/Hsort
// Date: Fri Jan 31 13:22:27 2014
// (C) OntoOO/ Dennis de Champeaux

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
  int end = count-1; // in languages with zero-based arrays 
                     // the children are 2*i+1 and 2*i+2
  while ( end > 0 ) {
    // (swap the root(maximum value) of the heap with the last element of the heap)
    // swap(a[end], a[0]);
    // iswap(end, 0, a);
    { int t = a[end]; a[end] = a[0]; a[0] = t; }
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
    // (sift down the node at index start to the proper place such 
    // that all nodes below the start index are in heap order)
    siftDown(a, start, count-1);
    start = start - 1;
  } // (after sifting down the root all nodes/elements are in heap order)
} // end heapify
 
void siftDown(int *a, int start, int end) {
  // input:  end represents the limit of how far down the heap to sift.
  int root = start;
  int child, swapi;
  // while root * 2 + 1 <= end do   (While the root has at least one child)
  while ( root * 2 + 1 <= end ) {
    child = root * 2 + 1; // (root*2 + 1 points to the left child)
    swapi = root; //(keeps track of child to swap with)
    // (check if root is smaller than left child)
    if ( a[swapi] < a[child] ) 
      swapi = child;
    // (check if right child exists, and if it's bigger 
    // than what we're currently swapping with)
    if ( child+1 <= end && a[swapi] < a[child+1] )
      swapi = child + 1;
    // (check if we need to swap at all)
    if ( swapi != root ) {
      // swap(a[root], a[swapi]);
      // iswap(root, swapi, a);
      { int t = a[root]; a[root] = a[swapi]; a[swapi] = t; }
      root = swapi; // (repeat to continue sifting down the child now)
    } else return;
  }
} // end siftDown
