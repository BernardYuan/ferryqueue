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
int queueOnBoard;
int queueUnloading;
int queueLeave;
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

    if (msgctl(queueOnBoard, IPC_RMID, 0) == -1) {
        printf("queueOnBoard not destroyed\n");
    }
    else printf("queueUnloading destroyed\n");

    if (msgctl(queueToMain, IPC_RMID, 0) == -1) {
        printf("queueToMain not destroyed\n");
    }
    else printf("queueToMain destroyed\n");

    if (msgctl(queueLeave, IPC_RMID, 0) == -1) {
        printf("QueueLeave not destroyed\n");
    }
    else printf("queueLeave destroyed\n");
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
    printf("Queue To Main ID: %d \n", queueOnBoard);

    queueLeave = msgget(IPC_PRIVATE, IPC_CREAT | IPC_EXCL | 0660);
    if (queueLeave <= 0) {
        printf("QueueLeave not created\n");
        destroyQueue();
        exit(0);
    }
    printf("QueueLeave ID: %d\n", queueLeave);
}

int msgReceive(int msqid, void *msgp, size_t msgsz, long int msgtype, int msgflg) {
    if (msgflg == IPC_NOWAIT) {
        return msgrcv(msqid, msgp, msgsz, msgtype, msgflg);
    }
    else {
        int ret = msgrcv(msqid, msgp, msgsz, msgtype, msgflg);
        if (ret == -1) {
//            printf("Receive Message Error\n");
            exit(0);
        }
        else return ret;
    }

}

void msgSend(int msqid, void *msgp, size_t msgsz, int msgflg) {
    if (msgsnd(msqid, msgp, msgsz, msgflg) == -1) {
        printf("Send Message Error\n");
        exit(0);
    }
    else return;
}

