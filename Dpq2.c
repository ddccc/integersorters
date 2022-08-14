// File: c:/bsd/rigel/sort/C7/Dpq2.c
// Date: Wed Jul 14 13:46:00 2021

// dpq2.c
//
// This is the Oracle implementation of the version of Arrays.Sort used
// for sorting simple types. It is translated to a C version that sorts
// arrays of pointers to heap objects.
// Code to support parallelism has been removed.
//
// C translation by Nigel Horspool, 2021-04-18

/*
 * Copyright (c) 2009, 2019, Oracle and/or its affiliates. All rights reserved.
 * ORACLE PROPRIETARY/CONFIDENTIAL. Use is subject to license terms.
 */

/**
 * This class implements powerful and fully optimized versions, both
 * sequential and parallel, of the Dual-Pivot Quicksort algorithm by
 * Vladimir Yaroslavskiy, Jon Bentley and Josh Bloch. This algorithm
 * offers O(n log(n)) performance on all data sets, and is typically
 * faster than traditional (one-pivot) Quicksort implementations.
 *
 * There are also additional algorithms, invoked from the Dual-Pivot
 * Quicksort, such as mixed insertion sort, merging of runs and heap
 * sort, counting sort and parallel merge sort.
 *
 * @author Vladimir Yaroslavskiy
 * @author Jon Bentley
 * @author Josh Bloch
 * @author Doug Lea
 *
 * @version 2018.08.18
 *
 * @since 1.7 * 14
 */


//#include <math.h>
//#include <stdio.h>
//#include <stdlib.h>

//#include "utils.h"
//#include "heapsort.h"
//#include "insertionsort.h"
//#include "dpq2.h"

/**
 * Max array size to use mixed insertion sort.
 */
#define MAX_MIXED_INSERTION_SORT_SIZE 65

/**
 * Max array size to use insertion sort.
 */
#define  MAX_INSERTION_SORT_SIZE 44

/**
 * Min array size to try merging of runs.
 */
#define  MIN_TRY_MERGE_SIZE (4 << 10)

/**
 * Min size of the first run to continue with scanning.
 */
#define  MIN_FIRST_RUN_SIZE 16

/**
 * Min factor for the first runs to continue scanning.
 */
#define  MIN_FIRST_RUNS_FACTOR 7

/**
 * Max capacity of the index array for tracking runs.
 */
#define  MAX_RUN_CAPACITY 5 << 10

/**
 * Min number of runs, required by parallel merging.
 */
#define MIN_RUN_COUNT 4

/**
 * Min size of a byte array to use counting sort.
 */
#define  MIN_BYTE_COUNTING_SORT_SIZE 64

/**
 * Threshold of mixed insertion sort is incremented by this value.
 */
#define  DELTA (3 << 1)

/**
 * Max recursive partitioning depth before using heap sort.
 */
#define  MAX_RECURSION_DEPTH (64 * DELTA)

typedef int (*COMPAREFN)(const void *, const void *);

// forward declarations
static void dpq2b(void **a, int bits, int low, int high, COMPAREFN compareXY);
static void mixedInsertionSort(void **a, int low, int end, int high, COMPAREFN compareXY);
static int tryMergeRuns(void **a, int low, int size, COMPAREFN compareXY);
static void **mergeRuns(void **a, void **b, int offset,
        int aim, int *run, int lo, int hi, COMPAREFN compareXY);
static void mergeParts(void **dst, int k, void **a1, int lo1, int hi1,
        void **a2, int lo2, int hi2, COMPAREFN compareXY);

/**
 * Sorts the specified array using the Dual-Pivot Quicksort and/or
 * other sorts in special-cases.
 *
 * a: the array to be sorted
 * low: the index of the first element to be sorted
 * high: the index of the last element (inclusive) to be sorted
 */
static void dpq2(void **a, int low, int high, COMPAREFN compareXY) {
    dpq2b(a, 0, low, high+1, compareXY);
}


/**
 * Sorts the specified array using the Dual-Pivot Quicksort and/or
 * other sorts in special-cases, possibly with parallel partitions.
 *
 * a: the array to be sorted
 * bits: the combination of recursion depth and bit flag, where
 *        the right bit "0" indicates that array is the leftmost part
 * low: the index of the first element, inclusive, to be sorted
 * high: the index of the last element (exclusive) to be sorted
 */
