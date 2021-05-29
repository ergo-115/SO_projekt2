#include "data.h"


//walidacja
int validate(int argc, char* argv[]){
	if(argc != 2){
		//printf("Niepoprawna ilość argumentów\n");
		return 0;
	}
	char* string = argv[1];
	if(string[0] == '0' || string[0] == '-') {
		//printf("Wymagana liczba całkowita większa od zera\n");
		return 0;
	}
	//sprawdzenie każdego znaku w argumencie 1 czy na pewno jest liczbą
	while(*string != '\0'){
        	if (!isdigit(*string))
            		return 0;
		string++;
	}
	return 1;
}
//Ustawienie stanu na moście
struct bridgeInfo resetBridge(bridgeInfo data){
	data.carNumber = -1;
	data.direction = -1;
	return data;
}
//Zmiana miasta w którym jest samochód
int changeCity(int x){
	return x == 1 ? 0 : 1;
}
//funkcje do losowania
int selectRandomCity(){
	srand(time(NULL));
	return rand()%2;
}
int randTime(){
	srand(time(NULL));
	int rnd = 0;
	rnd = (rand() * 654);
	if(rnd <0) rnd *= (-1);
	rnd %= 400000;
	return rnd;
}
int randTimeOnBridge(){
	srand(time(NULL));
	int time = 100;
	time = 75 + (rand() % 100);
	return time;
}
