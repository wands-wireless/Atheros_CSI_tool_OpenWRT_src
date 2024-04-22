/*
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  
 *  version 3.0: add -d option for packet intervel control
 *
 *
 */
 
#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/types.h>
#include <netdb.h>
#include <unistd.h>
#include <arpa/inet.h>

unsigned char send_buf[100];
unsigned char recv_buf[100];
void usage(char *argv[]){
	printf("\t -i <interface>                Radio interface\n");
	printf("\t -m <Min MCS index>            Minimum MCS index\n");
	printf("\t -x <Max MCS index>            Maximum MCS index\n");
	printf("\t -n <Pkt number>               Number of packet to send\n");
	printf("\t -d <delay>			 Pkt transmit delay\n");
       
	printf("\nExample:\n");
	printf("\t%s -i wlan0 -m 0 -x 7 -n 10 -d 1000000\n\n",argv[0]);	
}
int main(int argc, char *argv[]){
	char command[100];
	char cmd_1[50];
	char cmd_2[50];
	char iface[12];
	char *interface = NULL;
	char mcs_idx[12];
	char pkt_no[12];
	char p_flag[12];
        char delay[12];

	unsigned int MCS_min = 0;
	unsigned int delay_tx = 0;
	unsigned int MCS_max = 0;
	unsigned int packet_no = 10;
	unsigned int print_flag = 1;
	int c,i,ret,SW_idx,recv_cnt;

	int socket_desc, client_sock, read_size, tmp;
	struct sockaddr_in server,client;

	printf("/* ********************************************************/\n");
	printf("/*               Sender Side Applications                 */\n");
	printf("/* ********************************************************/\n");

	if (1 == argc){
		usage(argv);
		interface = "wlan0";
		packet_no = 10;
		MCS_min = 0;
		MCS_max = MCS_min;
	}else{
		while ((c = getopt(argc,argv,"hi:m:x:n:d:p:")) != EOF){
			switch (c){
				case 'i':
					interface = strdup(optarg);
					break;
				case 'm':
					if(sscanf(optarg, "%u", &MCS_min) != 1){
						printf("ERROR: Unable to parse MCS min index\n");
						return -1;
					}
					break;
				case 'x':
					if(sscanf(optarg, "%u", &MCS_max) != 1){
						printf("ERROR: Unable to parse MCS max index\n");
						return -1;
					}
					break;
				case 'n':
					if(sscanf(optarg, "%u", &packet_no) != 1){
						printf("ERROR: Unable to parse number of packets\n");
						return -1;
					}
					break;
				case 'd':
					if(sscanf(optarg, "%u", &delay_tx) != 1){
                                                printf("ERROR: Unable to delay between tx pkts\n");
                                                return -1;
                                        }
                                        break;
				case 'p':
					if(sscanf(optarg, "%u", &print_flag) != 1){
						printf("ERROR: Unable to parse print_flag (default open)\n");
						return -1;
					}
					break;
				case 'h':
					usage(argv);
					return -1;
				default:
					usage(argv);
					break;
					return -1;
			}
		}
	}

	if (interface == NULL){
		printf("ERROR: Interface is not set!\n");
		return -1;
	}
	if (MCS_min > MCS_max){
		MCS_max = MCS_min;
	}

	strcpy(iface,interface);  //copy the interface
	sprintf(pkt_no,"%d",packet_no);
	sprintf(p_flag,"%d",print_flag);
	sprintf(delay,"%d",delay_tx);
	strcpy(cmd_1,"injector -i ");    
	strcat(cmd_1,iface);
	strcat(cmd_1," -c 3HT20 -m ");
	strcpy(cmd_2," -n ");
	strcat(cmd_2,pkt_no);
	strcat(cmd_2," -d ");
	strcat(cmd_2,delay);
	strcat(cmd_2," -p ");
	strcat(cmd_2,p_flag);
	
	socket_desc = socket(AF_INET, SOCK_STREAM,0);
	if(socket_desc == -1){
		printf("ERROR: Unable to create Socket!\n");
	}
	//prepare the socket
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(6767);

	// bind
	if(bind(socket_desc,(struct sockaddr *)&server, sizeof(server)) < 0){
		perror("Bind failed. ERROR");
		return -1;
	}

	ret = listen(socket_desc,3);
	if (ret == 0){
		printf("listen succeed!\n");
	}else{
		perror("listen failed!\n");
	}
	printf("waiting for incoming connections...\n");

	tmp = sizeof(struct sockaddr_in);

	client_sock = accept(socket_desc,(struct sockaddr *)&client,(socklen_t*)&tmp);
	if(client_sock < 0){
		perror("accept failed\n");
		return -1;
	}
	printf("connection is built!\n");

	for(SW_idx=1;SW_idx<=16;SW_idx++){
		recv_cnt = 0;
		while(1){
			recv_cnt += 1;
			ret = recv(client_sock,recv_buf,1,0);
			if(recv_buf[0] == 0x01){				
				//printf("Transmisson begins!\n");
				break;
			}
			if(recv_cnt > 10){
				printf("ERROR: No begin signal received!\n");
				return -1;
			}
		}
		for(i=MCS_min;i<=MCS_max;i++){
			memset(command,0,sizeof(command)); // clear the command buffer
			memset(mcs_idx,0,sizeof(mcs_idx)); // clear the mcs index buffer
			sprintf(mcs_idx,"%d",i);
			
			strcpy(command,cmd_1);    
			strcat(command,mcs_idx);
			strcat(command,cmd_2);
			//printf("command is: %s\n",command);
			system(command);
		}
		send_buf[0] = 0x02;
		ret = send(client_sock,send_buf,1,0);
		//printf("Transmisson ends!\n");
	
	}
	recv_cnt = 0;
	while(1){
		recv_cnt += 1;
		ret = recv(client_sock,recv_buf,1,0);
		if(recv_buf[0] == 0x03){				
			printf("WE EXIT!\n");
			sleep(1);
			close(socket_desc);
			close(client_sock);
			return 0;
		}
		if(recv_cnt > 10){
			printf("ERROR: No begin signal received, WE EXIT!\n");
			close(socket_desc);
			close(client_sock);
			return 0;
		}
	}
	return 0;
}
