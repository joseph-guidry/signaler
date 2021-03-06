#define _GNU_SOURCE	

#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <sysexits.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <math.h>

#define	MILLISECONDS_PER_SECOND	1000
#define MAX_INT 1000000
/* Prototypes for Signal Handling */
int set_signal_handler(void);
void signal_handler (int sig);

/* Driver for PID and Prime output */
static void display_data(int * start, int reverse_flag);

/* Find the next prime number */
int is_prime(int number);
int next_prime( int number, int reverse_flag);

/* Get and display date/time information */
static void display_date();

/* Waiting Timer */
void* timer(void *milliseconds);

static int current_prime;
static int reverse_flag = 1;

int main(int argc, char **argv)
{

	int sflag = 1, rflag = 0, eflag = MAX_INT;

	int option;

	while ( (option = getopt (argc, argv, "e:rs:")) != -1)
	{
		printf("%c: %s\n", option, optarg);
		switch(option)
		{
			case 'e':
				eflag = atoi(optarg);
				break;
			case 'r':
				reverse_flag = -1;
				break;
			case 's':
				sflag = atoi(optarg);
				break;
			default:
				exit(1);
		}
	}

	if (reverse_flag == -1 && sflag == 1)
	{
		perror("The -r options requires -s option\n");
	}

	//printf("Option values -e %d -r %d -s %d \n", eflag, rflag, sflag);

	current_prime = sflag;
    printf("We are looking for prime numbers\n");
	
	if (set_signal_handler() == -1)
	{
		perror("Could Not reassign signals\n");
	}

	pthread_t wait_thread;
	long wait_time = 800;

	for ( ; (current_prime = next_prime(current_prime, reverse_flag) ) < eflag; )
	{

		if ( pthread_create(&wait_thread, NULL, timer, (void *)wait_time) != 0 )
		{
			fprintf(stderr, "Failed to create thread\n");
			return(2);
		}


		;
		printf("[%u] %d \n", (int)getpid(), current_prime);
		display_date();

		if ( pthread_join(wait_thread, NULL) != 0)
		{
			fprintf(stderr, "Failed to join thread\n");
			return(3);
		}
		//printf("reverse_flag = %d \n", reverse_flag);

	}
}

int set_signal_handler(void)
{
	int retval = 0;

	struct sigaction action;
	action.sa_handler = signal_handler;
	action.sa_flags = 0;
	sigemptyset(&action.sa_mask);

	/* Check if the SIGHUP action has been properly set */
	if ( (retval = sigaction(SIGHUP, &action, NULL)) != 0)
		return retval;
	/* Check if the SIGUSR1 action has been properly set */
	if ( (retval = sigaction(SIGUSR1, &action, NULL)) != 0)
		return retval;
	/* Check if the SIGUSR2 action has been properly set */
	if ( (retval = sigaction(SIGUSR2, &action, NULL)) != 0)
		return retval;

	/* Check if the SIGINT action has been properly set */
	if ( (retval = sigaction(SIGINT, &action, NULL)) != 0)
		return retval;

	/* Check if the SIGTERM action has been properly set */
	if ( (retval = sigaction(SIGTERM, &action, NULL)) != 0)
		return retval;

	return retval;
}

void signal_handler (int sig)
{
	switch(sig)
	{
		case SIGHUP:
			printf("RECEIVED A SIGHUP signal. Restarting sequence at 2.\n");
			current_prime = 1; 
			break;
		case SIGUSR1:
			printf("RECEIVED A SIGUSR1 signal.");
			printf("SKIPPING NEXT PRIME %d \n", current_prime = next_prime(current_prime, reverse_flag));
			break;
		case SIGUSR2:
			printf("RECEIVED A SIGUSR2 signal. Reverse the prime sequence.\n");
			reverse_flag *= -1;
			break;
		case SIGINT:
			printf("RECEIVED A SIGINT. Do Nothing\n");
			break;
		case SIGTERM:
			printf("RECEIVED A SIGTERM. Do Nothing\n");
			break;
		default:
			printf("RECEIVED A %d signal \n", sig);
			break;
	}
}

int next_prime(int num, int reverse_flag)
{
	int c;
#if 0
	/* 
	   Test if number would go below the min number set 
	   by the -e option 
	*/
	if (num < end)
#endif

	if (num < 2)
		c = 2;
	else if (num == 2)
	{
		// This will allow the program to exit if it reaches 2 after down.
		if (reverse_flag == -1)
			exit(0);
		c = 3;
	}
	else if (num == 3 && reverse_flag == -1)
		c = 2;
	else if ( num & 1 )
	{
		// Value should be 1 for increment/ -1 to decrement
		num += (2 * reverse_flag);  
		c = is_prime(num) ?  num : next_prime(num, reverse_flag);
	} 
	else
		c = next_prime(num - 1, reverse_flag);

	return c;
}

int is_prime(int num)
{
	//int c = 0;
	if ((num & 1) == 0 )
		return num == 2;
	else
	{
		int i, limit = sqrt(num);
		for (i = 3; i <= limit; i+=2)
		{
			if (num % i == 0)
				return 0;
		}
	}
	return 1;
}

static void display_date()
{
	const unsigned char B_SIZE = 100;
	char output[B_SIZE];
	memset(&output, 0, B_SIZE);
	struct tm *tm_info;
	struct timeval tv;
	int m_second;

	gettimeofday(&tv, NULL);

	m_second = round((double)tv.tv_usec/MILLISECONDS_PER_SECOND);

	if ( m_second >= MILLISECONDS_PER_SECOND)
	{
		m_second -= MILLISECONDS_PER_SECOND;
		tv.tv_sec++;
	}

	tm_info = localtime(&tv.tv_sec);
	strftime(output, sizeof(output), "%Y:%m:%d %H:%M:%S", tm_info);
	printf("%s %0.1lf\n", output, (double)m_second/MILLISECONDS_PER_SECOND);
}

void* timer(void *milliseconds)
{
    clock_t t = clock();
    while((double)(clock()-t)/CLOCKS_PER_SEC < (long)milliseconds*0.001){
        ;	// Just keep waiting
    }
    return NULL;
}
	
