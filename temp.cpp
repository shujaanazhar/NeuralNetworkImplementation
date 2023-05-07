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

        return NULL;
    }
};

struct Layer
{
    Neuron* neuron;
    pthread_mutex_t mutex;
    int nNodes = 0;
    int numlayer;

    pthread_mutex_t barrier_mutex;
    pthread_cond_t barrier_cond;
    int barrier_count;

    Layer()
    {
        pthread_mutex_init(&mutex, NULL);
        pthread_mutex_init(&barrier_mutex, NULL);
        pthread_cond_init(&barrier_cond, NULL);
        barrier_count = 0;
    }

    void barrier_wait()
    {
        pthread_mutex_lock(&barrier_mutex);
        barrier_count++;

        if (barrier_count == nNodes)
        {
            barrier_count = 0;
            pthread_cond_broadcast(&barrier_cond);
        }
        else
        {
            pthread_cond_wait(&barrier_cond, &barrier_mutex);
        }

        pthread_mutex_unlock(&barrier_mutex);
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

        barrier_wait();
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
            layers[i].nNodes = numNodes[i];
            layers[i].neuron = new Neuron[numNodes[i]];
            layers[i].numlayer = i + 1;
            layers[i].forward();
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
