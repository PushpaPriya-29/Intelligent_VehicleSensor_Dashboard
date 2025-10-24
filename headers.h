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

typedef struct collision{
	float distance;
      	float relativeSpeed;
      	int collisionflag;
}collision;

typedef struct lane_departure{
      	int laneflag;
}lane_departure;

/*typedef struct cruise_control{
	float speed;
	float distance;
      	int cruise_button;
}cruise_control;*/

typedef struct autobrake{
      	int brake_status;
}autobrake;

typedef struct blind_spot{
	int leftblind;
	int rightblind;
      	int blindspot_flag;
}blind_spot;



typedef struct {
    float front_distance;   // cm
    float rear_distance;    // cm
    int parking_flag;       // 1 = parked safely, 0 = still moving
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
typedef struct EnvData 
{
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
#endif
