#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <signal.h>

#define MAXARGS 128
#define MAXLINE 128
#define MAXPROGRAM 100
#define LENGTH 10

void eval(char *cmdline);
int builtin_command(char **argv);
int parseline(char *buf, char **argv, char str[MAXPROGRAM][MAXARGS][LENGTH], int numArg[], int *count, int *mode);

char **environ;

int main() {
	char cmdline[MAXLINE];

	while(1) {
		printf("> ");
		fgets(cmdline, MAXLINE, stdin);
		if(feof(stdin))
			exit(0);

		eval(cmdline);
	}
}


void eval(char *cmdline) {
	char *argv[MAXARGS];
	char buf[MAXLINE];
	char str[MAXPROGRAM][MAXARGS][LENGTH];
	int numArg[MAXPROGRAM] = {0};
	int bg;
	int mode = 0;
	int i, j, k, count = 0;
	pid_t pid;

	strcpy(buf, cmdline);
	bg = parseline(buf, argv, str, numArg, &count, &mode);

	// ignore empty lines
	if(argv[0] == NULL)
		return;

	if(!builtin_command(argv)) {
		// how many programs need to run
		for(i = 0; i < count; i++) {
			for(j = 0; j < MAXARGS; j++)
				argv[j] = (char*)0;

			// copy arguments
			k = 0;
			for(j = 0; j < numArg[i]; j++)
				argv[j] = str[i][k++];

			if(mode == 0 || mode == 1) {
			if((pid = fork()) == 0) {
				if(execve(argv[0], argv, environ) < 0) {
					fprintf(stderr, "%s\n", strerror(errno));
					printf("%s: Command not found.\n", argv[0]);
					exit(0);
				}
			}
			if(mode == 1) {
				int status;
				waitpid(pid, &status, 0);

			}
			}

			else if (mode == 2)
				printf("%s", str[i]);
				while(system(str[i]) == -1);
				
		}

		if(!bg && !mode) { // foreground
			int status;
			if(waitpid(pid, &status, 0) < 0)
				fprintf(stderr, "%s\n", strerror(errno));
		}
		else // background
			printf("%d %s\n", getpid(), cmdline);
	}
	return;
}


int builtin_command(char **argv) {
	if(!strcmp(argv[0], "quit")) {
		kill(0, SIGKILL);
		exit(0);
	}
	if(!strcmp(argv[0], "&"))
		return 1;
	return 0;
}


int parseline(char *buf, char **argv, char str[MAXPROGRAM][MAXARGS][LENGTH], int numArg[], int *count, int *mode) {
	char *result = NULL;
	char *left = NULL;
	char temp[100];
	int argc = 0;
	int i = 0, j = 0, k;
	int bg;

	strcpy(temp, buf);
	temp[strlen(temp)-1] = '\0';
	result = strtok_r(temp, " ", &left);
	while(result) {
		argv[argc++] = result;
		result = strtok_r(NULL, " ", &left);
	}

	if(argc == 0)
		return 1;

	if(strcmp(argv[argc-1], "&") == 0) {
		bg = 1;
		for(k = 0; k < argc; k++) {
			if(strcmp(argv[k], "&") == 0) {
				i++;
				j = 0;
				(*count)++;
				continue;
			}
			else {
				strcpy(str[i][j++], argv[k]);
				numArg[i]++;
			}
		}
	}
	else {
		bg = 0;
		(*count)++;
		for(k = 0; k < argc; k++) {
			if(strcmp(argv[k], ";") == 0) {
				*mode = 1;
				break;
			}
			else if(strcmp(argv[k], "&&") == 0) {
				*mode = 2;
				break;
			}
			else if(strcmp(argv[k], "&") == 0)
				break;
		}

		if(*mode == 1) {
			for(k = 0; k < argc; k++) {
				if(strcmp(argv[k], ";") == 0) {
					i++;
					j = 0;
					(*count)++;
					continue;
				}
				else {
					strcpy(str[i][j++], argv[k]);
					numArg[i]++;
				}
			}
		}
		
		else if(*mode == 2) {
			for(k = 0; k < argc; k++) {
				if(strcmp(argv[k], "&&") == 0) {
					i++;
					j = 0;
					(*count)++;
					continue;
				}
				else {
					strcpy(str[i][j++], argv[k]);
					numArg[i]++;
				}
			}
		}

		else if(*mode == 0) {
			numArg[i] = argc;
			for(k = 0; k < argc; k++)
				strcpy(str[0][k++], argv[k]);
		}
	}

	return bg;
}
