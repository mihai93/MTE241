#include <LPC17xx.h>
#include <RTL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "quicksort.h"
#include "array_tools.h"

// You decide what the threshold will be
#define USE_INSERTION_SORT 50

typedef struct {
	array_t array;
	size_t a;
	size_t c;
} array_interval_t;

typedef struct{
	array_interval_t interval;
	unsigned char priority;
} qsort_task_parameters_t;

// __inline void swap(array_type *aptr, size_t a, size_t b)
// {
// 	array_type tmp;
// 	tmp = aptr[a];
// 	aptr[a] = aptr[b];
// 	aptr[b] = tmp;
// }

__inline void insertion_sort( array_interval_t interval ) {
	size_t n, i, index; 
	array_type val; 
	array_type *a = interval.array.array;
	n = interval.c - interval.a + 1;
 
	for (i = interval.a + 1; i < interval.c + 1; i++) { 
		index = i;
		while (index > interval.a && a[index] < a[index-1]) {
			val = a[index];
			a[index] = a[index-1];
			a[index-1] = val;
			
			index--;
		}
	}
}

// __inline size_t partition(array_type *aptr, size_t a, size_t c)
// {
// 	int i;
// 	size_t b, pivotIndex;
// 	array_type pivot;
// 	
// 	b = a;
// 	pivotIndex = (size_t) (a + (c - a + 1)*(1.0*rand()/RAND_MAX));
// 	pivot = aptr[pivotIndex];
// 	
// 	// median of three steps
// 	if (aptr[a] > aptr[pivotIndex])
// 		swap(aptr, a, pivotIndex);
// 	
// 	if (aptr[a] > aptr[c])
// 		swap(aptr, a, c);
// 	
// 	if (aptr[pivotIndex] > aptr[c])
// 		swap(aptr, pivotIndex, c);
// 	
// 	swap(aptr, pivotIndex, c - 1);
// 	// end median of three
// 	
// 	for (i = a; i < c; i ++)
// 	{
// 		if (aptr[i] < pivot)
// 		{
// 			swap(aptr, i, b);
// 			b++;
// 		}
// 	}
// 	
// 	swap(aptr, c, b - 1); // -1 for median of three
// 	return b;
// }

__task void quick_sort_task( void* void_ptr){
	
	// Make all declarations at head of fcn
	array_type *aptr, pivot, tmp;
	size_t pivotIndex, a, c, lo, hi, length;
	
	array_interval_t int_right, int_left;
	qsort_task_parameters_t *param, left_param, right_param;
	// End declarations
	
	param = (qsort_task_parameters_t *) void_ptr;
	aptr = param->interval.array.array;
	a = param->interval.a;
	c = param->interval.c;
	
	length = c - a + 1;
	
	if (length <= USE_INSERTION_SORT)
	{
		insertion_sort(param->interval);
		os_tsk_delete_self();
	}
	
	pivotIndex = (size_t) (a + (c - a + 1)*(1.0*rand()/RAND_MAX));
	pivot = aptr[pivotIndex];
	lo = a;
	hi = c;
	
	while(lo <= hi)
	{
		while(aptr[lo] < pivot)
			lo += 1;
		while(aptr[hi] > pivot)
			hi -= 1;
		if (lo <= hi)
		{
			tmp = aptr[lo];
			aptr[lo] = aptr[hi];
			aptr[hi] = tmp;
			lo += 1;
			hi -= 1;
		}
	}
	
	int_left.array = param->interval.array;
	int_right.array = param->interval.array;
	left_param.priority = param->priority + 1;
	right_param.priority = param->priority + 1;
	
	int_left.a = a;
	int_left.c = hi;
	int_right.a = lo;
	int_right.c = c;
	
	left_param.interval = int_left;
	right_param.interval = int_right;
	
	os_tsk_create_ex( quick_sort_task, left_param.priority, &left_param ); 
	os_tsk_create_ex( quick_sort_task, right_param.priority, &right_param );
	
	os_tsk_delete_self();
}

void quicksort( array_t array ) {
	array_interval_t interval;
	qsort_task_parameters_t task_param;
	
	// Based on MTE 241 course notes--you can change this if you want
	//  - in the course notes, this sorts from a to c - 1
	interval.array =  array;
	interval.a     =  0;
	interval.c     =  array.length-1;
	
	task_param.interval = interval;

	// If you are using priorities, you can change this
	task_param.priority = 10;
	
	//start the quick_sort threading
	os_tsk_create_ex( quick_sort_task, task_param.priority, &task_param ); 
}
