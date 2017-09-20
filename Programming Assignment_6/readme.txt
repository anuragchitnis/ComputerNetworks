##############################################
Programming assignment - 6

Objective:
To find the least cost routing path using Dijkstra’s and distance vector algorithm.

Author : Anurag Chitnis
##############################################

Instructions to run the program:

1. Login to remote server: cse01.cse.unt.edu
2. compile the shortest-path.c file using
   >:$ gcc shortest-path.c -o shortest-path

3. Execute the program using
   >:$ ./shortest-path

/************************************/
Code Structure:

Following functions are used in the program:

/**
* This method reads the cost information of router from a file "router.txt" and creates the adjacancy matrix, 
* which we further use to calculate shortest path.
*/
void readRouterInformation(char nodes[10], int** cost)


/**
* Dijsktra's algorithm to calculate the shortest path from source node to all other nodes.
*
*/
void dijsktras(const int n, int source, int **cost, FILE *dijFile, char nodes[])

/**
* Returns the next node from the source node, based on the target node and the array of previous nodes
*/
int getNextNode(int prev[], int target)


/**
* Distance vector algorithm using the Bellmen-Ford equation.
* This function calculates the shortest path and writes the link data to a file "DV.txt"
*/
void distanceVector(const int n, int **cost, char nodes[])