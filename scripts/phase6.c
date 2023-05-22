#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_NUMBERS 6
#define INPUT_SIZE (MAX_NUMBERS * 2 + 7)

int numbers[] = {1, 2, 3, 5, 6};
int length = sizeof(numbers) / sizeof(int);
int permutation[MAX_NUMBERS];
char input[INPUT_SIZE];
char *argv[2] = { "/home/laurie/bomb", NULL };

void swap(int* a, int* b)
{
	int temp = *a;
	*a = *b;
	*b = temp;
}

void try_combinations(int depth) {
	if (depth == length) {
		int pipefd[2];
		pipe(pipefd);

		// Set input
		snprintf(input, INPUT_SIZE, "4");
		for (int i = 0; i < length; i++) {
			char num_str[3];
			snprintf(num_str, 3, " %d", permutation[i]);
			strcat(input, num_str);
		}
		strcat(input, "\n");

		// Execute
		pid_t pid = fork();
		if (pid == 0) {
			write(pipefd[1], "Public speaking is very easy.\n", 30);
			write(pipefd[1], "1 2 6 24 120 720\n", 17);
			write(pipefd[1], "0 q 777\n", 8);
			write(pipefd[1], "9\n", 2);
			write(pipefd[1], "o`ekma\n", 7);
			write(pipefd[1], input, strlen(input));
			close(pipefd[1]);

			dup2(pipefd[0], 0);
			close(pipefd[0]);

			// Open /dev/null and duplicate it onto stdout
			int dev_null = open("/dev/null", O_WRONLY);
			if (dev_null == -1) {
				perror("/dev/null");
				exit(EXIT_FAILURE);
			}
			if (dup2(dev_null, STDOUT_FILENO) == -1) {
				perror("dup2");
				exit(EXIT_FAILURE);
			}
			close(dev_null);

			// Execute
			execvp(argv[0], argv);
			exit(EXIT_FAILURE);
		} else if (pid > 0) {
			close(pipefd[1]);

			int status;
			waitpid(pid, &status, 0);
			close(pipefd[0]);

			if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
				printf("Cracked: %s", input);
				fflush(stdout);
				exit(EXIT_SUCCESS);
			}
		}
	} else {
		for (int i = 0; i < length; i++) {
			int already_chosen = 0;
			for (int j = 0; j < depth; j++) {
				if (permutation[j] == numbers[i]) {
					already_chosen = 1;
					break;
				}
			}
			if (!already_chosen) {
				permutation[depth] = numbers[i];
				try_combinations(depth + 1);
			}
		}
	}
}

int main(void) {
	try_combinations(0);
	printf("Exhausted\n");
	return EXIT_FAILURE;
}
