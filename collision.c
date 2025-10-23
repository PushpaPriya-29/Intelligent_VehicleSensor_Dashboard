#include "headers.h"
pthread_mutex_t data_lock;
sem_t channel_sem;
collision c;
autobrake a;
int cruise_flag=0;

void handler(int sig){
	if(sig==SIGUSR1){
		printf("Cruise control mode on.\n");
		cruise_flag=1;
	}
}


void* coll(void* arg){
	while(1){
		if(a.brake_status==0){
			pthread_mutex_lock(&data_lock);
			c.collisionflag=0;
			c.distance=0+((float)rand()/RAND_MAX)*100;
			if(cruise_flag==0){
				c.relativeSpeed=0+((float)rand()/RAND_MAX)*100;
			}
			if(c.distance<30 && c.relativeSpeed>50){
				c.collisionflag=1;
				
			}
			
			printf("Distance:%f, speed:%f, flag:%d\n",c.distance,c.relativeSpeed,c.collisionflag);
			pthread_mutex_unlock(&data_lock);
			sleep(1);
		}
	}
	return NULL;	
}

void* brake(void* arg){
	while(1){
		pthread_mutex_lock(&data_lock);
		
		if(c.relativeSpeed>80||c.collisionflag==1){
			a.brake_status=1;
			c.relativeSpeed-=5;
			if(c.collisionflag==1){
					cruise_flag=0;
				}
			if(c.relativeSpeed<30){
				c.collisionflag=0;
			}
		}
		else if(c.relativeSpeed<30){
			c.collisionflag=0;
			a.brake_status=0;
		}
		else{
			a.brake_status=0;
		}
		printf("speed:%f, status:%d\n",c.relativeSpeed,a.brake_status);
		pthread_mutex_unlock(&data_lock);
		sleep(1);
	}
	return NULL;
}

void* lane(void* arg){
	lane_departure l;
	while(1){
		l.laneflag=rand()%2;
		if(l.laneflag==1){
			printf("Lane deviation detected\n");
		}
		sleep(5);
	}
}


int main(){
	srand(time(NULL));
	signal(SIGUSR1,handler);
	pid_t pid = getpid();
	int fd=open("cuise_collision_fifo",O_WRONLY);
	write(fd,&pid,sizeof(pid));
	a.brake_status=0;
	pthread_t col_thread,brake_thread,lane_thread;
	pthread_mutex_init(&data_lock, NULL);
	pthread_create(&col_thread, NULL, coll, NULL);
	pthread_create(&brake_thread, NULL, brake, NULL);
	pthread_create(&lane_thread, NULL, lane, NULL);

	pthread_join(col_thread,NULL);
	pthread_join(brake_thread,NULL);
	pthread_join(lane_thread,NULL);
	
	pthread_mutex_destroy(&data_lock);

}
