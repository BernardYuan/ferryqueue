//
// Created by Bernard Yuan on 2016-08-02.
//

#include "ferry.h"

pthread_mutex_t mtxFerryStatus;
int ferryStatus;

pthread_mutex_t mtxNumTruckWait;
sem_t semNumTruckWait;
int numTruckWait;
sem_t semTruckWait;

pthread_mutex_t mtxNumTruckLate;
sem_t semNumTruckLate;
int numTruckLate;
sem_t semTruckLate;

pthread_mutex_t mtxNumCarWait;
sem_t semNumCarWait;
int numCarWait;
sem_t semCarWait;

pthread_mutex_t mtxNumCarLate;
sem_t semNumCarLate;
int numCarLate;
sem_t semCarLate;

pthread_mutex_t mtxTerminate;
int flagTerminate;

sem_t semTruckSwitch;
sem_t semCarSwitch;
sem_t semTruckBoard;
sem_t semCarBoard;
sem_t semTruckUnload;
sem_t semCarUnload;
sem_t semTruckLeft;
sem_t semCarLeft;
void initMutex(pthread_mutex_t *mtx, const pthread_mutexattr_t *attr) {
    if (pthread_mutex_init(mtx, attr) == 0) printf("Initializing mutex success\n");
    else {
        printf("Initializing mutex failure\n");
//        releaseResource();
        exit(1);
    }
}

void removeMutex(pthread_mutex_t *mtx) {
    if (pthread_mutex_destroy(mtx) == 0) printf("Destroying mutex success\n");
    else {
        printf("Destroying mutex failure\n");
    }
}

void initSem(sem_t *sem, int share, unsigned int value) {
    if(sem_init(sem, share, value)==-1) {
        printf("Initializing semaphore error\n");
    }
    else {
        printf("Initializeing semaphore success\n");
    }
}

void removeSem(sem_t *sem) {
    if(sem_destroy(sem)==-1) {
        printf("Destroying semaphore error\n");
    }
    else printf("Destroying semaphores success\n");
}

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



void sail() {
    pthread_mutex_lock(&mtxFerryStatus);
    ferryStatus = FERRY_SAILING;
    pthread_mutex_unlock(&mtxFerryStatus);
    printf("Sailing starts\n");
    usleep(SAIL_TIME);
    printf("Sailing ends\n");
}

