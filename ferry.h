//
// Created by Bernard Yuan on 2016-07-24.
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

/* States for captain */
#define DOCKED_LOADING 0
#define SAILING 1
#define DOCKED_UNLOADING 2
#define SAILING_BACK 3
#define IDLE 4

/* Message TYPES */
#define CAR_ARRIVING 1
#define TRUCK_ARRIVING 2
#define CAR_LOADING 3
#define TRUCK_LOADING 4
#define CAR_LOADED 5
#define TRUCK_LOADED 6
#define CAR_UNLOADING 7
#define TRUCK_UNLOADING 8
#define CAR_UNLOADED 9
#define TRUCK_UNLOADED 10
#define START_LOADING 12
#define LOADING_COMPLETE 13
#define LOADING_COMPLETE_ACK 14
#define UNLOADING_COMPLETE 15
#define UNLOADING_COMPLETE_ACK 15
#define FERRY_ARRIVED 20
#define FERRY_ARRIVED_ACK 20
#define TRUCK_ARRIVED 21
#define CAR_ARRIVED 22
#define FERRY_RETURNED 23
#define FERRY_RETURNED_ACK 23
#define CAR_TRAVELING 24
#define TRUCK_TRAVELING 25
#define TERMINATION 49

/* Constants */
#define TOTAL_SPOTS_ON_FERRY 6
#define MAX_LOADS 3
#define CROSSING_TIME 1000000

/* Redefines the message structure */
typedef struct mymsgbuf {
    long mtype;
    int arrivalInfo[2];
} mess_t;

int timeChange(const struct timeval startTime);

#endif //FERRYQUEUE_FERRY_H
