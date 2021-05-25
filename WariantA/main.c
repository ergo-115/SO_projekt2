#include "data.h"


int iter = 1;
//tworzymy most jako zmienną globalną, ponieważ ma być
//dostępny dla każdego samochodu
pthread_mutex_t bridge;

pthread_mutex_t cs;


int minSleepTime = 10000;
int maxSleepTime = 100000;
int carsInA=0;
int carsInB=0;
int carOnBridge=-1;
int carsBeforeBridgeA=0;
int carsBeforeBridgeB=0;
char *direction;

//Funkcja do wyświetlania statusu w postaci:
//A-5 10>>> [>> 4 >>] <<<4 6-B

void PrintStatus()
{
    printf("A-%d %d>>> [%s %d %s] <<< %d %d-B\n",carsInA, carsBeforeBridgeA,
           direction, carOnBridge, direction,carsBeforeBridgeB, carsInB);
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
        PrintStatus();
        pthread_mutex_unlock(&cs);

        ClockSleep(2000, 5000);

        //czekamy na możliwość wjazdu na most, jeśli mamy możliwość
        //od razu wjeżdzamy
        pthread_mutex_lock(&bridge);
        pthread_mutex_lock(&cs);
        carsBeforeBridgeA--;
        carOnBridge=vehicleNo;
        direction=">>";
        PrintStatus();
        pthread_mutex_unlock(&cs);
        pthread_mutex_unlock(&bridge);

        ClockSleep(1000, 3999);

        //wjeżdzamy do miasta B
        pthread_mutex_lock(&cs);
        carOnBridge = -1;
        direction = "||";
        carsInB++;
        PrintStatus();
        pthread_mutex_unlock(&cs);

        ClockSleep(4000, 7000);

        //ustawiamy się w kolejce, zwiększamy liczniki itd.
        pthread_mutex_lock(&cs);
        carsInB -- ;
        carsBeforeBridgeB ++;
        PrintStatus();
        pthread_mutex_unlock(&cs);

        ClockSleep(3000, 6000);


        //czekamy na możliwość wjazdu na most, jeśli mamy możliwość
        //od razu wjeżdzamy
        pthread_mutex_lock(&bridge);
        pthread_mutex_lock(&cs);
        carsBeforeBridgeB--;
        carOnBridge=vehicleNo;
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

    int CarNumber = atoi(argv[1]);

    //Samochody
    pthread_t car[CarNumber];

    //zmienna iteracyjna, pomaga przy tworzeniu wątków
    int i;

    if(0!= pthread_mutex_init(&cs,NULL))
    {
        printf("Błąd inicjalizacji sekcji krytycznej");
        errno=-1;
        exit(EXIT_FAILURE);
    }
    for(i = 0; i<CarNumber; i++)
    {
        if(0 != pthread_create(&car[i], NULL, CarRoutine,&i))
        {
            printf("Blad inicjalizacji %d filozofa\n",i);
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
