
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <locale.h>
#include <wchar.h>

// Shared structure for environmental sensor data
struct EnvData 
{
    float temp;
    float humid;
    float rain;
    float fog;
    int env_flag;
};
struct EnvData data;
pthread_mutex_t lock;
void* temp_humid_sensor(void* arg) 
{
    while (1) 
    {
        pthread_mutex_lock(&lock);
        data.temp = ((float)rand() / RAND_MAX) * 50;
        data.humid = ((float)rand() / RAND_MAX) * 100;
        pthread_mutex_unlock(&lock);
        sleep(1);
    }
    return NULL;
}
void* rain_sensor(void* arg) 
{
    while (1) 
    {
        pthread_mutex_lock(&lock);
        data.rain = ((float)rand() / RAND_MAX) * 100;
        pthread_mutex_unlock(&lock);
        sleep(1);
    }
    return NULL;
}
void* fog_sensor(void* arg) 
{
    while (1) 
    {
        pthread_mutex_lock(&lock);
        data.fog = ((float)rand() / RAND_MAX) * 100;
        pthread_mutex_unlock(&lock);
        sleep(1);
    }
    return NULL;
}

void* dashboard(void* arg) 
{
    setlocale(LC_ALL, "");
    while (1) 
    {
        pthread_mutex_lock(&lock);
        wprintf(L"\n--- Environmental Sensor Dashboard ---\n");
        wprintf(L"Temperature: %.2f\n", data.temp);
        wprintf(L"Humidity: %.2f\n", data.humid);
        wprintf(L"Rain Level: %.2f\n", data.rain);
        wprintf(L"Fog Level: %.2f\n", data.fog);

        data.env_flag = 0;
        if (data.temp < 20 || data.humid < 30) {
            wprintf(L"ALERT: Temperature and Humidity are Low 🔔 BUZZER ON\n");
            data.env_flag = 1;
        } else if (data.temp > 50 || data.humid > 70) {
            wprintf(L"ALERT: Temperature and Humidity are High 🔔 BUZZER ON\n");
            data.env_flag = 1;
        }

        if (data.rain > 70) {
            wprintf(L"ALERT: Heavy Rain Detected 🚨 SIREN ON\n");
            data.env_flag = 1;
        }

        if (data.fog > 60) {
            wprintf(L"ALERT: Dense Fog Detected 🚨 SIREN ON\n");
            data.env_flag = 1;
        }

        wprintf(L"Environment Flag: %d\n", data.env_flag);
        pthread_mutex_unlock(&lock);
        sleep(2);
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    pthread_mutex_init(&lock, NULL);

    pthread_t temp_thread, rain_thread, fog_thread, dashboard_thread;

    pthread_create(&temp_thread, NULL, temp_humid_sensor, NULL);
    pthread_create(&rain_thread, NULL, rain_sensor, NULL);
    pthread_create(&fog_thread, NULL, fog_sensor, NULL);
    pthread_create(&dashboard_thread, NULL, dashboard, NULL);

    pthread_join(temp_thread, NULL);
    pthread_join(rain_thread, NULL);
    pthread_join(fog_thread, NULL);
    pthread_join(dashboard_thread, NULL);

    pthread_mutex_destroy(&lock);
    return 0;
}

