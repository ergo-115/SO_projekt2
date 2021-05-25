#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include <unistd.h>

/* Minimalny czas pobytu w mieście */
#define MIN_CITY_TIME 200000
/* Maksymalny czas pobytu w mieście */
#define MAX_CITY_TIME 450000
/* Czas przejazdu przez most */
#define CROSSING_TIME 100

/* Losowanie czasu, który samochód spędzi w mieście */
#define RANDOM_CITY_TIME random_range(MIN_CITY_TIME, MAX_CITY_TIME)

/* 
    Zwolnienie pamięci przy wystąpieniu błędu w programie 
*/
#define EXIT_ON_ERR \
    free(cars); \
    pthread_mutex_destroy(&stats); \
    pthread_mutex_destroy(&bridge); \
    exit(EXIT_FAILURE)

/* 
    Nałożenie blokady na mutex podany w paramtrze oraz uniemożliwienie przerwania wątku 
*/
#define lock_mutex(mutex_name) \
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL); \
    pthread_mutex_lock(&mutex_name)

/* 
    Zdjęcie blokady z mutexu podanego w parametrze oraz umożliwienie przerwania wątku
*/
#define unlock_mutex(mutex_name) \
    pthread_mutex_unlock(&mutex_name); \
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL)

/* 
    Nałożenie blokady na mutex, wypisanie statusu mostu i zdjęcie blokady z wykorzystaniem zdefiniowanych makr 
*/
#define write_status_NLOCKED() \
    lock_mutex(stats); \
    write_status(); \
    unlock_mutex(stats)

/* 
    Nałożenie blokady na mutex, wypisanie statusu mostu i zdjęcie blokady 
*/
#define write_status_LOCKED() \
    pthread_mutex_lock(&stats); \
    write_status(); \
    pthread_mutex_unlock(&stats)

/* Wartość odpowiadająca miastu A */
#define A 0
/* Wartość odpowiadająca miastu B */
#define B 1
/* Wartość odpowiadająca kierunkowi przejazdu auta z A do B */
#define DIRECTION_AB A
/* Wartość odpowiadająca kierunkowi przejazdu auta z B do A */
#define DIRECTION_BA B
/* Wartość odpowiadająca samochodowi przebywającemu w mieście */
#define STATE_CITY 0
/* Wartość odpowiadająca samochodowi przebywającemu w kolejce */
#define STATE_QUEUE 1
/* Wartość odpowiadająca samochodowi przejeżdżającemu przez most */
#define STATE_CROSSING 2

/* Funkcja wymuszająca natychmiastowe zamknięcie programu */
void force_quit(int sig);
/* Funkcja zamykająca program */
void program_quit(int sig);
/* Funkcja generująca losową wartość z przedziału*/
int random_range(int min, int max);
/* Funkcja definiująca zachowanie samochodów */
void * car_bahavior(void *threadId);
/* Funkcja wypisująca ilość samochodów w miastach, kolejkach oraz numer samochodu obecnie przejeżdżającego*/
void write_status();
/* Funkcja sprawdzająca czy podany argument składa się z cyfr*/
int isNumeric(const char *string);

/* Definicja struktury zawierającej informacje na temat samochodu*/
struct Car {
    pthread_t thread;
    int times_crossed;
    int state;
    int city;
};

/* Definicja struktury zawierającej informacje o aucie znajdującym się na moście */
struct CrossindState {
    int car_number;
    int direction;
} crossing;

// Zmienna informująca o tym czy zostało wymuszone zamknięcie programu 
int running = 1;
// Zmienna przechowująca ilość samochodów
int n_cars;

// Wskaźnik na strukturę zawierającą informacje o samochodach 
struct Car * cars;
// Inicjalizacja mutexu odpowiadającego za most
pthread_mutex_t bridge = PTHREAD_MUTEX_INITIALIZER;
// Inicjalizacja mutexu odpowiadającego za zmianę stanu samochodów
pthread_mutex_t stats = PTHREAD_MUTEX_INITIALIZER;

