#include "data.h"
#include "init.h"

//Stała - gdy samochód jest w mieście
#define CITY 0
//Stała - gdy samochód czeka w kolejce
#define QUEUE 1
//Stała, gdy samochod przejeżdza przez most
#define ON_BRIDGE 2
//Stała - miasto A
#define A 0
//Stała - miasto B
#define B 1
//Inicjalizacja mutexów oraz zmiennych warunkowych
int initializeStart();
//Kasowanie mutexów oraz zmiennych warunkowych
void closeThreads();
//Przechwycenie sygnału by zakończyć działanie programu
void closeProgram(int sig);
//Ponowne przechwycenie sygnału w razie problemów z normalnym zakończeniem prorgamu
void forceCloseProgram(int sig);
//Funkcja definiująca zachowanie wątku(samochodu)
void* threadRoutine(void *threadId);
//Funkcja wypisująca obecną sytuację w programie, w formacie jak było podane
void currentStatus();
//Inicjalizacja struktur
struct car* cars;
struct bridgeInfo strBridgeInfo;

//Zmienna pilnująca, czy program dalej działa
int isActive = 1;
//Zmienna przechowująca ilość samochodów
int amountOfCars;

//Inicjalizacja mutexu oraz zmiennej warunkowej odpowiadających za most
pthread_mutex_t bridge;
pthread_cond_t condBridge;
//Inicjalizacja mutexu oraz zmiennej warunkowej odpowiadających za zmianę stanu samochodów
pthread_mutex_t stats;
pthread_cond_t condStats;

int main(int argc, char *argv[]){
	//sprawdzanie podanego argumentu
	if (!validate(argc, argv)){
		printf("Uruchomienie programu: ./main <liczbaCałkowitaDodatnia>");
		return EXIT_FAILURE;
 	}
 	//Ilość utworzonych wątków itd. (samochodów)
    	amountOfCars = atoi(argv[1]);
	//Inicjalizacja mutexow i zmiennych warunkowych
	if(!initializeStart()){
		return EXIT_FAILURE;
	}
	
	signal(SIGINT, closeProgram);
	
	//Inicjalizacja struktury, przydzielenie pamięci
	cars = (struct car*) calloc(amountOfCars, sizeof(struct car));
	//Ustawienie wartosci domyslnych dla samochodu przejezdzajacego przez most
	strBridgeInfo = resetBridge(strBridgeInfo);
	//Tworzenie wątków
	int i = 0;
	for (i = 0; i < amountOfCars; i++){
		if (pthread_create(&(cars[i].thread), NULL, &threadRoutine, (void *)(intptr_t)i)){
			printf("Thread create failed for id %d\n", i);
			closeThreads();
			free(cars);
		}
	}
	//Łączenie wątków
	for (i = 0; i < amountOfCars; i++){
		if (pthread_join(cars[i].thread, NULL)){
			printf("Thread join failed for id %d\n", i);
			closeThreads();
			free(cars);
		}
	}

	//Kasowanie mutexów i zmiennych warunkowych
	closeThreads();

	printf("\n");
    	//Podsumowanie tzn ilość przejazdów danego samochodu przez most
    	/*
	for (i = 0; i < amountOfCars; i++)
		printf("Car \e[0;32m%d\e[0m crossed the bridge \e[0;32m%d\e[0m times\n", i, cars[i].timesCrossed);
	*/
	//Zwolnienie pamięci
	free(cars);
	printf("\nProgram closed\n");
	exit(EXIT_SUCCESS);
}

int initializeStart(){
	int status = 1;
	if(pthread_mutex_init(&bridge, NULL)){
		status = 0;
	}

	if(pthread_mutex_init(&stats, NULL)){
		status = 0;
	}
	if(pthread_cond_init(&condBridge, NULL)){
		status = 0;
	}
	if(pthread_cond_init(&condStats, NULL)){
		status = 0;
	}
	return status;
}
void closeThreads(){
	if (pthread_cond_destroy(&condStats) != 0)
		printf("\e[0;31mCond destroy for condStats failed\e[0m\n");
	if (pthread_cond_destroy(&condBridge) != 0)
		printf("\e[0;31mCond destroy for condBridge failed\e[0m\n");
	if (pthread_mutex_destroy(&stats) != 0)
		printf("\e[0;31mMutex destroy for stats failed\e[0m\n");
	if (pthread_mutex_destroy(&bridge) != 0)
		printf("\e[0;31mMutex destroy for bridge failed\e[0m\n");
}
void closeProgram(int sig){
	printf("\nStopping work\n");
	int i = 0;
	//Zmiana wartości, by zakończyć program
	isActive = 0;
	//Anulowanie wątków
	for (i = 0; i < amountOfCars; i++)
		pthread_cancel(cars[i].thread);
	//Kolejne odebranie sygnału by wymusić zamknięcie programu
	signal(sig, forceCloseProgram);
}

void forceCloseProgram(int sig){
	printf("\nProgram shutting down\n");
	closeThreads();
	free(cars);
	printf("Program closed\n");
	exit(EXIT_SUCCESS);
}




