#include "data.h"
#include "init.h"

#define lockMutex(varMutex) \
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL); \
    pthread_mutex_lock(&varMutex)

/* 
    Zdjęcie blokady z mutexu podanego w parametrze oraz umożliwienie przerwania wątku
*/
#define unlockMutex(varMutex) \
    pthread_mutex_unlock(&varMutex); \
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL)

/* 
    Zawieszenie wykonania procesu przed rozpoczęciem wypisywania statusu 
*/
#define startWriting() \
    while (writing) \
        pthread_cond_wait(&condStats, &stats);\
    writing = 1

/* 
    Wnowienie wykonania procesu po zakończeniu wypisywania statusu 
*/
#define stopWriting() \
    writing = 0;\
    pthread_cond_signal(&condStats)

/* Wartość odpowiadająca miastu A */
#define A 0
/* Wartość odpowiadająca miastu B */
#define B 1
/* Wartość odpowiadająca samochodowi przebywającemu w mieście */
#define STATE_CITY 0
/* Wartość odpowiadająca samochodowi przebywającemu w kolejce */
#define STATE_QUEUE 1
/* Wartość odpowiadająca samochodowi przejeżdżającemu przez most */
#define STATE_CROSSING 2
int initializeStart();
void createJoinThreads();
void closeThreads();
/* Funkcja wymuszająca natychmiastowe zamknięcie programu */
void forceCloseProgram(int sig);
/* Funkcja zamykająca program */
void closeProgram(int sig);
/* Funkcja generująca losową wartość z przedziału*/
int random_range(int min, int max);
/* Funkcja definiująca zachowanie samochodów */
void* threadRoutine(void *threadId);
/* Funkcja wypisująca ilość samochodów w miastach, kolejkach oraz numer samochodu obecnie przejeżdżającego*/
void currentStatus();

struct car* cars;

struct bridgeInfo strBridgeInfo;
// Zmienna informująca o tym czy zostało wymuszone zamknięcie programu 
int isActive = 1;
// Zmienna przechowująca ilość samochodów
int amountOfCars;

// Inicjalizacja mutexu odpowiadającego za most
pthread_mutex_t bridge; //= PTHREAD_MUTEX_INITIALIZER;
// Inicjalizacja mutexu odpowiadającego za zmianę stanu samochodów
pthread_mutex_t stats; //= PTHREAD_MUTEX_INITIALIZER;
// Inicjalizacja zmiennej warunkowej odpowiedzialnej za most
pthread_cond_t condBridge;// = PTHREAD_COND_INITIALIZER;
// Inicjalizacja zmiennej warunkowej odpowiedzianej za zmianę stanu samochodów
pthread_cond_t condStats; //= PTHREAD_COND_INITIALIZER;

int main(int argc, char *argv[])
{
    // Sprawdzanie poprawności wprowadzonych parametrów
    // Sprawdzanie ilości parametrów

    // Sprawdzenie czy parametr jest liczbą
	if (!validate(argc, argv)){
		printf("Uruchomienie programu: ./main <liczbaCałkowitaDodatnia>");
		//np ./main 5
		return EXIT_FAILURE;
 	}
 	
    	amountOfCars = atoi(argv[1]);

	if(!initializeStart()){
		return EXIT_FAILURE;
	}
	
	signal(SIGINT, closeProgram);
    // Operacje na wątkach
    // Stworzenie tablicy struktur dla samochodów
	cars = (struct car*) calloc(amountOfCars, sizeof(struct car));
    // Ustawianie wartosci domyslnych dla samochodu przejezdzajacego przez most
	strBridgeInfo = resetBridge(strBridgeInfo);
    // Tworzenie wątków
	int i = 0;
	for (i = 0; i < amountOfCars; i++){
		if (pthread_create(&(cars[i].thread), NULL, &threadRoutine, (void *)(intptr_t)i)){
			printf("Thread create failed for id %d\n", i);
			closeThreads();
			free(cars);
		}
	}
    // Łączenie wątków
	for (i = 0; i < amountOfCars; i++){
		if (pthread_join(cars[i].thread, NULL)){
			printf("Thread join failed for id %d\n", i);
			closeThreads();
			free(cars);
		}
	}

    // Usuwanie mutexów i zmiennych warunkowych
	closeThreads();

	printf("\n");
    // Wypisanie podsumowania ilości przejazdów danych samochodów przez most
	for (i = 0; i < amountOfCars; i++)
		printf("Car \e[0;32m%d\e[0m crossed the bridge \e[0;32m%d\e[0m times\n", i, cars[i].timesCrossed);
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
    // Ustawienie wartości wymuszającej zamknięcie programu
	isActive = i;
    // Odebranie sygnału do wymuszenia zamknięcia programu
    // Anulowanie wątków
	for (i = 0; i < amountOfCars; i++)
		pthread_cancel(cars[i].thread);
	signal(sig, forceCloseProgram);
}

void forceCloseProgram(int sig){
	printf("\nProgram shutting down\n");
    // Usuwanie mutexów i zmiennych warunkowych
	closeThreads();
	free(cars);
	printf("Program closed\n");
	exit(EXIT_SUCCESS);
}