int main(int argc, char *argv[])
{
    // Sprawdzanie poprawności wprowadzonych parametrów
    #pragma region Input validation
    // Sprawdzanie ilości parametrów 
    if (argc != 2)
    {
        fprintf(stderr, "Invalid number of arguments. Usage: \n");
        fprintf(stderr, "\t%s <number_of_cars>\n", argv[0]);
        return EXIT_FAILURE;
    }
    // Sprawdzenie czy parametr jest liczbą
    if (!isNumeric(argv[1]))
    {
        fprintf(stderr, "Value %s is not a value for N - number of cars; number expected. Usage: \n", argv[1]);
        fprintf(stderr, "\t%s <number_of_cars>\n", argv[0]);
        return EXIT_FAILURE;
    }
    else n_cars = atoi(argv[1]);
    #pragma endregion

    srand(time(NULL));

    // Operacje na wątkach
    #pragma region Threads
    // Stworzenie tablicy struktur dla samochodów
    cars = (struct Car *) calloc(n_cars, sizeof(struct Car));
    // Ustawianie wartosci domyslnych dla samochodu przejezdzajacego przez most
    crossing.car_number = -1;
    crossing.direction = -1;
    // Tworzenie wątków
    int i;
    for (i = 0; i < n_cars; i++)
    {
        if (pthread_create(&(cars[i].thread), NULL, &car_bahavior, (void *)i))
        {
            fprintf(stderr, "Failed to create thread for car [%02d].\n", i);
            EXIT_ON_ERR;
        }
    }
    // Złapanie Ctrl+C
    signal(SIGINT, program_quit);
    // Łączenie wątków
    for (i = 0; i < n_cars; i++)
    {
        if (pthread_join(cars[i].thread, NULL))
        {
            fprintf(stderr, "Failed to join thread for car [%02d].\n", i);
            EXIT_ON_ERR;
        }
    }
    // Usuwanie mutexów
    if (pthread_mutex_destroy(&stats) != 0)
        fprintf(stderr, "Mutex stats failed to destory");
    if (pthread_mutex_destroy(&bridge) != 0)
        fprintf(stderr, "Mutex bridge failed to destory");
    #pragma endregion

    // Wypisanie podsumowania ilości przejazdów danych samochodów przez most
    for (i = 0; i < n_cars; i++)
        printf("Car [%02d] crossed the bridge [%02d] times\n", i, cars[i].times_crossed);
    
    // Zwolnienie pamięci
    free(cars);
    exit(EXIT_SUCCESS);
}

void force_quit(int sig)
{
    printf("Forcing quit...\n");
    // Usuwanie mutexów
    if (pthread_mutex_destroy(&stats) != 0)
        fprintf(stderr, "Mutex stats failed to destory");
    if (pthread_mutex_destroy(&bridge) != 0)
        fprintf(stderr, "Mutex bridge failed to destory");
    free(cars);
    exit(EXIT_SUCCESS);
}

void program_quit(int sig)
{
    printf("Quitting (Ctrl+C again to force)...\n");
    // Ustawienie wartości wymuszającej zamknięcie programu
    running = 0;
    // Odebranie sygnału do wymuszenia zamknięcia programu
    signal(sig, force_quit);
    int i;
    // Anulowanie wątków
    for (i = 0; i < n_cars; i++)
        pthread_cancel(cars[i].thread);
}

int random_range(int min, int max)
{
    int r = min, d = max - min + 1;
    r += (rand() * d) % d;
    return r;
}

