#include "headers.h"

env data;
pthread_mutex_t lock;
cumulative* vehicleshm;
alert_message* alertshm;

void* temp_humid_sensor(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        data.temp = ((float)rand() / RAND_MAX) * 50;
        data.humid = ((float)rand() / RAND_MAX) * 100;
        pthread_mutex_unlock(&lock);
        sleep(1);
    }
    return NULL;
}

void* rain_sensor(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        data.rain = ((float)rand() / RAND_MAX) * 100;
        pthread_mutex_unlock(&lock);
        sleep(1);
    }
    return NULL;
}

void* fog_sensor(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        data.fog = ((float)rand() / RAND_MAX) * 100;
        pthread_mutex_unlock(&lock);
        sleep(1);
    }
    return NULL;
}

void* environmental_monitor(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        
        data.env_flag = 0;
        
        // Check temperature and humidity
        if (data.temp < 20 || data.humid < 30) {
            add_alert(alertshm, "🌡 Temperature/Humidity LOW!", WARNING);
            data.env_flag = 1;
        } else if (data.temp > 50 || data.humid > 70) {
            add_alert(alertshm, "🌡 Temperature/Humidity HIGH!", WARNING);
            data.env_flag = 1;
        } else {
            clear_alert_by_prefix(alertshm, "Temperature/Humidity");
        }
        
        // Check rain level
        if (data.rain > 70) {
            add_alert(alertshm, "🌧 HEAVY RAIN DETECTED!", WARNING);
            data.env_flag = 1;
        } else {
            clear_alert_by_prefix(alertshm, "HEAVY RAIN");
        }
        
        // Check fog level
        if (data.fog > 60) {
            add_alert(alertshm, "🌫 DENSE FOG DETECTED!", WARNING);
            data.env_flag = 1;
        } else {
            clear_alert_by_prefix(alertshm, "DENSE FOG");
        }
        
        printf("Temp: %.2f°C, Humid: %.2f%%, Rain: %.2f%%, Fog: %.2f%%, Flag: %d\n",
               data.temp, data.humid, data.rain, data.fog, data.env_flag);
        
        memcpy(&(vehicleshm->e), &data, sizeof(data));
        pthread_mutex_unlock(&lock);
        sleep(2);
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

    pthread_t temp_thread, rain_thread, fog_thread, monitor_thread;

    pthread_create(&temp_thread, NULL, temp_humid_sensor, NULL);
    pthread_create(&rain_thread, NULL, rain_sensor, NULL);
    pthread_create(&fog_thread, NULL, fog_sensor, NULL);
    pthread_create(&monitor_thread, NULL, environmental_monitor, NULL);

    pthread_join(temp_thread, NULL);
    pthread_join(rain_thread, NULL);
    pthread_join(fog_thread, NULL);
    pthread_join(monitor_thread, NULL);

    pthread_mutex_destroy(&lock);
    return 0;
}