void captain() {
    pid_t localpid = getpid();
    int load = 0;
    while (load < MAX_LOAD) {
        // find an arrived car
        while (msgReceive(queueToCaptain, &bufCaptain, length, REQ_CAR_ARRIVE, IPC_NOWAIT) != -1) {
            bufCaptain.mtype = bufCaptain.pid;
            bufCaptain.pid = localpid;
            bufCaptain.data = RPL_VEHICLE_WAIT;
            printf("CAPTAINCAPTAINCAP     Get Car %ld into waiting queue\n", bufCaptain.mtype);
            msgSend(queueToVehicle, &bufCaptain, length, 0);
        }

        // find an arrived truck
        while (msgReceive(queueToCaptain, &bufCaptain, length, REQ_TRUCK_ARRIVE, IPC_NOWAIT) != -1) {
            bufCaptain.mtype = bufCaptain.pid;
            bufCaptain.pid = localpid;
            bufCaptain.data = RPL_VEHICLE_WAIT;
            printf("CAPTAINCAPTAINCAP     Get truck %ld into waiting queue\n", bufCaptain.mtype);
            msgSend(queueToVehicle, &bufCaptain, length, 0);
        }

        printf("==============================STARTS LOADING=========================\n");
        int spotsOnFerry = 0;
        int i;
        int trucksOnFerry = 0;
        int carsOnFerry = 0;

        while (spotsOnFerry < MAXSIZE_FERRY) {
//            printf("CAPTAINCAPTAINCAP     Stil not Full\n");

            // now get all late vehicles into the late queue
            while (msgReceive(queueToCaptain, &bufCaptain, length, REQ_CAR_ARRIVE, IPC_NOWAIT) != -1) {
                bufCaptain.mtype = bufCaptain.pid;
                bufCaptain.pid = localpid;
                bufCaptain.data = RPL_VEHICLE_LATE;
                printf("CAPTAINCAPTAINCAP     Get car %ld into the late queue\n", bufCaptain.mtype);
                msgSend(queueToVehicle, &bufCaptain, length, 0);
            }

            while (msgReceive(queueToCaptain, &bufCaptain, length, REQ_TRUCK_ARRIVE, IPC_NOWAIT) != -1) {
                bufCaptain.mtype = bufCaptain.pid;
                bufCaptain.pid = localpid;
                bufCaptain.data = RPL_VEHICLE_LATE;
                printf("CAPTAINCAPTAINCAP     Get truck %ld into the late queue\n", bufCaptain.mtype);
                msgSend(queueToVehicle, &bufCaptain, length, 0);
            }

            while (trucksOnFerry < MAXNUM_TRUCK && spotsOnFerry <= MAXSIZE_FERRY - SIZE_TRUCK &&
                   (msgReceive(queueWait, &bufCaptain, length, REQ_TRUCK_WAIT, IPC_NOWAIT) != -1)) {

                bufCaptain.mtype = bufCaptain.pid;
                bufCaptain.pid = localpid;
                bufCaptain.data = 0;
                spotsOnFerry += SIZE_TRUCK;
                trucksOnFerry++;
                msgSend(queueOnBoard, &bufCaptain, length, 0);
                printf("CAPTAINCAPTAINCAP     Load Truck %ld from waiting queue, now %d trucks and %d cars on ferry\n",
                       bufCaptain.mtype, trucksOnFerry, carsOnFerry);
            }

            while (spotsOnFerry < MAXSIZE_FERRY &&
                   (msgReceive(queueWait, &bufCaptain, length, REQ_CAR_WAIT, IPC_NOWAIT) != -1)) {
                bufCaptain.mtype = bufCaptain.pid;
                bufCaptain.data = RPL_BOARDING;
                bufCaptain.pid = localpid;
                spotsOnFerry += SIZE_CAR;
                carsOnFerry++;
                msgSend(queueToVehicle, &bufCaptain, length, 0);
                printf("CAPTAINCAPTAINCAP     Load Car %ld from waiting queue, now %d trucks and %d cars on ferry\n",
                       bufCaptain.mtype, trucksOnFerry, carsOnFerry);
            }

            // check the late queue for trucks
            while (trucksOnFerry < MAXNUM_TRUCK && spotsOnFerry <= MAXSIZE_FERRY - 2 &&
                   (msgReceive(queueLate, &bufCaptain, length, REQ_TRUCK_LATE, IPC_NOWAIT) != -1)) {

                bufCaptain.mtype = bufCaptain.pid;
                bufCaptain.data = RPL_BOARDING;
                bufCaptain.pid = localpid;
                spotsOnFerry += SIZE_TRUCK;
                trucksOnFerry++;
                msgSend(queueLate, &bufCaptain, length, 0);
                printf("CAPTAINCAPTAINCAP     Load truck %ld from late queue, now %d trucks and %d cars on ferry\n",
                       bufCaptain.mtype, trucksOnFerry, carsOnFerry);
            }
            // check the late queue for cars
            while (spotsOnFerry < MAXSIZE_FERRY &&
                   (msgReceive(queueLate, &bufCaptain, length, REQ_CAR_LATE, IPC_NOWAIT) != -1)) {
                bufCaptain.mtype = bufCaptain.pid;
                bufCaptain.data = RPL_BOARDING;
                bufCaptain.pid = localpid;
                msgSend(queueLate, &bufCaptain, length, 0);
                spotsOnFerry += SIZE_CAR;
                carsOnFerry++;
                printf("CAPTAINCAPTAINCAP     Load car %ld from late queue, now %d trucks and %d cars on ferry\n",
                       bufCaptain.mtype, trucksOnFerry, carsOnFerry);
            }
        }

        // now the ferry is full, put the rest of the late arrival vehicles into the late queue
        // now get all late vehicles into the late queue
        while (msgReceive(queueLate, &bufCaptain, length, REQ_CAR_LATE, IPC_NOWAIT) != -1) {
            bufCaptain.mtype = bufCaptain.pid;
            bufCaptain.pid = localpid;
            bufCaptain.data = RPL_VEHICLE_WAIT;
            msgSend(queueLate, &bufCaptain, length, 0);
            printf("CAPTAINCAPTAINCAP     Get car %ld from late line to waiting line\n", bufCaptain.mtype);
        }
        while (msgReceive(queueLate, &bufCaptain, length, REQ_TRUCK_LATE, IPC_NOWAIT) != -1) {
            bufCaptain.mtype = bufCaptain.pid;
            bufCaptain.pid = localpid;
            bufCaptain.data = RPL_VEHICLE_WAIT;
            msgSend(queueLate, &bufCaptain, length, 0);
            printf("CAPTAINCAPTAINCAP     Get car %ld from late line to waiting line\n", bufCaptain.mtype);
        }

        printf("CAPTAINCAPTAINCAP     The ferry starts sailing to the destination\n");
        usleep(SAIL_TIME);
        printf("CAPTAINCAPTAINCAP     Unloading starts\n");

        bufCaptain.mtype = TYPE_TRUCK;
        bufCaptain.pid = localpid;
        bufCaptain.data = 0;
        for (i = 0; i < trucksOnFerry; i++) msgSend(queueUnloading, &bufCaptain, length, 0);
        bufCaptain.mtype = TYPE_CAR;
        for (i = 0; i < carsOnFerry; i++) msgSend(queueUnloading, &bufCaptain, length, 0);

        int truckLeft = 0;
        int carLeft = 0;
        while (truckLeft < trucksOnFerry && carLeft < carsOnFerry) {
            if (msgReceive(queueLeave, &bufCaptain, length, 0, IPC_NOWAIT) != -1) {
                if (bufCaptain.data == TYPE_TRUCK) truckLeft++;
                if (bufCaptain.data == TYPE_CAR) carLeft++;
                printf("CAPTAINCAPTAINCAP     Captain receives ACK's from %d trucks and %d cars\n", truckLeft, carLeft);
            }
        }
        //starts sailing
        printf("CAPTAINCAPTAINCAP     The ferry starts sailing back to home\n");
        usleep(SAIL_TIME);
        load++;
        printf("=========================Arrived home, Now %d loads finished=========================\n", load);
    }
    bufCaptain.mtype = REQ_TERMINATE;
    bufCaptain.pid = localpid;
    bufCaptain.data = 0;
    msgSend(queueToMain, &bufCaptain, length, 0);
    printf("===============================Captain's office hour is over============================\n");
    exit(0);
}

