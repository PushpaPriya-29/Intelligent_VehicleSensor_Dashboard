#include "headers.h"

int main(){
	int fd=open("cuise_collision_fifo",O_RDONLY);
	if(fd<0){
		perror("fifo open failed\n");
		return 1;
	}
	pid_t pid;
	int l=read(fd,&pid,sizeof(pid));
	if(l<0){
		perror("fifo read failed\n");
		return 1;
	}
	int n=0;
	while(1){
		printf("enter 1 to turn on the cruise control\n");
		scanf("%d",&n);
		if(n==1){
			if(kill(pid,SIGUSR1)==0){
				printf("signal sent successfully\n");
			}
			else{
				printf("couldn't send the signal\n");
			}
		}
	}
}
