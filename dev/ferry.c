//
// Created by Bernard Yuan on 2016-07-30.
//

#include "ferry.h"
mess_t bufMain;
mess_t bufCaptain;
mess_t bufCar;
mess_t bufTruck;

int length;
// time parameters
struct timeval last;
struct timeval current;
long long elapsed = 0;
long long lastArrive = 0;
long long maxArriveInterval = 0;
int truckProb = 0;
//queue ID's
int queueToVehicle;
int queueToCaptain;
int queueWait;
int queueLate;
int queueUnloading;
int queueOnBoard;
int queueToMain;
//groupids
int processCaptain;
int groupCar = 661;
int groupTruck = 662;

void destroyQueue() {
    if (msgctl(queueToVehicle, IPC_RMID, 0) == -1) {
        printf("queueToVehicle not destroyed\n");
    }
    else printf("queueToVehicle destroyed\n");

    if (msgctl(queueToCaptain, IPC_RMID, 0) == -1) {
        printf("queueToCaptain not destroyed\n");
    }
    else printf("queueToCaptain destroyed\n");

    if (msgctl(queueWait, IPC_RMID, 0) == -1) {
        printf("queueWait not destroyed\n");
    }
    else printf("queueWait destroyed\n");

    if (msgctl(queueLate, IPC_RMID, 0) == -1) {
        printf("queueLate not destroyed\n");
    }
    else printf("queueLate destroyed\n");


    if (msgctl(queueUnloading, IPC_RMID, 0) == -1) {
        printf("queueUnloading not destroyed\n");
    }
    else printf("queueUnloading destroyed\n");

    if(msgctl(queueOnBoard, IPC_RMID, 0) == -1) {
        printf("queueOnBoard not destroyed\n");
    }
    else printf("queueUnloading destroyed\n");

    if(msgctl(queueToMain, IPC_RMID, 0) == -1) {
        printf("queueToMain not destroyed\n");
    }
    else printf("queueToMain destroyed\n");
}

void createQueue() {
    queueToVehicle = msgget(IPC_PRIVATE, IPC_CREAT | IPC_EXCL | 0660);
    if (queueToVehicle == -1) {
        printf("QueueToVehicle not created\n");
        destroyQueue();
        exit(0);
    }
    printf("QueueToVehicle ID: %d \n", queueToVehicle);

    queueToCaptain = msgget(IPC_PRIVATE, IPC_CREAT | IPC_EXCL | 0660);
    if (queueToCaptain == -1) {
        printf("queueToCaptain not created\n");
        destroyQueue();
        exit(0);
    }
    printf("queueToCaptain ID: %d \n", queueToCaptain);

    queueWait = msgget(IPC_PRIVATE, IPC_CREAT | IPC_EXCL | 0660);
    if (queueWait == -1) {
        printf("Waiting Queue not created\n");
        destroyQueue();
        exit(0);
    }
    printf("waiting queue ID: %d \n", queueWait);


    queueLate = msgget(IPC_PRIVATE, IPC_CREAT | IPC_EXCL | 0660);
    if (queueLate <= 0) {
        printf("Late Queue not created\n");
        destroyQueue();
        exit(0);
    }
    printf("late queue ID: %d \n", queueLate);

    queueUnloading = msgget(IPC_PRIVATE, IPC_CREAT | IPC_EXCL | 0660);
    if (queueUnloading <= 0) {
        printf("Unloading Queue not created\n");
        destroyQueue();
        exit(0);
    }
    printf("Unloading queue ID: %d \n", queueUnloading);

    queueOnBoard = msgget(IPC_PRIVATE, IPC_CREAT | IPC_EXCL | 0660);
    if (queueOnBoard <= 0) {
        printf("On Board Queue not created\n");
        destroyQueue();
        exit(0);
    }
    printf("late queue ID: %d \n", queueOnBoard);

    queueToMain = msgget(IPC_PRIVATE, IPC_CREAT | IPC_EXCL | 0660);
    if (queueToMain <= 0) {
        printf("Queue To Main not created\n");
        destroyQueue();
        exit(0);
    }
    printf("Queue To Main process ID: %d \n", queueOnBoard);
}

