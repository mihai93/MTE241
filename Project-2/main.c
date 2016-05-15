//Yu Tung Kuo   ID : 20418147
//Mihai Listov  ID : 20432939
#include <stdio.h>
#include <limits.h>
#include "Header.h"
#include <stdint.h>
#include <math.h>
#include <time.h>

unsigned char mem[32768];
void* bucket[11];
int availableChunks = 1024;

/*
 * GET HEADER FUNCTION
 * Combines four byte header into single (easier to work with) 32 bit integer
 */
uint32_t getHeader(void *headerPos)
{
	unsigned char headerByte[4] = {0,0,0,0};
	void *tempPtr = NULL;
	int i = 0;
	uint32_t headerVal = 0;

	tempPtr = headerPos;

	for (i=0;i<4;i++)
		headerByte[i] = *((unsigned char*)tempPtr+i);

	headerVal = headerByte[0] << 24 | headerByte[1] << 16 | 
		headerByte[2] << 8 | headerByte[3];
	return headerVal;
}

/*
 * HEADER RETRIEVAL FUNCTIONS
 * Simply masks getHeader return value for desired result
 */
uint16_t getPrevBucketBlk(void *headerPos)
{
	unsigned char headerByte[2] = {0,0};
	void *tempPtr = NULL;
	int i = 0;
	uint16_t prevBucketBlk = 0;

	tempPtr = headerPos;

	// Read bytes five and six from start of header
	for (i=4;i<=5;i++)
		headerByte[i-4] = *((unsigned char*)tempPtr+i);

	prevBucketBlk = headerByte[0] << 2 | (headerByte[1] & 48) >> 6;
	return prevBucketBlk;
}
uint16_t getNextBucketBlk(void *headerPos)
{
	unsigned char headerByte[2] = {0,0};
	void *tempPtr = NULL;
	int i = 0;
	uint16_t nextBucketBlk = 0;

	tempPtr = headerPos;

	// Read bytes five and six from start of header
	for (i=5;i<=6;i++)
		headerByte[i-5] = *((unsigned char*)tempPtr+i);

	nextBucketBlk = (headerByte[0] & ~48) << 4 | (headerByte[1] & 240) >> 4;
	return nextBucketBlk;
}
uint16_t getPrevBlk(void *headerPos)
{
	uint32_t headerVal = getHeader(headerPos);
	uint16_t PrevBlk = ((headerVal >> 16) & 65472) >> 6;
	return PrevBlk;

}
uint16_t getNextBlk(void *headerPos)
{
	uint32_t headerVal = getHeader(headerPos);
	uint16_t nextIndex = ((headerVal >> 6) & 65472) >> 6;
	return nextIndex;

}
uint16_t getSize(void *headerPos)
{
	uint32_t headerVal = getHeader(headerPos);
	uint16_t size = ((headerVal << 4) & 65472) >> 6;
	return size;

}
uint16_t getAllocFlag(void *headerPos)
{
	uint32_t headerVal = getHeader(headerPos);
	uint16_t getAllocFlag = ((headerVal << 14) & 32768) >> 15;
	return getAllocFlag;
}

/*
 * HEADER SET FUNCTION
 * Splits 32-bit header int into four bytes for placing in mem array
 */
void setHeader(void *initPos, uint32_t headerVal)
{
	unsigned char headerByte[4] = {0,0,0,0};
	int i = 0;
	void *tempPtr = NULL;

	tempPtr = initPos;

	headerByte[0] = (headerVal & 4278190080) >> 24;
	headerByte[1] = (headerVal & ~4278255615) >> 16;
	headerByte[2] = (headerVal & ~4294902015) >> 8;
	headerByte[3] = headerVal & ~4294967040;

	for (i=0;i<4;i++)
		*((unsigned char*)tempPtr+i) = headerByte[i];


}
/*
 * HEADER SET FUNCTIONS
 * Generates new 32 bit int based on desired input and
 * calls setHeader to modify actual header in mem array
 */
