#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "headers.h"
#define PORT 8080
cumulative* vehicleshm;
int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    
    key_t vehicle_key = 8108;
    int vehicle_shmid = shmget(vehicle_key, sizeof(cumulative), 0777 | IPC_CREAT);
    if (vehicle_shmid == -1) {
        printf("Error creating shared mem for vehicle\n");
        return 1;
    }
    vehicleshm = (cumulative*)shmat(vehicle_shmid, NULL, 0);
    
    
    char buffer[1024] = {0};

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation error");
        return -1;
    }

    // Set server address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 address from text to binary
    if (inet_pton(AF_INET, "192.168.1.118", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        return -1;
    }

    // Connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }
    while(1){
    	if (vehicleshm->g.dest.latitude == 0.0 && vehicleshm->g.dest.longitude == 0.0) {
            sleep(1);
            continue;
        }
        
    	// Send message to server
		send(sock, &(vehicleshm->g), sizeof(vehicleshm->g), 0);
		sleep(1);
		if(vehicleshm->g.reached_flag==1){
			break;
		}
		// Read response from server
    }

    close(sock);
    return 0;
}