void captain() {
    pid_t localpid = getpid();
    int load = 0;
    while (load < MAX_LOAD) {
        // find an arrived car
        while (msgrcv(queueToCaptain, &bufCaptain, length, REQ_CAR_ARRIVE, IPC_NOWAIT) != -1) {
            bufCaptain.mtype = bufCaptain.pid;
            bufCaptain.pid = localpid;
            bufCaptain.data = RPL_VEHICLE_WAIT;
            printf("CAPTAINCAPTAINCAP     Get Car %ld into waiting queue\n", bufCaptain.mtype);
            msgsnd(queueToVehicle, &bufCaptain, length, 0);
        }

        // find an arrived truck
        while (msgrcv(queueToCaptain, &bufCaptain, length, REQ_TRUCK_ARRIVE, IPC_NOWAIT) != -1) {
            bufCaptain.mtype = bufCaptain.pid;
            bufCaptain.pid = localpid;
            bufCaptain.data = RPL_VEHICLE_WAIT;
            printf("CAPTAINCAPTAINCAP     Get truck %ld into waiting queue\n", bufCaptain.mtype);
            msgsnd(queueToVehicle, &bufCaptain, length, 0);
        }

        int spotsOnFerry = 0;
        int i;
        int trucksOnFerry = 0;
        printf("CAPTAINCAPTAINCAP     STARTS LOADING\n");

        while (spotsOnFerry < MAXSIZE_FERRY) {
            // now get all late vehicles into the late queue
            while (msgrcv(queueToCaptain, &bufCaptain, length, REQ_CAR_ARRIVE, IPC_NOWAIT) != -1) {
                bufCaptain.mtype = bufCaptain.pid;
                bufCaptain.pid = localpid;
                bufCaptain.data = RPL_VEHICLE_LATE;
                printf("CAPTAINCAPTAINCAP     Get car %ld into the late queue\n", bufCaptain.mtype);
                msgsnd(queueToVehicle, &bufCaptain, length, 0);
            }

            while (msgrcv(queueToCaptain, &bufCaptain, length, REQ_TRUCK_ARRIVE, IPC_NOWAIT) != -1) {
                bufCaptain.mtype = bufCaptain.pid;
                bufCaptain.pid = localpid;
                bufCaptain.data = RPL_VEHICLE_LATE;
                printf("CAPTAINCAPTAINCAP     Get truck %ld into the late queue\n", bufCaptain.mtype);
                msgsnd(queueToVehicle, &bufCaptain, length, 0);
            }

            while (trucksOnFerry < MAXNUM_TRUCK && spotsOnFerry <= MAXSIZE_FERRY - 2 &&
                   (msgrcv(queueWait, &bufCaptain, length, REQ_TRUCK_WAIT, IPC_NOWAIT) != -1)) {
                bufCaptain.mtype = bufCaptain.pid;
                bufCaptain.data = RPL_BOARDING;
                bufCaptain.pid = localpid;
                printf("CAPTAINCAPTAINCAP     Load Truck %ld from waiting queue\n", bufCaptain.mtype);
                msgsnd(queueToVehicle, &bufCaptain, length, 0);
                spotsOnFerry += SIZE_TRUCK;
                trucksOnFerry++;
            }

            while (spotsOnFerry < MAXSIZE_FERRY && (msgrcv(queueWait, &bufCaptain, length, REQ_CAR_WAIT, IPC_NOWAIT) != -1)) {
                bufCaptain.mtype = bufCaptain.pid;
                bufCaptain.data = RPL_BOARDING;
                bufCaptain.pid = localpid;
                printf("CAPTAINCAPTAINCAP     Load Car %ld from waiting queue\n", bufCaptain.mtype);
                msgsnd(queueToVehicle, &bufCaptain, length, 0);
                spotsOnFerry += SIZE_CAR;
            }
            // check the late queue for trucks
            while (trucksOnFerry < MAXNUM_TRUCK && spotsOnFerry <= MAXSIZE_FERRY - 2 &&
                   (msgrcv(queueLate, &bufCaptain, length, REQ_TRUCK_LATE, IPC_NOWAIT) != -1)) {

                bufCaptain.mtype = bufCaptain.pid;
                bufCaptain.data = RPL_BOARDING;
                bufCaptain.pid = localpid;
                msgsnd(queueToVehicle, &bufCaptain, length, 0);
                spotsOnFerry += SIZE_TRUCK;
                trucksOnFerry++;
                printf("CAPTAINCAPTAINCAP     Load truck %ld from late queue\n", bufCaptain.mtype);
            }
            // check the late queue for cars
            while (spotsOnFerry < MAXSIZE_FERRY && (msgrcv(queueLate, &bufCaptain, length, REQ_CAR_LATE, IPC_NOWAIT) != -1)) {
                bufCaptain.mtype = bufCaptain.pid;
                bufCaptain.data = RPL_BOARDING;
                bufCaptain.pid = localpid;
                msgsnd(queueToVehicle, &bufCaptain, length, 0);
                spotsOnFerry += SIZE_CAR;
                printf("CAPTAINCAPTAINCAP     Load car %ld from late queue\n", bufCaptain.mtype);
            }
        }

        // now the ferry is full, put the rest of the late arrival vehicles into the late queue
        // now get all late vehicles into the late queue
        while (msgrcv(queueLate, &bufCaptain, length, REQ_CAR_LATE, IPC_NOWAIT) != -1) {
            bufCaptain.mtype = bufCaptain.pid;
            bufCaptain.pid = localpid;
            bufCaptain.data = RPL_VEHICLE_WAIT;
            msgsnd(queueToVehicle, &bufCaptain, length, 0);
            printf("CAPTAINCAPTAINCAP     Get car %ld from late line to waiting line\n", bufCaptain.mtype);
        }
        while (msgrcv(queueLate, &bufCaptain, length, REQ_TRUCK_LATE, IPC_NOWAIT) != -1) {
            bufCaptain.mtype = bufCaptain.pid;
            bufCaptain.pid = localpid;
            bufCaptain.data = RPL_VEHICLE_WAIT;
            msgsnd(queueToVehicle, &bufCaptain, length, 0);
            printf("CAPTAINCAPTAINCAP     Get car %ld from late line to waiting line\n", bufCaptain.mtype);
        }

        int boardAck = 0;
        int truckBoard = 0;
        int carBoard = 0;
        pid_t boardVehicles[MAXSIZE_FERRY];

        while (boardAck < MAXSIZE_FERRY) {
            if (msgrcv(queueOnBoard, &bufCaptain, length, ACK_BOARDED, IPC_NOWAIT) != 0) {
                if (bufCaptain.data == TYPE_TRUCK) {
                    boardAck += SIZE_TRUCK;
                    boardVehicles[truckBoard + carBoard] = bufCaptain.pid;
                    truckBoard += 1;
                }
                else if (bufCaptain.data == TYPE_CAR) {
                    boardAck += SIZE_CAR;
                    boardVehicles[truckBoard + carBoard] = bufCaptain.pid;
                    carBoard += 1;
                }
            }
        }

        printf("CAPTAINCAPTAINCAP     The ferry starts sailing\n");
        usleep(SAIL_TIME);

        printf("CAPTAINCAPTAINCAP     Unloading starts\n");
        bufCaptain.data = REQ_UNLOAD;
        for (i = 0; i < (truckBoard + carBoard); i++) {
            bufCaptain.mtype = boardVehicles[i];
            msgsnd(queueToVehicle, &bufCaptain, length, 0);
        }

        i = 0;
        while (i < (truckBoard + carBoard)) {
            if (msgrcv(queueUnloading, &bufCaptain, length, ACK_UNLOADED, IPC_NOWAIT) != -1) {
                if (bufCaptain.data == TYPE_TRUCK) printf("CAPTAINCAPTAINCAP     Truck %d is unloaded\n", bufCaptain.pid);
                else if (bufCaptain.data = TYPE_CAR) printf("CAPTAINCAPTAINCAP     Car %d is unloaded\n", bufCaptain.pid);
                i += 1;
            }
        }

        //starts sailing
        load++;
        printf("CAPTAINCAPTAINCAP     Unload done, now %d loads finished\n", load);
    }
    bufCaptain.mtype = REQ_TERMINATE;
    bufCaptain.pid = localpid;
    bufCaptain.data = 0;
    msgsnd(queueToMain, &bufCaptain, length, 0);
    printf("CAPTAINCAPTAINCAP     Captain's office hour is over\n");
    exit(0);
}

