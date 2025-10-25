#include "headers.h"

drowsiness d;
blind_spot bs;
pthread_mutex_t lock;
cumulative* vehicleshm;
alert_message* alertshm;

void* driver_drowsiness(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        
        d.eye_closure = ((float)rand() / RAND_MAX) * 100;
        d.yawn = ((float)rand() / RAND_MAX) * 101;
        
        if (d.eye_closure > 75 || d.yawn > 60) {
            d.drowsy_flag = 1;
            printf("ALERT: Driver feeling drowsy!!\n");
            add_alert(alertshm, "😴 DRIVER DROWSINESS ALERT!", CRITICAL);
        } else {
            d.drowsy_flag = 0;
            clear_alert_by_prefix(alertshm, "DRIVER DROWSINESS");
        }
        
        printf("Eye closure: %.2f%% Yawn_level: %.2f%% Flag: %d\n", 
               d.eye_closure, d.yawn, d.drowsy_flag);
        memcpy(&(vehicleshm->d), &d, sizeof(d));
        
        pthread_mutex_unlock(&lock);
        sleep(1);
    }
    return NULL;
}

void* blindspot(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        
        bs.leftblind = rand() % 2;
        bs.rightblind = rand() % 2;
        
        if (bs.leftblind || bs.rightblind) {
            bs.blindspot_flag = 1;
            printf("Vehicle detected in blindspot!\n");
            add_alert(alertshm, "👁 VEHICLE IN BLIND SPOT!", WARNING);
        } else {
            bs.blindspot_flag = 0;
            clear_alert_by_prefix(alertshm, "VEHICLE IN BLIND SPOT");
        }
        
        printf("Blindspot: %d %d Flag: %d\n", bs.leftblind, bs.rightblind, bs.blindspot_flag);
        memcpy(&(vehicleshm->bs), &bs, sizeof(bs));
        
        pthread_mutex_unlock(&lock);
        sleep(1);
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    
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
    
    pthread_mutex_init(&lock, NULL);
    pthread_t drowsy_thread, bspot_thread;
    
    pthread_create(&drowsy_thread, NULL, driver_drowsiness, NULL);
    pthread_create(&bspot_thread, NULL, blindspot, NULL);
    
    pthread_join(drowsy_thread, NULL);
    pthread_join(bspot_thread, NULL);
    
    pthread_mutex_destroy(&lock);
    return 0;
}
