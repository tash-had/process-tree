#include <stdio.h>
#include <stdlib.h>

#include "ptree.h"

int main(int argc, char *argv[]) {
    // Creates a ptree to test printing
    // Notice that in this tree the names are string literals. This is fine for
    // testing but is not what the assignment asks you to do in generate_ptree.
    // Read the handout carefully. 

    struct TreeNode root, child_one, child_two, grandchild;
    root.pid = 4511;
    root.name = "root process";
    root.child = &child_one;
    root.sibling = NULL;

    child_one.pid = 4523;
    child_one.name = "first child";
    child_one.child = NULL;
    child_one.sibling = &child_two;

    child_two.pid = 4524; 
    child_two.name = "second child";
    child_two.child = &grandchild;
    child_two.sibling = NULL;

    grandchild.pid = 4609;
    grandchild.name = "grandchild";
    grandchild.child = NULL;
    grandchild.sibling = NULL; 
    
    printf("\nTest 1:\n");
    print_ptree(&root, 1);

    struct TreeNode tgc1, tgc2, tggc1, tgggc1, tggggc1, tggggc2;
    tgc1.pid = 5952;
    tgc1.name = "Test-Grand-Child 1 of root"; 
    tgc1.child = &tggc1; 
    tgc1.sibling = &tgc2; 

    tgc2.pid = 6447;
    tgc2.name = "Test Grand-Child 2 of root"; 
    tgc2.child = NULL; 
    tgc2.sibling = NULL; 

    tggc1.pid = 5655;
    tggc1.name = "Great-Grand-Child of root"; 
    tggc1.child = &tgggc1; 
    tggc1.sibling = NULL; 

    // Initial Test
    // tgggc1.pid = 9778;
    // tgggc1.name = "Great-Great-Grand-Child of root"; 
    // tgggc1.sibling = NULL; 
    // tgggc1.child = NULL; 

    // Larger test
    tgggc1.pid = 9778;
    tgggc1.name = NULL; 
    tgggc1.sibling = NULL; 
    tgggc1.child = &tggggc1; 

    tggggc1.pid = 9558;
    tggggc1.name = NULL; 
    tggggc1.sibling = &tggggc2; 
    tggggc1.child = NULL; 

    tggggc2.pid = 9718;
    tggggc2.name = "Great-Great-Great-Grand-Child 2 of root"; 
    tggggc2.sibling = NULL; 
    tggggc2.child = NULL; 

    printf("\nTest 2:\n");
    child_one.child = &tgc1; 
    // grandchild.child = &tgggc1; 
    print_ptree(&root, 2);

    printf("\nTest 3:\n");
    print_ptree(&root, 1);
    printf("\nTest 4:\n");
    print_ptree(&root, 0);

    return 0;
    }