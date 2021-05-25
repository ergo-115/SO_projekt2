#include "data.h"


int iter = 1;
//tworzymy most jako zmienną globalną, ponieważ ma być
//dostępny dla każdego samochodu
pthread_mutex_t bridge;

pthread_mutex_t cs;

pthread_mutex_t *mutexArray;


int minSleepTime = 10000;
int maxSleepTime = 100000;
int carsInA=0;
int carsInB=0;
int carOnBridge=-1;
int carsBeforeBridgeA=0;
int carsBeforeBridgeB=0;
char *direction;


// A structure to represent a queue
struct Queue {
    int front, rear, size;
    unsigned capacity;
    int* array;
};

//inicjalizacja wskaźnika na strukturę do przechowywania kolejki do mostu
//kolejka do mostu obsługuje żądania z obydwu stron!
struct Queue* queueAB;
struct Queue* queueBA;
 
// function to create a queue
// of given capacity.
// It initializes size of queue as 0
struct Queue* createQueue(unsigned capacity)
{
    struct Queue* queue = (struct Queue*)malloc(
        sizeof(struct Queue));
    queue->capacity = capacity;
    queue->front = queue->size = 0;
 
    // This is important, see the enqueue
    queue->rear = capacity - 1;
    queue->array = (int*)malloc(
        queue->capacity * sizeof(int));
    return queue;
}
 
// Queue is full when size becomes
// equal to the capacity
int isFull(struct Queue* queue)
{
    return (queue->size == queue->capacity);
}
 
// Queue is empty when size is 0
int isEmpty(struct Queue* queue)
{
    return (queue->size == 0);
}
 
// Function to add an item to the queue.
// It changes rear and size
void enqueue(struct Queue* queue, int item)
{
    if (isFull(queue))
        return;
    queue->rear = (queue->rear + 1)
                  % queue->capacity;
    queue->array[queue->rear] = item;
    queue->size = queue->size + 1;
    printf("%d enqueued to queue\n", item);
}
 
// Function to remove an item from queue.
// It changes front and size
int dequeue(struct Queue* queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    int item = queue->array[queue->front];
    queue->front = (queue->front + 1)
                   % queue->capacity;
    queue->size = queue->size - 1;
    return item;
}
 
// Function to get front of queue
int front(struct Queue* queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    return queue->array[queue->front];
}
 
// Function to get rear of queue
int rear(struct Queue* queue)
{
    if (isEmpty(queue))
        return INT_MIN;
    return queue->array[queue->rear];
}


//Funkcja do wyświetlania statusu w postaci:
//A-5 10>>> [>> 4 >>] <<<4 6-B

void PrintStatus()
{
    printf("A-%d %d>>> [%s %d %s] <<< %d %d-B\n",carsInA, carsBeforeBridgeA,
           direction, carOnBridge, direction,carsBeforeBridgeB, carsInB);
}


//tutaj dodamy czynności sędziego, który będzie rozsrztygał, kto przejeżdza przez most
//musi on zablokować na początku wszystkie mutexy, a później odblokowywać odbowiedni mutex, kt
//ry stoi w kolejce i jest pierwszyls
void *Referee(void *args)
{
    //dobra to działa!
    int carsNo=*((int*)args);
    printf("Referee succesfully created!, with %d cars\n",carsNo);
}

void *CarRoutine(void *args)
{
    int vehicleNo = iter;
    iter++;
    while(1)
    {
        //wjazd do miasta A
        pthread_mutex_lock(&cs);
        carOnBridge = -1;
        direction = "||";
        carsInA++;
        PrintStatus();
        pthread_mutex_unlock(&cs);

        ClockSleep(minSleepTime, maxSleepTime);

        //wyjazd z miasta A
        //ustawiamy się w kolejce, zwiększamy liczniki itd.
        pthread_mutex_lock(&cs);
        carsInA -- ;
        carsBeforeBridgeA ++;
        enqueue(queueAB,vehicleNo);
        PrintStatus();
        pthread_mutex_unlock(&cs);

        ClockSleep(2000, 5000);

        //czekamy na możliwość wjazdu na most, jeśli mamy możliwość
        //od razu wjeżdzamy
        pthread_mutex_lock(&bridge);
        pthread_mutex_lock(&cs);
        carsBeforeBridgeA--;
        carOnBridge=dequeue(queueAB);
        direction=">>";
        PrintStatus();
        pthread_mutex_unlock(&cs);
        pthread_mutex_unlock(&bridge);

        ClockSleep(1000, 3999);

        //wjeżdzamy do miasta B
        pthread_mutex_lock(&cs);
        carsInB++;
        PrintStatus();
        pthread_mutex_unlock(&cs);

        ClockSleep(4000, 7000);

        //ustawiamy się w kolejce, zwiększamy liczniki itd.
        pthread_mutex_lock(&cs);
        carsInB -- ;
        carsBeforeBridgeB ++;
        enqueue(queueBA,vehicleNo);
        PrintStatus();
        pthread_mutex_unlock(&cs);

        ClockSleep(3000, 6000);


        //czekamy na możliwość wjazdu na most, jeśli mamy możliwość
        //od razu wjeżdzamy
        pthread_mutex_lock(&bridge);
        pthread_mutex_lock(&cs);
        carsBeforeBridgeB--;
        carOnBridge=dequeue(queueBA);
        direction="<<";
        PrintStatus();
        pthread_mutex_unlock(&cs);
        pthread_mutex_unlock(&bridge);

        ClockSleep(2000, 5000);

    }
}



int main(int argc, char* argv[])
{
    if(ValidateData(argc,argv) == false)
    {
        printf("Błąd danych wejściowych, anulowanie\n");
        exit(EXIT_FAILURE);
    }

    //program needs car number to make mutexArray and other stuff
    int CarNumber = atoi(argv[1]);

    //dynamically inicalised mutex array, for bridge hanling
    mutexArray = (pthread_mutex_t*)malloc(CarNumber*sizeof(pthread_mutex_t));


    //kolejka
    queueAB = createQueue(CarNumber);
    queueBA = createQueue(CarNumber);

    //Samochody
    pthread_t car[CarNumber];
    pthread_t referee;

    //variable for iterations
    int i;

    for(i=0;i<CarNumber;i++)
    {
        if(0!=pthread_mutex_init(&mutexArray[i],NULL))
        {
            printf("Error during mutex %d inicialisation",i);
            errno=-1;
            exit(EXIT_FAILURE);
        }
    }

    if(0!= pthread_mutex_init(&cs,NULL))
    {
        printf("Error occured during inicialisation of critical section");
        errno=-1;
        exit(EXIT_FAILURE);
    }
    if(0!= pthread_create(&referee,NULL,Referee,&CarNumber))
    {
        printf("Some error connected to referee occured, aborting!");
        errno=-1;
        exit(EXIT_FAILURE);
    }


    for(i = 0; i<CarNumber; i++)
    {
        if(0 != pthread_create(&car[i], NULL, CarRoutine,&i))
        {
            printf("Some error occured during inicialization of %d car\n",i);
            errno=1;
            exit(EXIT_FAILURE);
        }
    }

    for(i=0; i<CarNumber; i++)
    {
        pthread_join(car[i], NULL);
    }
    return 0;
}