void car() {
    pid_t localpid = getpid();
    printf("CARCARCARCARCARCAR    Car %d comes\n", localpid);
    setpgid(localpid, groupCar);

    bufCar.mtype = REQ_CAR_ARRIVE;
    bufCar.pid = localpid;
    bufCar.data = TYPE_CAR;
    msgsnd(queueToCaptain, &bufCar, length, 0);

    msgrcv(queueToVehicle, &bufCar, length, localpid, 0);
    printf("CARCARCARCARCARCAR    buffer address: %p\n", &bufCar);
    printf("CARCARCARCARCARCAR    buffer type:%ld\n", bufCar.mtype);
    printf("CARCARCARCARCARCAR    buffer data:%d\n", bufCar.data);

    // find out whether the vehicle is late
    if (bufCar.data == RPL_VEHICLE_WAIT) {
        bufCar.mtype = REQ_CAR_WAIT;
        bufCar.pid = localpid;
        msgsnd(queueWait, &bufCar, length, 0);
        printf("CARCARCARCARCARCAR    Car %d is waiting\n", localpid);
        msgrcv(queueToVehicle, &bufCar, length, localpid, 0);
    } else if (bufCar.data == RPL_VEHICLE_LATE) {
        bufCar.mtype = REQ_CAR_WAIT;
        bufCar.pid = localpid;
        msgsnd(queueLate, &bufCar, length, 0);
        printf("CARCARCARCARCARCAR    Car %d is late\n", localpid);
        msgrcv(queueToVehicle, &bufCar, length, localpid, 0);

        if (bufCar.data == RPL_VEHICLE_WAIT) {
            bufCar.mtype = REQ_CAR_WAIT;
            bufCar.pid = localpid;
            bufCar.data = 0;
            msgsnd(queueWait, &bufCar, length, 0);
            printf("CARCARCARCARCARCAR    Car %d is moved into the waiting queue\n", localpid);
            msgrcv(queueToVehicle, &bufCar, length, localpid, 0);
        }
    }

    bufCar.mtype = ACK_BOARDED;
    bufCar.pid = localpid;
    bufCar.data = TYPE_CAR;
    msgsnd(queueOnBoard, &bufCar, length, 0);
    printf("CARCARCARCARCARCAR    Car %d is boarded\n", localpid);
    msgrcv(queueToVehicle, &bufCar, length, localpid, 0);
    printf("CARCARCARCARCARCAR    Car %d starts unloading\n", localpid);
    bufCar.mtype = ACK_UNLOADED;
    bufCar.pid = localpid;
    bufCar.data = TYPE_CAR;
    msgsnd(queueUnloading, &bufCar, length, 0);
    printf("CARCARCARCARCARCAR    Car %d leaves\n", localpid);
}

