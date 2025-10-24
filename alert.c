
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <locale.h>
#include <wchar.h>

// Shared structure for sensor data and flags
struct SensorData {
    float tyre_pressure;
    int pressure_flag;
    float fuel_level;
    int fuel_flag;
    int engine_rpm;
    float engine_temp;
    int battery;
    int engine_flag;
};

struct SensorData data;

pthread_mutex_t lock;

void* tyre_sensor(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        data.tyre_pressure = 20 + ((float)rand() / RAND_MAX) * 20;
        data.pressure_flag = (data.tyre_pressure < 25) ? 1 : 0;
        pthread_mutex_unlock(&lock);
        sleep(1);
    }
    return NULL;
}

void* fuel_sensor(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        data.fuel_level = ((float)rand() / RAND_MAX) * 100;
        data.fuel_flag = (data.fuel_level < 30) ? 1 : 0;
        pthread_mutex_unlock(&lock);
        sleep(1);
    }
    return NULL;
}

void* engine_sensor(void* arg) {
    while (1) {
        pthread_mutex_lock(&lock);
        data.engine_rpm = ((float)rand() / RAND_MAX) * 9000;
        data.engine_temp = ((float)rand() / RAND_MAX) * 100;
        data.battery = ((float)rand() / RAND_MAX) * 100;
        if(data.engine_rpm < 3000 || data.engine_temp < 20 || data.battery < 20) {
            data.engine_flag = 1;
        } else if((data.engine_rpm > 3000 && data.engine_rpm < 9000) &&
                  (data.engine_temp > 20 && data.engine_temp < 80) &&
                  (data.battery > 80)) {
            data.engine_flag = 0;
        } else {
            data.engine_flag = 1;
        }
        pthread_mutex_unlock(&lock);
        sleep(1);
    }
    return NULL;
}

void* dashboard(void* arg) {
    setlocale(LC_ALL, "");
    while (1) {
        pthread_mutex_lock(&lock);
        wprintf(L"\n --- Dashboard ---\n");
        wprintf(L" Tyre Pressure: %.2f, Flag: %d\n", data.tyre_pressure, data.pressure_flag);
        if (data.pressure_flag) wprintf(L" ALERT: Tyre Pressure Low! 🔔 BUZZER ON\n");

        wprintf(L" Fuel Level: %.2f%%, Flag: %d", data.fuel_level, data.fuel_flag);
        if (data.fuel_flag) wprintf(L" ALERT: Fuel Level Low! 🔔 BUZZER ON\n");

        wprintf(L" Engine RPM: %d\n", data.engine_rpm);
        wprintf(L" Engine Temp: %.2f\n", data.engine_temp);
        wprintf(L" Battery: %d%%\n", data.battery);
        wprintf(L" Engine Flag: %d\n", data.engine_flag);
        if (data.engine_flag) wprintf(L" ALERT: Engine Condition Not Optimal! 🚨 SIREN ON\n");

        pthread_mutex_unlock(&lock);
        sleep(2);
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    pthread_mutex_init(&lock, NULL);

    pthread_t tyre_thread, fuel_thread, engine_thread, dashboard_thread;

    pthread_create(&tyre_thread, NULL, tyre_sensor, NULL);
    pthread_create(&fuel_thread, NULL, fuel_sensor, NULL);
    pthread_create(&engine_thread, NULL, engine_sensor, NULL);
    pthread_create(&dashboard_thread, NULL, dashboard, NULL);

    pthread_join(tyre_thread, NULL);
    pthread_join(fuel_thread, NULL);
    pthread_join(engine_thread, NULL);
    pthread_join(dashboard_thread, NULL);

    pthread_mutex_destroy(&lock);
    return 0;
}