static void dpq2b(void **a, int bits, int low, int high, COMPAREFN compareXY) {
    for( ; ; ) {
        int end = high - 1, size = high - low;

        if (size <= 1)
            return;

        /*
         * Run mixed insertion sort on small non-leftmost parts.
         */
        if (size < MAX_MIXED_INSERTION_SORT_SIZE + bits && (bits & 1) > 0) {
            mixedInsertionSort(a, low, high - 3 * ((size >> 5) << 3), high, compareXY);
            return;
        }

        /*
         * Invoke insertion sort on small leftmost part.
         */
        if (size < MAX_INSERTION_SORT_SIZE) {
            insertionsort(a, low, end, compareXY);
            return;
        }

        /*
         * Check if the whole array or large non-leftmost
         * parts are nearly sorted and then merge runs.
         */
        if ((bits == 0 || size > MIN_TRY_MERGE_SIZE && (bits & 1) > 0)
                && tryMergeRuns(a, low, size, compareXY)) {
            return;
        }

        /*
         * Switch to heap sort if execution
         * time is becoming quadratic.
         */
        if ((bits += DELTA) > MAX_RECURSION_DEPTH) {
            //heapsort(a, low, high-1, compareXY);
            heapc(a, low, end, compareXY);
            return;
        }

        /*
         * Use an inexpensive approximation of the golden ratio
         * to select five sample elements and determine pivots.
         */
        int step = (size >> 3) * 3 + 3;

        /*
         * Five elements around (and including) the central element
         * will be used for pivot selection as described below. The
         * unequal choice of spacing these elements was empirically
         * determined to work well on a wide variety of inputs.
         */
        int e1 = low + step;
        int e5 = end - step;
        int e3 = (e1 + e5) >> 1;
        int e2 = (e1 + e3) >> 1;
        int e4 = (e3 + e5) >> 1;
        void *a3 = a[e3];

        /*
         * Sort these elements in place by the combination
         * of 4-element sorting network and insertion sort.
         *
         *    5 ------o-----------o------------
         *            |           |
         *    4 ------|-----o-----o-----o------
         *            |     |           |
         *    2 ------o-----|-----o-----o------
         *                  |     |
         *    1 ------------o-----o------------
         */
        if (compareXY(a[e5], a[e2]) < 0) { void *t = a[e5]; a[e5] = a[e2]; a[e2] = t; }
        if (compareXY(a[e4], a[e1]) < 0) { void *t = a[e4]; a[e4] = a[e1]; a[e1] = t; }
        if (compareXY(a[e5], a[e4]) < 0) { void *t = a[e5]; a[e5] = a[e4]; a[e4] = t; }
        if (compareXY(a[e2], a[e1]) < 0) { void *t = a[e2]; a[e2] = a[e1]; a[e1] = t; }
        if (compareXY(a[e4], a[e2]) < 0) { void *t = a[e4]; a[e4] = a[e2]; a[e2] = t; }

        if (compareXY(a3, a[e2]) < 0) {
            if (compareXY(a3, a[e1]) < 0) {
                a[e3] = a[e2]; a[e2] = a[e1]; a[e1] = a3;
            } else {
                a[e3] = a[e2]; a[e2] = a3;
            }
        } else if (compareXY(a3, a[e4]) > 0) {
            if (compareXY(a3, a[e5]) > 0) {
                a[e3] = a[e4]; a[e4] = a[e5]; a[e5] = a3;
            } else {
                a[e3] = a[e4]; a[e4] = a3;
            }
        }

        // Pointers
        int lower = low; // The index of the last element of the left part
        int upper = end; // The index of the first element of the right part

        /*
         * Partitioning with 2 pivots in case of different elements.
         */
        if (compareXY(a[e1], a[e2]) < 0 && compareXY(a[e2], a[e3]) < 0
            && compareXY(a[e3], a[e4]) < 0 && compareXY(a[e4], a[e5]) < 0) {

            /*
             * Use the first and fifth of the five sorted elements as
             * the pivots. These values are inexpensive approximation
             * of tertiles. Note, that pivot1 < pivot2.
             */
            void *pivot1 = a[e1];
            void *pivot2 = a[e5];

            /*
             * The first and the last elements to be sorted are moved
             * to the locations formerly occupied by the pivots. When
             * partitioning is completed, the pivots are swapped back
             * into their final positions, and excluded from the next
             * subsequent sorting.
             */
            a[e1] = a[lower];
            a[e5] = a[upper];

            /*
             * Skip elements, which are less or greater than the pivots.
             */
            while (compareXY(a[++lower], pivot1) < 0)
                ;
            while (compareXY(a[--upper], pivot2) > 0)
                ;

            /*
             * Backward 3-interval partitioning
             *
             *   left part                 central part          right part
             * +------------------------------------------------------------+
             * |  < pivot1  |   ?   |  pivot1 <= && <= pivot2  |  > pivot2  |
             * +------------------------------------------------------------+
             *             ^       ^                            ^
             *             |       |                            |
             *           lower     k                          upper
             *
             * Invariants:
             *
             *              all in (low, lower] < pivot1
             *    pivot1 <= all in (k, upper)  <= pivot2
             *              all in [upper, end) > pivot2
             *
             * Pointer k is the last index of ?-part
             */
	    int unused,k;
            for (unused = --lower, k = ++upper; --k > lower; ) {
                void *ak = a[k];

                if (compareXY(ak, pivot1) < 0) { // Move a[k] to the left side
                    while (lower < k) {
                        if (compareXY(a[++lower], pivot1) >= 0) {
                            if (compareXY(a[lower], pivot2) > 0) {
                                a[k] = a[--upper];
                                a[upper] = a[lower];
                            } else {
                                a[k] = a[lower];
                            }
                            a[lower] = ak;
                            break;
                        }
                    }
                } else if (compareXY(ak, pivot2) > 0) { // Move a[k] to the right side
                    a[k] = a[--upper];
                    a[upper] = ak;
                }
            }

            /*
             * Swap the pivots into their final positions.
             */
            a[low] = a[lower]; a[lower] = pivot1;
            a[end] = a[upper]; a[upper] = pivot2;

            /*
             * Sort non-left parts recursively (possibly in parallel),
             * excluding known pivots.
             */

            dpq2b(a, bits | 1, lower + 1, upper, compareXY);
            dpq2b(a, bits | 1, upper + 1, high, compareXY);


        } else { // Use single pivot in case of many equal elements
            /*
             * Use the third of the five sorted elements as the pivot.
             * This value is inexpensive approximation of the median.
             */
            void *pivot = a[e3];

            /*
             * The first element to be sorted is moved to the
             * location formerly occupied by the pivot. After
             * completion of partitioning the pivot is swapped
             * back into its final position, and excluded from
             * the next subsequent sorting.
             */
            a[e3] = a[lower];

            void *temp = a[e3];

            /*
             * Traditional 3-way (Dutch National Flag) partitioning
             *
             *   left part                 central part    right part
             * +------------------------------------------------------+
             * |   < pivot   |     ?     |   == pivot   |   > pivot   |
             * +------------------------------------------------------+
             *              ^           ^                ^
             *              |           |                |
             *            lower         k              upper
             *
             * Invariants:
             *
             *   all in (low, lower] < pivot
             *   all in (k, upper)  == pivot
             *   all in [upper, end] > pivot
             *
             * Pointer k is the last index of ?-part
             */
	    int k;
            for (k = ++upper; --k > lower; ) {
                void *ak = a[k];
                if (compareXY(ak, pivot) != 0) {
                    a[k] = pivot;   // Puts a sentinel at end of unknown region
                    if (compareXY(ak, pivot) < 0) { // Move a[k] to the left side
                         while (compareXY(a[++lower], pivot) < 0)
                                ;
                        // now lower refers to an element >= pivot

                        if (compareXY(a[lower], pivot)  > 0) {
                            // this section has been modified so that we do not
                            // duplicate the object used as the pivot
                            --upper;
                            a[k] = a[upper];
                            a[upper] = a[lower];
                        } else {  // lower refers to an element == pivot
                            // added code so that we do not duplicate
                            // the object used as the pivot
                            pivot = a[lower];
                        }
                        a[lower] = ak;
                    } else { // ak > pivot - Move a[k] to the right side
                        // this section has been modified so that we do not
                        // duplicate the object used as the pivot
                        --upper;
                        a[k] = a[upper];
                        a[upper] = ak;
                    }
                }
            }

            /*
             * Swap the pivot into its final position.
             */
            a[low] = a[lower]; a[lower] = pivot;

            /*
             * Sort the right part (possibly in parallel), excluding
             * known pivot. All elements from the central part are
             * equal and therefore already sorted.
             */

             dpq2b(a, bits | 1, upper, high, compareXY);

        }
        high = lower; // Iterate along the left part

    }
}

