#include <iostream>
#include <unistd.h>
#include <pthread.h>
#include <wait.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <vector>
#include <cstring>
#include <fstream>
#include <sstream>

using namespace std;


double** Mul(double** x, int r1, int c1, double** y, int r2, int c2) {

    double** result = new double*[r1];
    for (int i = 0; i < r1; ++i) {
        result[i] = new double[c2];
    }

    int z = 0;
    while (z < 0)
    {
        int y = 0;
        while (y < 0)
        {
            result [z][y] = 0;
            y++;
        }
        z++;
    }
    for(int i = 0; i < r1; i++)
    {
        for (int j = 0; j < c2; ++j)
        {
            result[i][j] = 0;
        }
    }

    for (int i = 0; i < r1; i++)
    {
        for (int j = 0; j < c2; j++)
        {
            for (int k = 0; k < c1; k++)
            {
                result[i][j] += x[i][k] * y[k][j];
            }
        }
    }

    return result;
}

struct Neuron
{
    int input;
    int nWeights;
    double** weights;

    double* activationFtn(double** input)
    {
        
        double** x = Mul(input, 1, nWeights, weights,nWeights, 1);
        
        double* output=new double(x[0][0]);

        return output;
    }
};

struct threadArgs
{
    double** data;
    Neuron* neu;
    pthread_mutex_t* mutex;
    double* outputArr;
    int index;

};

void* funct(void* args)
{
    threadArgs* arg = (threadArgs*)args;

    pthread_mutex_t mutex = *(arg->mutex);
    
    Neuron* neuron = arg->neu;
    
    double** inp = arg->data;

    pthread_mutex_lock(&mutex);
    double* output = neuron->activationFtn(inp);
    pthread_mutex_unlock(&mutex);

    arg->outputArr[arg->index] = *output;

    delete output;
    output = nullptr;

    pthread_exit(NULL);
}

struct Layer
{
    double** onData;
    Neuron* neuron;
    int nNodes = 0;
    pthread_mutex_t nodeMutex;

    Layer()
    {

        pthread_mutex_init(&nodeMutex, NULL);

    }

    double** forward()
    {

        double** outputArr = new double*[1];
        outputArr[0] = new double[nNodes];

        threadArgs args[nNodes];

        pthread_t tid[nNodes];
        

        

        for (int i = 0; i < nNodes; i++) 
        {
            args[i].outputArr = outputArr[0]; 
            args[i].index = i;
            
            args[i].neu = &neuron[i];
            args[i].data = onData;
            
            args[i].mutex = &nodeMutex;
            args[i].data = onData;
            
            pthread_create(&tid[i], NULL, funct, &args[i]);
        }


        int x = 0;
        while ( x < nNodes)
        {
            usleep(1000);
            x++;
        }


        return outputArr;
    }

};

double* calculateFx(double x) 
{
    double* result = new double[2];

    double y = 0;
    y = (x * x) + x + 1; 
    y = y / 2.0;
    result[0] = y;

    double z = 0;
    z = (x * x) - x;
    z = z / 2.0;
    result[1] = z;

    
    return result;
}

struct NeuralNetwork
{

    Layer* layers;
    int nLayers;
    int* nNodes;

    NeuralNetwork(int numLayers, int* numNodes, int* weights, vector<double> values){

        int counter = 0;

        nLayers = numLayers;
        nNodes = numNodes;

        layers = new Layer[nLayers];

        for(int i = 0; i < nLayers; i++)
        {
            layers[i].nNodes = numNodes[i];
            layers[i].neuron = new Neuron[numNodes[i]];
        }

        for(int i = 0; i < nLayers; i++)
        {
            for(int j = 0; j < nNodes[i]; j++)
            {
                layers[i].neuron[j].weights = new double*[weights[i]];
                layers[i].neuron[j].nWeights = weights[i];

                

            }
        }

        for(int i = 0; i < nLayers; i++)
        {
            for(int j = 0; j < nNodes[i]; j++)
            {
                for(int k = 0; k < weights[i]; k++)
                {
                    layers[i].neuron[j].weights[k] = new double[1];
                    layers[i].neuron[j].weights[k][0] = values[counter++];
                }
            }
        }

        int y = 0;
        while (y < numLayers)
        {
            string pipeName = "pipe" + to_string(y) + to_string(y + 1);

            int x = mkfifo(pipeName.c_str(), 0666);

            if(x)
            {
                cout << "Pipes not created\n\n";
                exit(0);
            }
            y++;
        }
        

    }

