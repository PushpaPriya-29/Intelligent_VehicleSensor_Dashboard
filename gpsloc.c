#include "headers.h"

pthread_mutex_t lock;
cumulative* vehicleshm;
alert_message* alertshm;

void* gps_navigation(void* arg) {
    srand(time(NULL));
    
    while (1) {
        pthread_mutex_lock(&lock);
        
        // Wait until destination is set (not 0,0)
        if (vehicleshm->g.dest.latitude == 0.0 && vehicleshm->g.dest.longitude == 0.0) {
            pthread_mutex_unlock(&lock);
            sleep(1);
            continue;
        }
        
        // Check if already reached
        if (vehicleshm->g.src.latitude >= vehicleshm->g.dest.latitude && 
            vehicleshm->g.src.longitude >= vehicleshm->g.dest.longitude) {
            vehicleshm->g.src.latitude = vehicleshm->g.dest.latitude;
            vehicleshm->g.src.longitude = vehicleshm->g.dest.longitude;
            vehicleshm->g.dist = 0;
            vehicleshm->g.reached_flag = 1;
            printf("Location reached\n");
            add_alert(alertshm, "📍 DESTINATION REACHED!", INFO);
            pthread_mutex_unlock(&lock);
            sleep(1);
            continue;
        }
        
        // Update current position towards destination
        if (vehicleshm->g.src.latitude < vehicleshm->g.dest.latitude) {
            vehicleshm->g.src.latitude += (((float)rand() / RAND_MAX) * 2);
            if (vehicleshm->g.src.latitude > vehicleshm->g.dest.latitude) {
                vehicleshm->g.src.latitude = vehicleshm->g.dest.latitude;
            }
        }
        
        if (vehicleshm->g.src.longitude < vehicleshm->g.dest.longitude) {
            vehicleshm->g.src.longitude += (((float)rand() / RAND_MAX) * 2);
            if (vehicleshm->g.src.longitude > vehicleshm->g.dest.longitude) {
                vehicleshm->g.src.longitude = vehicleshm->g.dest.longitude;
            }
        }
        
        // Calculate remaining distance
        vehicleshm->g.dist = (double)sqrt(
            pow((vehicleshm->g.dest.latitude - vehicleshm->g.src.latitude), 2) +
            pow((vehicleshm->g.dest.longitude - vehicleshm->g.src.longitude), 2)
        );
        
        printf("Current location: Latitude: %.2f Longitude: %.2f Distance: %.2lf\n",
               vehicleshm->g.src.latitude, vehicleshm->g.src.longitude, vehicleshm->g.dist);
        
        // Alert when close to destination
        if (vehicleshm->g.dist < 5.0 && vehicleshm->g.dist > 0.5) {
            add_alert(alertshm, "📍 Approaching destination...", INFO);
        } else {
            clear_alert_by_prefix(alertshm, "Approaching destination");
        }
        
        pthread_mutex_unlock(&lock);
        sleep(1);
    }
    
    return NULL;
}

int main() {
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
    
    // Initialize GPS data
    vehicleshm->g.src.latitude = 0.0;
    vehicleshm->g.src.longitude = 0.0;
    vehicleshm->g.dest.latitude = 0.0;
    vehicleshm->g.dest.longitude = 0.0;
    vehicleshm->g.dist = 0.0;
    vehicleshm->g.reached_flag = 0;
    
    pthread_t gps_thread;
    pthread_mutex_init(&lock, NULL);
    pthread_create(&gps_thread, NULL, gps_navigation, NULL);
    pthread_join(gps_thread, NULL);
    
    pthread_mutex_destroy(&lock);
    return 0;
}
