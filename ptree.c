#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>

// Add your other system includes here.

#include "ptree.h"

// Defining the constants described in ptree.h
const unsigned int MAX_PATH_LENGTH = 1024;

// If TEST is defined (see the Makefile), will look in the tests 
// directory for PIDs, instead of /proc.
#ifdef TEST
const char *PROC_ROOT = "tests";
#else
const char *PROC_ROOT = "/proc";
#endif

char **generate_paths(pid_t); 
FILE **get_files(pid_t); 
void construct_children(struct TreeNode *, FILE *);
int name_node(struct TreeNode *, FILE *); 
void fclose_free(FILE **, char **, int); 
int error_code; 

/*
* Construct a TreeNode for the process with PID pid. 
* Returns a pointer to the generated TreeNode
*/
struct TreeNode *construct_node(pid_t pid){
    FILE **files = get_files(pid); 
    if (files == NULL){
        // error msg taken care of by get_files
        error_code = 1; 
        fclose_free(files, NULL, 2); 
        return NULL;
    }
    FILE *cmdline_file = files[0], *children_file = files[1]; 

    struct TreeNode *node = malloc(sizeof(struct TreeNode));    
    if (node == NULL){
        fprintf(stderr, "failed to allocate space for node in construct_node\n");
        error_code = 1; 
        fclose_free(files, NULL, 2); 
        return NULL; 
    }
    int name_fail = name_node(node, cmdline_file); 
    if (name_fail == 1){
        fclose_free(files, NULL, 2); 
        return NULL; 
    }
    node->pid = pid; 
    node->child = NULL; 
    node->sibling = NULL; 
    
    if (children_file != NULL){
        construct_children(node, children_file);
    }
    fclose_free(files, NULL, 2); 

    return node;    
}


/*
* Name the node pointed to by 'node' with the name in cmdline_file. 
* Returns 0 iff the name was set successfully. 
*/
int name_node(struct TreeNode *node, FILE *cmdline_file){
    char node_name[MAX_PATH_LENGTH + 1];
    if ((cmdline_file == NULL) || (fscanf(cmdline_file, "%s", node_name) != 1)){
        node->name = NULL; 
    }else{
        node->name = malloc(strlen(node_name) + 1); 
        if (node->name == NULL){
            fprintf(stderr, "Failed to allocate space for node->name in construct_node\n");
            error_code = 1; 
            return 1; 
        }
        strcpy(node->name, node_name); 
        (node->name)[strlen(node_name)] = '\0'; 
    }
    return 0; 
}


/*
* Given a pointer to a node 'root' and a pointer to the file containing its children, 
* construct TreeNodes for all of root's children and attatch them to root
*/
void construct_children(struct TreeNode *root, FILE *children_file){
    pid_t child_pid; 
    while(fscanf(children_file, "%d", &child_pid) > 0){
        struct TreeNode *child = construct_node(child_pid); 
        if (child == NULL){
            continue;
        }else if (root->child == NULL){
            root->child = child; 
        }else{
            struct TreeNode *kid = root->child; 
            while (kid->sibling != NULL){
                kid = kid->sibling;
            }
            // kid now holds the node that has can hold child as a sibling
            kid->sibling = child; 
        }
    }
}


/*
* Given a pid, generate an array containing paths to the procfile, cmdline file and children
* file corresponding to the pid. 
*/
char **generate_paths(pid_t pid){
    char *procfile = malloc(MAX_PATH_LENGTH + 1);
    char *cmdline =  malloc(MAX_PATH_LENGTH + 1);
    char *children =  malloc(MAX_PATH_LENGTH + 1);
    if (procfile == NULL || cmdline == NULL || children == NULL){
        fprintf(stderr, "failed to allocate memory for path name in generate_paths\n");

    }

    int failed_procfile = sprintf(procfile, "%s/%d/exe", PROC_ROOT, pid) < 0; 
    int failed_cmdline = sprintf(cmdline, "%s/%d/cmdline", PROC_ROOT, pid) < 0; 
    int failed_children = sprintf(children, "%s/%d/task/%d/children", PROC_ROOT, pid, pid) < 0; 

    if (failed_procfile || failed_cmdline || failed_children) {
        fprintf(stderr, "sprintf failed to produce a filename\n");
        return NULL; 
    }

    char **paths = malloc(sizeof(char*)*3); 
    if (paths == NULL){
        fprintf(stderr, "failed to allocate memory for paths array in generate_paths\n");
        return NULL; 
    }
    paths[0] = procfile, paths[1] = cmdline, paths[2] = children;
    return paths; 
}


