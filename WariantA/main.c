#include "data.h"


//global mutexes to use them everywhere
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
int iter=1;


// A structure to represent a queue
struct Queue {
    int front, rear, size;
    unsigned capacity;
    int* array;
};


//queue to wait for the bridge to be free
struct Queue* queue;
 
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
    //printf("%d enqueued to queue\n", item);
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


//Function for printing status
void PrintStatus()
{
    //do nothing if no car is on the bridge
    if(carOnBridge == -1) return;

    //otherwise print status
    printf("A-%d %d>>> [%s %d %s] <<< %d %d-B\n",carsInA, carsBeforeBridgeA,
           direction, carOnBridge, direction,carsBeforeBridgeB, carsInB);
}


//referee is for deciding wheather car can cross the bridge or not
void *Referee(void *args)
{
    int carsNo=*((int*)args), i;

    for(i=1;i<carsNo+1;i++)
    {
        //block every mutex of the car, so no car can cross the bridge
        pthread_mutex_lock(&mutexArray[i]);
    }
    
    //handling FIFO queue - first in first out
    while(1)
    {
        //if the queue is not empty
        while(!isEmpty(queue))
        {
            //take first number, we will give it acces to the bridge
            int number = dequeue(queue);

            //unlock the mutex for this car, so it can cross the bridge
            pthread_mutex_unlock(&mutexArray[number]);

            //wait for car to pass the bridge
            ClockSleep(2000,5000);

            //lock the mutex again, so same car cannot cross the bridge again without waiting
            pthread_mutex_lock(&mutexArray[number]);
        }
    }
}

void *CarRoutine(void *args)
{
    int vehicleNo = iter;
    iter++;
    while(1)
    {
        //the very first thing to do is 
        //stay in queue to bridge for your time

        //wyjazd z miasta A
        //ustawiamy się w kolejce, zwiększamy liczniki itd.
        pthread_mutex_lock(&cs);
        carsInA -- ;
        carsBeforeBridgeA ++;
        PrintStatus();
        enqueue(queue,vehicleNo);
        pthread_mutex_unlock(&cs);


        //wait for unlock of the bridge
        pthread_mutex_lock(&mutexArray[vehicleNo]);
        pthread_mutex_lock(&cs);
        carsBeforeBridgeA--;
        direction=">>";
        carOnBridge = vehicleNo;
        PrintStatus();

        ClockSleep(minSleepTime,maxSleepTime);
        //wjeżdzamy do miasta B
        carOnBridge = -1;
        direction="||";
        carsInB++;

        //unlock the mutex, so it can be blocked again
        pthread_mutex_unlock(&mutexArray[vehicleNo]);
        pthread_mutex_unlock(&cs);

        //wait in the city
        ClockSleep(minSleepTime, maxSleepTime);

        //ustawiamy się w kolejce, zwiększamy liczniki itd.
        pthread_mutex_lock(&cs);
        carsInB -- ;
        carsBeforeBridgeB ++;
        PrintStatus();
        enqueue(queue,vehicleNo);
        pthread_mutex_unlock(&cs);



         //wait for unlock of the bridge
        pthread_mutex_lock(&mutexArray[vehicleNo]);
        
        //wait for critical section mutex
        pthread_mutex_lock(&cs);
        carsBeforeBridgeB--;
        carOnBridge=vehicleNo;
        direction="<<";
        PrintStatus();
        ClockSleep(minSleepTime,maxSleepTime);
        

        //wjazd do miasta A
        carOnBridge = -1;
        direction = "||";
        carsInA++;
        PrintStatus();
        //unlock the mutex, so it can be blocked again
        pthread_mutex_unlock(&mutexArray[vehicleNo]);
        pthread_mutex_unlock(&cs);

        ClockSleep(minSleepTime, maxSleepTime);

        

    }
}



int main(int argc, char* argv[])
{
    //vaildate given data
    if(ValidateData(argc,argv) == false)
    {
        printf("There's been error connected to validation, aborting!\n");
        exit(EXIT_FAILURE);
    }

    //program needs car number to make mutexArray and other stuff
    int CarNumber = atoi(argv[1]);
    carsInA = CarNumber;

    //dynamically inicalised mutex array, for bridge hanling
    mutexArray = (pthread_mutex_t*)malloc((CarNumber+1)*sizeof(pthread_mutex_t));

    
    queue = createQueue(CarNumber);

    //threads for cars
    pthread_t car[CarNumber];

    //thread for referee
    pthread_t referee;

    //variable for iterations
    int i;

    //initialise all mutexes for cars, those are for allowing and not allowing specific car
    //to cross the bridge
    for(i=0;i<CarNumber;i++)
    {
        if(0!=pthread_mutex_init(&mutexArray[i],NULL))
        {
            printf("Error during mutex %d inicialisation",i);
            errno=-1;
            exit(EXIT_FAILURE);
        }
    }

    //initialise critical section mutex
    if(0!= pthread_mutex_init(&cs,NULL))
    {
        printf("Error occured during inicialisation of critical section");
        errno=-1;
        exit(EXIT_FAILURE);
    }

    //make thread for referee, which is for managing the bridge
    if(0!= pthread_create(&referee,NULL,Referee,&CarNumber))
    {
        printf("Some error connected to referee occured, aborting!");
        errno=-1;
        exit(EXIT_FAILURE);
    }

    //wait for initialisation of all mutexes, referee has to lock every mutex for cars
    //to prevent from pasing without its turn
    ClockSleep(8000,10000);

    //make threads for cars
    for(i = 0; i<CarNumber; i++)
    {
        if(0 != pthread_create(&car[i], NULL, CarRoutine,&i))
        {
            printf("Some error occured during inicialization of %d car\n",i);
            errno=1;
            exit(EXIT_FAILURE);
        }
    }

    //doesnt have to be here, because cars are going forever
    for(i=0; i<CarNumber; i++)
    {
        pthread_join(car[i], NULL);
    }
    //end program, if everything is ok
    exit(EXIT_SUCCESS);
}
