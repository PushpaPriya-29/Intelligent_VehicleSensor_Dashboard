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

typedef struct collision{
	float distance;
      	float relativeSpeed;
      	int collisionflag;
}collision;

typedef struct lane_departure{
	int lane_pos;
      	int laneflag;
}lane_departure;

typedef struct cruise_control{
	float speed;
	float distance;
      	int cruise_button;
}cruise_control;

typedef struct auto_brake{
	float forcebrake;
      	int brake_status;
}auto_brake;

typedef struct blind_spot{
	int leftblind;
	int rightblind;
      	int blindspot_flag;
}blind_spot;

typedef struct tire_pressure{
	float pressure;
      	int pressure_flag;
}tire_pressure;

typedef struct env{
	float temp;
	float humid;
      	int env_flag;
}env;

typedef struct fuel_monitor{
	float fuel;
      	int fuel_flag;
}fuel_monitor;

typedef struct engine_health{
	float eng_temp;
      	int ehealth_flag;
}engine_health;

typedef struct parking{
	float front_dist;
	float rear_dist;
      	int park_mode;
}parking;

typedef struct drowsiness{
	float eye;
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
	float dist;
      	int reached_flag;
}gps;

typedef struct general{
	float speed;
	float fuel;
	float eng_health;
	env env_monitor;
}gnrl;

#endif
