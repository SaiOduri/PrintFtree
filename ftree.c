#include <stdio.h>
#include "ftree.h"
#include "hash.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <string.h>

// Helper funcation that concatenates two strings
char* concat(const char *s1, const char *s2)
{
    const size_t len1 = strlen(s1);
    const size_t len2 = strlen(s2);
    char *result = malloc(len1+len2+1);//Increments
    memcpy(result, s1, len1);
    memcpy(result+len1, s2, len2+1);//Incerements
    return result;
}

int copy_ftree(const char *src, const char *dest){
    const char* name = malloc(sizeof(char *));
    struct dirent *entry;
    struct stat *file = malloc(sizeof(struct stat));
    int i = 0;
    name = src;

    // get the name of the file or directory
    // and save it as name
    if(strrchr(src, '/') != NULL){
        name = strrchr(src, '/');
        name = name + 1;
    }

    dest = concat(dest, concat("/", name));

    // Checks for errors in file info
    if (lstat(src, file) == -1){
        fprintf(stderr, "lstat: the name '%s' is not a file or directory\n", name);
        exit(1);
    }


    // Check if path is a directory
    if(S_ISDIR(file->st_mode)){
        // Make the directory at dest
        mkdir(dest, (file->st_mode & 0777));

        // Initialize variables
        DIR *directory;
        const char *child_path;
        int fork_result;

        // Checks if directory can be opened
        if((directory = opendir(src)) == NULL){
            // Error if directory can not be opened
            fprintf(stderr, "Could not open directory at path %s\n", src);
            exit(1);
        }

        // Loop through all contents of the directory
        while((entry = readdir(directory)) != NULL) {
            // Skips all hidden files
            if(entry->d_name[0] != '.'){
                if(entry->d_type == DT_DIR){
                    // Start fork with incrementing i
                    i++;
                    // Have a path holder to save src for recursion
                    // Change src for recursion purposes
                    child_path = concat(src, concat("/", entry->d_name));
                    // Start the fork
                    fork_result = fork();
                    // If the fork is a child
                    if(!fork_result){
                        // Call the function again on the subdirectory
                        copy_ftree(child_path, dest);
                        exit(i);
                    }
                }
                // If the child is a file, then call use recursion on the file
                else if(entry->d_type == DT_REG){
                        copy_ftree(concat(src, concat("/", entry->d_name)), dest);
                }
            }
        }
    }

    else if(S_ISREG(file->st_mode)){
        // Intialize Variables
        char * read = 0;
        long size;
        FILE *newfile;
        FILE * oldfile;
        mode_t permissions;

        // Sets permissions to permissions of oldfile
        permissions = file->st_mode;

        // Create the newfile
        if(!(newfile = fopen(dest, "wb"))){
        fprintf(stderr, "Cannot open file \'%s\' \n", name);
        exit(0);
        }

        // Open the old file
        if(!(oldfile = fopen (src, "rb"))){
        fprintf(stderr, "Cannot open file \'%s\' \n", name);
        exit(0);
        }

        // Read all of the contents of oldfile
        if (oldfile)
        {
          fseek (oldfile, 0, SEEK_END);
          size = ftell (oldfile);
          fseek (oldfile, 0, SEEK_SET);
          read = malloc (size);
          if (read)
          {
            // Saves all the contents of oldfile into read
            fread (read, 1, size, oldfile);
          }
        }
        // Place all the contents of oldfile into newfile
        fputs(read, newfile);

        // Close files
        fclose (oldfile);
        fclose(newfile);

        // Sets permissions of oldfile to newfile
        chmod(dest, permissions);
    }

    // Waits for forking
    for(int j = 0; j < i; j++){
        pid_t pid;
        int status;
        if((pid = wait(&status)) == -1){
            perror("wait");
        }
    }
    return i;
}
