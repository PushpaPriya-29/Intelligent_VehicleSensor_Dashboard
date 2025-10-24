
#include "headers.h"



pthread_mutex_t lock;
Parking park_data;
cumulative* vehicleshm;
// Function to generate random distances for front/rear sensors
void* sensor_thread(void* arg) {
    unsigned int seed = time(NULL) ^ getpid();
    while (!park_data.parking_flag) {
        pthread_mutex_lock(&lock);

        park_data.front_distance = ((float)(rand_r(&seed) % 1000)) / 10.0f; // 0–100 cm
        park_data.rear_distance  = ((float)(rand_r(&seed) % 1000)) / 10.0f; // 0–100 cm

        pthread_mutex_unlock(&lock);
        usleep(500000); // update every 0.5s
    }
    return NULL;
}

// Function to assist the driver based on distance readings
void* assist_thread(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);

        float front = park_data.front_distance;
        float rear  = park_data.rear_distance;
        int parked  = park_data.parking_flag;
	memcpy(&(vehicleshm->p),&park_data,sizeof(park_data));
        pthread_mutex_unlock(&lock);

        // Decision logic
        if (parked == 1) {
            printf("✅ Vehicle Parked Safely.\n");
            break;
        } 
        else if (rear < 10.0) {
            printf("🟥 STOP! Rear distance = %.2f cm. You are too close.\n", rear);
            pthread_mutex_lock(&lock);
            park_data.parking_flag = 1; // set as parked
            pthread_mutex_unlock(&lock);
        } 
        else if (rear < 30.0) {
            printf("🟧 Caution: Rear distance = %.2f cm. Slow down.\n", rear);
        } 
        else if (front < 20.0) {
            printf("🟡 Warning: Object in front (%.2f cm). Don’t move forward.\n", front);
        } 
        else {
            printf("🟢 Safe to move. Front = %.2f cm, Rear = %.2f cm\n", front, rear);
        }

        sleep(1);
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    pthread_mutex_init(&lock, NULL);
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
    park_data.front_distance = 100.0;
    park_data.rear_distance = 100.0;
    park_data.parking_flag = 0;

    pthread_t sensor_tid, assist_tid;
    pthread_create(&sensor_tid, NULL, sensor_thread, NULL);
    pthread_create(&assist_tid, NULL, assist_thread, NULL);

    pthread_join(sensor_tid, NULL);
    pthread_join(assist_tid, NULL);

    pthread_mutex_destroy(&lock);
    return 0;
}
