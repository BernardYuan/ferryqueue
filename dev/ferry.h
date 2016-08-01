//
// Created by Bernard Yuan on 2016-07-30.
//

#ifndef FERRYQUEUE_FERRY_H
#define FERRYQUEUE_FERRY_H
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/types.h>
#include <sys/time.h>

#define SIZE_CAR 1
#define SIZE_TRUCK 2
#define MAXNUM_TRUCK 2
#define MAXSIZE_FERRY 6
#define MAX_LOAD 11
#define SAIL_TIME 1e6

#define TYPE_CAR    1
#define TYPE_TRUCK  2
#define REQ_TRUCK_ARRIVE 3
#define REQ_CAR_ARRIVE 4
#define RPL_VEHICLE_WAIT 4
#define RPL_VEHICLE_LATE 5
#define REQ_TRUCK_WAIT 6
#define REQ_TRUCK_LATE 7
#define REQ_CAR_WAIT 8
#define REQ_CAR_LATE 9
#define RPL_BOARDING 10
#define ACK_BOARDED 11
#define REQ_UNLOAD 12
#define ACK_UNLOADED 13

typedef struct mymsgbuf {
    long mtype;
    pid_t pid;
    int data;
} mess_t;

#define START_LOADING 1

#endif //FERRYQUEUE_FERRY_H
