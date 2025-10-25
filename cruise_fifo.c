#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/stat.h>

int main(){
	int res=mkfifo("cuise_collision_fifo",0777);
	if(res==-1){
		perror("fifo failed\n");
	}
	printf("fifo created\n");
	return 0;
}