void *captain(void *arg) {
    int load = 0;

    while (load < MAX_LOAD) {
        pthread_mutex_lock(&mtxFerryStatus);
        ferryStatus = FERRY_LOADING;
        pthread_mutex_unlock(&mtxFerryStatus);
        printf("Ferry starts loading\n");

        // check number of trucks
        int truckOnFerry = 0;
        int carOnFerry = 0;
        int spotsOnFerry = 0;
        while (truckOnFerry < MAX_TRUCK && spotsOnFerry < SIZE_FERRY - 2) {
            pthread_mutex_lock(&mtxNumTruckWait);
            if (numTruckWait > 0) {
                sem_wait(&semNumTruckWait);
                numTruckWait--;
                truckOnFerry++;
                spotsOnFerry += SIZE_TRUCK;
                pthread_mutex_unlock(&mtxNumTruckWait);
                sem_post(&semTruckWait);
                printf("Load one truck from the waiting queue now %d trucks and %d cars loaded\n", truckOnFerry,
                       carOnFerry);

            }
            else {
                pthread_mutex_unlock(&mtxNumTruckWait);
                break;
            }
        }

        while (spotsOnFerry < SIZE_FERRY) {
            pthread_mutex_lock(&mtxNumCarWait);
            if (numCarWait > 0) {
                sem_wait(&semNumCarWait);
                numCarWait--;
                carOnFerry++;
                spotsOnFerry += SIZE_CAR;
                pthread_mutex_unlock(&mtxNumCarWait);
                sem_post(&semCarWait);
                printf("Load one car from the waiting queue now %d trucks and %d cars loaded\n", truckOnFerry,
                       carOnFerry);
            }
            else {
                pthread_mutex_unlock(&mtxNumCarWait);
                break;
            }
        }

        while (spotsOnFerry < SIZE_FERRY) {
            // load trucks from late arrival
            if (spotsOnFerry < SIZE_FERRY - 2 && truckOnFerry < MAX_TRUCK) {
                pthread_mutex_lock(&mtxNumTruckLate);
                if (numTruckLate > 0) {
                    sem_wait(&semNumTruckLate);
                    numTruckLate--;
                    truckOnFerry++;
                    spotsOnFerry += SIZE_TRUCK;
                    sem_post(&semTruckLate);
                    printf("Load one truck from the late queue, now %d trucks and %d cars loaded\n", truckOnFerry,
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
                    printf("Load one car from the late queue, now %d trucks and %d cars loaded\n", truckOnFerry,
                           carOnFerry);
                }
                pthread_mutex_unlock(&mtxNumCarLate);
            }
        }

        int i;
        int truckBoarded = 0;
        int carBoarded = 0;
        for (i = 0; i < truckOnFerry; i++) {
            sem_wait(&semTruckBoard);
            truckBoarded++;
            printf("One truck boarded, now %d trucks and %d cars on ferry\n", truckBoarded, carBoarded);
        }
        for (i = 0; i < carOnFerry; i++) {
            sem_wait(&semCarBoard);
            carBoarded++;
            printf("One car boarded, now %d trucks and %d cars on ferry\n", truckBoarded, carBoarded);
        }

        pthread_mutex_lock(&mtxFerryStatus);
        ferryStatus = FERRY_BOARDED;
        pthread_mutex_unlock(&mtxFerryStatus);
        // put all late arrival vehicles into waiting queue
        printf("All vehicles boarded, move late arrivals into waiting queue\n");

        int truckSwitch = 0;
        int carSwitch = 0;
        //keep in this order to avoid deadlock
        pthread_mutex_lock(&mtxNumTruckLate);
        pthread_mutex_lock(&mtxNumCarLate);
        while (numTruckLate > 0 && numCarLate > 0) {
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

            if(numCarLate <= 0 && numTruckLate <= 0) break;
        }
        pthread_mutex_unlock(&mtxNumCarLate);
        pthread_mutex_unlock(&mtxNumTruckLate);

        for(i=0;i<truckSwitch;i++) {
            sem_wait(&semTruckSwitch);
            printf("Moved one truck from late queue into waiting queue\n");

        }
        for(i=0;i<carSwitch;i++) {
            sem_wait(&semCarSwitch);
            printf("Moved one car from late queue into waiting queue\n");
        }

        sail();

        pthread_mutex_lock(&mtxFerryStatus);
        ferryStatus = FERRY_UNLOADING;
        pthread_mutex_unlock(&mtxFerryStatus);

        printf("Starts unloading\n");
        for (i = 0; i < truckOnFerry; i++) {
            sem_post(&semTruckUnload);
        }
        for (i = 0; i < carOnFerry; i++) {
            sem_post(&semCarUnload);
        }
        // return, untill cars left
        int truckUnloaded = 0;
        int carUnloaded = 0;

        for (i = 0; i < truckOnFerry; i++) {
            sem_wait(&semTruckLeft);
            truckUnloaded++;
            printf("One truck is unloaded, now %d trucks and %d cars unloaded\n", truckUnloaded, carUnloaded);
        }
        for (i = 0; i < carOnFerry; i++) {
            sem_wait(&semCarLeft);
            carUnloaded++;
            printf("One car is unloaded, now %d trucks and %d cars unloaded\n", truckUnloaded, carUnloaded);
        }
        printf("All vehicles unloaded\n");
        sail();
        load++;
    }
    pthread_mutex_lock(&mtxTerminate);
    flagTerminate = 1;
    pthread_mutex_unlock(&mtxTerminate);
    exit(0);
}

int checkStatus() {
    int ret = 0;
    pthread_mutex_lock(&mtxFerryStatus);
    ret = ferryStatus;
    pthread_mutex_unlock(&mtxFerryStatus);

    return ret;
}

void *car(void *arg) {
    int id = *(int *) arg;
    printf("Car %d comes\n", id);
    if (checkStatus() == FERRY_LOADING) {
        printf("Car %d is late \n", id);
        pthread_mutex_lock(&mtxNumCarLate);
        numCarLate++;
        pthread_mutex_unlock(&mtxNumCarLate);
        sem_post(&semNumCarLate);
        sem_wait(&semCarLate);

        if(checkStatus()==FERRY_LOADING) {
            sem_post(&semCarBoard);
            printf("Car %d is boarded\n", id);
        }
        else if(checkStatus()==FERRY_BOARDED) {
            sem_post(&semCarSwitch);
            printf("Car %d is switched into waiting queue\n",id);
            pthread_mutex_lock(&mtxNumCarWait);
            numCarWait ++;
            pthread_mutex_unlock(&mtxNumCarWait);
            sem_post(&semNumCarWait);
            sem_wait(&semCarWait);
            printf("Car %d is boarded\n", id);
            sem_post(&semCarBoard);
        }
    }
    else {
        printf("Car %d goes into waiting queue\n", id);
        pthread_mutex_lock(&mtxNumCarWait);
        numCarWait ++;
        pthread_mutex_unlock(&mtxNumCarWait);
        sem_post(&semNumCarWait);
        sem_wait(&semCarWait);
        printf("Car %d is boarded\n", id);
        sem_post(&semCarBoard);
    }

    sem_wait(&semCarUnload);
    printf("Car %d is unloaded and leaves\n", id);
    sem_post(&semCarLeft);
}

void *truck(void *arg) {
    int id = *(int *) arg;
    printf("Truck %d comes\n", id);
    if (checkStatus() == FERRY_LOADING) {
        printf("Truck %d is late \n", id);
        pthread_mutex_lock(&mtxNumTruckLate);
        numTruckLate++;
        pthread_mutex_unlock(&mtxNumTruckLate);
        sem_post(&semNumTruckLate);
        sem_wait(&semTruckLate);

        if(checkStatus()==FERRY_LOADING) {
            sem_post(&semTruckBoard);
            printf("Truck %d is boarded\n", id);
        }
        else if(checkStatus()==FERRY_BOARDED) {
            sem_post(&semTruckSwitch);
            printf("Truck %d is switched into waiting queue\n", id);
            pthread_mutex_lock(&mtxNumTruckWait);
            numTruckWait ++;
            pthread_mutex_unlock(&mtxNumTruckWait);
            sem_post(&semNumTruckWait);
            sem_wait(&semTruckWait);
            printf("Truck %d is boarded\n", id);
            sem_post(&semTruckBoard);
        }
    }
    else {
        printf("Truck %d goes into waiting queue\n", id);
        pthread_mutex_lock(&mtxNumTruckWait);
        numTruckWait ++;
        pthread_mutex_unlock(&mtxNumTruckWait);
        sem_post(&semNumTruckWait);
        sem_wait(&semTruckWait);
        printf("Truck %d is boarded\n", id);
        sem_post(&semTruckBoard);
    }

    sem_wait(&semTruckUnload);
    printf("Truck %d is unloaded and leaves\n", id);
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
        if (checkTerminate()) {
            printf("The simulation is to terminated\n");
            releaseResource();
            pthread_join(tCaptain, NULL);
            exit(0);
        }

        gettimeofday(&timeCurrent, NULL);
        elapsed += (timeCurrent.tv_sec - timeLast.tv_sec) * 1000000 + (timeCurrent.tv_usec - timeLast.tv_usec);
        timeLast = timeCurrent;

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
            arriveNext += maxVehicleInterval;
        }
    }
}