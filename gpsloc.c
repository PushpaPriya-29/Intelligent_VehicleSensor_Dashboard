#include "headers.h"
int main(){
	gps data;
	printf("Enter the longitude and the latitude of current location: ");
	scanf("%f %f",&data.src.latitude,&data.src.longitude);
	printf("Enter the longitude and the latitude of the destination: ");
	scanf("%f %f",&data.dest.latitude,&data.dest.longitude);
	srand(time(NULL));
	while(1){
		
		gnrl g;
		g.speed=(float)(rand()%120);
		//printf("Speed: %.2f\n",g.speed);
		if(data.dest.latitude<data.src.latitude || data.dest.longitude<data.src.longitude){
			data.src.latitude=data.dest.latitude;
			data.src.longitude=data.dest.longitude;
		}
		if(data.src.latitude<data.dest.latitude){
			data.src.latitude+=((g.speed)/10);
		}
		if(data.src.longitude<data.dest.longitude){
			data.src.longitude+=((g.speed)/10);
		}
		if(data.src.longitude>=data.dest.longitude && data.src.latitude>=data.dest.latitude){
			break;
		}
		printf("Current location: Latitude: %f Longitude: %f\n",data.src.latitude,data.src.longitude);
		sleep(1);
	}
	printf("Location reached\n");
	return 0;
}
	
	
