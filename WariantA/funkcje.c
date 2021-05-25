#include "data.h"

// A structure to represent a queue
struct Queue {
    int front, rear, size;
    unsigned capacity;
    int* array;
};
 
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

void ClockSleep(int minTime,int maxTime)
{
    //pobieramy aktualny czas systemowy
    time_t tt;

    //podstawiamy za ziarno czas
    int seed = time(&tt);

    //ustawiamy, by ziarno losowania było liczbą
    //czasu
    srand(seed);

    //śpimy nie więcej, niż to wynosi maxTime
    int sleepingTime=rand()%maxTime;

    //sprawdzamy, czy wartość sleepingTime nie jest mniejsza niż
    //minTime
    while(sleepingTime<minTime)
    {
        sleepingTime=rand()%maxTime;
    }


    usleep(sleepingTime);
    return;
}

/*Funkcja do sprawdzania długości liczby, dla 
liczb 0-9 zwraca 1, dla 10-99 zwraca 2 itd.*/

int lenHelper(unsigned carCounter) 
{
	if(carCounter == 0) return 1;
    float helper = (float)carCounter;
    int length=0;
    while(helper>=1)
    {
        length ++;
        helper /= 10;
    }
    return length;
}

//Funkcja do walidacji danych wejściowych, zwraca false, jeśli walidacja
//nie przeszła, zwraca true, jeśli dane są poprawne

bool ValidateData(int argc,char *argv[])
{
    //carCounter argumentów większa niż 2, oznacza
    //że ktoś podał więcej argumentów niż można
    //program przyjmuje tylko ilość samochodów
    if(argc > 2)
    {
        printf("carCounter argumentów jest zbyt duża, dozwolona tylko carCounter samochodów\n");
        errno=-1;
        return false;
    }

    //sprawdzamy, jaka carCounter jest w argumencie
    int carCounter = atoi(argv[1]);

    if(carCounter < 0)
    {
        printf("Liczba samochodów nie może być ujemna!\n");
        errno=-1;
        return false;
    }

    //sprawdzamy, czy była to carCounter całkowita
    //czy jakiś ciąg znaków
    if(lenHelper(carCounter) != strlen(argv[1]))
    {
        printf("Podana carCounter samochodów nie jest poprawną liczbą!\n");
        errno=-1;
        return false;;
    }

    //sprawdzamy, czy carCounter nie jest zerem
    //muszą być jakieś samochody przecież
    if(carCounter == 0)
    {
        printf("carCounter samochodów nie może wynosić 0!\n");
        errno=-1;
        return false;
    }

    return true;
}


