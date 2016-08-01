//
// Created by Bernard Yuan on 2016-07-30.
//

#include "ferry.h"

mess_t buf;
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
        while (msgrcv(queueToCaptain, &buf, length, REQ_CAR_ARRIVE, IPC_NOWAIT) != -1) {
            buf.mtype = buf.pid;
            buf.pid = localpid;
            buf.data = RPL_VEHICLE_WAIT;
            printf("Get Car %ld into waiting queue\n", buf.mtype);
            msgsnd(queueToVehicle, &buf, length, 0);
        }

        // find an arrived truck
        while (msgrcv(queueToCaptain, &buf, length, REQ_TRUCK_ARRIVE, IPC_NOWAIT) != -1) {
            buf.mtype = buf.pid;
            buf.pid = localpid;
            buf.data = RPL_VEHICLE_WAIT;
            printf("Get truck %ld into waiting queue\n", buf.mtype);
            msgsnd(queueToVehicle, &buf, length, 0);
        }

        int spotsOnFerry = 0;
        int i;
        int trucksOnFerry = 0;
        printf("CAPTAINCAPTAINCAP STARTING LOADING\n");

        while (spotsOnFerry < MAXSIZE_FERRY) {
            // now get all late vehicles into the late queue
            while (msgrcv(queueToCaptain, &buf, length, REQ_CAR_ARRIVE, IPC_NOWAIT) != -1) {
                buf.mtype = buf.pid;
                buf.pid = localpid;
                buf.data = RPL_VEHICLE_LATE;
                printf("Get car %ld into the late queue\n", buf.mtype);
                msgsnd(queueToVehicle, &buf, length, 0);
            }
            while (msgrcv(queueToCaptain, &buf, length, REQ_TRUCK_ARRIVE, IPC_NOWAIT) != -1) {
                buf.mtype = buf.pid;
                buf.pid = localpid;
                buf.data = RPL_VEHICLE_LATE;
                printf("Get truck %ld into the late queue\n", buf.mtype);
                msgsnd(queueToVehicle, &buf, length, 0);
            }

            while (trucksOnFerry < MAXNUM_TRUCK && spotsOnFerry <= MAXSIZE_FERRY - 2 &&
                   (msgrcv(queueWait, &buf, length, REQ_TRUCK_WAIT, IPC_NOWAIT) != -1)) {
                buf.mtype = buf.pid;
                buf.data = RPL_BOARDING;
                buf.pid = localpid;
                printf("Load Truck %ld from waiting queue\n", buf.mtype);
                msgsnd(queueToVehicle, &buf, length, 0);
                spotsOnFerry += SIZE_TRUCK;
                trucksOnFerry++;
            }

            while (spotsOnFerry < MAXSIZE_FERRY && (msgrcv(queueWait, &buf, length, REQ_CAR_WAIT, IPC_NOWAIT) != -1)) {
                buf.mtype = buf.pid;
                buf.data = RPL_BOARDING;
                buf.pid = localpid;
                printf("Load Car %ld from waiting queue\n", buf.mtype);
                msgsnd(queueToVehicle, &buf, length, 0);
                spotsOnFerry += SIZE_CAR;
            }
            // check the late queue for trucks
            while (trucksOnFerry < MAXNUM_TRUCK && spotsOnFerry <= MAXSIZE_FERRY - 2 &&
                   (msgrcv(queueLate, &buf, length, REQ_TRUCK_LATE, IPC_NOWAIT) != -1)) {

                buf.mtype = buf.pid;
                buf.data = RPL_BOARDING;
                buf.pid = localpid;
                msgsnd(queueToVehicle, &buf, length, 0);
                spotsOnFerry += SIZE_TRUCK;
                trucksOnFerry++;
                printf("Load truck %ld from late queue\n", buf.mtype);
            }
            // check the late queue for cars
            while (spotsOnFerry < MAXSIZE_FERRY && (msgrcv(queueLate, &buf, length, REQ_CAR_LATE, IPC_NOWAIT) != -1)) {
                buf.mtype = buf.pid;
                buf.data = RPL_BOARDING;
                buf.pid = localpid;
                msgsnd(queueToVehicle, &buf, length, 0);
                spotsOnFerry += SIZE_CAR;
                printf("Load car %ld from late queue\n", buf.mtype);
            }
        }

        // now the ferry is full, put the rest of the late arrival vehicles into the late queue
        // now get all late vehicles into the late queue
        while (msgrcv(queueLate, &buf, length, REQ_CAR_LATE, IPC_NOWAIT) != -1) {
            buf.mtype = buf.pid;
            buf.pid = localpid;
            buf.data = RPL_VEHICLE_WAIT;
            msgsnd(queueToVehicle, &buf, length, 0);
            printf("Get car %ld from late line to waiting line\n", buf.mtype);
        }
        while (msgrcv(queueLate, &buf, length, REQ_TRUCK_LATE, IPC_NOWAIT) != -1) {
            buf.mtype = buf.pid;
            buf.pid = localpid;
            buf.data = RPL_VEHICLE_WAIT;
            msgsnd(queueToVehicle, &buf, length, 0);
            printf("Get car %ld from late line to waiting line\n", buf.mtype);
        }

        int boardAck = 0;
        int truckBoard = 0;
        int carBoard = 0;
        pid_t boardVehicles[MAXSIZE_FERRY];

        while (boardAck < MAXSIZE_FERRY) {
            if (msgrcv(queueOnBoard, &buf, length, ACK_BOARDED, IPC_NOWAIT) != 0) {
                if (buf.data == TYPE_TRUCK) {
                    boardAck += SIZE_TRUCK;
                    boardVehicles[truckBoard + carBoard] = buf.pid;
                    truckBoard += 1;
                }
                else if (buf.data == TYPE_CAR) {
                    boardAck += SIZE_CAR;
                    boardVehicles[truckBoard + carBoard] = buf.pid;
                    carBoard += 1;
                }
            }
        }

        printf("The ferry starts sailing\n");
        usleep(SAIL_TIME);

        printf("Unloading starts\n");
        buf.data = REQ_UNLOAD;
        for (i = 0; i < (truckBoard + carBoard); i++) {
            buf.mtype = boardVehicles[i];
            msgsnd(queueToVehicle, &buf, length, 0);
        }

        i = 0;
        while (i < (truckBoard + carBoard)) {
            if (msgrcv(queueUnloading, &buf, length, ACK_UNLOADED, IPC_NOWAIT) != -1) {
                if (buf.data == TYPE_TRUCK) printf("Truck %d is unloaded\n", buf.pid);
                else if (buf.data = TYPE_CAR) printf("Car %d is unloaded\n", buf.pid);
                i += 1;
            }
        }

        //starts sailing
        load++;
        printf("Unload done, now %d loads finished\n", load);
    }
    buf.mtype = REQ_TERMINATE;
    buf.pid = localpid;
    buf.data = 0;
    msgsnd(queueToMain, &buf, length, 0);
    printf("Captain's office hour is over\n");
    exit(0);
}

