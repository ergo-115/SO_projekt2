#include "data.h"

//costam

void CityTourSleep(int maxTime)
{
    //pobieramy aktualny czas systemowy
    time_t tt;

    //podstawiamy za ziarno czas
    int seed = time(&tt);

    //ustawiamy, by ziarno losowania było liczbą
    //czasu
    srand(seed);

    //śpimy nie więcej, niż to wynosi maxTime
    usleep(rand()%maxTime);
    return;
}

int lenHelper(unsigned liczba) {
	if(liczba == 0) return 1;
    float helper = (float)liczba;
    int length=0;
    while(helper>=1)
    {
        length ++;
        helper /= 10;
    }
    return length;
}

bool ValidateData(int argc,char *argv[])
{
    //liczba argumentów większa niż 2, oznacza
    //że ktoś podał więcej argumentów niż można
    //program przyjmuje tylko ilość samochodów
    if(argc > 2)
    {
        printf("Liczba argumentów jest zbyt duża, dozwolona tylko liczba samochodów\n");
        errno=-1;
        return false;
    }

    //sprawdzamy, jaka liczba jest w argumencie
    int liczba = atoi(argv[1]);

    

    //sprawdzamy, czy była to liczba całkowita
    //czy jakiś ciąg znaków
    if(lenHelper(liczba) != strlen(argv[1]))
    {
        printf("Podana liczba samochodów nie jest poprawną liczbą!\n");
        errno=-1;
        return false;;
    }

    //sprawdzamy, czy liczba nie jest zerem
    //muszą być jakieś samochody przecież
    if(liczba == 0)
    {
        printf("Liczba samochodów nie może wynosić 0!");
        errno=-1;
        return false;
    }

    return true;
}