//
// Created by Bernard Yuan on 2016-08-02.
//

#include "ferry.h"

//the mutex of the ferry status
pthread_mutex_t mtxFerryStatus;
int ferryStatus;

//mutex and semaphore for waiting truck
pthread_mutex_t mtxNumTruckWait;
sem_t semNumTruckWait;
int numTruckWait;
sem_t semTruckWait;

// mutex and semaphore for the late truck
pthread_mutex_t mtxNumTruckLate;
sem_t semNumTruckLate;
int numTruckLate;
sem_t semTruckLate;

// mutex and semaphore for the waiting cars
pthread_mutex_t mtxNumCarWait;
sem_t semNumCarWait;
int numCarWait;
sem_t semCarWait;

// mutex and semaphore for the late cars
pthread_mutex_t mtxNumCarLate;
sem_t semNumCarLate;
int numCarLate;
sem_t semCarLate;

// mutex for the termination flag
pthread_mutex_t mtxTerminate;
int flagTerminate;

sem_t semTruckSwitch; // switch a truck into waiting queue from late queue
sem_t semCarSwitch;   // switch a car into waiting queue from late queue
sem_t semTruckBoard;  // truck on board semaphore
sem_t semCarBoard;    // car on board semaphore
sem_t semTruckUnload; // truck unload semaphore
sem_t semCarUnload;   // car unload semaphore
sem_t semTruckLeft;   // truck leave semaphore
sem_t semCarLeft;     // car leave semaphore

// function wrapper for initializing mutexes
void initMutex(pthread_mutex_t *mtx, const pthread_mutexattr_t *attr) {
    if (pthread_mutex_init(mtx, attr) == 0) printf("Initializing mutex success\n");
    else {
        printf("Initializing mutex failure\n");
//        releaseResource();
        exit(1);
    }
}

// function wrapper for removing mutexes
void removeMutex(pthread_mutex_t *mtx) {
    if (pthread_mutex_destroy(mtx) == 0) printf("Destroying mutex success\n");
    else {
        printf("Destroying mutex failure\n");
    }
}

// function wrapper for initializing semaphores
void initSem(sem_t *sem, int share, unsigned int value) {
    if(sem_init(sem, share, value)==-1) {
        printf("Initializing semaphore error\n");
    }
    else {
        printf("Initializeing semaphore success\n");
    }
}

// function wrapper for removing semaphores
void removeSem(sem_t *sem) {
    if(sem_destroy(sem)==-1) {
        printf("Destroying semaphore error\n");
    }
    else printf("Destroying semaphores success\n");
}

// release all the resources
void releaseResource() {
    removeMutex(&mtxFerryStatus);

    removeMutex(&mtxNumTruckWait);
    removeSem(&semNumTruckWait);
    removeSem(&semTruckWait);

    removeMutex(&mtxNumTruckLate);
    removeSem(&semNumTruckLate);
    removeSem(&semTruckLate);

    removeMutex(&mtxNumCarWait);
    removeSem(&semNumCarWait);
    removeSem(&semCarWait);

    removeMutex(&mtxNumCarLate);
    removeSem(&semNumCarLate);
    removeSem(&semCarLate);

    removeMutex(&mtxTerminate);

    removeSem(&semTruckSwitch);
    removeSem(&semCarSwitch);
    removeSem(&semTruckBoard);
    removeSem(&semCarBoard);
    removeSem(&semTruckUnload);
    removeSem(&semCarUnload);
    removeSem(&semTruckLeft);
    removeSem(&semCarLeft);
}
// initialing all mutexes and semaphores
void init() {
    initMutex(&mtxFerryStatus, NULL);
    initMutex(&mtxNumTruckWait, NULL);
    initSem(&semNumTruckWait, 1, 0);
    initSem(&semTruckWait, 1, 0);

    initMutex(&mtxNumTruckLate, NULL);
    initSem(&semNumTruckLate, 1, 0);
    initSem(&semTruckLate,1,0);

    initMutex(&mtxNumCarWait, NULL);
    initSem(&semNumCarWait, 1, 0);
    initSem(&semCarWait, 1, 0);

    initMutex(&mtxNumCarLate, NULL);
    initSem(&semNumCarLate, 1, 0);
    initSem(&semCarLate, 1, 0);

    initMutex(&mtxTerminate, NULL);

    initSem(&semTruckSwitch, 1, 0);
    initSem(&semCarSwitch, 1, 0);
    initSem(&semTruckBoard, 1, 0);
    initSem(&semCarBoard, 1, 0);
    initSem(&semTruckUnload, 1, 0);
    initSem(&semCarUnload, 1, 0);
    initSem(&semTruckLeft, 1, 0);
    initSem(&semCarLeft, 1, 0);
}

