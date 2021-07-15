#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
 
#define MAX_VERTICES 10000
//~ #define MAX_EDGES 30		//For high values it is taking a lot of time, so decreased its value(gave input in the main)

int V,E;		//V = no of vertices,E = no of max edges for a vertex

struct gnode{			//Node in the graph
	int id;
	int day;		//Stores the day at which it got the infection
	int recovery_day;
	bool infected;
};

struct node{			//Priority queue
	int day;
	char event;
	struct gnode* person;
	struct node* next;
};

struct list {		//Linked list
	int id;
	struct gnode* person;
	struct list *next;
};

//Heads of the respective lists
struct list *susceptible = NULL;
struct list *infected = NULL;
struct list *recovered = NULL;


//Aray for storing the number of days,infected,susceptible and recovered nodes
int data[301][3];
FILE *f;

//variables for storing the number of days,infected,susceptible and recovered nodes
int number_of_days = 0,ns=0,ni=0,nr=0;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Functions for linked list(S,I,R lists)

struct list *insert(struct list *head,struct gnode* person,int x)
{
	struct list *t, *temp;

	t = (struct list*)malloc(sizeof(struct list));
	t->person = person;
	t->id = x;
	
	if (head == NULL) {
		head = t;
		head->next = NULL;
		return head;
	}

	temp = head;

	while (temp->next != NULL)
		temp = temp->next;

	temp->next = t;
	t->next   = NULL;
	return head;
}

struct list* delete(struct list *head,int id)
{

    struct list* temp = head;
    struct list* prev; 
   
    if (temp != NULL && temp->id == id){ 
        head = temp->next;   
        free(temp);          
        return head; 
    } 
  
    while (temp != NULL && temp->id != id){ 
        prev = temp; 
        temp = temp->next; 
    } 
    if (temp == NULL) return NULL; 
  
    prev->next = temp->next; 
    free(temp);  
	return head;
}

bool find(struct list *head,int id)
{
	struct list* temp = head;


    if (temp != NULL && temp->id == id)          
        return true; 
  
    while (temp != NULL && temp->id != id)
        temp = temp->next; 
    
    if (temp == NULL) 
		return false;
    
    return true; 
}

