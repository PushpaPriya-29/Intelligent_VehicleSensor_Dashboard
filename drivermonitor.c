#include "headers.h"
drowsiness d;
blind_spot bs;
pthread_mutex_t lock;
cumulative* vehicleshm;
void* driver_drowsiness(void* arg){
	
	//srand(time(NULL));
	while(1){
		pthread_mutex_lock(&lock);
		d.eye_closure=((float)rand()/RAND_MAX)*100;
		d.yawn=((float)rand()/RAND_MAX)*101;
		if(d.eye_closure>75 || d.yawn>60){
			d.drowsy_flag=1;
			printf("ALERT: Driver feeling drowsy!!\n");
		}
		else{
			d.drowsy_flag=0;
		}
		printf("Eye closure: %.2f%% Yawn_level: %.2f%% Flag: %d\n",d.eye_closure,d.yawn,d.drowsy_flag);
		memcpy(&(vehicleshm->d),&d,sizeof(d));
		pthread_mutex_unlock(&lock);
		sleep(1);
	}
	return NULL;
}
void* blindspot(void*){
	
	
	while(1){
		pthread_mutex_lock(&lock);
		bs.leftblind=rand()%2;
		bs.rightblind=rand()%2;
		if(bs.leftblind || bs.rightblind){
			bs.blindspot_flag=1;
			printf("Vehicle detected in blindspot!\n");
		}
		else{
			bs.blindspot_flag=0;
		}
		printf("Blindspot: %d %d Flag: %d\n",bs.leftblind,bs.rightblind,bs.blindspot_flag);
		memcpy(&(vehicleshm->bs),&bs,sizeof(bs));
		pthread_mutex_unlock(&lock);
		sleep(1);
	}
	return NULL;
}
int main(){
	srand(time(NULL));
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
	pthread_mutex_init(&lock,NULL);
	pthread_t drowsy_thread,bspot_thread;
	//while(1){
		pthread_create(&drowsy_thread,NULL,driver_drowsiness,NULL);
		pthread_create(&bspot_thread,NULL,blindspot,NULL);
	
		pthread_join(drowsy_thread,NULL);
		pthread_join(bspot_thread,NULL);
		/*if(d.drowsy_flag==1){
			printf("ALERT: Driver feeling drowsy!!\n");
		}
		if(bs.blindspot_flag==1){
			printf("Vehicle detected in blindspot!\n");
		}*/
		sleep(1);
	//}	
	return 0;
}