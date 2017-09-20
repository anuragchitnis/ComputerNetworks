/**
* Programming Assignment #6
* Objective: To find the least cost routing path using Dijkstra’s and distance vector algorithm.
*
* Author : Anurag Chitnis
*/


#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// take infinity as a large value
#define INF 99

void dijsktras(const int n, int source, int **cost, FILE *dijFile, char nodes[]);
void distanceVector(const int n, int **cost, char nodes[]);

/**
* This method reads the cost information of router from a file "router.txt" and creates the adjacancy matrix, 
* which we further use to calculate shortest path.
*/
void readRouterInformation(char nodes[10], int** cost) {
    
    int costValue;
    char *line = NULL;
    char firstNode, secondNode;
    int firstIndex, secondIndex;
    size_t len = 0;
    ssize_t readFirst;
    FILE *routerFile = fopen("router.txt", "r");
    
    while ((readFirst = getline(&line, &len, routerFile)) != -1) {
        sscanf(line, "%c %c %d", &firstNode, &secondNode, &costValue);
//        printf("%c %c %d\n",firstNode, secondNode, costValue);
        char *ptr = strchr(nodes, firstNode);
        if(ptr != NULL) {
            firstIndex = ptr - nodes;
//            printf("First Index : %d\n", firstIndex);
        }
        else {
            int nodesLen = strlen(nodes);
            nodes[nodesLen] = firstNode;
            nodes[nodesLen + 1] = '\0';
            firstIndex = nodesLen;
//            printf("inserting first node at %d\n", firstIndex);
        }
        
        char *ptr1 = strchr(nodes, secondNode);
        if(ptr1 != NULL) {
            secondIndex = ptr1 - nodes;
//            printf("Second Index : %d\n", secondIndex);
        }
        else {
            int nodesLen = strlen(nodes);
            nodes[nodesLen] = secondNode;
            nodes[nodesLen + 1] = '\0';
            secondIndex = nodesLen;
            printf("inserting second node at %d\n", secondIndex);
        }
        cost[firstIndex][secondIndex] = cost[secondIndex][firstIndex]= costValue;
//        printf("Cost Value : %d\n", costValue);
    }
    
    fclose(routerFile);
}

int main() {
    char nodes[10] ;
    int i,j,n, **cost;
    memset ( nodes, 0, 10 );
    /* allocate the array */
    cost = malloc(10 * sizeof *cost);
    for (i=0; i<10; i++)
        cost[i] = malloc(10 * sizeof *cost[i]);
	
	// Initialize all the cost values to infinity
    for(i=0;i<10;i++) 
        for(j=0;j<10;j++)
            cost[i][j] = INF;
			
	// Read the router information and create the adjacency cost matrix
    readRouterInformation(nodes, cost);
	// find the number of nodes
    n = strlen(nodes);
// Debuggings Code to print the adjacancy matrix
//    for(i=0;i<n;i++) {
//        for(j=0;j<n;j++) {
//            printf("%d ",cost[i][j]);
//        }
//        printf("\n");
//    }
	
	FILE *dijFile = fopen("LS.txt", "w");
	
	printf("######### Dijsktra's Algorithm #############\n");
	for(i=0;i<n;i++) {
		dijsktras(n,i,cost,dijFile,nodes);
	}
	
	fclose(dijFile);
	
	distanceVector(n,cost,nodes);
    
    return 0;
}

/**
* Dijsktra's algorithm to calculate the shortest path from source node to all other nodes.
*
*/
void dijsktras(const int n, int source, int **cost, FILE *dijFile, char nodes[]) {
    int dist[n],prev[n],selected[n],i,m,min,d,count=1;
    char path[n];
    for(i=0;i< n;i++)
    {
	  selected[i] = 0;     
	  if(cost[source][i] != 0) {
	  	prev[i] = 0;
	  }
	  else {
	  	cost[source][i] = INF;
		prev[i] = -1;
	  }
	  // Initialize the dist by finding the distance of all the nodes from source node
      dist[i]= cost[source][i];
    }
	
    while(count < n) {
	    min = INF;

		for(i=0;i<n;i++) {
		   if(dist[i]<min && selected[i]==0) {
			min = dist[i];
			m = i;
		   }
		}
		selected[m] = 1;
		count++;
	    for(i=0;i< n;i++) {
	       d = dist[m] +cost[m][i];
	       if(d< dist[i] && selected[i]==0)
	        {
	           dist[i] = d;
	           prev[i] = m;
	        }
	    }
    }
    
    for(i=0;i<n;i++) {
	  if(i != source) {
          fprintf(dijFile,"%c (%c,%c)\n",nodes[i],nodes[source],nodes[getNextNode(prev, i)]);
		  printf("Target Node: %c, Link : (%c,%c)\n",nodes[i],nodes[source],nodes[getNextNode(prev, i)]);
		}
    }
	
	fprintf(dijFile,"--------------\n");
	printf("------------------------------------\n");    
}

/**
* Returns the next node from the source node, based on the target node and the array of previous nodes
*/
int getNextNode(int prev[], int target) {
	int temp;
 	do {
		temp = prev[target];
		if(temp!=0)
			target = temp;
	} while(temp!=0);
	return target;
 }
 
/**
* Distance vector algorithm using the Bellmen-Ford equation.
* This function calculates the shortest path and writes the link data to a file "DV.txt"
*/
void distanceVector(const int n, int **cost, char nodes[]) {
	printf("######### Distance Vector Algorithm #############\n");
	int dist[n][n],next[n][n],i,j,k,count;
	FILE *dvFile = fopen("DV.txt","w");
	for(i=0;i<n;i++) {
		for(j=0;j<n;j++) {
			dist[i][j] = cost[i][j];
			next[i][j] = j;
		}
	}
    
    do {
       count=0;
       for(i=0;i<n;i++)
       for(j=0;j<n;j++)
       for(k=0;k<n;k++)
           if(dist[i][j] > cost[i][k] + dist[k][j]) {
                dist[i][j]= dist[i][k] + dist[k][j];
                next[i][j]=k;
                count++;
           }
       } while(count!=0);
		
    for(i=0;i<n;i++) {
      for(j=0;j<n;j++) {
		if(i != j) {
			fprintf(dvFile,"%c (%c,%c)\n",nodes[j],nodes[i],nodes[next[i][j]]);
			printf("Target Node: %c, Link : (%c,%c)\n",nodes[j],nodes[i],nodes[next[i][j]]);
		}
      }
			fprintf(dvFile,"--------------\n");
			printf("---------------------------------------\n");
    }
	fclose(dvFile);
 	
 }