void truck() {
    pid_t localpid = getpid();
    printf("TRUCKTRUCKTRUCKTRU    Truck %d comes\n", localpid);
    setpgid(localpid, groupTruck);
    bufTruck.mtype = REQ_TRUCK_ARRIVE;
    bufTruck.pid = localpid;
    bufTruck.data = TYPE_TRUCK;
    msgsnd(queueToCaptain, &bufTruck, length, 0);

    msgrcv(queueToVehicle, &bufTruck, length, localpid, 0);
    printf("TRUCKTRUCKTRUCKTRU    Buf address: %p\n", &bufTruck);
    printf("TRUCKTRUCKTRUCKTRU    buffer type:%ld\n", bufTruck.mtype);
    printf("TRUCKTRUCKTRUCKTRU    buffer data:%d\n", bufTruck.data);
    // find out whether the vehicle is late
    if (bufTruck.data == RPL_VEHICLE_WAIT) {
        bufTruck.mtype = REQ_TRUCK_WAIT;
        bufTruck.pid = localpid;
        msgsnd(queueWait, &bufTruck, length, 0);
        printf("TRUCKTRUCKTRUCKTRU    Truck %d goes to the waiting queue\n", localpid);
        msgrcv(queueToVehicle, &bufTruck, length, localpid, 0);
    } else if (bufTruck.data == RPL_VEHICLE_LATE) {
        bufTruck.mtype = REQ_TRUCK_LATE;
        bufTruck.pid = localpid;
        msgsnd(queueLate, &bufTruck, length, 0);
        printf("TRUCKTRUCKTRUCKTRU    Truck %d goes to the late queue\n", localpid);
        msgrcv(queueToVehicle, &bufTruck, length, localpid, 0);

        if (bufTruck.data == RPL_VEHICLE_WAIT) {
            bufTruck.mtype = REQ_TRUCK_WAIT;
            bufTruck.pid = localpid;
            bufTruck.data = 0;
            msgsnd(queueWait, &bufTruck, length, 0);
            printf("TRUCKTRUCKTRUCKTRU    Truck %d is moved into the waiting queue\n", localpid);
            msgrcv(queueToVehicle, &bufTruck, length, localpid, 0);
        }
    }

    bufTruck.mtype = ACK_BOARDED;
    bufTruck.pid = localpid;
    bufTruck.data = TYPE_TRUCK;
    msgsnd(queueOnBoard, &bufTruck, length, 0);
    printf("TRUCKTRUCKTRUCKTRU    Truck %d is boarded\n", localpid);
    msgrcv(queueToVehicle, &bufTruck, length, localpid, 0);
    printf("TRUCKTRUCKTRUCKTRU    Truck %d starts unloading\n", localpid);
    bufTruck.mtype = ACK_UNLOADED;
    bufTruck.pid = localpid;
    bufTruck.data = TYPE_TRUCK;
    msgsnd(queueUnloading, &bufTruck, length, 0);
    printf("TRUCKTRUCKTRUCKTRU    Truck %d leaves\n", localpid);
}