// sailing function
void sail() {
    pthread_mutex_lock(&mtxFerryStatus);
    ferryStatus = FERRY_SAILING;
    pthread_mutex_unlock(&mtxFerryStatus);
    printf("Sailing starts\n");
    usleep(SAIL_TIME);
    printf("Sailing ends\n");
}

// routine of the captain
void *captain(void *arg) {
    int load = 0;
    while (load < MAX_LOAD) {
        // change the status to loading
        pthread_mutex_lock(&mtxFerryStatus);
        ferryStatus = FERRY_LOADING;
        pthread_mutex_unlock(&mtxFerryStatus);
        printf("===============Ferry starts loading===============\n");

        int truckOnFerry = 0;
        int carOnFerry = 0;
        int spotsOnFerry = 0;
        // check number of trucks in the waiting queue
        while (truckOnFerry < MAX_TRUCK && spotsOnFerry <= SIZE_FERRY - 2) {
            pthread_mutex_lock(&mtxNumTruckWait);
            if (numTruckWait > 0) {
                sem_wait(&semNumTruckWait);
                numTruckWait--;
                truckOnFerry++;
                spotsOnFerry += SIZE_TRUCK;
                pthread_mutex_unlock(&mtxNumTruckWait);
                sem_post(&semTruckWait);
                printf("CAPTAINCAPTAINCAPTAIN    Load one truck from the waiting queue now %d trucks and %d cars loaded\n", truckOnFerry,
                       carOnFerry);

            }
            else {
                pthread_mutex_unlock(&mtxNumTruckWait);
                break;
            }
        }

        // check number of cars in the waiting queue
        while (spotsOnFerry < SIZE_FERRY) {
            pthread_mutex_lock(&mtxNumCarWait);
            if (numCarWait > 0) {
                sem_wait(&semNumCarWait);
                numCarWait--;
                carOnFerry++;
                spotsOnFerry += SIZE_CAR;
                pthread_mutex_unlock(&mtxNumCarWait);
                sem_post(&semCarWait);
                printf("CAPTAINCAPTAINCAPTAIN    Load one car from the waiting queue now %d trucks and %d cars loaded\n", truckOnFerry,
                       carOnFerry);
            }
            else {
                pthread_mutex_unlock(&mtxNumCarWait);
                break;
            }
        }

        // load vehicles from late arrival
        while (spotsOnFerry < SIZE_FERRY) {
            // load trucks from late arrival
            if (spotsOnFerry <= SIZE_FERRY - 2 && truckOnFerry < MAX_TRUCK) {
                pthread_mutex_lock(&mtxNumTruckLate);
                if (numTruckLate > 0) {
                    sem_wait(&semNumTruckLate);
                    numTruckLate--;
                    truckOnFerry++;
                    spotsOnFerry += SIZE_TRUCK;
                    sem_post(&semTruckLate);
                    printf("CAPTAINCAPTAINCAPTAIN    Load one truck from the late queue, now %d trucks and %d cars loaded\n", truckOnFerry,
                           carOnFerry);
                }
                pthread_mutex_unlock(&mtxNumTruckLate);
            }
            // load cars from late arrival
            if (spotsOnFerry < SIZE_FERRY) {
                pthread_mutex_lock(&mtxNumCarLate);
                if (numCarLate > 0) {
                    sem_wait(&semNumCarLate);
                    numCarLate--;
                    carOnFerry++;
                    spotsOnFerry += SIZE_CAR;
                    sem_post(&semCarLate);
                    printf("CAPTAINCAPTAINCAPTAIN    Load one car from the late queue, now %d trucks and %d cars loaded\n", truckOnFerry,
                           carOnFerry);
                }
                pthread_mutex_unlock(&mtxNumCarLate);
            }
        }

        // waiting till all vehicles are really boarded
        int i;
        int truckBoarded = 0;
        int carBoarded = 0;
        for (i = 0; i < truckOnFerry; i++) {
            sem_wait(&semTruckBoard);
            truckBoarded++;
            printf("CAPTAINCAPTAINCAPTAIN    One truck boarded, now %d trucks and %d cars on ferry\n", truckBoarded, carBoarded);
        }
        for (i = 0; i < carOnFerry; i++) {
            sem_wait(&semCarBoard);
            carBoarded++;
            printf("CAPTAINCAPTAINCAPTAIN    One car boarded, now %d trucks and %d cars on ferry\n", truckBoarded, carBoarded);
        }

        // change the status into boarded
        pthread_mutex_lock(&mtxFerryStatus);
        ferryStatus = FERRY_BOARDED;
        pthread_mutex_unlock(&mtxFerryStatus);
        // put all late arrival vehicles into waiting queue
        printf("CAPTAINCAPTAINCAPTAIN    All vehicles boarded, move late arrivals into waiting queue\n");

        // move all the vehicles in the late arrival into waiting queue
        int truckSwitch = 0;
        int carSwitch = 0;
        //keep in this order to avoid deadlock
        pthread_mutex_lock(&mtxNumTruckLate);
        pthread_mutex_lock(&mtxNumCarLate);
        while (numTruckLate > 0 || numCarLate > 0) {
            if (numTruckLate > 0) {
                numTruckLate --;
                sem_wait(&semNumTruckLate);
                sem_post(&semTruckLate);
                truckSwitch ++;
            }
            if (numCarLate > 0) {
                numCarLate --;
                sem_wait(&semNumCarLate);
                sem_post(&semCarLate);
                carSwitch ++;
            }
        }
        pthread_mutex_unlock(&mtxNumCarLate);
        pthread_mutex_unlock(&mtxNumTruckLate);

        // wait till all vehicles are switched into the waiting queue from late queue
        printf("CAPTAINCAPTAINCAPTAIN    Need to move %d trucks and %d cars from late arrival to waiting\n", truckSwitch, carSwitch);
        for(i=0;i<truckSwitch;i++) {
            sem_wait(&semTruckSwitch);
            printf("CAPTAINCAPTAINCAPTAIN    Moved one truck from late queue into waiting queue\n");

        }
        for(i=0;i<carSwitch;i++) {
            sem_wait(&semCarSwitch);
            printf("CAPTAINCAPTAINCAPTAIN    Moved one car from late queue into waiting queue\n");
        }

        printf("CAPTAINCAPTAINCAPTAIN    All late arrivals so far is moved into waiting queue\n");

        sail();

        // arrived at the destination dock, change status into unloading
        pthread_mutex_lock(&mtxFerryStatus);
        ferryStatus = FERRY_UNLOADING;
        pthread_mutex_unlock(&mtxFerryStatus);

        // tell all the vehicles on board to unload
        printf("CAPTAINCAPTAINCAPTAIN    Starts unloading\n");
        for (i = 0; i < truckOnFerry; i++) {
            sem_post(&semTruckUnload);
        }
        for (i = 0; i < carOnFerry; i++) {
            sem_post(&semCarUnload);
        }

        // wait till all vehicles confirmed unloaded
        int truckUnloaded = 0;
        int carUnloaded = 0;

        for (i = 0; i < truckOnFerry; i++) {
            sem_wait(&semTruckLeft);
            truckUnloaded++;
            printf("CAPTAINCAPTAINCAPTAIN    One truck is unloaded, now %d trucks and %d cars unloaded\n", truckUnloaded, carUnloaded);
        }
        for (i = 0; i < carOnFerry; i++) {
            sem_wait(&semCarLeft);
            carUnloaded++;
            printf("CAPTAINCAPTAINCAPTAIN    One car is unloaded, now %d trucks and %d cars unloaded\n", truckUnloaded, carUnloaded);
        }
        printf("CAPTAINCAPTAINCAPTAIN    All vehicles unloaded\n");
        sail();
        load++;
        printf("===============One more load is done, now %d loads finished\n===============", load);
    }

    //finished 11 loads, set termination flag 1
    pthread_mutex_lock(&mtxTerminate);
    flagTerminate = 1;
    pthread_mutex_unlock(&mtxTerminate);
    exit(0);
}

