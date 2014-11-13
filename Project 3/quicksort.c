#include <LPC17xx.h>
#include <RTL.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "quicksort.h"
#include "array_tools.h"

// Optimized threshold deteremined from testing
#define USE_INSERTION_SORT 50

typedef struct {
	array_t array;
	size_t a;
	size_t c;
} array_interval_t;

// Seperate struct for semaphore implementation
typedef struct{
	OS_SEM sem; 
	array_interval_t interval;
	size_t priority;
} qsortsem_task_parameters_t;

// Struct for priority implementation
typedef struct{
	array_interval_t interval;
	unsigned char priority;
} qsort_task_parameters_t;

/*
 * INSERTION SORT
 * Inline insertion sort function used in both implementations
 * of quicksort and only called when size of array is < threshold
 */
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

/*
 * QUICKSORT TASK (USING SEMAPHORES)
 */
__task void quick_sort_sem_task( void* void_ptr){
	
	// Make all declarations at head of fcn
	array_type *aptr, pivot, tmp;
	size_t pivotIndex, a, c, lo, hi, length;
	
	array_interval_t int_right, int_left;
	qsortsem_task_parameters_t *param, left_param, right_param;
	// End declarations
	
	param = (qsortsem_task_parameters_t *) void_ptr;
	aptr = param->interval.array.array;
	a = param->interval.a;
	c = param->interval.c;
	
	// Determine array length
	length = c - a + 1;
	
	// Use insertion sort if threshold is met
	if (length <= USE_INSERTION_SORT)
	{
		insertion_sort(param->interval);
		os_sem_send(&param->sem);
		os_tsk_delete_self();
	}
	
	// Pick a random index somewhere around the centre
	pivotIndex = (size_t) (a + (c - a + 1)*(1.0*rand()/RAND_MAX));
	pivot = aptr[pivotIndex];
	lo = a;
	hi = c;
	
	// While low and high indexes haven't met yet
	while(lo <= hi)
	{
		// Ensure everything to left of pivot is lower
		while(aptr[lo] < pivot)
			lo += 1;
		// Ensure everything to right of pivot is greater
		while(aptr[hi] > pivot)
			hi -= 1;
		// Swap low and high
		if (lo <= hi)
		{
			tmp = aptr[lo];
			aptr[lo] = aptr[hi];
			aptr[hi] = tmp;
			lo += 1;
			hi -= 1;
		}
	}
	
	// Assign array pointers to child tasks
	int_left.array = param->interval.array;
	int_right.array = param->interval.array;
	left_param.priority = param->priority;
	right_param.priority = param->priority;

	// Initialize semaphores for children to zero
	os_sem_init(&left_param.sem, 0);
	os_sem_init(&right_param.sem, 0);
	
	// Assign indices to children
	int_left.a = a;
	int_left.c = hi;
	int_right.a = lo;
	int_right.c = c;
	
	// Assign divided intervals to respective child tasks
	left_param.interval = int_left;
	right_param.interval = int_right;
	
	// Create left child and wait for that child to return 
	// before creating the right child
	os_tsk_create_ex( quick_sort_sem_task, left_param.priority, &left_param ); 
	os_sem_wait(&left_param.sem, 0xffff);
	os_tsk_create_ex( quick_sort_sem_task, right_param.priority, &right_param );
	os_sem_wait(&right_param.sem, 0xffff);
	
	// Once both children have returned return semaphore to
	// parent task and delete current task
	os_sem_send(&param->sem);
	os_tsk_delete_self();
}

/*
 * QUICKSORT TASK (USING PRIORITIES)
 */
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
	
	// Determine array length
	length = c - a + 1;
	
	// Use insertion sort if threshold is met
	if (length <= USE_INSERTION_SORT)
	{
		insertion_sort(param->interval);
		os_tsk_delete_self();
	}
	
	// Pick a random index somewhere around the centre
	pivotIndex = (size_t) (a + (c - a + 1)*(1.0*rand()/RAND_MAX));
	pivot = aptr[pivotIndex];
	lo = a;
	hi = c;
	
	// While low and high indexes haven't met yet
	while(lo <= hi)
	{
		// Ensure everything to left of pivot is lower
		while(aptr[lo] < pivot)
			lo += 1;
		// Ensure everything to right of pivot is greater
		while(aptr[hi] > pivot)
			hi -= 1;
		// Swap low and high
		if (lo <= hi)
		{
			tmp = aptr[lo];
			aptr[lo] = aptr[hi];
			aptr[hi] = tmp;
			lo += 1;
			hi -= 1;
		}
	}
	
	// Assign array pointers to child tasks
	int_left.array = param->interval.array;
	int_right.array = param->interval.array;
	
	// Increment child task priorties
	left_param.priority = param->priority + 1;
	right_param.priority = param->priority + 1;
	
	// Assign indices to children
	int_left.a = a;
	int_left.c = hi;
	int_right.a = lo;
	int_right.c = c;
	
	// Assign divided intervals to respective child tasks
	left_param.interval = int_left;
	right_param.interval = int_right;
	
	// Create left and right child tasks
	os_tsk_create_ex( quick_sort_task, left_param.priority, &left_param ); 
	os_tsk_create_ex( quick_sort_task, right_param.priority, &right_param );
	
	// Once both children have returned delete current task
	os_tsk_delete_self();
}

/*
 * QUICKSORT FUNCTION (using semaphores)
 * Calls and initializes the first quicksort task
 */
void quicksort_sem( array_t array ) {
	array_interval_t interval;
	qsortsem_task_parameters_t task_param;
	
	// Based on MTE 241 course notes--you can change this if you want
	//  - in the course notes, this sorts from a to c - 1
	interval.array =  array;
	interval.a     =  0;
	interval.c     =  array.length-1;
	
	task_param.interval = interval;

	// If you are using priorities, you can change this
	task_param.priority = 10;
	
	// Initialize semaphore to zero
	os_sem_init(&task_param.sem, 0);
	
	// Start the quick_sort_sem threading
	os_tsk_create_ex( quick_sort_sem_task, task_param.priority, &task_param ); 
	
	// Wait on sempahore from first task before printing results
	os_sem_wait(&task_param.sem, 0xffff);
}

/*
 * QUICKSORT FUNCTION (using priorities)
 * Calls and initializes the first quicksort task
 */
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