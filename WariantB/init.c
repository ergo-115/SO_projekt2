#include "data.h"


//costam
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
	while(*string != '\0'){
        	if (!isdigit(*string))
            		return 0;
		string++;
	}
	return 1;
}

struct bridgeInfo resetBridge(bridgeInfo data){
	data.carNumber = -1;
	data.direction = -1;
	return data;
}

int changeCity(int x){
	return x == 1 ? 0 : 1;
}
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