// check the status of the ferry
int checkStatus() {
    int ret = 0;
    pthread_mutex_lock(&mtxFerryStatus);
    ret = ferryStatus;
    pthread_mutex_unlock(&mtxFerryStatus);
    return ret;
}

// routine of the car thread
void *car(void *arg) {
    int id = *(int *) arg;
    printf("CARCARCARCARCARCARCAR    Car %d comes\n", id);
    // check the status of the ferry
    // if it is loading, then the vehicle is late
    // else, goes to the waiting queue
    if (checkStatus() == FERRY_LOADING) {
        printf("CARCARCARCARCARCARCAR    Car %d is late \n", id);
        pthread_mutex_lock(&mtxNumCarLate);
        numCarLate++;
        pthread_mutex_unlock(&mtxNumCarLate);
        sem_post(&semNumCarLate);
        sem_wait(&semCarLate);

         // if a car goes out of the late queue and the ferry is loading
         // it is on board
        if(checkStatus()==FERRY_LOADING) {
            printf("CARCARCARCARCARCARCAR    Car %d is boarded\n", id);
            sem_post(&semCarBoard);
        }
        else if(checkStatus()==FERRY_BOARDED) {
            // the car goes into the waiting queue if the ferry is already boarded
            printf("CARCARCARCARCARCARCAR    Car %d is switched into waiting queue\n",id);
            sem_post(&semCarSwitch);
            pthread_mutex_lock(&mtxNumCarWait);
            numCarWait ++;
            pthread_mutex_unlock(&mtxNumCarWait);
            sem_post(&semNumCarWait);
            sem_wait(&semCarWait);
            printf("CARCARCARCARCARCARCAR    Car %d is boarded\n", id);
            sem_post(&semCarBoard);
        }
    }
    else {
        printf("CARCARCARCARCARCARCAR    Car %d goes into waiting queue\n", id);
        pthread_mutex_lock(&mtxNumCarWait);
        numCarWait ++;
        pthread_mutex_unlock(&mtxNumCarWait);
        sem_post(&semNumCarWait);
        sem_wait(&semCarWait);
        printf("CARCARCARCARCARCARCAR    Car %d is boarded\n", id);
        sem_post(&semCarBoard);
    }

    // the car waits to be unloaded
    sem_wait(&semCarUnload);
    printf("CARCARCARCARCARCARCAR    Car %d is unloaded and leaves\n", id);
    sem_post(&semCarLeft);
}

