#include "data.h"

int iter = 1;
//tworzymy most jako zmienną globalną, ponieważ ma być
//dostępny dla każdego samochodu
pthread_mutex_t bridge;

void *CarRoutine(void *args)
{
    int vehicleNo = iter;
    iter++;
	while(1)
	{
		printf("Samochód %d załatwia swoje sprawy w mieście A\n",vehicleNo);
		ClockSleep(10000);
		printf("Samochód %d zatankował i wyjeżdza do miasta B\n",vehicleNo);
		
		//przejazd przez most, który zajmuje do 5 s czasu
		//most jest wtedy zablokowany i nie przejezdny
		pthread_mutex_lock(&bridge);
		printf("Samochód %d przejeżdza przez most\n",vehicleNo);
		ClockSleep(5000);
		pthread_mutex_unlock(&bridge);
		printf("Samochód %d przejechał przez most\n",vehicleNo);

		printf("Samochód %d załatwia swoje sprawy na mieście B\n",vehicleNo);
		ClockSleep(10000);
		printf("Samochód %d zatankował i wyjeżdza do miasta A\n",vehicleNo);

		//przejazd przez most, który zajmuje do 5 s czasu
		//most jest wtedy zablokowany i nie przejezdny
		pthread_mutex_lock(&bridge);
		printf("Samochód %d przejeżdza przez most\n",vehicleNo);
		ClockSleep(5000);
		pthread_mutex_unlock(&bridge);
		printf("Samochód %d przejechał przez most\n",vehicleNo);
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

    if(0 != pthread_mutex_init(&bridge,NULL))
    {
        printf("Bląd inicjalizacji mostu!\n");
        errno=-1;
        return -1;
    }

    for(i = 0; i<CarNumber; i++)
    {
        if(0 != pthread_create(&car[i], NULL, CarRoutine,&i))
        {
            printf("Blad inicjalizacji %d filozofa\n",i);
            errno=1;
            return -1;
        }
    }

	for(i=0;i<CarNumber;i++)
    {
        pthread_join(car[i], NULL);
    }
    return 0;
}
