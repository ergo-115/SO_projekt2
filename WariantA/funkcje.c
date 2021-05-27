#include "data.h"


void ClockSleep(int minTime,int maxTime)
{
    //get system time
    time_t tt;

    //set seed to time value
    int seed = time(&tt);

    //set seed for rand() to time value
    srand(seed);

    //max sleep time is maxTime
    int sleepingTime=rand()%maxTime;

    //if time is less than minTime, we make random number again
    while(sleepingTime<minTime)
    {
        sleepingTime=rand()%maxTime;
    }


    usleep(sleepingTime);
    return;
}


/*lenHelper returns length of number for example for 100 it would be 3, for 1 it would be 1 */
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



//Function for walidation, returns true if everything is ok,
//returns false if validation is not succesfull
bool ValidateData(int argc,char *argv[])
{
    
    if(argc > 2)
    {
        printf("Number of arguments exceeds max of 2!\n");
        errno=-1;
        return false;
    }

    int carCounter = atoi(argv[1]);

    if(carCounter < 0)
    {
        printf("Car number cannot be negative!\n");
        errno=-1;
        return false;
    }

    //check itnegrity of number wheather it's number or some string
    if(lenHelper(carCounter) != strlen(argv[1]))
    {
        printf("Given number is not valid!\n");
        errno=-1;
        return false;;
    }

    //number of cars cannot equal 0!
    if(carCounter == 0)
    {
        printf("carCounter cannot be 0!\n");
        errno=-1;
        return false;
    }

    return true;
}