void * car_bahavior(void *threadId)
{
    int car_number = (int)threadId;
    // Umożliwienie opóźnienia anulowania wątku
    pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
    // Podpisanie wskaźnika na informacje o konkretnym samochodzie
    struct Car * car = &(cars[car_number]);

    lock_mutex(stats);
    // Wyzerowanie ilości przejazdów przez most
    car->times_crossed = 0;
    // Przypisanie stanu przebywania w mieście
    car->state = STATE_CITY;
    // Losowanie, w którym mieście przebywa samochód (0 - miasto A, 1 - miasto B)
    car->city = rand() % 2;
    unlock_mutex(stats);

    // Struktura wykorzystywana do czasu czekania w mieście i przejazdu przez most
    struct timespec sleep_time;
    sleep_time.tv_sec = 0;
    sleep_time.tv_nsec = 0;

    for (;;)
    {
        // Ustawienie długości czekania w mieście
        sleep_time.tv_nsec = RANDOM_CITY_TIME;
        nanosleep(&sleep_time, NULL);

        lock_mutex(stats);
        // Przypisanie stanu czekania w kolejce
        car->state = STATE_QUEUE;
        unlock_mutex(stats);

        write_status_NLOCKED();
        lock_mutex(bridge);

        // Nałożenie blokady na mutex
        pthread_mutex_lock(&stats);
        // Przypisanie stanu przejazdu przez most
        car->state = STATE_CROSSING;
        // Zaktualizowanie stanu mostu (samochód przejeżdża)
        crossing.car_number = car_number;
        crossing.direction = car->city;
        //Zdjęcie blokady z mutexu
        pthread_mutex_unlock(&stats);

        write_status_LOCKED();

        // Ustawienie długości przejazdu przez most
        sleep_time.tv_nsec = CROSSING_TIME;
        if (running) nanosleep(&sleep_time, NULL);

        pthread_mutex_lock(&stats);
        // Zmiana miasta, w którym przebywa samochód
        car->city ^= 1;
        // Ustawienie stanu przebywania w mieście
        car->state = STATE_CITY;
        // Zaktualizowanie stanu mostu (samochód przejechał)
        crossing.car_number = -1;
        crossing.direction = -1;
        // Zwiększenie ilości przejazdów przez most danego samochodu
        if (running) car->times_crossed++;
        pthread_mutex_unlock(&stats);

        unlock_mutex(bridge);
        write_status_NLOCKED();
    }
}

void write_status()
{
    // Wyświetlenie informacji o czyszczeniu mostu po wymuszeniu zamknięcia programu
    if (!running) printf("\033[31m**CLEARING BRIDGE** | \033[0m");
    // Tablica wszystkich samochodów z podziałem na stany (w mieście A, w kolejce miasta A, przejeżdżające z A do B, w kolejce miasta B, w mieście B, przejeżdżające z B do A)
    int car_sum[3][2] = {0, 0, 0, 0, 0, 0};
    int i;
    for (i = 0; i < n_cars; i++)
        car_sum[cars[i].state][cars[i].city]++;
    // Suma wszystkich aut w programie
    int sum = car_sum[STATE_CITY][A] + car_sum[STATE_CITY][B] + car_sum[STATE_QUEUE][A] + car_sum[STATE_QUEUE][B]; 
    // Sprawdzenie czy jakiś samochód przejeżdża przez most
    if (crossing.car_number >= 0)
    {
        // Zwiększenie sumy o auto przejeżdżające przez most
        sum += 1;
        // Sprawdzenie czy samochód przejeżdża a miasta A do miasta B
        if (crossing.direction == DIRECTION_AB)
        {
            printf("A-%02d %02d>>> [>> %02d >>] <<<%02d %02d-B\t", car_sum[STATE_CITY][A], car_sum[STATE_QUEUE][A], crossing.car_number, car_sum[STATE_QUEUE][B], car_sum[STATE_CITY][B]);
        }
        else
            printf("A-%02d %02d>>> [<< %02d <<] <<<%02d %02d-B\t", car_sum[STATE_CITY][A], car_sum[STATE_QUEUE][A], crossing.car_number, car_sum[STATE_QUEUE][B], car_sum[STATE_CITY][B]);
    }
    else
        printf("A-%02d %02d>>> [        ] <<<%02d %02d-B\t", car_sum[STATE_CITY][A], car_sum[STATE_QUEUE][A], car_sum[STATE_QUEUE][B], car_sum[STATE_CITY][B]);
    // Sprawdzenie czy suma wszystkich samochodów w programie jest równa ilości wprowadzonej w parametrze 
    if (sum == n_cars)
        printf("All cars: %02d\n", sum);
    else
        printf("All cars: \033[31m %02d \033[0m \n", sum);
}

int isNumeric(const char *string)
{
    for (; *string != '\0'; string++)
        if (!isdigit(*string))
            return 0;
    return 1;
}