/*
* Given a pid, return an array continer pointers to the procfile, cmdline file, and 
* children file corresponding to the pid. 
*/
FILE **get_files(pid_t pid){
    char **paths = generate_paths(pid); 
    char *proc_path = paths[0], *cmdline_path = paths[1], *children_path = paths[2]; 

    if (paths == NULL){
        // error msg already taken care of by generate_paths
        error_code = 1; 
        return NULL;  
    }

    struct stat stat_buf; 
    int valid_proc = lstat(proc_path, &stat_buf); 
    if (valid_proc == -1){ // check to see if PID is a dead process
        fprintf(stderr, "skipping node with PID %d as it is not a running process.\n", pid);
        error_code = 1; 
        fclose_free(NULL, paths, 3); 
        return NULL; 
    }
    
    FILE *cmdline_file = fopen(cmdline_path, "r"); 
    FILE *children_file = fopen(children_path, "r");        

    FILE **files = malloc(sizeof(FILE*)*2);
    if (files == NULL){
        fprintf(stderr, "failed to allocate space for files array in get_files\n");
        error_code = 1; 
        fclose_free(NULL, paths, 3); 
        return NULL; 
    }else if (children_file == NULL || cmdline_file == NULL){
        fprintf(stderr, "Error opening children file or cmdline file for PID %d\n", pid); 
        error_code = 1; 
    }
    files[0] = cmdline_file, files[1] = children_file;
    
    // free all space associated with the paths array 
    fclose_free(NULL, paths, 3); 
    return files; 
}

/*
* If files is not null, close all the streams in 'it and free the space 
* allocated for the array and its contents. If strings is not null, free all 
* the memory allocated for it and its contents.   
*/
void fclose_free(FILE **files, char **strings, int size){
    if (files != NULL){
        for (int i = 0; i < size; i++){
            fclose(files[i]);
        }
        free(files); 
    }else if (strings != NULL){
        for (int i = 0; i < size; i++){
            free(strings[i]);
        }
        free(strings); 
    }
}

/*
 * Creates a PTree rooted at the process pid.
 * The function returns 0 if the tree was created successfully 
 * and 1 if the tree could not be created or if at least
 * one PID was encountered that could not be found or was not an 
 * executing process.
 */
int generate_ptree(struct TreeNode **root, pid_t pid) {
    error_code = 0; 
    (*root) = construct_node(pid); 
    // print_ptree((*root), 0); 
    return error_code;
}

/**
* Generates a string representing a single node. The string is in the format
* 'PID: process_name' or just 'PID' if the process_name doesn't exist.  
* Returns a pointer to the generated string
*/
char *get_process_str(struct TreeNode *node){
    int pid = (int) node->pid; 
    char *name = node->name; 

    // max val of int is 10 digits + 3 bytes for colon + space + null terminator
    int process_str_size = 13; 
    if (name != NULL){
        process_str_size += strlen(name); 
    }

    // declare string for appropriately sized process_str and insert pid
    char *process_str = malloc(process_str_size);
    if (process_str == NULL){
        fprintf(stderr, "failed to allocate space for process_str in get_process_str\n");
    }
    sprintf(process_str, "%d", pid);

    if (name != NULL){; 
        strcat(process_str, ": ");
        strcat(process_str, name); 
    }

    return process_str; 
}

/*
 * Prints the TreeNodes encountered on a preorder traversal of an PTree
 * to a specified maximum depth. If the maximum depth is 0, then the 
 * entire tree is printed.
 */
void print_ptree(struct TreeNode *root, int max_depth) {
    // Here's a way to keep track of the depth (in the tree) you're at
    static int depth = 0;

    if (root != NULL){
        int local_depth = depth; // store local depth since depth may change from recursive calls
        char *process_str = get_process_str(root); 
        // print process info about the root
        printf("%s\n", process_str); 
        if ((depth < max_depth) || (max_depth == 0)){
            if (root->child != NULL){
                depth += 1;    
                printf("%*s", depth * 2, "");  
                print_ptree(root->child, max_depth); 
                depth = local_depth; // reset depth to its local value 
            }
        }
        // siblings are at the same depth as child, so they don't need to pass depth < max_depth
        if (root->sibling != NULL){        
            printf("%*s", depth * 2, "");
            print_ptree(root->sibling, max_depth);
            depth = local_depth;
        }
    }
}