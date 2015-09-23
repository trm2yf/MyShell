/* Compile with: g++ -Wall –Werror -o shell shell.c */
using namespace std;
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
 
/* The array below will hold the arguments: args[0] is the command. */
static char* args[512];
pid_t pid;
int command_pipe[2];
 
#define READ  0
#define WRITE 1
 
/*
 * Handle commands separatly
 * input: return value from previous command (pipe file descriptor)
 * first: 1 if first command in pipe-sequence (no input from previous pipe)
 * last: 1 if last command in pipe-sequence (no input from previous pipe)
 *
 * EXAMPLE: If you type "ls | grep shell | wc":
 *    fd1 = command(0, 1, 0), with args[0] = "ls"
 *    fd2 = command(fd1, 0, 0), with args[0] = "grep" and args[1] = "shell"
 *    fd3 = command(fd2, 0, 1), with args[0] = "wc"
 *
 * So if 'command' returns a file descriptor, the next 'command' has this
 * descriptor as its 'input'.
 */

static void redirect(char** cmd, char**n file){
  int fds[2];
  int count;
  int fd;
  char c;
  //pid_t pid;


  pipe(fds);

  if (fork() == 0){
    fd = open(file[0], O_RDONLY);

    if (fd<0){
      printf("Error: child process 1 redirect");
      return;
    }
    dup2(fds[0],0);
    close(fds[1]);
    while ((count = read(0, &c, 1)) > 0){
      write(fd, &c, 1);
    }
  }
  else if ((pid = fork()) == 0){
    dup2(fds[1], 1);
    close(fds[0]);
    execvp(cmd[0], cmd);
    perror("execvp failed");
  }
  else{
    waitpid(pid, NULL, 0);
    close(fds[0]);
    close(fds[1]);
  }
}
    

static int command(int input, int first, int last)
{
	int pipettes[2];
	cout << "---IN COMMAND FUNC---- " << endl;
	cout << "COMMAND: " << args[0]<< endl;
	cout << "INPUT: " << input << " (ret val from prev command)" << endl;
	cout << "FIRST: " << first << " (1 if  first in pip)" << endl;
	cout << "LAST: " << last <<" (1 if last command in a pipe seq)" << endl;
	// Invoke pipe
	pipe( pipettes );
	cout << "Creating pipe: "<< pipettes[0] << " to " << pipettes[1]  << endl;
	pid = fork();
 
	/*
	 	STDIN --> O --> O --> O --> STDOUT
	*/
	// Child Process
	if (pid == 0) {
	  
	  cout << "Child process "<< getpid() << " First: " << first << " LAST: "<< last<< " IN: " << input << endl;
		if (first == 1 && last == 0 && input == 0) {
			// First command
		        //replace standard output with output part of pipe
			dup2( pipettes[WRITE], STDOUT_FILENO );
			close(pipettes[READ]);
			cout << "FIRST dup2  - old: " << pipettes[WRITE] << " new: " << "stdout fds" << endl; 
		} else if (first == 0 && last == 0 && input != 0) {
			// All internal commands  input is from previous command
		        // Replace standard input with input from previous command
		        // Replace standard output with output part of pipe
		        
	cout << "MIDDLE dup2 - old: " << input << " new: " << "0(stdin)"<< endl; 		cout << "MIDDLE dup2 - old: " << pipettes[WRITE] << " new: " << "1(stdout)" << endl;
 // first/inner ->  middle
	dup2(input, STDIN_FILENO);
 // middle/inner ->  middle/last
			dup2(pipettes[WRITE], STDOUT_FILENO);
		} else {
			// Last command
		        // replace standard input with input from previous command
		  // middle/inner/first -> last
		  cout << "LAST dup2 - old: " << pipettes[WRITE] << " new: " << "0(stdin) " << endl; 
		      dup2( input, STDIN_FILENO );
	close(pipettes[WRITE]);
		}
 
		if (execvp( args[0], args) == -1)
			_exit(EXIT_FAILURE); // If child fails
	}

	if (input != 0) 
		close(input);
 
	// done
	close(pipettes[WRITE]);
 
	// If it's the last command close read
	if (last == 1)
		close(pipettes[READ]);
		cout << "----END COMMAND FUNC----- " << endl;
	return pipettes[READ];
}
 
/* Final cleanup, 'wait' for processes to terminate.
 *  n : Number of times 'command' was invoked.
 */
static void cleanup(int n)
{
	int i;
	//cout << "Clean "<< n << endl;
	for (i = 0; i < n; ++i) 
		wait(NULL); 
}
 
static int run(char* cmd, int input, int first, int last);
static char line[1024];
static int n = 0; /* number of calls to 'command' */
 
int main()
{
	printf("SIMPLE SHELL: Type 'exit' or send EOF to exit.\n");
	while (1) {
		/* Print the command prompt */
		printf("$> ");
		fflush(NULL);
 
		/* Read a command line */
		if (!fgets(line, 1024, stdin)) 
			return 0;
 
		int input = 0;
		int first = 1;
 
		char* cmd = line;
		// pointer to the first found pipe in command arg
		char* next = strchr(cmd, '|'); /* Find first '|' */
	
		while (next != NULL) {
			/* 'next' points to '|' */
			*next = '\0';
			cout << "INPUT BEFORE : " << input << endl;
			input = run(cmd, input, first, 0);
			cout << "INPUT AFTER: " << input << endl;
			// find next command (split by pipe)
			cout <<"Previous CMD: " << cmd << endl;
			cmd = next + 1;
			cout <<"New CMD:  " << cmd << endl;
			next = strchr(cmd, '|'); /* Find next '|' */
			first = 0;
		}
		//input = 0 if no args
		input = run(cmd, input, first, 1);
		cleanup(n);
		n = 0;
	}
	return 0;
}
 
static void split(char* cmd);
 
static int run(char* cmd, int input, int first, int last)
{
	cout << "----IN RUN---" << endl;  
cout << "Spliting CMD into args " << cmd << endl;
	split(cmd);
 cout << "Split CMD " << cmd << endl;
	if (args[0] != NULL) {
		if (strcmp(args[0], "exit") == 0) 
			exit(0);
		n += 1;
			cout << "----END1 RUN---" << endl; 
		return command(input, first, last);
	}
	cout << "----END2 RUN---" << endl; 
	return 0;
}
 
static char* skipwhite(char* s)
{
	while (isspace(*s)) ++s;
	return s;
}
 
static void split(char* cmd)
{
  cout << "CMD "<< cmd << endl;
	cmd = skipwhite(cmd);
	char* next = strchr(cmd, ' ');
	int i = 0;
 
	while(next != NULL) {
		next[0] = '\0';
		args[i] = cmd;
		cout << "SETTING ARG: " << i <<" " << cmd << endl;
		++i;
		cmd = skipwhite(next + 1);
		next = strchr(cmd, ' ');
	}

	if (cmd[0] != '\0') {
		args[i] = cmd;
	cout << "SETTING ARG: " << i <<" " << cmd << endl;
		next = strchr(cmd, '\n');
		next[0] = '\0';
		++i; 
	}
 
	args[i] = NULL;
}