int length(struct list *head)
{
	int n =0;
	struct list* temp = head;
	if(head == NULL)
		return 0;
	
	while(temp != NULL){
		n++;
		temp = temp->next;
	}
	return n;
	
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////

//Functions for the priority queue

typedef struct node* element;
element head = NULL;		//Pointer for top of the queue
void enqueue(char event,int day,struct gnode* person)			//inserts an event in the priority queue
{
	element temp,trav;

	temp = (element)malloc(sizeof(struct node));
	temp->event = event;
	temp->day = day;
	temp->person = person;
	/*queue is empty or item to be added has priority(day) more than first item*/
	if( head == NULL || day < head->day ){
		temp->next = head;
		head = temp;
		
	}else{
		trav = head;
		while( trav->next != NULL && trav->next->day <= day )
			trav=trav->next;
		temp->next = trav->next;
		trav->next = temp;
	}
}

void dequeue()				//Deletes the topmost event in the queue(with highest priority)
{
	element temp;
	if(head == NULL)
		printf("Queue Underflow\n");
		
	else{
		temp = head;
		head = head->next;
		free(temp);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//Main functions for the project


int **create_graph()				//For creating a random undirected graph and representing it with adjecency matrix
{
	int **adjacency;
	int i = 0;
    int j = 0;
    int counter=0;
	
	adjacency = (int **)malloc(V * sizeof(*adjacency));
    for(i = 0; i < V; i++)
         adjacency[i] = (int *)malloc(V * sizeof(*adjacency[i]));
	
	for(i = 0; i < V; i++)
	{
		adjacency[i][i] = 0;
		counter = 0;
		for(j = 0; j < i; j++)
		{
			if(rand()%2 == 1)
			{
				adjacency[i][j] = 1;			//Undirected => matrix should be symmetric
				adjacency[j][i] = 1;
				counter++;
			}
			
			if(counter >= E)			
				break;
		}
	}
	
	return adjacency;
}

void initialize_nodes(struct gnode *nodes[V])		//Creates an array of graph.nodes(Representing people) 
{
	for(int i=0;i<V;i++){
		struct gnode *add = (struct gnode *)malloc(sizeof(struct gnode));
		add->id = i;
		add->day = 0;
		add->infected = false;
		nodes[i] = add;
		susceptible = insert(susceptible,add,i);			//Inserting all the nodes to the susceptible list
	}
}

void recover(int id,struct gnode *nodes[V],float bias)				//recovery event
{
	
	int toss, days_for_recovery = 1;		//Number of days taken by the node to recover
	while(days_for_recovery <= 300 - nodes[id]->day){
		toss = rand()%101;
		
		if(toss < (bias*100)){			//Probability = bias
			enqueue('r',(nodes[id]->day + days_for_recovery),nodes[id]);		//Inserting the recovery event to the queue
			break;
		}
		days_for_recovery++;
	}
	nodes[id]->recovery_day = nodes[id]->day + days_for_recovery;
}

void transmit(int id,struct gnode *nodes[V],int **graph,float bias)			//Transmission event
{
	int toss;
	int days_for_transmit = 1;		//Number of days taken by the node to transmit to the other
	for(int i = 0;i < V;i++){
		if(graph[id][i] == 1){
			if(!nodes[i]->infected){
				days_for_transmit = 1;
				while(days_for_transmit <= 300 - nodes[id]->day){
					toss = rand()%101;
					if(toss < (bias*100) && nodes[i]->recovery_day < days_for_transmit){		//Probability = bias
						enqueue('t',(nodes[id]->day + days_for_transmit),nodes[i]);				//Inserting the transmission event to the queue
						break;
					}
					days_for_transmit++;
				}
				nodes[i]->day = nodes[id]->day+days_for_transmit;		//adding the days taken for transmission to the day of previous node to get the final day at which it gets infected
			}
		}
	}
}


void initial_infection(struct gnode *nodes[V],int **graph)			//infecting some nodes initially
{
	int counter = 0,x = 0;
	int no_of_initial_infectanats;
	printf("Enter initial number of initial infectants : ");
	scanf("%d",&no_of_initial_infectanats);
	while(counter < no_of_initial_infectanats){
		x = rand()%V;
		if(!nodes[x]->infected){		//finding if the node is not infected  
			nodes[x]->infected = true;
			susceptible = delete(susceptible,x);			//Removing the node from susceptible list
			infected = insert(infected,nodes[x],x);			//Adding the initial infectants to the infected list
			recover(x,nodes,0.2);							//Adding the recovery event to the queue
			transmit(x,nodes,graph,0.5);					//Adding the transmission events to the queue
			counter++;
		}
	}
	ns = V - no_of_initial_infectanats;
	ni += no_of_initial_infectanats;
}

void SIR_algo(struct gnode *nodes[V],int **graph)		//implementing the fast SIR algorithm
{
	while(head != NULL){
		if(head->event == 't'){
			if(find(susceptible,head->person->id)){			//If the node is susceptible
				number_of_days = head->day;
				ns--;
				ni++;
				data[number_of_days][0] = ns;
				data[number_of_days][1] = ni;
				data[number_of_days][2] = nr;
				head->person->infected = true;
				recover(head->person->id,nodes,0.2);								//inserting the recovery event of the newly infected node
				susceptible = delete(susceptible,head->person->id);					//Removing the node from susceptible list
				infected = insert(infected,head->person,head->person->id);			//Adding the initial infectants to the infected list
				transmit(head->person->id,nodes,graph,0.5);
			}
		}else if(head->event == 'r'){
			number_of_days = head->day;
			ni--;
			nr++;
			data[number_of_days][0] = ns;
			data[number_of_days][1] = ni;
			data[number_of_days][2] = nr;
			head->person->infected = false;
			infected = delete(infected,head->person->id);							//Removing the node from infected list
			recovered = insert(recovered,head->person,head->person->id);			//Adding the node to the recoverd list
		}

		dequeue();
	}
}


int main(){
	
	srand ( time(NULL) );
	
	//number of nodes in a graph
	V = rand() % MAX_VERTICES;
	
	//number of maximum edges a vertex can have
	E = 15;
	
	printf("Total Vertices = %d, Max # of Edges = %d\n",V, E);

	int **graph = create_graph();		//adjecency matrix
    
    struct gnode *nodes[V];				//Array for storing the nodes of the graph(people)
    
    initialize_nodes(nodes);
	initial_infection(nodes,graph);
	
	data[number_of_days][0] = ns;
	data[number_of_days][1] = ni;
	data[number_of_days][2] = nr;
	
	SIR_algo(nodes,graph);

	//output.dat contains the number of S,I and R people for every day
	
	//Writing the output in the file
	f = fopen("output.dat","w");
	for(int i=0; i<=number_of_days; i++){
		if(data[i][0]==0 && data[i][1]==0 && data[i][2]==0){		//If no event was present on the day i
			data[i][0] = data[i-1][0];
			data[i][1] = data[i-1][1];
			data[i][2] = data[i-1][2];
		}
		fprintf(f,"%d %d %d %d\n", i,data[i][0],data[i][1],data[i][2]);
	}
	printf("susceptilble = %d, infected =  %d, recovered = %d\n", length(susceptible),length(infected),length(recovered));

    return 1;
}
