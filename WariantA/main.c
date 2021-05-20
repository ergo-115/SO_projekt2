#include "data.h"

int iter = 1;

void *CarRoutine(void *args)
{
	int vehicleNo = iter;
	iter++;
}

int main(int argc, char* argv[])
{
	if(ValidateData(argc,argv) == false)
	{
		printf("Błąd danych wejściowych, anulowanie");
		exit(EXIT_FAILURE);
	}
	return 0;
}