void* threadRoutine(void* threadId){
	//numer wątku(samochodu)
	int carNumber = (int)(intptr_t)threadId;
	int writing = 0;
	//Umożliwienie opóźnienia anulowania wątku
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    	//Wskaźnik na konkretny samochód
	struct car* car = &(cars[carNumber]);
	//założenie blokady na mutex, uniemożliwienie przerwania wątku
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	pthread_mutex_lock(&stats);
	//zawiesza proces, by móc wypisać obecny stan
	while (writing)
		pthread_cond_wait(&condStats, &stats);
	writing = 1;
	//Początkowa ilosć przejazdów przez most
	car->timesCrossed = 0;
	//Przypisanie stanu samochodu na miasto
	car->state = CITY;
	//Losowanie, w którym mieście będzie przebywał samochód (0 - miasto A, 1 - miasto B)
	car->city = selectRandomCity();
	//wznawia proces po wypisaniu statusu
	writing = 0;
	pthread_cond_signal(&condStats);
	//zdjęcie blokady z mutexa, umożliwienie przerwania wątku
	pthread_mutex_unlock(&stats);
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

	//Struktura timespec, przechowuje czas czekania w mieście i przejazdu przez most
	struct timespec tvTime;
	tvTime.tv_sec = 0;
	tvTime.tv_nsec = 0;

	while(1){
		//Samochód czeka w mieście, wartość pseudolosowa
		tvTime.tv_nsec = randTime();
		nanosleep(&tvTime, NULL);
		//założenie blokady na mutex, uniemożliwienie przerwania wątku
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		pthread_mutex_lock(&stats);
		//zawiesza proces, by móc wypisać obecny stan
		while (writing)
			pthread_cond_wait(&condStats, &stats);
		writing = 1;
		//Samochód czeka w kolejce
		car->state = QUEUE;
		currentStatus();
		//wznawia proces po wypisaniu statusu
		writing = 0;
		pthread_cond_signal(&condStats);
		//zdjęcie blokady z mutexa, umożliwienie przerwania wątku
		pthread_mutex_unlock(&stats);
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

		//założenie blokady na mutex bridge, uniemożliwienie przerwania wątku
		pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		pthread_mutex_lock(&bridge);
		//Zawieszenie wykonania procesu dopóki most nie będzie pusty
		while (strBridgeInfo.carNumber != -1)
			pthread_cond_wait(&condBridge, &bridge);
        
		//Nałożenie blokady na mutex
		pthread_mutex_lock(&stats);
		//zawiesza proces, by móc wypisać obecny stan
		while (writing)
			pthread_cond_wait(&condStats, &stats);
		writing = 1;
		//Samochód przejeżdza przez most
		car->state = ON_BRIDGE;
		//Zaktualizowanie stanu mostu (samochód przejeżdża)
		strBridgeInfo.carNumber = carNumber;
		strBridgeInfo.direction = car->city;
		currentStatus();
		//wznawia proces po wypisaniu statusu
		writing = 0;
		pthread_cond_signal(&condStats);
		//Zdjęcie blokady z mutexu
		pthread_mutex_unlock(&stats);

		//Czas przejazdu przez most
		tvTime.tv_nsec = randTimeOnBridge();
		if (isActive) 
			nanosleep(&tvTime, NULL);

		pthread_mutex_lock(&stats);
		//zawiesza proces, by móc wypisać obecny stan
		while (writing)
			pthread_cond_wait(&condStats, &stats);
		writing = 1;
		//Zmiana miasta, w którym przebywa samochód, 0 staje się 1, 1 staje się 0
        	car->state = CITY;
		car->city = changeCity(car->city);
		
		//Zaktualizowanie stanu mostu (samochód przejechał)
		strBridgeInfo = resetBridge(strBridgeInfo);
		//Informacja o kolejnym przejezdzie samochodu
		if (isActive) 
			car->timesCrossed++;
		currentStatus();
		//wznawia proces po wypisaniu statusu
		writing = 0;
		pthread_cond_signal(&condStats);
		pthread_mutex_unlock(&stats);

		//Wznowienie wykonywania procesu
		pthread_cond_signal(&condBridge);
		//zdjęcie blokady z mutexa, umożliwienie przerwania wątku
		pthread_mutex_unlock(&bridge);
		pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	}
}

void currentStatus(){
	//Wyświetlenie informacji o czyszczeniu mostu po wymuszeniu zamknięcia programu
	if (!isActive) 
		printf("\e[0;36mClearing...\t\e[0m");
	//Tablica wszystkich samochodów z podziałem na stany 
	//Kolejno: w mieście A, w kolejce miasta A, przejeżdżające z A do B,
	//w kolejce miasta B, w mieście B, przejeżdżające z B do A)
	int carsTotal[3][2] = {0, 0, 0, 0, 0, 0};
	int i = 0;
	for (i = 0; i < amountOfCars; i++)
		carsTotal[cars[i].state][cars[i].city]++;
	//Czy jakiś samochód przejeżdża obecnie przez most
	if (strBridgeInfo.carNumber >= 0){
		//Sprawdzenie dokąd zmierza samochód(w którą stronę)
        	if (strBridgeInfo.direction == A){
            	printf("A-%02d \e[0;32m%02d\e[0m>>> [>> \e[0;32m %02d \e[0m >>] <<<\e[0;32m%02d \e[0m%02d-B\n", carsTotal[CITY][A], carsTotal[QUEUE][A], 
            	strBridgeInfo.carNumber, carsTotal[QUEUE][B], carsTotal[CITY][B]);
        	}else{
            	printf("A-%02d \e[0;32m%02d\e[0m>>> [<< \e[0;32m %02d \e[0m <<] <<<\e[0;32m%02d \e[0m%02d-B\n", carsTotal[CITY][A], carsTotal[QUEUE][A], 
            	strBridgeInfo.carNumber, carsTotal[QUEUE][B], carsTotal[CITY][B]);
            	}
	}else{
        	printf("A-%02d \e[0;32m%02d\e[0m>>> [          ] <<<\e[0;32m%02d \e[0m%02d-B\n", carsTotal[CITY][A], carsTotal[QUEUE][A], 
        	carsTotal[QUEUE][B], carsTotal[CITY][B]);
        }
}

