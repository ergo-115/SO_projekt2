#include "data.h"

int iter = 1;

void *CarRoutine(void *args)
{
	int vehicleNo = iter;
	iter++;
}

//tworzymy most jako zmienną globalną, ponieważ ma być
//dostępny dla każdego samochodu
pthread_mutex_t bridge;

int main(int argc, char* argv[])
{
	if(ValidateData(argc,argv) == false)
	{
		printf("Błąd danych wejściowych, anulowanie");
		exit(EXIT_FAILURE);
	}

	int CarNumber = atoi(argv[1]);
	return 0;
}