void* threadRoutine(void* threadId){
	int carNumber = (int)(intptr_t)threadId;
    // Zmienna sprawdzająca czy wypisywany jest status mostu
    
    // Umożliwienie opóźnienia anulowania wątku
	pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    // Podpisanie wskaźnika na informacje o konkretnym samochodzie
	struct car* car = &(cars[carNumber]);
	int writing = 0;
	lockMutex(stats);
	startWriting();
    // Wyzerowanie ilości przejazdów przez most
	car->timesCrossed = 0;
    // Przypisanie stanu przebywania w mieście
	car->state = STATE_CITY;
    // Losowanie, w którym mieście przebywa samochód (0 - miasto A, 1 - miasto B)
	car->city = selectRandomCity();
	stopWriting();
	unlockMutex(stats);

    // Struktura wykorzystywana do czasu czekania w mieście i przejazdu przez most
	struct timespec tvTime;
	tvTime.tv_sec = 0;
	tvTime.tv_nsec = 0;

	while(1){
        // Ustawienie długości czekania w mieście
		tvTime.tv_nsec = randTime();
		nanosleep(&tvTime, NULL);
		lockMutex(stats);
		startWriting();
        // Przypisanie stanu czekania w kolejce
		car->state = STATE_QUEUE;
		currentStatus();
		stopWriting();
		unlockMutex(stats);

		lockMutex(bridge);
        // Zawieszenie wykonania procesu dopóki most nie będzie pusty
		while (strBridgeInfo.carNumber != -1)
			pthread_cond_wait(&condBridge, &bridge);
        
        // Nałożenie blokady na mutex
		pthread_mutex_lock(&stats);
		startWriting();
        // Ustawienie stanu wypisywania statusu mostu
		writing = 1;
        // Przypisanie stanu przejazdu przez most
		car->state = STATE_CROSSING;
        // Zaktualizowanie stanu mostu (samochód przejeżdża)
		strBridgeInfo.carNumber = carNumber;
		strBridgeInfo.direction = car->city;
		currentStatus();
		stopWriting();
        //Zdjęcie blokady z mutexu
		pthread_mutex_unlock(&stats);

        // Ustawienie długości przejazdu przez most
		tvTime.tv_nsec = 100;
		if (isActive) 
			nanosleep(&tvTime, NULL);

		pthread_mutex_lock(&stats);
		startWriting();
        // Zmiana miasta, w którym przebywa samochód
        	car->state = STATE_CITY;
		car->city = changeCity(car->city);
		
        // Zaktualizowanie stanu mostu (samochód przejechał)
		strBridgeInfo = resetBridge(strBridgeInfo);
        // Zwiększenie ilości przejazdów przez most danego samochodu
		if (isActive) 
			car->timesCrossed++;
		currentStatus();
		stopWriting();
		pthread_mutex_unlock(&stats);

        // Wznowienie wykonywania procesu
		pthread_cond_signal(&condBridge);
		unlockMutex(bridge);
	}
}

void currentStatus(){
    // Wyświetlenie informacji o czyszczeniu mostu po wymuszeniu zamknięcia programu
	if (!isActive) 
		printf("\e[0;36m\nClearing...\n\e[0m");
    // Tablica wszystkich samochodów z podziałem na stany (w mieście A, w kolejce miasta A, przejeżdżające z A do B, w kolejce miasta B, w mieście B, przejeżdżające z B do A)
	int car_sum[3][2] = {0, 0, 0, 0, 0, 0};
	int i = 0;
	for (i = 0; i < amountOfCars; i++)
		car_sum[cars[i].state][cars[i].city]++;
    // Suma wszystkich aut w programie
	int sum = car_sum[STATE_CITY][A] + car_sum[STATE_CITY][B] + 
        car_sum[STATE_QUEUE][A] + car_sum[STATE_QUEUE][B]; 
    // Sprawdzenie czy jakiś samochód przejeżdża przez most
	if (strBridgeInfo.carNumber >= 0){
        // Zwiększenie sumy o auto przejeżdżające przez most
        	sum += 1;
        // Sprawdzenie czy samochód przejeżdża a miasta A do miasta B
        	if (strBridgeInfo.direction == A){
            	printf("A-%02d \e[0;32m%02d\e[0m>>> [>> \e[0;32m %02d \e[0m >>] <<<\e[0;32m%02d \e[0m%02d-B\n", car_sum[STATE_CITY][A], car_sum[STATE_QUEUE][A], 
            	strBridgeInfo.carNumber, car_sum[STATE_QUEUE][B], car_sum[STATE_CITY][B]);
        	}else{
            	printf("A-%02d \e[0;32m%02d\e[0m>>> [<< \e[0;32m %02d \e[0m <<] <<<\e[0;32m%02d \e[0m%02d-B\n", car_sum[STATE_CITY][A], car_sum[STATE_QUEUE][A], 
            	strBridgeInfo.carNumber, car_sum[STATE_QUEUE][B], car_sum[STATE_CITY][B]);
            	}
	}else{
        	printf("A-%02d \e[0;32m%02d\e[0m>>> [          ] <<<\e[0;32m%02d \e[0m%02d-B\n", car_sum[STATE_CITY][A], car_sum[STATE_QUEUE][A], 
        	car_sum[STATE_QUEUE][B], car_sum[STATE_CITY][B]);
        }
    // Sprawdzenie czy suma wszystkich samochodów w programie jest równa ilości wprowadzonej w parametrze 

}