void terminateSimulation() {
    printf("==================================================================\n");
    printf("========================Termination starts========================\n");
    printf("==================================================================\n");

    if (kill(processCaptain, SIGKILL) == -1 && errno == EPERM) {
        printf("Captain process not killed\n");
    }
    else printf("Captain process killed\n");

    if (killpg(groupCar, SIGKILL) == -1 && errno == EPERM) {
        printf("Cars not killed\n");
    }
    else printf("Cars killed\n");

    if (killpg(groupTruck, SIGKILL) == -1 && errno == EPERM) {
        printf("Trucks not killed\n");
    }
    else printf("Trucks killed\n");

}

int main(void) {
    pid_t localpid = getpid();

    // define the length of message
    length = sizeof(mess_t) - sizeof(long);

    printf("Input the maximum interval (integer, ms) of the arrival of each vehicle:");
    scanf("%lld", &maxArriveInterval);
    printf("Input the probability of truck (integer, %%):");
    scanf("%d", &truckProb);

    // start time
    gettimeofday(&last, NULL);

    //buffer in main
    pid_t pid;
    //child process the captain
    if (!(pid = fork())) captain();
    else {
        processCaptain = pid;
        while (1) {
            //check termination condition
            //and kill all the processes
            if (msgrcv(queueToMain, &bufMain, length, REQ_TERMINATE, IPC_NOWAIT) != -1) {
                printf("The captain informs the main function to terminate\n");
                terminateSimulation();
                destroyQueue();
                printf("Simulation ends\n");
                exit(0);
            }


            gettimeofday(&current, NULL);
            elapsed += (current.tv_sec - last.tv_sec) * 1000000 + (current.tv_usec - last.tv_usec);
            last = current;

            if (elapsed > lastArrive) {
                if (!(pid = fork())) break;
                lastArrive += maxArriveInterval;
            }
        }
        srand(time(NULL));
        int isTruck = rand() % 100;
        printf("isTruck: %d\n", isTruck);
        if (isTruck <= truckProb) truck();
        else car();
    }
}