void setPrevBucketBlk(void *headerPos, uint16_t prevBlk)
{
	unsigned char headerByte[2] = {0,0};
	int i = 0;
	void *tempPtr = NULL;

	tempPtr = headerPos;

	headerByte[0] = (prevBlk & ~64515) >> 2;
	headerByte[1] = (prevBlk & ~65532);

	for (i=4;i<=5;i++)
		*((unsigned char*)tempPtr+i) = headerByte[i-4];
}
void setNextBucketBlk(void *headerPos, uint16_t nextBlk)
{
	unsigned char headerByte[2] = {0,0};
		int i = 0;
		void *tempPtr = NULL;

		tempPtr = headerPos;

		headerByte[0] = (nextBlk & ~64515) >> 2;
		headerByte[1] = (nextBlk & ~65532);

		for (i=5;i<=6;i++)
			*((unsigned char*)tempPtr+i) = headerByte[i-4];
}
void setPrevBlk(void *headerPos, uint16_t prevBlk)
{
	uint32_t temp = 0,
			headerVal = getHeader(headerPos);
	temp = headerVal & ~4290772992;
	headerVal = prevBlk << 22 | temp;
	setHeader(headerPos, headerVal);
}
void setNextBlk(void *headerPos, uint16_t nextBlk)
{
	uint32_t temp = 0,
			headerVal = getHeader(headerPos);
	temp = headerVal & 4290777087;
	headerVal = nextBlk << 12 | temp;
	setHeader(headerPos, headerVal);
}
void setSize(void *headerPos, uint16_t blkSize)
{
	uint32_t temp = 0,
			headerVal = getHeader(headerPos);
	temp = headerVal & 4294963203;
	headerVal = blkSize << 2 | temp;
	setHeader(headerPos, headerVal);
}
void setAllocFlag(void *headerPos, int flag)
{
	uint32_t temp = 0,
			headerVal = getHeader(headerPos);
	temp = headerVal & 4294967292;
	headerVal = temp | flag << 1;
	setHeader(headerPos, headerVal);
}

/*
 * HEADER PRINT FUNCTION
 * Function used for testing, prints all relevant header information
 */
void printAll(void *initPos)
{
	void *tempPtr = NULL;
	uint32_t headerVal = 0;

	tempPtr = initPos;

	printf("PrevBlk: %d \nNextBlk: %d \nSize: %d \nAllocFlag: %d \n",
		getPrevBlk(tempPtr), getNextBlk(tempPtr), getSize(tempPtr), getAllocFlag(tempPtr));
}

/*
 * Allocates block of size n bytes and returns a pointer to allocated block
 */
void *half_alloc(size_t n) {

	void *allocMem = NULL;

	int i, rangeMin, rangeMax,
		sizeReq = n+4,
		chunks = ceil((double)sizeReq/32), // Get number of 32 byte chunks and round up
		prefBucket = 0,
		bucketSize = pow(2, prefBucket+5)-1; // Set first bucket size

	// Check to see if user is requesting too much or if pool is full
	if (chunks > availableChunks || availableChunks <= 0)
	{
		printf("\n\nSIZE NOT AVAILABLE\n\n");
		return NULL;
	}

	// Search for an appropriate bucket
	while((sizeReq  > bucketSize || bucket[prefBucket] == NULL)
			&& prefBucket <= 10)
	{
		prefBucket++;
		bucketSize = pow(2, prefBucket+5)-1;
	}

	// Special case where size 1024 is request and pool is empty
	if (prefBucket == 11 && bucket[10] != NULL)
	{
		availableChunks -= 1024;

		setNextBlk(bucket[10], 0);
		setSize(bucket[10], 0);
		setAllocFlag(bucket[10], 1);

		printf("[Allocated Block]\n");
		printAll(bucket[10]);

		allocMem = bucket[10];

		for (i=0;i<11;i++)
			bucket[i] = NULL;

		printf("\n/***WARNING: MEMORY POOL NOW FULL***/\n");

	} else // All other cases
	{
		void *initPos = bucket[prefBucket], *currPos;
		availableChunks -= chunks;

		if (availableChunks < 0)
		{
			printf("EXCEPTION: MEMORY POOL FULL\n\n");
			return NULL;
		}

		// Set allocated block header
		currPos = initPos;
		setNextBlk(currPos, chunks);
		setSize(currPos, chunks);
		setAllocFlag(currPos, 1);
		printf("\n[Allocated Block]\n");
		printAll(currPos);
		
		// If there is any more space left is allocate setup an unallocated block
		if (availableChunks > 0)
		{
			currPos = (unsigned char*)initPos + chunks*32;
			setPrevBlk(currPos, chunks);
			setNextBlk(currPos, 0);
			setSize(currPos, availableChunks);
			setAllocFlag(currPos, 0);
			printf("\n[New Unallocated Block]\n");
			printAll(currPos);


			// Determine bucket location for new unallocated block
			prefBucket = 0;
			bucketSize = pow(2, prefBucket+5)-1;

			while((availableChunks*32  > bucketSize) && prefBucket <= 10)
			{
				prefBucket++;
				bucketSize = pow(2, prefBucket+5)-1;
			}
		}

		// If pool is full warn user
		if (availableChunks <= 0)
		{
			for (i=0;i<=10;i++)
				bucket[i] = NULL;
			printf("\n\nWARNING: MEMORY POOL NOW FULL\n\n");
		} else
			bucket[prefBucket] = currPos; // Point bucker to unallocated block

		allocMem = initPos; // Set allocated memory pointer to initially unallocated block
	}

	printf("Chunks Req: %d \nChunks Available: %d \n", chunks, availableChunks);
	printf("--------------------------------------------------\n");

	return allocMem;
}