void car() {
    pid_t localpid = getpid();
    printf("Car %d comes\n", localpid);
    setpgid(localpid, groupCar);

    buf.mtype = REQ_CAR_ARRIVE;
    buf.pid = localpid;
    buf.data = TYPE_CAR;
    msgsnd(queueToCaptain, &buf, length, 0);

    msgrcv(queueToVehicle, &buf, length, localpid, 0);
    // find out whether the vehicle is late
    if (buf.data == RPL_VEHICLE_WAIT) {
        buf.mtype = REQ_CAR_WAIT;
        buf.pid = localpid;
        msgsnd(queueWait, &buf, length, 0);
        printf("Car %d is waiting\n", localpid);
        msgrcv(queueToVehicle, &buf, length, localpid, 0);
    } else if (buf.data == RPL_VEHICLE_LATE) {
        buf.mtype = REQ_CAR_WAIT;
        buf.pid = localpid;
        msgsnd(queueLate, &buf, length, 0);
        printf("car %d is late\n", localpid);
        msgrcv(queueToVehicle, &buf, length, localpid, 0);

        if (buf.data == RPL_VEHICLE_WAIT) {
            buf.mtype = REQ_CAR_WAIT;
            buf.pid = localpid;
            buf.data = 0;
            msgsnd(queueWait, &buf, length, 0);
            printf("Car %d is moved into the waiting queue\n", localpid);
            msgrcv(queueToVehicle, &buf, length, localpid, 0);
        }
    }

    buf.mtype = ACK_BOARDED;
    buf.pid = localpid;
    buf.data = TYPE_CAR;
    msgsnd(queueOnBoard, &buf, length, 0);
    printf("Car %d is boarded\n", localpid);
    msgrcv(queueToVehicle, &buf, length, localpid, 0);
    printf("Car %d starts unloading\n", localpid);
    buf.mtype = ACK_UNLOADED;
    buf.pid = localpid;
    buf.data = TYPE_CAR;
    msgsnd(queueUnloading, &buf, length, 0);
    printf("Car %d leaves\n", localpid);
}

