#include "headers.h"

pthread_mutex_t lock;
Parking park_data;
cumulative* vehicleshm;
alert_message* alertshm;

void* sensor_thread(void* arg) {
    unsigned int seed = time(NULL) ^ getpid();
    while (!park_data.parking_flag) {
        pthread_mutex_lock(&lock);
        
        park_data.front_distance = ((float)(rand_r(&seed) % 1000)) / 10.0f;
        park_data.rear_distance = ((float)(rand_r(&seed) % 1000)) / 10.0f;
        
        pthread_mutex_unlock(&lock);
        usleep(500000);
    }
    return NULL;
}

void* assist_thread(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        
        float front = park_data.front_distance;
        float rear = park_data.rear_distance;
        int parked = park_data.parking_flag;
        
        memcpy(&(vehicleshm->p), &park_data, sizeof(park_data));
        
        // Clear previous parking alerts
        clear_alert_by_prefix(alertshm, "\U0001F7E5 STOP");
        clear_alert_by_prefix(alertshm, "\U0001F7E7 Caution");
        clear_alert_by_prefix(alertshm, "\U0001F7E1 Warning");
        clear_alert_by_prefix(alertshm, "\U0001F7E2 Safe");
        
        pthread_mutex_unlock(&lock);
        
        // Decision logic with alerts
        if (parked == 1) {
            printf("\U00002705 Vehicle Parked Safely.\n");
            clear_alert_by_prefix(alertshm, "\U0001F7E5");
            clear_alert_by_prefix(alertshm, "\U0001F7E7");
            clear_alert_by_prefix(alertshm, "\U0001F7E1");
            add_alert(alertshm, "\U00002705 VEHICLE PARKED SAFELY", INFO);
            break;
        } else if (rear < 10.0) {
            printf("\U0001F7E5 STOP! Rear distance = %.2f cm. You are too close.\n", rear);
            add_alert(alertshm, "\U0001F7E5 STOP! Too close to rear obstacle!", CRITICAL);
            pthread_mutex_lock(&lock);
            park_data.parking_flag = 1;
            pthread_mutex_unlock(&lock);
        } else if (rear < 30.0) {
            printf("\U0001F7E7 Caution: Rear distance = %.2f cm. Slow down.\n", rear);
            add_alert(alertshm, "\U0001F7E7 Caution: Slow down, approaching obstacle", WARNING);
        } else if (front < 20.0) {
            printf("\U0001F7E1 Warning: Object in front (%.2f cm). Don't move forward.\n", front);
            add_alert(alertshm, "\U0001F7E1 Warning: Object detected in front", WARNING);
        } else {
            printf("\U0001F7E2 Safe to move. Front = %.2f cm, Rear = %.2f cm\n", front, rear);
            add_alert(alertshm, "\U0001F7E2 Safe to move - All clear", INFO);
        }
        
        sleep(1);
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    pthread_mutex_init(&lock, NULL);
    
    key_t vehicle_key = 8108;
    int vehicle_shmid = shmget(vehicle_key, sizeof(cumulative), 0777 | IPC_CREAT);
    if (vehicle_shmid == -1) {
        printf("Error creating shared mem for vehicle\n");
        return 1;
    }
    vehicleshm = (cumulative*)shmat(vehicle_shmid, NULL, 0);
    
    key_t alert_key = 9999;
    int alert_shmid = shmget(alert_key, sizeof(alert_message), 0777 | IPC_CREAT);
    if (alert_shmid == -1) {
        printf("Error creating alert shared mem\n");
        return 1;
    }
    alertshm = (alert_message*)shmat(alert_shmid, NULL, 0);
    
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