void half_init() {

	int i;

	for(i = 0; i < 10; i++)
		bucket[i] = NULL;

	bucket[10] = &mem[0];
	setHeader(bucket[10], 0);
}

void half_free(void *n ) {
	void *freePtr, *prevPtr, *nextPtr;
	int chunks = getSize(freePtr), // Get number of 32 byte chunks and round up
		prefBucket = 0,
		bucketSize = pow(2, prefBucket+5)-1;

	freePtr = n;
	setAllocFlag(freePtr, 0);
	availableChunks += chunks;

	if (getPrevBlk(freePtr) != 0)
	{
		prevPtr = (unsigned char*)freePtr - getPrevBlk(freePtr)*32;

		if (getAllocFlag(prevPtr) == 0)
		{
			setSize(prevPtr, getSize(prevPtr)+getSize(freePtr));
			setNextBlk(prevPtr, getNextBlk(prevPtr)+getNextBlk(freePtr));
			setHeader(freePtr, 0);

			freePtr = prevPtr; // Point freed block pointer to block previous to it
		}
	}

	if (getNextBlk(freePtr) != 0)
	{
		nextPtr = (unsigned char*)freePtr + getNextBlk(freePtr)*32;

		if (getAllocFlag(nextPtr) == 0)
		{
			setSize(freePtr, getSize(freePtr)+getSize(nextPtr));
			setNextBlk(freePtr, 0);
			setAllocFlag(freePtr, 0);
			setHeader(nextPtr, 0);
		}
	}

	// Point bin to freed block
	while((chunks*32  > bucketSize) && prefBucket <= 10)
	{
		prefBucket++;
		bucketSize = pow(2, prefBucket+5)-1;
	}

	bucket[prefBucket] = freePtr;

	printf("--------------------------------------------------\n");
	printf("Chunks Available: %d \n", availableChunks);
	printf("--------------------------------------------------\n");

}

int main( void )
{
	void *allocatedPtr, *nextPtr, *nextNextPtr;
	half_init();

	allocatedPtr = half_alloc(6112);
	half_alloc(3300);
	half_alloc(8572);

	half_free(allocatedPtr);
	printAll(allocatedPtr);

	nextPtr = (unsigned char*)allocatedPtr + getNextBlk(allocatedPtr)*32;
	//printf("\n\n", allocatedPtr);
	//half_free(nextPtr);
	//printAll(nextPtr);

	nextNextPtr = (unsigned char*)nextPtr + getNextBlk(nextPtr)*32;
	printf("\n\n", allocatedPtr);
	half_free(nextNextPtr);
	printAll(nextNextPtr);

	allocatedPtr = half_alloc(2556);
	printAll((unsigned char*)allocatedPtr + getNextBlk(allocatedPtr)*32);

	allocatedPtr = half_alloc(6112);
	printAll(allocatedPtr);
}