/**
 * Sorts the specified range of the array using mixed insertion sort.
 *
 * Mixed insertion sort is combination of simple insertion sort,
 * pin insertion sort and pair insertion sort.
 *
 * In the context of Dual-Pivot Quicksort, the pivot element
 * from the left part plays the role of sentinel, because it
 * is less than any elements from the given part. Therefore,
 * expensive check of the left range can be skipped on each
 * iteration unless it is the leftmost call.
 *
 * @param a the array to be sorted
 * @param low the index of the first element, inclusive, to be sorted
 * @param end the index of the last element for simple insertion sort
 * @param high the index of the last element, exclusive, to be sorted
 */
static void mixedInsertionSort(void **a, int low, int end, int high, COMPAREFN compareXY) {
    if (end == high) {

        /*
         * Invoke simple insertion sort on tiny array.
         */
        int i;
        for (i; ++low < end; ) {
            void *ai = a[i = low];

            while (compareXY(ai, a[--i]) < 0) {
                a[i + 1] = a[i];
            }
            a[i + 1] = ai;
        }
    } else {

        /*
         * Start with pin insertion sort on small part.
         *
         * Pin insertion sort is extended simple insertion sort.
         * The main idea of this sort is to put elements larger
         * than an element called pin to the end of array (the
         * proper area for such elements). It avoids expensive
         * movements of these elements through the whole array.
         */
        void *pin = a[end];
	int i,p;
        for (i, p = high; ++low < end; ) {
            void *ai = a[i = low];

            if (compareXY(ai, a[i - 1]) < 0) { // Small element

                /*
                 * Insert small element into sorted part.
                 */
                // a[i] = a[--i];    // WRONG!! undefined behaviour in C
                a[i] = a[i-1];
                i--;

                while (compareXY(ai, a[--i]) < 0) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;

            } else if (p > i && compareXY(ai, pin) > 0) { // Large element

                /*
                 * Find element smaller than pin.
                 */
                while (compareXY(a[--p], pin) > 0)
                    ;

                /*
                 * Swap it with large element.
                 */
                if (p > i) {
                    ai = a[p];
                    a[p] = a[i];
                }

                /*
                 * Insert small element into sorted part.
                 */
                while (compareXY(ai, a[--i]) < 0) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = ai;
            }
        }

        /*
         * Continue with pair insertion sort on remain part.
         */
        for ( ; low < high; ++low ) {
            int i = low;
            void *a1 = a[low];
            void *a2 = a[++low];

            /*
             * Insert two elements per iteration: at first, insert the
             * larger element and then insert the smaller element, but
             * from the position where the larger element was inserted.
             */
            if (compareXY(a1, a2) > 0) {

                while (compareXY(a1, a[--i]) < 0) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a1;

                while (compareXY(a2, a[--i]) < 0) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a2;

            } else if (compareXY(a1, a[i - 1]) < 0) {

                while (compareXY(a2, a[--i]) < 0) {
                    a[i + 2] = a[i];
                }
                a[++i + 1] = a2;

                while (compareXY(a1, a[--i]) < 0) {
                    a[i + 1] = a[i];
                }
                a[i + 1] = a1;
            }
        }
    }
}


