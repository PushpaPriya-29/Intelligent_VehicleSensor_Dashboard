#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/socket.h>
#include <time.h>

#define GPS_PORT1 8080
#define GPS_PORT2 4444
#define HTTP_PORT 8081
#define BUFFER_SIZE 1024

// GPS data structure (matching your client)
typedef struct {
    float latitude;
    float longitude;
} loc;

typedef struct {
    loc src;
    loc dest;
    double dist;
    int reached_flag;
} gps;

// Global GPS data
gps current_gps;
pthread_mutex_t data_lock;
int data_received = 0;
char ch;

// Function to handle GPS client connection
void* handle_client(void* arg) {
	int client_socket = *(int*)arg;
    free(arg);
   

     printf("GPS Client connected!\n");
      
	while(!current_gps.reached_flag){
		pthread_mutex_lock(&data_lock);
		
    		// Read message from client
    		int bytesread=read(client_socket,&current_gps, sizeof(current_gps));
    		if(bytesread<=0){
    			printf("Client disconnected\n");
    			pthread_mutex_unlock(&data_lock);
    			break;
    		}
    		
   		 	
    data_received = 1;
    /*if((current_gps.dest.latitude!=0.0 && current_gps.dest.longitude!=0) && (current_gps.src.latitude==current_gps.dest.latitude) &&(current_gps.src.longitude==current_gps.dest.longitude)){
    	printf("Destination reached\n");
    	pthread_mutex_unlock(&data_lock);
    	close(client_socket);
    	//close(http_server);
    	sleep(2);
    	exit(0);
    }*/
    pthread_mutex_unlock(&data_lock);
    printf("Received from client:\nSource: Latitude: %f Longitude: %f\nDestination: Lat: %f Long: %f\nDistance:%lf Reached: %d\n", current_gps.src.latitude,current_gps.src.longitude,current_gps.dest.latitude,current_gps.dest.longitude,current_gps.dist,current_gps.reached_flag);

    // Send response to client
    //send(new_socket, message, strlen(message), 0);
   // printf("Message sent to client\n");
   sleep(1);
	}
    close(client_socket);
    return NULL;
}

