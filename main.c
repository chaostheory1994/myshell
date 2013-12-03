/* 
 * File:   main.c
 * Author: jacob
 *
 * Created on November 25, 2013, 2:58 PM
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <dirent.h>
#include <errno.h>
#define BUFFER_MAX 100
/*
 *      Global Variables
 */
// Strings
char* input_str = "thot:%s$ ";
// Variables
// current directory buffer
char current_directory_buff[100];
// input buffer
char input[100];
// dirent.h stuff
DIR * directory;
struct dirent * file;
// input buffer
char buff[100];
// While loop boolean
char still_running;
char has_child;
// This is here to align memory neatly to prevent errors.
short alignment;
// child process id and args
pid_t child;
char** args;
// Linked List Struct
struct linked_list {
    char* string;
    struct linked_list* next;
};
// List Stuff
struct linked_list* first_node;
struct linked_list* curr_node;
// Prototypes
void init();
void sigint_handler(int);
struct linked_list* new_node(char*);
void delete_list(struct linked_list*);

int main(int argc, char** argv) {
    int count;
    char* curr_token;
    char* temp;
    int i, k;
    // The start of our shell program.
    // First lets just display some basic details.
    puts("Welcome to likebash shell!\nBy Jacob Schreiner\n");
    // The functionality of this shell is just a loop.
    // Ask a command and execute if possible. ask then execute.
    // We can just do while loop with a boolean.
    // The boolean cafn be set two ways.
    // Calling exit or ctrl c while another program is not running.
    // First lets call init
    init();
    // Now lets start our loop.
    while(still_running){
        // Well first thing is first is to display some information on the line.
        printf(input_str, current_directory_buff);
        // Now we can get some input
        fgets(input, BUFFER_MAX, stdin);
        // Now we have to analyze the input.
        // We are going to use strtok.
        // I am going to use a linked list to gather the tokens.
        // Then I will turn it into an args array.
        count = 0;
        // Lets initialize strtok
        curr_token = strtok(input, " ");
        while(curr_token != NULL){
            if(count == 0){
                // If there is no nodes, then first_node must be set.
                curr_node = new_node(curr_token);
                first_node = curr_node;
            }
            else{
                // This means we itterated at least once.
                curr_node -> next = new_node(curr_token);
                curr_node = curr_node -> next;
            }
            count++;
            curr_token = strtok(NULL, " ");
        }
        // Now that we have our linked list, lets turn it into an args array.
        args = (char**)malloc(sizeof(char*) * (count + 1));
        curr_node = first_node;
        i = 0;
        while(curr_node != NULL){
            args[i] = curr_node -> string;
            curr_node = curr_node -> next;
            i++;
        }
        // Because the last arguement has a\n, that is no good.
        // We need to get rid of it.
        temp = args[i - 1];
        k = 0;
        while(temp[k] != '\n' && temp[k] != '\0'){
            k++;
        } 
        temp[k] = '\0';
        args[i] = NULL;
        // Now that we have a good set of args, lets decide whats going on.
        if(count > 0){
            // This will execute if something was entered.
            // Lets check if it was an internal command.
            if(strcmp(args[0], "exit") == 0){
                return 0;
            }
            else if(strcmp(args[0], "cd") == 0){
                // This is easily how we will change the directory.
                chdir(args[1]);
                getcwd(current_directory_buff, 100);
            }
            else{
                child = fork();
                if(child == 0){
                    execvp(args[0], args);
                    // If we reach this point in the code.
                    // The execvp failed.
                    printf("Error: ");
                    switch(errno){
                        case EIO:
                            puts("An I/O error occured.");
                            break;
                        case ENOENT:
                            puts("File not found.");
                            break;
                        case ENOEXEC:
                            puts("The executable is not in a recognizable format.");
                            break;
                        case ENOMEM:
                            puts("Kernal was unable to allocate memory.");
                            break;
                        case EACCES:
                            puts("Permission denied.");
                            break;
                    }
                    // If we reach here, we cant let this process go.
                    return 0;
                }
                else if(child < 0){
                    // Fork has errored.
                    printf("Error: ");
                    switch(errno){
                        case EAGAIN:
                            puts("Unable to allocate memory needed for fork().");
                            break;
                        case ENOMEM:
                            puts("Kernal unable to get memory for fork().");
                            break;
                        case ENOSYS:
                            puts("fork() is not supported by this platform.");
                            break;
                    }
                }
                else{
                    // Fork had worked. 
                    // Lets do stuff. Also wait for the child process to finish.
                    has_child = 1;
                    int status;
                    wait(&status);
                    // The process has finished.
                    has_child = 0;
                    child = 0;
                }
            }
        }
        // We do not need the list anymore;
        delete_list(first_node);
        first_node = NULL;
        curr_node = NULL;
        // Or the args.
        free(args);
        args = NULL;
    }
    return (EXIT_SUCCESS);
}
void init(){
    // Lets initialize the shell
    // First we setup the while loop bool
    still_running = 1;
    // Lets also setup the current directory string.
    getcwd(current_directory_buff, BUFFER_MAX);
    // We also want to catch the SIGINT signal.
    signal(SIGINT, sigint_handler);
    // We dont have  a child yet
    has_child = 0;
}
void sigint_handler(int signal_number){
    // This will execute if the singal SIGINT is sent to our shell.
    // There are two possible cases of things happening.
    // One if a child process is running.
    // Two if just the bash is running.
    // We can check this using waitpid() with the WNOHANG option
    if(!has_child){
        // If we dont have a child, the user want to exit our shell.
        if(first_node != NULL){
            delete_list(first_node);
        }
        if(args != NULL){
            free(args);
        }
        exit(0);
    }
    // We do not need an else because hitting ctrl c will send
    // the signal to all processes so the child will automatically get it.
}
struct linked_list* new_node(char *data){
    // This is where we will create a new node 
    struct linked_list* return_value = (struct linked_list*)malloc(sizeof(struct linked_list));
    return_value -> string = (char*)malloc(sizeof(char) * strlen(data));
    strcpy(return_value -> string, data);
    return_value -> next = NULL;
    return return_value;
}
void delete_list(struct linked_list* first_node){
    // We are going to delete all of the linked list.
    struct linked_list* temp;
    while(first_node != NULL){
        temp = first_node -> next;
        first_node -> next = NULL;
        // I am always going to have the string value malloc'd
        free(first_node -> string);
        free(first_node);
        first_node = temp;
    }
    // Linked list is gone :)
}