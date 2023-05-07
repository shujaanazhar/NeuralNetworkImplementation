#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <sys/wait.h>

using namespace std;

struct Node{

    int nodeNumber;
    double* weights;
    double bias;

    void doAlgorithm(){


    }

};

struct FunkyFuncArgs {
    Node* node;
    pthread_mutex_t* mutex;
};

void* funkyFunc(void* args) {
    FunkyFuncArgs* funcArgs = (FunkyFuncArgs*)args;
    Node* n = funcArgs->node;
    pthread_mutex_t mutex = *(funcArgs->mutex);

    pthread_mutex_lock(&mutex);

    n->doAlgorithm();
    cout << "AI stuff being done by Node " << n->nodeNumber << endl;

    pthread_mutex_unlock(&mutex);

    pthread_exit(NULL);

}

struct Layer{

    int layerNumber;
    Node* Nodes;
    int numberNodes = 0;
    pthread_mutex_t nodeMutex;

    Layer(){

        pthread_mutex_init(&nodeMutex, NULL);

    }

    void callNodes(){

        cout << "Processing begun by layer " << layerNumber << endl;

        pthread_t tid[numberNodes];
        FunkyFuncArgs args[numberNodes];

        for (int i = 0; i < numberNodes; i++) {
            args[i].node = &Nodes[i];
            args[i].mutex = &nodeMutex;
            pthread_create(&tid[i], NULL, funkyFunc, &args[i]);
        }

        for(int i = 0; i < numberNodes; i++){

            pthread_join(tid[i], NULL);

        }

    }

    void backProp(){

        cout << "BackProp begun by layer " << layerNumber << endl;

        pthread_t tid[numberNodes];
        FunkyFuncArgs args[numberNodes];

        for (int i = 0; i < numberNodes; i++) {
            args[i].node = &Nodes[i];
            args[i].mutex = &nodeMutex;
            pthread_create(&tid[i], NULL, funkyFunc, &args[i]);
        }

        for(int i = 0; i < numberNodes; i++){

            pthread_join(tid[i], NULL);

        }

    }

    friend void* funkyFunc(void* argc);

};

struct NueralNetwork{

    Layer* Layers;
    int numLayers;

    NueralNetwork(int nL, int* nodesInLayer){

        numLayers = nL;

        Layers = new Layer[nL];

        for(int i = 0; i < nL; i++){

            Layers[i].layerNumber = i + 1;
            Layers[i].numberNodes = nodesInLayer[i];
            Layers[i].Nodes = new Node[nodesInLayer[i]];

            for(int j = 0; j < nodesInLayer[i]; j++){

                Layers[i].Nodes[j].nodeNumber = j + 1;

            }

        }

        for(int i = 0; i < nL; i++){

            string fifoString = "";
            fifoString += "DataFifo";
            fifoString += to_string(i);
            fifoString += to_string(i + 1);

            mkfifo(fifoString.c_str(), 0666);

            string semString = "";
            semString += "forwardSem";
            semString += to_string(i);

            sem_t *fsemaphore = sem_open(semString.c_str(), O_CREAT | O_EXCL, 0644, 0);

            string backSemString = "";
            backSemString += "backSem";
            backSemString += to_string(i);

            sem_t *bsemaphore = sem_open(backSemString.c_str(), O_CREAT | O_EXCL, 0644, 0);

        }

    }

void stuff(){

    string fifoString = "DataFifo01";

    sem_t* parent_semaphore = sem_open("forwardSem0", 0);

    cout << "Ai stuff begun by nueral network" << endl;

    string s = "backSem" + to_string(numLayers - 1);

    sem_t* parent_semaphore2 = sem_open(s.c_str(), 0);

    sem_post(parent_semaphore2);

    sem_post(parent_semaphore);

    int value;

    sem_close(parent_semaphore);
    sem_close(parent_semaphore2);

    for(int i = 0; i < numLayers; i++){

        pid_t layerProcess = fork();

        if(layerProcess == 0){

            string semString = "";
            semString += "forwardSem";
            semString += to_string(i);

            string semString2 = "";
            semString2 += "forwardSem";
            semString2 += to_string(i + 1);

            string backsem = "";
            backsem += "backSem";
            backsem += to_string(i);

            string backsem2 = "";
            backsem2 += "backSem";
            backsem2 += to_string(i - 1);

            sem_t* bchild_semaphore = sem_open(backsem.c_str(), 0);
            sem_t* bchild_semaphore2 = sem_open(backsem2.c_str(), 0);

            sem_t* child_semaphore = sem_open(semString.c_str(), 0);
            sem_t* child_semaphore2;

            if(i != numLayers - 1){

                child_semaphore2 = sem_open(semString2.c_str(), 0);

            }

            sem_getvalue(child_semaphore, &value);

            sem_wait(child_semaphore);

            sem_close(child_semaphore);

            Layers[i].callNodes();

            if(i != numLayers - 1){

                sem_post(child_semaphore2);

                sem_close(child_semaphore2);

            }

            sem_wait(bchild_semaphore);

            sem_close(bchild_semaphore);

            Layers[i].backProp();

            sem_post(bchild_semaphore2);

            sem_close(bchild_semaphore2);

            exit(0);

        }

        else{


        }

    }

    for(int i = 0; i < numLayers; i++){

        wait(NULL);

    }

    cout << "Nueral Network Pass Completed" << endl;

    for(int i = 0; i < numLayers; i++){

        string fifoString = "";
        fifoString += "DataFifo";
        fifoString += to_string(i);
        fifoString += to_string(i + 1);

        mkfifo(fifoString.c_str(), 0666);

        string semString = "";
        semString += "forwardSem";
        semString += to_string(i);

        sem_unlink(semString.c_str());

        string backSemString = "";
        backSemString += "backSem";
        backSemString += to_string(i);

        sem_unlink(backSemString.c_str());

    }

}


};

int main(){

    int numLayers = 3;
    int* nodesPerLayer = new int[3];
    nodesPerLayer[0] = 2;
    nodesPerLayer[1] = 3;
    nodesPerLayer[2] = 1;

    NueralNetwork NN(3, nodesPerLayer);

    NN.stuff();

}