void car() {
    pid_t localpid = getpid();
    printf("CARCARCARCARCARCAR    Car %d comes\n", localpid);
    setpgid(localpid, groupCar);

    bufCar.mtype = REQ_CAR_ARRIVE;
    bufCar.pid = localpid;
    bufCar.data = TYPE_CAR;
    msgSend(queueToCaptain, &bufCar, length, 0);

    msgReceive(queueToVehicle, &bufCar, length, localpid, 0);

    // find out whether the vehicle is late
    if (bufCar.data == RPL_VEHICLE_WAIT) {
        bufCar.mtype = REQ_CAR_WAIT;
        bufCar.pid = localpid;
        bufCar.data = 0;
        msgSend(queueWait, &bufCar, length, 0);
        msgReceive(queueOnBoard, &bufCar, length, localpid, 0);
    } else if (bufCar.data == RPL_VEHICLE_LATE) {
        bufCar.mtype = REQ_CAR_LATE;
        bufCar.pid = localpid;
        bufCar.data = 0;
        msgSend(queueLate, &bufCar, length, 0);
        msgReceive(queueLate, &bufCar, length, localpid, 0);

        if (bufCar.data == RPL_VEHICLE_WAIT) {
            bufCar.mtype = REQ_CAR_WAIT;
            bufCar.pid = localpid;
            bufCar.data = 0;
            msgSend(queueWait, &bufCar, length, 0);
            printf("CARCARCARCARCARCAR    Car %d is moved into the waiting queue\n", localpid);
            msgReceive(queueOnBoard, &bufCar, length, localpid, 0);
        }
    }

    printf("CARCARCARCARCARCAR    Car %d is boarded\n", localpid);
    msgReceive(queueUnloading, &bufCar, length, TYPE_CAR, 0);
    printf("CARCARCARCARCARCAR    Car %d is unloaded\n", localpid);
    bufCar.mtype = ACK_LEAVE;
    bufCar.pid = localpid;
    bufCar.data = TYPE_CAR;
    msgSend(queueLeave, &bufCar, length, 0);
}

