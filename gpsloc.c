#include "headers.h"
pthread_mutex_t lock;
cumulative* vehicleshm;
void* gps_navigation(void* arg){
	pthread_mutex_lock(&lock);
	gps data;
	/*printf("Enter the longitude and the latitude of current location: ");
	scanf("%f %f",&data.src.latitude,&data.src.longitude);*/
	data.src.latitude=0.0;
	data.src.longitude=0.0;
	printf("Enter the longitude and the latitude of the destination: ");
	scanf("%f %f",&data.dest.latitude,&data.dest.longitude);
	srand(time(NULL));
	while(1){
		
		//gnrl g;
		//g.speed=(float)(rand()%120);
		//printf("Speed: %.2f\n",g.speed);
		/*if(data.dest.latitude<data.src.latitude || data.dest.longitude<data.src.longitude){
			data.src.latitude=data.dest.latitude;
			data.src.longitude=data.dest.longitude;
		}*/
		if(data.src.latitude<data.dest.latitude){
			data.src.latitude+=(((float)rand()/RAND_MAX)*5);
			//data.src.latitude+=((g.speed)/10);
		}
		if(data.src.longitude<data.dest.longitude){
			data.src.longitude+=(((float)rand()/RAND_MAX)*5);
			
			//data.src.longitude+=((g.speed)/10);
		}
		if(data.src.longitude>=data.dest.longitude && data.src.latitude>=data.dest.latitude){
			data.src.latitude=data.dest.latitude;
			data.src.longitude=data.dest.longitude;
			data.dist=0;
			printf("Current location: Latitude: %f Longitude: %f Distance: %lf\n",data.src.latitude,data.src.longitude,data.dist);
			break;
		}
		data.dist=(double)sqrt(pow((data.dest.latitude-data.src.latitude),2)+pow((data.dest.longitude-data.src.longitude),2));
		printf("Current location: Latitude: %f Longitude: %f Distance: %lf\n",data.src.latitude,data.src.longitude,data.dist);
		sleep(1);
	}
	printf("Location reached\n");
	memcpy(&(vehicleshm->g),&data,sizeof(data));
	pthread_mutex_unlock(&lock);
	return NULL;
}

	
int main(){
	key_t vehicle_key=8108;
	int vehicle_shmid=shmget(vehicle_key,sizeof(cumulative),0777|IPC_CREAT);
	if(vehicle_shmid==-1){
		printf("Error creatong shrd mem for vehicle");
		return 1;
	}
	vehicleshm=(cumulative*)shmat(vehicle_shmid,NULL,0);
	if(vehicleshm==(cumulative*)-1){
		printf("Error attaching vehicle shm mem");
		return 1;
	}
	pthread_t gps_thread;
	pthread_mutex_init(&lock,NULL);
	pthread_create(&gps_thread,NULL,gps_navigation,NULL);
	pthread_join(gps_thread,NULL);
	return 0;
}
	
