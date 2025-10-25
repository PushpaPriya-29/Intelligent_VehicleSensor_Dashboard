#ifndef HEADERS_H
#define HEADERS_H

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<time.h>
#include<semaphore.h>
#include<pthread.h>
#include<sys/wait.h>
#include<sys/msg.h>
#include<sys/shm.h>
#include<fcntl.h>
#include<signal.h>
#include<math.h>
#include<string.h>

#define MAX_ALERTS 20
#define MAX_ALERT_MSG 100

// Alert severity levels
typedef enum {
    INFO,
    WARNING,
    CRITICAL
} Severity;

// Individual alert structure
typedef struct {
    int active;                    // 1 if alert is active, 0 otherwise
    Severity severity;             // INFO, WARNING, or CRITICAL
    char message[MAX_ALERT_MSG];   // Alert message
} Alert;

// Alert shared memory structure
typedef struct {
    Alert alerts[MAX_ALERTS];
    int alert_count;
} alert_message;

typedef struct collision{
    float distance;
    float relativeSpeed;
    int collisionflag;
    int cruise_flag;
}collision;

typedef struct lane_departure{
    int laneflag;
}lane_departure;

typedef struct autobrake{
    int brake_status;
}autobrake;

typedef struct blind_spot{
    int leftblind;
    int rightblind;
    int blindspot_flag;
}blind_spot;

typedef struct {
    float front_distance;
    float rear_distance;
    int parking_flag;
} Parking;

typedef struct drowsiness{
    float eye_closure;
    float yawn;
    int drowsy_flag;
}drowsiness;

typedef struct location{
    float latitude;
    float longitude;
}loc;

typedef struct gps{
    loc src;
    loc dest;
    double dist;
    int reached_flag;
}gps;

typedef struct vehicleData {
    float tyre_pressure;
    float fuel_level;
    int engine_rpm;
    float engine_temp;
    int battery;
    int engine_flag;
    int pressure_flag;
    int fuel_flag;
}vehicle;

typedef struct EnvData {
    float temp;
    float humid;
    float rain;
    float fog;
    int env_flag;
}env;

typedef struct cumulative{
    collision c;
    lane_departure l;
    autobrake a;
    blind_spot bs;
    vehicle v;
    drowsiness d;
    Parking p;
    gps g;
    env e;
}cumulative;

// Helper function to add alert
void add_alert(alert_message* alertshm, const char* msg, Severity sev);
void clear_all_alerts(alert_message* alertshm);
void clear_alert_by_prefix(alert_message* alertshm, const char* prefix);

#endif