/**
 * Tries to sort the specified range of the array.
 *
 * @param a the array to be sorted
 * @param low the index of the first element to be sorted
 * @param size the array size
 * @return 1 if finally sorted, 0 otherwise
 */
static int tryMergeRuns(void **a, int low, int size, COMPAREFN compareXY) {

    /*
     * The run array is constructed only if initial runs are
     * long enough to continue, run[i] then holds start index
     * of the i-th sequence of elements in non-descending order.
     */
    int *run = NULL;
    int runLength = 0; // size of run array (# of elements)
    int high = low + size;
    int count = 1, last = low;

    /*
     * Identify all possible runs.
     */
    int k;
    for (k = low + 1; k < high; ) {

        /*
         * Find the end index of the current run.
         */
        if (compareXY(a[k - 1], a[k]) < 0) {

            // Identify ascending sequence
            while (++k < high && compareXY(a[k - 1], a[k]) <= 0)
                ;

        } else if (compareXY(a[k - 1], a[k]) > 0) {

            // Identify descending sequence
            while (++k < high && compareXY(a[k - 1], a[k]) >= 0)
                ;

            // Reverse into ascending order
	    int i,j;
            for (i = last - 1, j = k; ++i < --j && compareXY(a[i], a[j]) > 0; ) {
                void **ai = a[i]; a[i] = a[j]; a[j] = ai;
            }
        } else { // Identify constant sequence
	  for (void **ak = a[k]; ++k < high && compareXY(ak, a[k]) == 0; );
	  /*
	    // alternative
	  while (1) {
	  Again: ;
	  void **ak = a[k]; 
	    if ( ++k < high && compareXY(ak, a[k]) == 0 ) 
	      goto Again;
	    else break;
	  }
	  */

            if (k < high) {
                continue;
            }
        }

        /*
         * Check special cases.
         */
        if (runLength == 0) {
            if (k == high) {

                /*
                 * The array is monotonous sequence,
                 * and therefore already sorted.
                 */
                return 1;
            }

            if (k - low < MIN_FIRST_RUN_SIZE) {

                /*
                 * The first run is too small
                 * to proceed with scanning.
                 */
                return 0;
            }
            runLength = ((size >> 10) | 0x7F) & 0x3FF;
            run = (int*)myMalloc("tryMergeRuns", runLength*sizeof(int));
            run[0] = low;

        } else if (compareXY(a[last - 1], a[last]) > 0) {

            if (count > (k - low) >> MIN_FIRST_RUNS_FACTOR) {

                /*
                 * The first runs are not long
                 * enough to continue scanning.
                 */
                free(run);
                return 0;
            }

            if (++count == MAX_RUN_CAPACITY) {

                /*
                 * Array is not highly structured.
                 */
                return 0;
            }

            if (count == runLength) {

                /*
                 * Increase capacity of index array.
                 */
                runLength <<= 1;
                run = (int*)realloc(run, runLength*sizeof(int));
            }
        }
        run[count] = (last = k);
    }

    /*
     * Merge runs of highly structured array.
     */
    if (count > 1) {
        void *b[size];  // b = calloc(size*sizeof(void*));
        mergeRuns(a, b, low, 1, run, 0, count, compareXY);
    }
    free(run);
    return 1;
}

