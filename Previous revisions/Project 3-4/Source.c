#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#define DNUM 1000000
#define THREAD_LEVEL 10

#define SWAP(a, b)  a ^= b; b ^= a; a ^= b
#define GETEC(tsk_c) printf("ERROR; code from pthread_create() is %d\n", tsk_c); exit(-1)

//for sequential and parallel implementation
int partition(unsigned char a[], int left, int right);
void qsort_help(unsigned char a[], int left, int right);
void qsort(unsigned char a[], int size);
int isSorted(unsigned char a[], int size);

//for parallel implementation
void pqsort(double lyst[], int size, int tlevel);
void *pqsort_help(void *threadarg);

typedef struct tsk_data
{
    int left;
    int right;
	int level;
    unsigned char *a;
} tsk_param;

int partition( unsigned char a[], int l, int r)
{
    unsigned char pivot;
	int i, j, t;

    pivot = a[l];
    i = l; j = r+1;

    while(1)
    {
        do ++i; while( a[i] <= pivot && i <= r );
        do --j; while( a[j] > pivot );
        if( i >= j ) break;
		SWAP(a[i],a[j]);
    }

	SWAP(a[l],a[j]);

    return j;
}

void qsort(unsigned char a[], int size)
{
	qsort_help(a, 0, size-1);
}

void qsort_help(unsigned char a[], int l, int r)
{
	int pivot;
	if (l >= r) return;
	pivot = partition(a, l, r);
	qsort_help(a, l, pivot-1);
	qsort_help(a, pivot+1, r);
}

void pqsort(unsigned char a[], int size, int tsk_level)
{
	int tsk_c;
	void *status;
	pthread_attr_t attr;
	pthread_t thread;
	tsk_param tsk;

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	tsk.a = a;
	tsk.left = 0;
	tsk.right = size-1;
	tsk.level = tsk_level;

	tsk_c = pthread_create(&thread, &attr, pqsort_help, (void *) &tsk);

	if(tsk_c) {GETEC(tsk_c);}

	pthread_attr_destroy(&attr);
	tsk_c = pthread_join(thread, &status);
	
	if(tsk_c) {GETEC(tsk_c);}
}

void *pqsort_help(void *taskarg)
{
	int pivot, t, tsk_c;
	void *status;
	tsk_param *tsk_data;
	tsk_param tsk_array[2];
	pthread_attr_t attr;
	pthread_t tasks[2];

	tsk_data = (tsk_param *) taskarg;

	if (tsk_data->level <= 0 || tsk_data->left == tsk_data->right)
	{
		pqsort_help(tsk_data);
		pthread_exit(NULL);
	}

	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	pivot = partition(tsk_data->a, tsk_data->left,tsk_data->right);

	for (t=0;t<2;t++)
	{
		tsk_array[t].a = tsk_data->a;
		tsk_array[t].level = tsk_data->level - 1;
	}
	tsk_array[0].left = tsk_data->left;
	tsk_array[0].right = pivot - 1;
	tsk_array[1].left = pivot + 1;
	tsk_array[1].right = tsk_data->right;

	for (t = 0;t < 2;t++)
	{
		tsk_c = pthread_create(&tasks[t], &attr, pqsort_help, (void *) &tsk_array[t]);

		if(tsk_c) {GETEC(tsk_c);}
	}

	pthread_attr_destroy(&attr);

	for (t = 0; t < 2; t ++)
	{
		tsk_c = pthread_join(tasks[t], &status);

		if(tsk_c) {GETEC(tsk_c);}
	}

	pthread_exit(NULL);

}

int isSorted(unsigned char a[], int size)
{
	int i;
	for (i = 1; i < size; i ++)
	{
		if (a[i] < a[i-1]) 
		{
			printf("at loc %d, %e < %e \n", i, a[i], a[i-1]);
			return 0;
		}
	}
	return 1;
}

int main() 
{
	int i;
	unsigned char *a = (unsigned char *) malloc(10*sizeof(unsigned char));
	srand((unsigned)time(NULL));

	printf("Unsorted list:\n");
	for (i = 0; i < 10; i++)
	{
		a[i] = (rand() % 10 + 1);
		printf("%d \n", a[i]);
	}

	pqsort(a, 10, 10);

	printf("Sorted list:\n");
	for (i = 0; i < 10; i++)
	{
		printf("%d \n", a[i]);
	}

	if (!isSorted(a, 10))
	{
		printf("Oops, lyst did not get sorted by parallelQuicksort.\n");
	}
}