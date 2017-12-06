#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "ftree.h"
#include "hash.h"

char* concat(const char *s1, const char *s2)
{
    // malloc return value
    char *rtv = malloc(strlen(s1)+strlen(s2)+1);
    strcpy(rtv, s1);
    strcat(rtv, s2);
    return rtv;
}
int copy_ftree(const char *cpypath, const char *destpath){
	const char* name = malloc(sizeof(char *));
    struct dirent *entry;
    struct stat *file = malloc(sizeof(struct stat));
    int i = 0;
    name = cpypath;

    // get the base name from cpypath
    if(strrchr(cpypath, '/') != NULL){
    	name = strrchr(cpypath, '/');
    	name = name + 1;
    }

    // Checks for errors in file info
    if (lstat(cpypath, file) == -1){
        fprintf(stderr, "lstat: the name '%s' is not a file or directory\n", name);
        exit(1);
    }


    // Check if path is a directory
    if(S_ISDIR(file->st_mode)){
    	// Make the directory at destpath
    	mkdir(destpath, (file->st_mode & 0777));

        DIR *directory;
        const char *pathholder;
        int result;

        // Checks if directory can be opened
        if((directory = opendir(cpypath)) == NULL){
            // Error if directory can not be opened
            fprintf(stderr, "Could not open directory at path %s\n", fname);
            exit(1);
        }

        // Loop through all contents of the directory
    	while((entry = readdir(directory)) != NULL) {
            // Skips all hidden files
    	    if(entry->d_name[0] != '.'){
    	    	if(entry->d_type == DT_DIR){
					// b/c a new directory has been found a fork will commence, so i++
				    i++;
				    // get the old cpy path saved
				    pathhoder = cpypath;
				    // make the cpypath equal to the subdirectory path
				    cpypath = concat(cpypath, concat("/", entry->d_name));
				    // fork
				    result = fork();
				    // forked child will handle the subdirectory
				    if(!result){
				        // call using new cpypath and same destpath
				        copy_ftree(cpypath, destpath);
				        exit(i);
				    }
				    else if(result > 0){
				    	// restore old cpypath for the parent
				    	cpypath = pathholder;
				    }
	
    	    	}
				else if(entry->d_type == DT_REG){
                	        copy_ftree(concat(cpypath, concat("/", entry->d_name)), destpath);
           		}
         	}
        }
    }

        for(int j = 0; j < i; j++){
        pid_t pid;
        int status;
        if( (pid = wait(&status)) == -1) {
            perror("wait");
        } else {
            if (WIFEXITED(status)) {
                printf("Child %d terminated with %d\n", pid, WEXITSTATUS(status));
            }
                else if(WIFSIGNALED(status)){
                printf("Child %d terminated with signal %d\n", pid, WTERMSIG(status));
            } else {
                printf("Shouldn't get here\n");
            }
          }
    }
    return i;
}