/**
 * Merges the specified runs.
 *
 * @param a the source array
 * @param b the temporary buffer used in merging
 * @param offset the start index in the source, inclusive
 * @param aim specifies merging: to source ( > 0), buffer ( < 0) or any ( == 0)
 * @param run the start indexes of the runs, inclusive
 * @param lo the start index of the first run, inclusive
 * @param hi the start index of the last run, inclusive
 * @return the destination where runs are merged
 */
static void **mergeRuns(void **a, void **b, int offset,
        int aim, int *run, int lo, int hi, COMPAREFN compareXY) {

    if (hi - lo == 1) {
        if (aim >= 0) {
            return a;
        }
	int i,j,low;
        for (i = run[hi], j = i - offset, low = run[lo]; i > low;
            b[--j] = a[--i]
        );
        return b;
    }

    /*
     * Split into approximately equal parts.
     */
    int mi = lo;
    int rmi = (run[lo] + run[hi]) >> 1;
    while (run[++mi + 1] <= rmi)
        ;

    /*
     * Merge the left and right parts.
     */
    void **a1, **a2;

    a1 = mergeRuns(a, b, offset, -aim, run, lo, mi, compareXY);
    a2 = mergeRuns(a, b, offset,    0, run, mi, hi, compareXY);

    void **dst = a1 == a ? b : a;

    int k   = compareXY(a1, a) == 0 ? run[lo] - offset : run[lo];
    int lo1 = compareXY(a1, b) == 0 ? run[lo] - offset : run[lo];
    int hi1 = compareXY(a1, b) == 0 ? run[mi] - offset : run[mi];
    int lo2 = compareXY(a2, b) == 0 ? run[mi] - offset : run[mi];
    int hi2 = compareXY(a2, b) == 0 ? run[hi] - offset : run[hi];

    mergeParts(dst, k, a1, lo1, hi1, a2, lo2, hi2, compareXY);

    return dst;
}

/**
 * Merges the sorted parts.
 *
 * @param dst the destination where parts are merged
 * @param k the start index of the destination, inclusive
 * @param a1 the first part
 * @param lo1 the start index of the first part, inclusive
 * @param hi1 the end index of the first part, exclusive
 * @param a2 the second part
 * @param lo2 the start index of the second part, inclusive
 * @param hi2 the end index of the second part, exclusive
 */
static void mergeParts(void **dst, int k, void **a1, int lo1, int hi1,
        void **a2, int lo2, int hi2, COMPAREFN compareXY) {

    /*
     * Merge small parts sequentially.
     */
    while (lo1 < hi1 && lo2 < hi2) {
        dst[k++] = compareXY(a1[lo1], a2[lo2]) < 0 ? a1[lo1++] : a2[lo2++];
    }
    if (dst != a1 || k < lo1) {
        while (lo1 < hi1) {
            dst[k++] = a1[lo1++];
        }
    }
    if (dst != a2 || k < lo2) {
        while (lo2 < hi2) {
            dst[k++] = a2[lo2++];
        }
    }
}


