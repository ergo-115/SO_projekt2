#include "data.h"


void ClockSleep(int minTime,int maxTime)
{
    //get system time
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