void truck() {
    pid_t localpid = getpid();
    printf("TRUCKTRUCKTRUCKTRU    Truck %d comes\n", localpid);
    setpgid(localpid, groupTruck);

    bufTruck.mtype = REQ_TRUCK_ARRIVE;
    bufTruck.pid = localpid;
    bufTruck.data = TYPE_TRUCK;
    msgSend(queueToCaptain, &bufTruck, length, 0);

    msgReceive(queueToVehicle, &bufTruck, length, localpid, 0);
//     find out whether the vehicle is late
    if (bufTruck.data == RPL_VEHICLE_WAIT) {
        bufTruck.mtype = REQ_TRUCK_WAIT;
        bufTruck.pid = localpid;
        bufTruck.data = 0;
        msgSend(queueWait, &bufTruck, length, 0);
        msgReceive(queueOnBoard, &bufTruck, length, localpid, 0);

    } else if (bufTruck.data == RPL_VEHICLE_LATE) {
        bufTruck.mtype = REQ_TRUCK_LATE;
        bufTruck.pid = localpid;
        bufTruck.data = 0;
        msgSend(queueLate, &bufTruck, length, 0);

        msgReceive(queueLate, &bufTruck, length, localpid, 0);

        if (bufTruck.data == RPL_VEHICLE_WAIT) {
            bufTruck.mtype = REQ_TRUCK_WAIT;
            bufTruck.pid = localpid;
            bufTruck.data = 0;
            msgSend(queueWait, &bufTruck, length, 0);
            msgReceive(queueOnBoard, &bufTruck, length, localpid, 0);
        }
    }
    printf("TRUCKTRUCKTRUCKTRU    Truck %d is boarded\n", localpid);
    msgReceive(queueUnloading, &bufTruck, length, TYPE_TRUCK, 0);
    printf("TRUCKTRUCKTRUCKTRU    Truck %d is unloaded\n", localpid);
    bufTruck.mtype = ACK_LEAVE;
    bufTruck.pid = localpid;
    bufTruck.data = TYPE_TRUCK;
    msgSend(queueLeave, &bufTruck, length, 0);
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

    usleep(2e6);
    int status;
    pid_t waited;
    wait(NULL);
    printf("All processed ends\n");

}

int main(void) {
    pid_t localpid = getpid();

    // define the length of message
    length = sizeof(mess_t) - sizeof(long);
//    printf("size of struct: %d\n", sizeof(mess_t));
//    printf("size of long: %d\n", sizeof(long));
//    printf("size of mtype: %d\n", sizeof(bufMain.mtype));
//    printf("size of pid_t: %d\n", sizeof(bufMain.pid));
//    printf("size of data: %d\n", sizeof(bufMain.data));
//    printf("length:%d\n", length);

    printf("Input the maximum interval (integer, us) of the arrival of each vehicle:");
    scanf("%lld", &maxArriveInterval);
    printf("Input the probability of truck [0,100):");
    scanf("%d", &truckProb);

    // start time
    gettimeofday(&last, NULL);

    //buffer in main
    pid_t pid;
    createQueue();
    //child process the captain
    if (!(pid = fork())) captain();
    else {
        processCaptain = pid;
        int zombieTick = 0;
        while (1) {
            //check termination condition
            //and kill all the processes
            if (msgReceive(queueToMain, &bufMain, length, REQ_TERMINATE, IPC_NOWAIT) != -1) {
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
                lastArrive += rand() % maxArriveInterval;
                // check zombie Process
                zombieTick++;
                if (zombieTick % 10 == 0) {
                    int pd;
                    int sts;
                    while ((pd = waitpid(-1, &sts, WNOHANG)) > 1) {
                        printf("                   REAP exited process %d\n", pd);
                    }
                    zombieTick = 0;
                }
            }
        }
        srand(time(NULL));
        int isTruck = rand() % 100;
//        printf("isTruck: %d\n", isTruck);
        if (isTruck <= truckProb) truck();
        else car();
    }
}