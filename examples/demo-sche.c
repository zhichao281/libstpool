/* COPYRIGHT (C) 2014 - 2020, piggy_xrh */

#include <stdio.h>
#include <stdlib.h>

#if defined(WIN32) || defined(_WIN32) || defined(_WIN64)
#ifdef _DEBUG
#ifdef _WIN64
#pragma comment(lib, "../x64/Debug/libmsglog.lib")
#pragma comment(lib, "../x64/Debug/libstpool.lib")
#else
#pragma comment(lib, "../Debug/libmsglog.lib")
#pragma comment(lib, "../Debug/libstpool.lib")
#endif
#else
#ifdef _WIN64
#pragma comment(lib, "../x64/Release/libmsglog.lib")
#pragma comment(lib, "../x64/Release/libstpool.lib")
#else
#pragma comment(lib, "../Release/libmsglog.lib")
#pragma comment(lib, "../Release/libstpool.lib")
#endif
#endif

#else
#include <stdint.h>
#include <sys/time.h>
#endif
#include "stpool.h"

/**
 * We extract the internal library Utils
 */
extern void us_start();
extern unsigned us_end();

/**
 * (log library)    depends  (task pool library) 
 * libmsglog.lib <-------------libstpool.lib 
 */
static void do_work(int *val) 
{
	*val += 100;
	*val *= 0.371;
}

void task_run(struct sttask *ptsk) 
{
	size_t i, j, sed = 20;
	
	for (i=0; i<sed; i++)
		for (j=0; j<sed; j++)
			do_work((int *)ptsk->task_arg);
	
	/**
	 * Do not call @printf in the test since it will waste our 
	 * so much time on competing the IO.
	 */
}

int main()
{
	time_t now;
	int i, times;
	int sum, *arg;
	stpool_t *pool;
		
	/** Creat a task pool */
	pool = stpool_create("mypool", /** pool name */
	                  eCAP_F_DYNAMIC|eCAP_F_SUSPEND|eCAP_F_ROUTINE|eCAP_F_WAIT_ALL,
					   50,  /** max servering threads */
			           0,   /** 0 servering threads that reserved for waiting for tasks */
			           1,   /** suspend the pool */
			           0);  /** default number of priority queue */

	/**
	 * Add tasks 
	 */
	times = 90000;
	arg = (int *)malloc(times * sizeof(int));
	for (i=0; i<times; i++) {
		/**
		 * It may take a long time to load a large amount of tasks 
		 * if the program is linked with the debug library 
		 */
		if (i % 4000 == 0 || (i + 1) ==times) {
			printf("\rLoading ... %.2f%%  ", (float)i * 100/ times);
			fflush(stdout);
		}
		arg[i] = i;
		stpool_add_routine(pool, "sche", task_run, NULL, (void *)&arg[i], NULL);	
	}
	printf("\nAfter having executed @stpool_add_routine for %d times:\n"
		   "--------------------------------------------------------\n%s\n", 
		   times, stpool_stat_print(pool));
	
	/**
	 * Wait for all tasks' being done. 
	 */
	{
		unsigned us;
	
		/**
		 * Wake up the pool to schedule tasks 
		 */
		stpool_resume(pool);		
		
		us_start();
		stpool_wait_all(pool, -1);
		us = us_end();

		printf("Test costs %.2f ms \n", (double)us / 1000);
	}

	/**
	 * Get the sum 
	 */
	for (i=0, sum=0; i<times; i++)
		sum += arg[i];
	free(arg);
	
	now = time(NULL);
	printf("--OK. finished. <sum: %d> %s\n%s\n", 
		sum, ctime(&now), stpool_stat_print(pool));

#if 0
	/**
	 * You can use debug library to watch the status of the pool 
	 */
	{
		int c;
		
		while ('q' != getchar()) {
			for (i=0; i<40; i++)
				stpool_add_routine(pool, "debug", task_run, NULL, &sum, NULL);	
		}

		/**
		 * Clear the stdio cache 
		 */
		while ((c=getchar()) && c != '\n' && c != EOF)
			;
	}
#endif
	getchar();
	/** 
	 * Release the pool 
	 */
	printf("Shut down the pool now.\n");
	stpool_release(pool);
	getchar();
		
	return 0;
}
