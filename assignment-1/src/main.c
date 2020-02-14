#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_NODES 32
#define MAX_EXEC_ARGS 10
#define MAX_STR_LENGTH 50

struct Connection {
	unsigned short source;
	unsigned short sink;
};

struct Data {
	unsigned short numberOfNodes;
	unsigned short numberOfConnections;
	char ***commands;
	struct Connection *connections;
};

struct Node {
	char **command;
	unsigned short numberOfChildren;
	unsigned short children[MAX_NODES];
};

void getNumberOfNodes(struct Data *data) {
	scanf("%hu\n\n", &(data->numberOfNodes));
}


void getCommands(struct Data *data) {
	char *ptr;
	char *tempBuffer = (char *) calloc(MAX_EXEC_ARGS * MAX_STR_LENGTH, sizeof(char));
	char ***commands = (char ***) calloc(data->numberOfNodes, sizeof(char **));
	for (int i = 0; i < data->numberOfNodes; i++) {
		fgets(tempBuffer, MAX_EXEC_ARGS * MAX_STR_LENGTH, stdin);
		if ((ptr = strchr(tempBuffer, '\n')) != NULL) *ptr = '\0';
		ptr = tempBuffer;
		unsigned partCount = 1;
		while (*ptr != '\0') {
			if (*ptr == ' ') partCount++;
			ptr++;
		}
		char **parts = (char **) calloc(MAX_EXEC_ARGS, sizeof(char *));
		for (int j = 0; j < partCount; j++) parts[j] = (char *) calloc(MAX_STR_LENGTH, sizeof(char));
		ptr = tempBuffer;
		int j = 0;
		int k = 0;
		while (*ptr != '\0') {
			if (*ptr == ' ') {
				j++;
				k = 0;
			} else parts[j][k++] = *ptr;
			ptr++;
		}
		memset(tempBuffer, 0, MAX_EXEC_ARGS * MAX_STR_LENGTH);
		commands[i] = parts;
	}
	scanf("\n");
	free(tempBuffer);
	data->commands = commands;
}


void getConnections(struct Data *data) {
	unsigned short source;
	unsigned short sink;
	struct Connection *connections = (struct Connection *) calloc(data->numberOfNodes, sizeof(struct Connection));
	int index = 0;
	while (scanf("%hu %hu\n", &source, &sink) == 2) {
		connections[index].source = source;
		connections[index].sink = sink;
		index++;
	}
	data->connections = connections;
	data->numberOfConnections = index;
}


void readInData(struct Data *data) {
	getNumberOfNodes(data);
	getCommands(data);
	getConnections(data);
}


struct Node *makeGraph(struct Data *data) {
	struct Node *graph = (struct Node *) calloc(data->numberOfNodes, sizeof(struct Node));
	for (int i = 0; i < data->numberOfNodes; i++) {
		graph[i].command = data->commands[i];
		graph[i].numberOfChildren = 0;
	}
	for (int i = 0; i < data->numberOfConnections; i++) {
		struct Connection curr = data->connections[i];
		graph[curr.source].children[graph[curr.source].numberOfChildren++] = curr.sink;
	}
	return graph;
}

void freeEverything(struct Data * data, struct Node * graph) {
	free(graph);
	free(data->connections);
	for (unsigned short i = 0; i < data->numberOfNodes; i++) {
		for (unsigned short j = 0; j < MAX_EXEC_ARGS; j++) free(data->commands[i][j]);
		free(data->commands[i]);
	}
	free(data->commands);
}


void topologicalExecute(struct Node *graph, unsigned short index, FILE *fp) {
	for (int i = 0; i < graph[index].numberOfChildren; i++)
		topologicalExecute(graph, graph[index].children[i], fp);
	int forkResult = fork();
	if (forkResult < 0) printf("An error occurred when asking the OS to fork this process.\n");
	else if (forkResult == 0) {
		execvp(graph[index].command[0], &(graph[index].command[0]));
		printf("A kernel error was encountered during execvp! Pruning this branch and "
			   "attempting to continue topological execution on the remaining branches.\n");
		exit(0);
	} else {
		int pid = wait(NULL);
		fprintf(fp, "\n%d %d", pid, getpid());
		for (int i = 0; i < MAX_EXEC_ARGS; i++) {
			if (graph[index].command[i] == NULL) break;
			fprintf(fp, " %s", graph[index].command[i]);
		}
	}
}


int main() {
	struct Data *data = (struct Data *) calloc(1, sizeof(struct Data));
	readInData(data);
	struct Node *graph = makeGraph(data);
	FILE *fp = fopen("results.txt", "w");
	topologicalExecute(graph, 0, fp);
	fclose(fp);
	freeEverything(data, graph);
	return 0;
}
