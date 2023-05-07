#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <wait.h>
#include <fcntl.h>
#include <sys/stat.h>

using namespace std;

pthread_mutex_t layerlock;
int counter = 0;

struct Neuron
{
    int number;
    int* weights;
    int bias;
    pthread_mutex_t lock;

    Neuron()
    {
        pthread_mutex_init(&lock, NULL);
    }

    void algo()
    {
        pthread_mutex_lock(&lock);
        cout << "Processing started for " << number << endl;
        pthread_mutex_unlock(&lock);
    }

    static void* startAlgo(void* arg)
    {
        Neuron* neuron = static_cast<Neuron*>(arg);
        neuron->algo();

        //pthread_mutex_lock(&layerlock);
        // counter++;
        // cout << "Counter = " << counter << endl;
        //pthread_mutex_unlock(&layerlock);

        return NULL;
    }
};

struct Layer
{
    Neuron* neuron;
    pthread_mutex_t mutex;
    int nNodes = 0;
    int numlayer;

    Layer()
    {
        pthread_mutex_init(&mutex, NULL);
    }

    void forward()
    {
        cout << "Starting layer " << numlayer << endl;

        pthread_t tid[nNodes];
        neuron = new Neuron[nNodes];

        for (int i = 0; i < nNodes; i++)
        {
            neuron[i].number = i + 1;
            pthread_create(&tid[i], NULL, Neuron::startAlgo, &neuron[i]);
        }
        for (int i = 0; i < nNodes; i++)
            pthread_join(tid[i], NULL);
    }

    void backPropogate()
    {

    }
};

struct NeuralNetwork
{
    Layer* layers;
    int numLayers;
    int numNodes;

    NeuralNetwork(int numLayers, int* numNodes)
    {
        // Creating layers
        layers = new Layer[numLayers];

        pthread_mutex_init(&layerlock, NULL);
        for (int i = 0; i < numLayers; i++)
        {
            pid_t layerID = fork();
            if(layerID == 0)
            {
                layers[i].nNodes = numNodes[i];
                layers[i].neuron = new Neuron[numNodes[i]];
                layers[i].numlayer = i + 1;
                layers[i].forward();
            }
            else
            {
                //FUCK U
            }
        }

        pthread_mutex_destroy(&layerlock);
    }
};

int main()
{
    int numLayers = 8;
    int* nodes = new int[numLayers];

    nodes[0] = 3;

    for (int i = 1; i < numLayers - 1; i++)
    {
        nodes[i] = 8;
    }
    nodes[7] = 1;

    NeuralNetwork nn(numLayers, nodes);

    return 0;
}