void truck() {
    pid_t localpid = getpid();
    printf("Truck %d comes\n", localpid);
    setpgid(localpid, groupTruck);
    buf.mtype = REQ_TRUCK_ARRIVE;
    buf.pid = localpid;
    buf.data = TYPE_TRUCK;
    msgsnd(queueToCaptain, &buf, length, 0);

    msgrcv(queueToVehicle, &buf, length, localpid, 0);
    // find out whether the vehicle is late
    if (buf.data == RPL_VEHICLE_WAIT) {
        buf.mtype = REQ_TRUCK_WAIT;
        buf.pid = localpid;
        msgsnd(queueWait, &buf, length, 0);
        printf("Truck %d goes to the waiting queue\n", localpid);
        msgrcv(queueToVehicle, &buf, length, localpid, 0);
    } else if (buf.data == RPL_VEHICLE_LATE) {
        buf.mtype = REQ_TRUCK_LATE;
        buf.pid = localpid;
        msgsnd(queueLate, &buf, length, 0);
        printf("Truck %d goes to the late queue\n", localpid);
        msgrcv(queueToVehicle, &buf, length, localpid, 0);

        if (buf.data == RPL_VEHICLE_WAIT) {
            buf.mtype = REQ_TRUCK_WAIT;
            buf.pid = localpid;
            buf.data = 0;
            msgsnd(queueWait, &buf, length, 0);
            printf("Truck %d is moved into the waiting queue\n", localpid);
            msgrcv(queueToVehicle, &buf, length, localpid, 0);
        }
    }

    buf.mtype = ACK_BOARDED;
    buf.pid = localpid;
    buf.data = TYPE_TRUCK;
    msgsnd(queueOnBoard, &buf, length, 0);
    printf("Truck %d is boarded\n", localpid);
    msgrcv(queueToVehicle, &buf, length, localpid, 0);
    printf("Truck %d starts unloading\n", localpid);
    buf.mtype = ACK_UNLOADED;
    buf.pid = localpid;
    buf.data = TYPE_TRUCK;
    msgsnd(queueUnloading, &buf, length, 0);
    printf("Truck %d leaves\n", localpid);
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
    scanf("%ld", &maxArriveInterval);
    printf("Input the probability of truck (integer, %%):");
    scanf("%d", &truckProb);

    // start time
    gettimeofday(&last, NULL);

    pid_t pid;
    //child process the captain
    if (!(pid = fork())) captain();
    else {
        processCaptain = pid;
        while (1) {
            //check termination condition
            //and kill all the processes
            if (msgrcv(queueToMain, &buf, length, REQ_TERMINATE, IPC_NOWAIT) != -1) {
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
        int isTruck = rand() % 100;
        if (isTruck <= truckProb) truck();
        else car();
    }
}