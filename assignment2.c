#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>

int summation(int start, int end)
{
	int sum = 0;
	if (start < end)
	{
		sum = ((end * (end + 1)) - (start * (start - 1))) / 2;
	}
	return sum;
}

int ith_part_start(int i, int N, int M)
{
	int part_size = N / M;
	int start = i * part_size;
	return start;
}

int ith_part_end(int i, int N, int M)
{
	int part_size = N / M;
	int end = (i < M - 1) ? ((i + 1) * part_size - 1) : N;
	return end;
}

int main(int argc, char **argv) 
{
	pid_t pid_1, pid_2;
	int port[2], status, N, M;

	N = atoi(argv[1]); // upper bound of the set of integers from 1 to N
	M = atoi(argv[2]); // number of partial sums to be calculated

	printf("parent(PID %d): process started\n\n", getpid());

	/* terminates the parent process if N or M are not in their allowed ranges */
	if (N < 1 || N > 100) {
		printf("parent(PID %d): error detected. N must be greater than or equal to 1 and smaller than or equal to 100\n", getpid());
		exit(1);
	}
	else if (M < 1 || M > 10) {
		printf("parent(PID %d): error detected. M must be greater than or equal to 1 and smaller than or equal to 10\n", getpid());
		exit(1);
	}
	
	/* forks child_1 from parent */
	printf("parent(PID %d): forking child_1\n", getpid());
	pid_1 = fork();

	/* checks that the fork was successful and makes parent wait for child_1 to finish execution
	   after child_1 has finished execution the parent process will then report completion and terminate */
	if (pid_1 > 0) {
		printf("parent(PID %d): fork successful for child_1(PID %d)\n", getpid(), pid_1);
		printf("parent(PID %d): waiting for child_1(PID %d) to complete\n\n", getpid(), pid_1);
		wait(NULL);
		printf("parent(PID %d): parent completed\n", getpid());
		exit(0);
	}

	/* terminates the parent process and notifies the user if child_1 could not be forked */
	else if (pid_1 == -1) {
		printf("parent(PID %d): there was an error and child_1 could not be created\n", getpid());
		exit(1);
	}

	printf("child_1(PID %d): process started from parent(PID %d)\n", getpid(), getppid());

	/* creates a pipe that will be used for communication between child_1 and its children */
	status = pipe(port);
	
	/* terminates child_1 and notifies the user if the pipe could not be created */
	if (status) {
		printf("child_1(PID %d): there was an error and the pipe could not be created\n", getpid());
		exit(1);
	}

	printf("child_1(PID %d): forking child_1.1....child_1.%d\n\n", getpid(), M);

	/* forks children processes child1_i from child_1
	   makes each child process calculate a partial sum using provided functions and write the result to the pipe before terminating
	   child_1 will not continue to execute code beyond this loop until all its children have terminated */
	for (int i = 0; i < M; i++) {
		pid_2 = fork();
		
		if(pid_2 == 0) {
			printf("child_1.%d(PID %d): fork() successful\n", i+1, getpid());
			int ith_sum = summation(ith_part_start(i, N, M), ith_part_end(i, N, M));
			printf("child_1.%d(PID %d): partial sum: [%d - %d] = %d\n", i+1, getpid(), ith_part_start(i, N, M), ith_part_end(i, N, M), ith_sum);
			write(port[1], &ith_sum, sizeof(int));
			exit(0);
		}

		wait(NULL);
	}

	int partial_sum = 0; // partial sum read from the pipe that will be added to total sum
	int total_sum = 0;   // sum of numbers from 1 to N

	/* reads partial sums from the pipe and adds them to the total sum */
	for (int i = 0; i < M; i++) {
		read(port[0], &partial_sum, sizeof(int));
		total_sum += partial_sum;
	}
	
	printf("\nchild_1(PID %d): total sum = %d\n", getpid(), total_sum);
	printf("child_1(PID %d): child_1 completed\n\n", getpid());
}