// Function to handle HTTP requests (web page)
void* handle_http_client(void* arg) {
    int client_socket = *(int*)arg;
    free(arg);
    
    char buffer[BUFFER_SIZE];
    recv(client_socket, buffer, BUFFER_SIZE, 0);
    
    // Create simple HTML response
    char response[16384];
    char html_content[8192];
    
    pthread_mutex_lock(&data_lock);
    /*if((current_gps.dest.latitude!=0.0 && current_gps.dest.longitude!=0) && (current_gps.src.latitude==current_gps.dest.latitude) &&(current_gps.src.longitude==current_gps.dest.longitude)){
    	pthread_mutex_unlock(&data_lock);
    	close(client_socket);
    	return NULL;
    }*/
    char status[50];
    if((current_gps.dest.latitude!=0.0 && current_gps.dest.longitude!=0) && (current_gps.src.latitude==current_gps.dest.latitude) &&(current_gps.src.longitude==current_gps.dest.longitude)){
    	strcpy(status,"✅️Location reached!!");
    }
    else{
    	strcpy(status,"📍️Location yet to reach!");
    }
    if (data_received) {
        sprintf(html_content,
            "<!DOCTYPE html>\n"
            "<html>\n"
            "<head>\n"
            "    <meta charset='UTF-8'>\n"
            "    <meta http-equiv='refresh' content='1'>\n"
            "    <title>GPS Tracking Dashboard</title>\n"
            "    <style>\n"
            "        * { margin: 0; padding: 0; box-sizing: border-box; }\n"
            "        body {\n"
            "            font-family: 'Arial', sans-serif;\n"
            "            background: linear-gradient(135deg, #667eea 0%%, #764ba2 100%%);\n"
            "            min-height: 100vh;\n"
            "            display: flex;\n"
            "            justify-content: center;\n"
            "            align-items: center;\n"
            "            padding: 20px;\n"
            "        }\n"
            "        .container {\n"
            "            background: white;\n"
            "            border-radius: 20px;\n"
            "            box-shadow: 0 20px 60px rgba(0,0,0,0.3);\n"
            "            padding: 40px;\n"
            "            max-width: 600px;\n"
            "            width: 100%%;\n"
            "        }\n"
            "        h1 {\n"
            "            color: #667eea;\n"
            "            text-align: center;\n"
            "            margin-bottom: 30px;\n"
            "            font-size: 2.5em;\n"
            "        }\n"
            "        .data-section {\n"
            "            background: #f8f9fa;\n"
            "            border-radius: 10px;\n"
            "            padding: 20px;\n"
            "            margin-bottom: 20px;\n"
            "            border-left: 5px solid #667eea;\n"
            "        }\n"
            "        .data-section h2 {\n"
            "            color: #764ba2;\n"
            "            margin-bottom: 15px;\n"
            "            font-size: 1.5em;\n"
            "        }\n"
            "        .data-row {\n"
            "            display: flex;\n"
            "            justify-content: space-between;\n"
            "            padding: 10px 0;\n"
            "            border-bottom: 1px solid #dee2e6;\n"
            "        }\n"
            "        .data-row:last-child { border-bottom: none; }\n"
            "        .label {\n"
            "            font-weight: bold;\n"
            "            color: #495057;\n"
            "        }\n"
            "        .value {\n"
            "            color: #667eea;\n"
            "            font-size: 1.1em;\n"
            "            font-weight: bold;\n"
            "        }\n"
            "        .status {\n"
            "            text-align: center;\n"
            "            padding: 10px;\n"
            "            background: #d4edda;\n"
            "            color: #155724;\n"
            "            border-radius: 5px;\n"
            "            margin-top: 20px;\n"
            "            font-weight: bold;\n"
            "        }\n"
            "    </style>\n"
            "</head>\n"
            "<body>\n"
            "    <div class='container'>\n"
            "        <h1>🚗 GPS Tracking</h1>\n"
            "        \n"
            "        <div class='data-section'>\n"
            "            <h2>📍 Current Location</h2>\n"
            "            <div class='data-row'>\n"
            "                <span class='label'>Latitude:</span>\n"
            "                <span class='value'>%.6f</span>\n"
            "            </div>\n"
            "            <div class='data-row'>\n"
            "                <span class='label'>Longitude:</span>\n"
            "                <span class='value'>%.6f</span>\n"
            "            </div>\n"
            "        </div>\n"
            "        \n"
            "        <div class='data-section'>\n"
            "            <h2>🎯 Destination</h2>\n"
            "            <div class='data-row'>\n"
            "                <span class='label'>Latitude:</span>\n"
            "                <span class='value'>%.6f</span>\n"
            "            </div>\n"
            "            <div class='data-row'>\n"
            "                <span class='label'>Longitude:</span>\n"
            "                <span class='value'>%.6f</span>\n"
            "            </div>\n"
            "        </div>\n"
            "        \n"
            "        <div class='data-section'>\n"
            "            <h2>📏 Distance</h2>\n"
            "            <div class='data-row'>\n"
            "                <span class='label'>Remaining:</span>\n"
            "                <span class='value'>%.2f km</span>\n"
            "            </div>\n"
            "        </div>\n"
            "        \n"
            "        <div class='status'>%s</div>\n"
            "    </div>\n"
            "</body>\n"
            "</html>",
            current_gps.src.latitude,
            current_gps.src.longitude,
            current_gps.dest.latitude,
            current_gps.dest.longitude,
            current_gps.dist,status);
            
    } /*else {
        sprintf(html_content,
            "<!DOCTYPE html>\n"
            "<html>\n"
            "<head>\n"
            "    <meta charset='UTF-8'>\n"
            "    <meta http-equiv='refresh' content='2'>\n"
            "    <title>GPS Tracking - Waiting</title>\n"
            "    <style>\n"
            "        * { margin: 0; padding: 0; box-sizing: border-box; }\n"
            "        body {\n"
            "            font-family: 'Arial', sans-serif;\n"
            "            background: linear-gradient(135deg, #667eea 0%%, #764ba2 100%%);\n"
            "            min-height: 100vh;\n"
            "            display: flex;\n"
            "            justify-content: center;\n"
            "            align-items: center;\n"
            "        }\n"
            "        .waiting {\n"
            "            background: white;\n"
            "            border-radius: 20px;\n"
            "            padding: 60px;\n"
            "            text-align: center;\n"
            "            box-shadow: 0 20px 60px rgba(0,0,0,0.3);\n"
            "        }\n"
            "        h1 { color: #667eea; margin-bottom: 20px; }\n"
            "        .spinner {\n"
            "            border: 5px solid #f3f3f3;\n"
            "            border-top: 5px solid #667eea;\n"
            "            border-radius: 50%%;\n"
            "            width: 50px;\n"
            "            height: 50px;\n"
            "            animation: spin 1s linear infinite;\n"
            "            margin: 20px auto;\n"
            "        }\n"
            "        @keyframes spin {\n"
            "            0%% { transform: rotate(0deg); }\n"
            "            100%% { transform: rotate(360deg); }\n"
            "        }\n"
            "    </style>\n"
            "</head>\n"
            "<body>\n"
            "    <div class='waiting'>\n"
            "        <h1>⏳ Waiting for GPS Data...</h1>\n"
            "        <div class='spinner'></div>\n"
            "        <p>Please start the client on the car system</p>\n"
            "    </div>\n"
            "</body>\n"
            "</html>");
    }*/
    
    pthread_mutex_unlock(&data_lock);
    
    // Create HTTP response
    sprintf(response,
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html; charset=UTF-8\r\n"
        "Access-Control-Allow-Origin: *\r\n"
        "Connection: close\r\n\r\n"
        "%s", html_content);
    
    send(client_socket, response, strlen(response), 0);
    close(client_socket);
    return NULL;
}
void* handle_http_loop(void* arg){	
	int http_server=*(int*)arg;
	 while (1) {
            int* client_sock = malloc(sizeof(int));
            *client_sock = accept(http_server, NULL, NULL);
            
            pthread_t tid;
            pthread_create(&tid, NULL, handle_http_client, client_sock);
            pthread_detach(tid);
        }
        
}
int main() {
	
	printf("Enter\n1: To get car 1 details\t\n2: To get car 2 details\n");
	scanf(" %c",&ch);
	
    int gps_server, http_server;
    struct sockaddr_in gps_addr, http_addr;
    
    // Initialize mutex
    pthread_mutex_init(&data_lock, NULL);
    
    // Create GPS socket (for client.c connection)
    gps_server = socket(AF_INET, SOCK_STREAM, 0);
    if (gps_server < 0) {
        perror("GPS socket creation failed");
        exit(1);
    }
    
    int opt = 1;
    setsockopt(gps_server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    gps_addr.sin_family = AF_INET;
    gps_addr.sin_addr.s_addr = INADDR_ANY;
    if(ch=='1'){
    	gps_addr.sin_port = htons(GPS_PORT1);
    }
    else if(ch=='2'){
    	gps_addr.sin_port = htons(GPS_PORT2);
    }
    else{
    	printf("Invalid choice for port Number\n");
    }
    if (bind(gps_server, (struct sockaddr*)&gps_addr, sizeof(gps_addr)) < 0) {
        perror("GPS bind failed");
        exit(1);
    }
    
    if (listen(gps_server, 5) < 0) {
        perror("GPS listen failed");
        exit(1);
    }
    
    // Create HTTP socket (for web page)
    http_server = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(http_server, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    
    http_addr.sin_family = AF_INET;
    http_addr.sin_addr.s_addr = INADDR_ANY;
    http_addr.sin_port = htons(HTTP_PORT);
    
    if (bind(http_server, (struct sockaddr*)&http_addr, sizeof(http_addr)) < 0) {
        perror("HTTP bind failed");
        exit(1);
    }
    
    if (listen(http_server, 10) < 0) {
        perror("HTTP listen failed");
        exit(1);
    }
    
    printf("===========================================\n");
    printf("     GPS Tracking Server Started\n");
    printf("===========================================\n");
    if(ch=='1'){
    printf("GPS Data Port: %d\n", GPS_PORT1);
    }
    else if(ch=='2'){
    printf("GPS Data Port: %d\n", GPS_PORT2);
    }
    printf("Web Dashboard: http://localhost:%d\n", HTTP_PORT);
    printf("===========================================\n");
    printf("Waiting for GPS client to connect...\n\n");
    
    // Fork to handle GPS and HTTP separately
   // if (fork() == 0) {
        // Child process handles HTTP requests
        pthread_t http_thread;
        pthread_create(&http_thread,NULL,handle_http_loop,&http_server);
        pthread_detach(http_thread);
       
    
        // Parent process handles GPS client
       while (1) {
            int* client_sock = malloc(sizeof(int));
            *client_sock = accept(gps_server, NULL, NULL);
            
            pthread_t tid;
            pthread_create(&tid, NULL, handle_client, client_sock);
            pthread_detach(tid);
        }
    
    
    close(gps_server);
    close(http_server);
    pthread_mutex_destroy(&data_lock);
    return 0;
}
