#include<stdio.h>
#include<stdint.h>
#include<time.h>
#include<stdlib.h>

/* Function pointer which points to functions with 
   input argument as (void *) and return as (void *) */
typedef void(*FUNC_POINTER) (void *); 

/* Input parameters to function add3Num */
typedef struct _addFuncInp
{
	int inp1;
	int inp2;
	int inp3;
}_addFuncInp;

/* Input parameters to function scram2Float */
typedef struct _floatScram
{
	int inpLen;
	float inpFlt[10];
}_floatScram;

/* Input to the different jobs */
typedef struct _st
{
	void *funcInp;
	FUNC_POINTER funcPtr;
}_st; 

/* Function definitions */
void *add3Num(void *);
void *genRandNum(void *);
void *scram2Float(void *);
void *runFunc(_st);

int main()
{
	_st st;

	/* Call to function 'add3Num' with it's inputs */
	_addFuncInp addInp = {3,4,5};
	st.funcInp = (void *)&addInp;
	st.funcPtr = (void *)add3Num;
	runFunc(st);

	/* Call to function 'genRandNum' with it's inputs */
	st.funcInp = NULL;
	st.funcPtr = (void *)genRandNum;
	runFunc(st);

	/* Call to function 'scram2Float' with it's inputs */
	_floatScram floatScram = {4, 0.2, 0.67, 0.33, -0.43};
	st.funcInp = (void *)&floatScram;
	st.funcPtr = (void *)scram2Float;
	runFunc(st);

	return 0;	
}

/* This is the function which calls different function 
   using function pointer */
void *runFunc(_st runSt)
{
	runSt.funcPtr(runSt.funcInp);

	return NULL;
}


/* Funtion to add three numbers */
void *add3Num(void *funcInp)
{
	/* Pointer to it's input structure types */
	_addFuncInp *recvInp;

	/* Typecaste the input pointer to it's input struct */
	recvInp = (_addFuncInp *)funcInp;

	int c = recvInp->inp1 + recvInp->inp2 + recvInp->inp3;

	printf("Add = %d\n",c);
}


void *genRandNum(void *funcInp)
{
	srand(time(NULL));
	int c = rand()%100;

	printf("Mult = %d\n",c);
}


void *scram2Float(void *funcInp)
{
	_floatScram *floatRecvInp;
	floatRecvInp = (_floatScram *)funcInp;

	float c[10];
	for(int idx=0; idx<floatRecvInp->inpLen; idx++)	
		c[idx] = 1 - 2*floatRecvInp->inpFlt[idx];

	for(int idx=0; idx<floatRecvInp->inpLen; idx++)	
		printf("%0.3f	",c[idx]);
	printf("\n");

	return NULL;
}