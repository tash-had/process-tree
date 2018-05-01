#include <stdio.h>
#include <stdlib.h>

#include "ptree.h"


int main(int argc, char **argv) {
    // TODO: Update error checking and add support for the optional -d flag
    // printf("Usage:\n\tptree [-d N] PID\n");

	int valid_flag = 0; 
	int N_arg = 0; // optional argument. 0 by default. 
	int pid; 

    if (argc == 3 || argc == 4){
    	// check flag to see if it is '-d' 
    	valid_flag = argv[1][0] == '-' && argv[1][1] == 'd' && argv[1][2] == '\0';
    	// in the case that a depth was provided after '-d' 
    	if (valid_flag && argc == 4){
    		N_arg = strtol(argv[2], NULL, 10); 
    		pid = strtol(argv[3], NULL, 10); 
    	}else if (valid_flag && argc == 3){
    		// in the case where '-d' was provided, but no depth was given
    		pid = strtol(argv[2], NULL, 10); 
    	}
    }else if (argc == 2){
    	// in the case where optional arg. wasn't provided
    	pid = strtol(argv[1], NULL, 10); 
    }

    if (!valid_flag && argc != 2){
    	fprintf(stderr, "%s\n", "Usage: \n\tptree [-d N] PID\n");
    	return 1;  
    }

    // NOTE: This only works if no -d option is provided and does not
    // error check the provided argument or generate_ptree. Fix this!
    
    struct TreeNode *root = NULL;
    int invalid_req = generate_ptree(&root, pid);
    if (invalid_req){
    	return 2;
    }
    print_ptree(root, N_arg);

    return 0;
}