    double* propogation(vector<vector<double>> input){

        string inpPipe = "pipe01";

        cout << "\n------------------------------------\n\n";
        cout << "FORWARD PROPOGATION STARTED\n\n";

        double* newInput = new double[2];
        double** readingArr;
        pid_t layerInp = fork();

        if(!layerInp)
        {
            double writeArr[1 * 2];
            for (int i = 0; i < 2; i++) 
            {
                writeArr[0 * 2 + i] = input[0][i];
            }

            int fd0 = open(inpPipe.c_str(), O_WRONLY);
            write(fd0, writeArr, sizeof(writeArr));
            close(fd0);
            exit(0);

        }
        else if(layerInp < 0)
        {
            perror("ERROR!!\n");
            cout << "Process 0 not created!!\n\n";
            exit(0);
        }

        for(int i = 0; i < nLayers; i++){

            pid_t layerProcess = fork();

            string readPipe = "pipe" + to_string(i) + to_string(i + 1);
            string writePipe = "pipe" + to_string(i + 1) + to_string(i + 2);

            int readingCols = layers[i].neuron->nWeights;

            double readingFarr[1 * readingCols];
            readingArr = new double*[1];
            double writeArr[1 * layers[i].nNodes];

            readingArr[0] = new double[readingCols];
            
            if(layerProcess)
            {
                int fd1 = open(readPipe.c_str(), O_RDONLY);   
                read(fd1, readingFarr, sizeof(readingFarr));
                close(fd1);

                for (int j = 0; j < readingCols; j++)
                {
                    readingArr[0][j] = readingFarr[(0 * readingCols) + j];
                }

                layers[i].onData = readingArr;

                double** output = layers[i].forward();
                

                if(i < nLayers - 1)
                {
                    for (int j = 0; j < layers[i].nNodes; ++j) 
                    {
                        writeArr[0 * layers[i].nNodes + j] = output[0][j];
                    }

                    int fd2 = open(writePipe.c_str(), O_WRONLY);
                    write(fd2, writeArr, sizeof(writeArr));
                    close(fd2);
                }
                else
                {
                    double* outputFx = calculateFx(output[0][0]);

                    cout << "\nForward Propogation Output: " << output[0][0] << endl << endl;
                    cout << "fx1: " << outputFx[0] << ", fx2:" << outputFx[1] << endl << endl;

                    cout << "**************************************\n\n";
                    cout << "BACKWARD PROPOGATION STARTED\n\n";

                    int fd3 = open(readPipe.c_str(), O_WRONLY);
                    write(fd3, outputFx, 16);
                    close(fd3);

                    exit(0);
                }

                int fd4 = open(writePipe.c_str(), O_RDONLY);
                read(fd4, newInput, 16);
                close(fd4);


                int fd5 = open(readPipe.c_str(), O_WRONLY);
                write(fd5, newInput, 16);
                close(fd5);

                if(i == 0)
                {
                    int fd6 = open(inpPipe.c_str(), O_WRONLY);
                    write(fd6, newInput, 16);
                    close(fd6);
                }
                exit(0);
            }
            else if(layerProcess < 0)
            {
                perror("ERROR!!!\n");
                cout << "Hidden Layers not being created!!!\n\n";
                exit(0);
            }
        }

        int fd7 = open(inpPipe.c_str(), O_RDONLY);
        read(fd7, newInput, 16);
        close(fd7);

        cout << "BACKWARD PROPOGATION COMPLETED\n\n";

        cout << "Data at Input Layer:\nInput 1: " << newInput[0] << ", Input 2: " << newInput[1] << "\n\n";
        delete[] readingArr[0];
        delete[] readingArr;
        readingArr = nullptr;
        return newInput;
    }

};

vector<double> read_file_to_1d_vector(const string& filename)
{
    vector<double> result;
    ifstream input_file(filename);

    if (input_file.is_open())
    {
        string line;
        while (getline(input_file, line))
        {
            double value;
            istringstream line_stream(line);
            string token;

            while (getline(line_stream, token, ','))
            {
                istringstream token_stream(token);
                token_stream >> value;
                result.push_back(value);
            }
        }

        input_file.close();
    }
    else
    {
        cout << "Unable to open file." << endl;
        exit(0);
    }

    

    return result;
}

void pipesUnlinker(int numLayers)
{
    for(int i = 0; i < numLayers; i++)
    {
        string pipeName = "pipe" + to_string(i) + to_string(i + 1);
        unlink(pipeName.c_str());
        remove(pipeName.c_str());
    }
}

int main()
{
    cout<<"******** Neural Network ********" << endl;

    cout << "Shujaan Azhar      21I-0406" << endl;
    cout << "Moatasim Zaman     21I-2996" << endl;
    cout << "Faisal             21I-2578" << endl;

    vector<vector<double>> input_value;

    double num1, num2;
    cout<< "Enter values of input values: "<<endl;
    cin >> num1 >> num2;

    input_value.resize(1); // Create one row
    input_value[0].resize(2); // Create two columns for the first row

    input_value[0][0] = num1;
    input_value[0][1] = num2;

    // Now here send this input forward to forward 

    int numLayers = 7;
    int* nodes = new int[numLayers];
    int* weights = new int[numLayers];


    string filename = "config.txt";

    vector<double> assigning_weights = read_file_to_1d_vector(filename);
    

    for (int i = 0; i < numLayers - 1; i++)
    {
        nodes[i] = 8;
    }
    nodes[numLayers - 1] = 1;


    weights[0] = 2;
    for(int i = 1; i < numLayers; i++)
    {
        weights[i] = 8;
    }

    NeuralNetwork nn(numLayers, nodes, weights, assigning_weights);
    
    double* firstpass = nn.propogation(input_value);


     // Getting output
    vector<vector<double>> output_from_first;
    
    output_from_first.resize(1); // Create one row
    output_from_first[0].resize(2); // Create two columns for the first row

    // Here change input values and send for second transfers
    output_from_first[0][0] = firstpass[0];
    output_from_first[0][1] = firstpass[1];

    pipesUnlinker(numLayers);
    
    cout << "\n\n\tSecond Propogation\n\n";
    // Again calling froward prop
    NeuralNetwork nn1(numLayers, nodes, weights, assigning_weights);
    
    nn1.propogation(output_from_first);

    pipesUnlinker(numLayers);

    //Deleting DMAs to avoid mem leaks
    delete[] nodes;
    nodes = nullptr;

    delete[] weights;
    weights = nullptr;

    delete[] firstpass;
    firstpass = nullptr;


    exit(0);

    return 0;
}