// the routine is similar to that of the car
void *truck(void *arg) {
    int id = *(int *) arg;
    printf("TRUCKTRUCKTRUCKTRUCKT    Truck %d comes\n", id);
    if (checkStatus() == FERRY_LOADING) {
        printf("TRUCKTRUCKTRUCKTRUCKT    Truck %d is late \n", id);
        pthread_mutex_lock(&mtxNumTruckLate);
        numTruckLate++;
        pthread_mutex_unlock(&mtxNumTruckLate);
        sem_post(&semNumTruckLate);
        sem_wait(&semTruckLate);

        if(checkStatus()==FERRY_LOADING) {
            printf("TRUCKTRUCKTRUCKTRUCKT    Truck %d is boarded\n", id);
            sem_post(&semTruckBoard);
        }
        else if(checkStatus()==FERRY_BOARDED) {
            printf("TRUCKTRUCKTRUCKTRUCKT    Truck %d is switched into waiting queue\n", id);
            sem_post(&semTruckSwitch);
            pthread_mutex_lock(&mtxNumTruckWait);
            numTruckWait ++;
            pthread_mutex_unlock(&mtxNumTruckWait);
            sem_post(&semNumTruckWait);
            sem_wait(&semTruckWait);
            printf("TRUCKTRUCKTRUCKTRUCKT    Truck %d is boarded\n", id);
            sem_post(&semTruckBoard);
        }
    }
    else {
        printf("TRUCKTRUCKTRUCKTRUCKT    Truck %d goes into waiting queue\n", id);
        pthread_mutex_lock(&mtxNumTruckWait);
        numTruckWait ++;
        pthread_mutex_unlock(&mtxNumTruckWait);
        sem_post(&semNumTruckWait);
        sem_wait(&semTruckWait);
        printf("TRUCKTRUCKTRUCKTRUCKT    Truck %d is boarded\n", id);
        sem_post(&semTruckBoard);
    }

    sem_wait(&semTruckUnload);
    printf("TRUCKTRUCKTRUCKTRUCKT    Truck %d is unloaded and leaves\n", id);
    sem_post(&semTruckLeft);
}

int checkTerminate() {
    int ret = 0;
    pthread_mutex_lock(&mtxTerminate);
    ret = flagTerminate;
    pthread_mutex_unlock(&mtxTerminate);

    return ret;
}

int main(void) {
    long long maxVehicleInterval = 0;
    int probTruck = 0;
    long long arriveNext = 0;
    //user input
    printf("Input the maximum interval of of vehicles [us]:");
    scanf("%lld", &maxVehicleInterval);
    printf("Input the probability of truck [0,100):");
    scanf("%d", &probTruck);

    pthread_t tCaptain;
    pthread_t tTruck;
    pthread_t tCar;

    pthread_create(&tCaptain, NULL, captain, NULL);

    struct timeval timeLast;
    struct timeval timeCurrent;
    long long elapsed = 0;

    arriveNext = maxVehicleInterval;
    gettimeofday(&timeLast, NULL);

    srand(time(NULL));
    long long numTruck = 0;
    long long numCar = 0;
    while (1) {

        // terminate the simulation
        if (checkTerminate()) {
            printf("=========================The simulation is to terminated===================\n");
            releaseResource();
            pthread_join(tCaptain, NULL);
            exit(0);
        }

        //update elapsed time
        gettimeofday(&timeCurrent, NULL);
        elapsed += (timeCurrent.tv_sec - timeLast.tv_sec) * 1000000 + (timeCurrent.tv_usec - timeLast.tv_usec);
        timeLast = timeCurrent;

        // generate a new vehicle
        if (elapsed >= arriveNext) {
            int isTruck = rand() % 100;
            if (isTruck < probTruck) {
                numTruck++;
                int arg = numTruck;
                pthread_create(&tTruck, NULL, truck, (void *) &arg);
            }
            else {
                numCar++;
                int arg = numCar;
                pthread_create(&tCar, NULL, car, (void *) &arg);
            }
            arriveNext += rand() % maxVehicleInterval;
        }
    }
}