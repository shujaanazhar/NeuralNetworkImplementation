#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/wait.h>

using namespace std;

struct Node {
    int nodeNumber;
    double *weights;
    double bias;

    void doAlgorithm() {
    }
};

struct FunkyFuncArgs {
    Node *node;
    pthread_mutex_t *mutex;
};

void *funkyFunc(void *args) {
    FunkyFuncArgs *funcArgs = (FunkyFuncArgs *) args;
    Node *n = funcArgs->node;
    pthread_mutex_t mutex = *(funcArgs->mutex);

    pthread_mutex_lock(&mutex);

    n->doAlgorithm();
    cout << "AI stuff being done by Node " << n->nodeNumber << endl;

    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);
}

struct Layer {
    int layerNumber;
    Node *Nodes;
    int numberNodes = 0;
    pthread_mutex_t nodeMutex;

    Layer() {
        pthread_mutex_init(&nodeMutex, NULL);
    }

    void callNodes() {
        cout << "Processing begun by layer " << layerNumber << endl;

        pthread_t tid[numberNodes];
        FunkyFuncArgs args[numberNodes];

        for (int i = 0; i < numberNodes; i++) {
            args[i].node = &Nodes[i];
            args[i].mutex = &nodeMutex;
            pthread_create(&tid[i], NULL, funkyFunc, &args[i]);
        }

        for (int i = 0; i < numberNodes; i++) {
            pthread_join(tid[i], NULL);
        }
    }

    void backProp() {
        cout << "BackProp begun by layer " << layerNumber << endl;

        pthread_t tid[numberNodes];
        FunkyFuncArgs args[numberNodes];

        for (int i = 0; i < numberNodes; i++) {
            args[i].node = &Nodes[i];
            args[i].mutex = &nodeMutex;
            pthread_create(&tid[i], NULL, funkyFunc, &args[i]);
        }

        for (int i = 0; i < numberNodes; i++) {
            pthread_join(tid[i], NULL);
        }
    }

    friend void *funkyFunc(void *argc);
};

struct NueralNetwork {
    Layer *Layers;
    int numLayers;
    int **pipes;

    NueralNetwork(int nL, int *nodesInLayer) {
        numLayers = nL;
        Layers = new Layer[nL];

        pipes = new int *[nL - 1];
        for (int i = 0; i < nL - 1; i++) {
            pipes[i] = new int[2];
            pipe(pipes[i]);
        }

        for (int i = 0; i < nL; i++) {
            Layers[i].layerNumber = i + 1;
            Layers[i].numberNodes = nodesInLayer[i];
            Layers[i].Nodes = new Node[nodesInLayer[i]];

            for (int j = 0; j < nodesInLayer[i]; j++) {
                Layers[i].Nodes[j].nodeNumber = j + 1;
            }
        }
    }

    void stuff() {
        cout << "AI stuff begun by neural network" << endl;

        for (int i = 0; i < numLayers; i++) {
            pid_t layerProcess = fork();
                    if (layerProcess == 0) { // Child process
            if (i > 0) {
                close(pipes[i - 1][1]); // Close unused write end of previous pipe
            }
            if (i < numLayers - 1) {
                close(pipes[i][0]); // Close unused read end of current pipe
            }

            // Forward pass
            if (i > 0) {
                int dummy;
                read(pipes[i - 1][0], &dummy, sizeof(dummy)); // Read from previous pipe
            }

            Layers[i].callNodes();

            if (i < numLayers - 1) {
                int dummy = 1;
                write(pipes[i][1], &dummy, sizeof(dummy)); // Write to current pipe
            }

            // Backward pass
            if (i < numLayers - 1) {
                int dummy;
                read(pipes[i][0], &dummy, sizeof(dummy)); // Read from current pipe
            }

            Layers[i].backProp();

            if (i > 0) {
                int dummy = 1;
                write(pipes[i - 1][1], &dummy, sizeof(dummy)); // Write to previous pipe
            }

            exit(0);
        } else { // Parent process
            if (i > 0) {
                close(pipes[i - 1][0]); // Close unused read end of previous pipe
            }
            if (i < numLayers - 1) {
                close(pipes[i][1]); // Close unused write end of current pipe
            }
        }
    }

    for (int i = 0; i < numLayers; i++) {
        wait(NULL);
    }

    cout << "Neural Network Pass Completed" << endl;

    for (int i = 0; i < numLayers - 1; i++) {
        delete[] pipes[i];
    }
    delete[] pipes;
}
};

int main() {
int numLayers = 8;
int *nodesPerLayer = new int[8];
nodesPerLayer[0] = 3;
for (int i = 1; i < 7; i++)
nodesPerLayer[i] = 8;
nodesPerLayer[7] = 1;
NueralNetwork NN(8, nodesPerLayer);

NN.stuff();

delete[] nodesPerLayer;
return 0;
    }
