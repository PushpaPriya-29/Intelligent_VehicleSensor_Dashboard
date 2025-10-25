#include "headers.h"

pthread_mutex_t data_lock;
collision c;
autobrake a;

cumulative* vehicleshm;
alert_message* alertshm;

void handler(int sig) {
    if (sig == SIGUSR1) {
        printf("Cruise control mode ON.\n");
        c.cruise_flag = 1;
    }
}

void* coll(void* arg) {
    while (1) {
        if (a.brake_status == 0) {
            pthread_mutex_lock(&data_lock);
            
            c.collisionflag = 0;
            c.distance = 10 + ((float)rand() / RAND_MAX) * 90;
            
            // Speed control based on cruise flag
            if (c.cruise_flag == 0) {
                c.relativeSpeed = 20 + ((float)rand() / RAND_MAX) * 80;
            }
            
            // Collision detection
            if (c.distance < 30 && c.relativeSpeed > 50) {
                c.collisionflag = 1;
                add_alert(alertshm, "🚨 COLLISION WARNING!", CRITICAL);
            } else {
                clear_alert_by_prefix(alertshm, "COLLISION WARNING");
            }
            
            // Disable cruise if collision or low distance
            if (c.distance < 30 || c.collisionflag == 1) {
                if (c.cruise_flag == 1) {
                    c.cruise_flag = 0;
                    printf("Cruise control DISABLED due to low distance or collision!\n");
                }
            }
            
            printf("Distance: %.1f m, Speed: %.1f km/h, Collision Flag: %d\n",
                   c.distance, c.relativeSpeed, c.collisionflag);
            
            memcpy(&(vehicleshm->c), &c, sizeof(c));
            pthread_mutex_unlock(&data_lock);
            sleep(1);
        } else {
            sleep(1);
        }
    }
    return NULL;
}

void* brake(void* arg) {
    while (1) {
        pthread_mutex_lock(&data_lock);
        
        // Auto-brake logic
        if (c.relativeSpeed > 80 || c.collisionflag == 1) {
            a.brake_status = 1;
            add_alert(alertshm, "🛑 AUTO-BRAKE ENGAGED!", WARNING);
            c.relativeSpeed -= 5;
            
            if (c.collisionflag == 1) {
                c.cruise_flag = 0;
            }
            
            if (c.relativeSpeed < 45) {
                c.collisionflag = 0;
            }
        } else if (c.relativeSpeed < 45) {
            c.collisionflag = 0;
            a.brake_status = 0;
            clear_alert_by_prefix(alertshm, "AUTO-BRAKE");
        } else {
            a.brake_status = 0;
            clear_alert_by_prefix(alertshm, "AUTO-BRAKE");
        }
        
        printf("Brake - Speed: %.1f km/h, Brake Status: %d\n", c.relativeSpeed, a.brake_status);
        memcpy(&(vehicleshm->a), &a, sizeof(a));
        pthread_mutex_unlock(&data_lock);
        sleep(1);
    }
    return NULL;
}

void* lane(void* arg) {
    lane_departure l;
    while (1) {
        pthread_mutex_lock(&data_lock);
        l.laneflag = rand() % 2;
        
        if (l.laneflag == 1) {
            printf("Lane deviation detected!\n");
            add_alert(alertshm, "⚠ LANE DEPARTURE DETECTED!", WARNING);
        } else {
            clear_alert_by_prefix(alertshm, "LANE DEPARTURE");
        }
        
        memcpy(&(vehicleshm->l), &l, sizeof(l));
        pthread_mutex_unlock(&data_lock);
        sleep(5);
    }
    return NULL;
}

int main() {
    srand(time(NULL));
    
    // Attach shared memory
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
    
    // Setup signal handler for cruise control
    signal(SIGUSR1, handler);
    
    // Write PID to FIFO for cruise control communication
    pid_t pid = getpid();
    int fd = open("cuise_collision_fifo", O_WRONLY);
    if (fd >= 0) {
        write(fd, &pid, sizeof(pid));
        close(fd);
    }
    /*int fd=open("pid_share.bin",O_CREAT|O_TRUNC|O_WRONLY,0777);
    if(fd<0){
    	perror("failed to open\n");
    	return 1;
    }
    write(fd,&pid,sizeof(pid));*/
    
    // Initialize
    a.brake_status = 0;
    c.relativeSpeed = 50.0;
    c.distance = 100.0;
    c.collisionflag = 0;
    c.cruise_flag = 0;
    // Create threads
    pthread_t col_thread, brake_thread, lane_thread;
    pthread_mutex_init(&data_lock, NULL);
    
    pthread_create(&col_thread, NULL, coll, NULL);
    pthread_create(&brake_thread, NULL, brake, NULL);
    pthread_create(&lane_thread, NULL, lane, NULL);
    
    pthread_join(col_thread, NULL);
    pthread_join(brake_thread, NULL);
    pthread_join(lane_thread, NULL);
    
    pthread_mutex_destroy(&data_lock);
    return 0;
}
