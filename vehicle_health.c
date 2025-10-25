#include "headers.h"

vehicle data;
cumulative* vehicleshm;
alert_message* alertshm;
pthread_mutex_t lock;

void* tyre_sensor(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        
        // Generate random tyre pressure
        data.tyre_pressure = 20 + ((float)rand() / RAND_MAX) * 20;
        
        // Check and set flag
        if (data.tyre_pressure < 25) {
            data.pressure_flag = 1;
            add_alert(alertshm, "⚠ TYRE PRESSURE LOW!", CRITICAL);
        } else {
            data.pressure_flag = 0;
            clear_alert_by_prefix(alertshm, "TYRE PRESSURE");
        }
        
        pthread_mutex_unlock(&lock);
        sleep(1);
    }
    return NULL;
}

void* fuel_sensor(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        
        // Generate random fuel level
        data.fuel_level = ((float)rand() / RAND_MAX) * 100;
        
        // Check and set flag
        if (data.fuel_level < 30) {
            data.fuel_flag = 1;
            add_alert(alertshm, "⚠ FUEL LEVEL CRITICALLY LOW!", CRITICAL);
        } else {
            data.fuel_flag = 0;
            clear_alert_by_prefix(alertshm, "FUEL LEVEL");
        }
        
        pthread_mutex_unlock(&lock);
        sleep(1);
    }
    return NULL;
}

void* engine_sensor(void* arg) {
    // Attach to shared memory
    /*key_t vehicle_key = 8108;
    int vehicle_shmid = shmget(vehicle_key, sizeof(cumulative), 0777 | IPC_CREAT);
    if (vehicle_shmid == -1) {
        perror("Error creating shared mem for vehicle\n");
        return NULL;
    }
    vehicleshm = (cumulative*)shmat(vehicle_shmid, NULL, 0);
    
    key_t alert_key = 9999;
    int alert_shmid = shmget(alert_key, sizeof(alert_message), 0777 | IPC_CREAT);
    alertshm = (alert_message*)shmat(alert_shmid, NULL, 0);*/
    
    while (1) {
        pthread_mutex_lock(&lock);
        
        // Generate random engine data
        data.engine_rpm = ((float)rand() / RAND_MAX) * 9000;
        data.engine_temp = ((float)rand() / RAND_MAX) * 100;
        data.battery = ((float)rand() / RAND_MAX) * 100;
        
        // Check engine condition
        if (data.engine_rpm < 3000 || data.engine_temp < 20 || data.battery < 20) {
            data.engine_flag = 1;
            add_alert(alertshm, "🚨 ENGINE CONDITION NOT OPTIMAL!", CRITICAL);
        } else if ((data.engine_rpm > 3000 && data.engine_rpm < 9000) &&
                   (data.engine_temp > 20 && data.engine_temp < 80) &&
                   (data.battery > 80)) {
            data.engine_flag = 0;
            clear_alert_by_prefix(alertshm, "ENGINE CONDITION");
        } else {
            data.engine_flag = 1;
            add_alert(alertshm, "🚨 ENGINE CONDITION NOT OPTIMAL!", CRITICAL);
        }
        
        // Copy to shared memory
        memcpy(&(vehicleshm->v), &data, sizeof(vehicle));
        
        pthread_mutex_unlock(&lock);
        sleep(1);
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    
    // Initialize shared memory
    key_t vehicle_key = 8108;
    int vehicle_shmid = shmget(vehicle_key, sizeof(cumulative), 0777 | IPC_CREAT);
    vehicleshm = (cumulative*)shmat(vehicle_shmid, NULL, 0);
    
    key_t alert_key = 9999;
    int alert_shmid = shmget(alert_key, sizeof(alert_message), 0777 | IPC_CREAT);
    alertshm = (alert_message*)shmat(alert_shmid, NULL, 0);
    
    pthread_mutex_init(&lock, NULL);

    pthread_t tyre_thread, fuel_thread, engine_thread;

    pthread_create(&tyre_thread, NULL, tyre_sensor, NULL);
    pthread_create(&fuel_thread, NULL, fuel_sensor, NULL);
    pthread_create(&engine_thread, NULL, engine_sensor, NULL);

    pthread_join(tyre_thread, NULL);
    pthread_join(fuel_thread, NULL);
    pthread_join(engine_thread, NULL);

    pthread_mutex_destroy(&lock);
    return 0;
}
