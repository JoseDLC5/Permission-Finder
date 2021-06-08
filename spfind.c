/*******************************************************************************
 * Name        : spfind.c
 * Author      : Breanna Shinn & Jose de la Cruz
 * Date        : 03/30/2021
 * Description : Sorted pfind using pipes, sort and pfind.
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char *argv[]) {

    int pfind_to_sort[2], sort_to_parent[2];
    if (pipe(pfind_to_sort) < 0) {
        fprintf(stderr, "Error: Cannot create pipe pfind_to_sort. %s.\n",
                strerror(errno));
        return EXIT_FAILURE;
    }
    if (pipe(sort_to_parent) < 0) {
        fprintf(stderr, "Error: Cannot create pipe sort_to_parent. %s.\n",
                strerror(errno));
        return EXIT_FAILURE;
    }

    pid_t pid[2];
    if ((pid[0] = fork()) == 0) {
    	//pfind
    	// Close all unrelated file descriptors.
        close(sort_to_parent[0]);
        close(sort_to_parent[1]);
        close(pfind_to_sort[0]);

        //dup pipe to STDOUT
        dup2(pfind_to_sort[1], STDOUT_FILENO);

        if (execv("./pfind",argv) < 0) {
            fprintf(stderr, "Error: pfind failed. %s.\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }

    if ((pid[1] = fork()) == 0) {
        // sort
        // Close all unrelated file descriptors.
        close(pfind_to_sort[1]);
        close(sort_to_parent[0]);

        //dup STDIN and STDOUT
        dup2(pfind_to_sort[0], STDIN_FILENO);
        dup2(sort_to_parent[1], STDOUT_FILENO);

        if (execlp("sort", "sort", NULL) < 0) {
            fprintf(stderr, "Error: sort failed. %s.\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
    }
    //parent
    // Close all unrelated file descriptors.
    close(pfind_to_sort[0]);
    close(pfind_to_sort[1]);
    close(sort_to_parent[1]);

    //dup STDIN
    dup2(sort_to_parent[0], STDIN_FILENO);

    int lines = 0;
    char buffer[4096];
    while (1) {
        ssize_t count = read(STDIN_FILENO, buffer, sizeof(buffer));
        if (count == -1) {
            if (errno == EINTR) {
                continue;
            } else {
                perror("read()");
                exit(EXIT_FAILURE);
            }
        } else if (count == 0) {
            break;
        } else {
        	for (int i = 0; i < count; ++i)
        	{
        		if (buffer[i] == '\n')
        		{
        			lines++;
        		}
        	}
            write(STDOUT_FILENO, buffer, count);
        }
    }
    
    

    close(sort_to_parent[0]);
    // Don't forget to wait for both children.
    int childStat = 0;
    waitpid(pid[0],&childStat,0);
    if (childStat == EXIT_SUCCESS)
    {
    	printf("Total matches: %d\n",lines);
    }

    wait(NULL);

    return EXIT_SUCCESS;
}
