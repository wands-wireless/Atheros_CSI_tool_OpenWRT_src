/*
 * =====================================================================================
 *
 *       Filename:  client_main.c
 *
 *    Description:  
 *
 *        Version:  1.0
 *        Created:  01/03/2017 14:21:14
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Yaxiong Xie, 
 *   Organization:  
 *
 * =====================================================================================
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <pthread.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "csi_func.h"

#define BUFSIZE 4096
unsigned char buf_addr[BUFSIZE];
unsigned char data_buf[1500];
unsigned char recv_buf[100];
unsigned char send_buf[100];

int quit;
COMPLEX csi_matrix[3][3][114];
csi_struct*   csi_status;

void sig_handler(int signo){
    if(signo == SIGINT){
        quit = 1;
    }
}

void usage(char *argv[]){
	printf("\t-s <server IP>           Server IP\n");
	printf("\t-t <Trace Index>         Index of the recorded traces\n");
	printf("\t-r <server Port>         Port number of the TCP connection to Server\n");
	printf("\t-p <Print Flag>          Flag that indicates print CSI related information or not \n");

	printf("\nExample:\n");	
	printf("\t%s -s 155.69.142.59 -r 6767 -t 1 -p 1 \n",argv[0]);
}

int main(int argc, char* argv[])
{
    FILE*  fp;
    int    sock,fd,fd_usb;
    int    i,total_msg_cnt=0,cnt=0;
    int    c,SW_idx,ret;
    unsigned int port = 0,T_index = 0,print_flag = 1;
    char   hostname[50];
    char   fileName[100]; 
    char   fileName_1[50]; 
    char   fileName_2[50]; 
    char   SW_idx_char[12];
    char   trace_idx[12];
    char   command[150];

    struct hostent *hp;
    struct sockaddr_in pin;

    u_int16_t   buf_len;
    u_int8_t 	RF_SW_IDX;
    csi_status = (csi_struct*)malloc(sizeof(csi_struct));

    while((c = getopt(argc,argv,"s:r:t:p:"))!= EOF){
    	switch(c){
		case 's':
			if(sscanf(optarg, "%s", hostname) != 1){
				printf("ERROR: Unable to parse host name!\n");
				return -1;		
			}
			printf("host name:%s\n",hostname);
			break;
		case 'r':
			if(sscanf(optarg, "%u", &port) != 1){
				printf("ERROR: Unable to parse TCP port!\n");
				return -1;
			}
			break;
		case 't':
			if(sscanf(optarg, "%u", &T_index) != 1){
				printf("ERROR: Unable to parse trace index!\n");
				return -1;
			}
			break;
		case 'p':
			if(sscanf(optarg, "%u", &print_flag) != 1){
				printf("ERROR: Unable to parse print flag!\n");
			}
			break;
		default:
        		strcpy(hostname,"155.69.142.59"); // the server IP
        		port = 6767;      // port 
			usage(argv);
			break;
	}
    }
    if(port == 0 || hostname[0] == '\0'){
    	printf("ERROR TCP port must be set!\n");
    }
    
    fd = open_csi_device();
    if (fd < 0){
        perror("Failed to open the CSI device...");
        return errno;
    }
   
    fd_usb = open("/dev/ttyACM0",O_RDWR);
    if(fd_usb < 0){
    	printf("ERROR: USB is not plugged in!\n");
	return -1;
    }

    memset(&pin,0,sizeof(pin));
    pin.sin_family = AF_INET;
    pin.sin_port   = htons(port);

    if ((hp = gethostbyname(hostname))!= NULL){
        pin.sin_addr.s_addr =
                        ((struct in_addr *)hp->h_addr)->s_addr;
    }else{
        pin.sin_addr.s_addr = inet_addr(hostname);
    }

    if((sock = (int)socket(AF_INET,SOCK_STREAM,0)) == -1){
        perror("socket"); 
        return 0;
    } 

    printf("Waiting for the connection!\n");

    ret = connect(sock,(const struct sockaddr *)&pin,sizeof(pin));

    if(ret == -1){
        perror("connect");
    	close(fd);
    	close(fd_usb);
    	close(sock);
        return -1;
    }
    printf("Connection with server is built!\n");

    if (signal(SIGINT, sig_handler) == SIG_ERR)
    {
        printf("Can't catch SIGINT\n");
	close(fd);
	close(fd_usb);
    	close(sock);
        return -1;
    }
   
    strcpy(fileName_1,"/tmp/log_RF_switch_");
    sprintf(trace_idx,"%d",T_index);
    strcpy(fileName_2,"_TraceIndex_");
    strcat(fileName_2,trace_idx);
    strcat(fileName_2,".txt");
    /* Now receive the message*/
    quit = 0;
    printf("# Receiving data! Press Ctrl+c to quit!\n");
    for(SW_idx=1;SW_idx<=16;SW_idx++){
	    RF_SW_IDX = SW_idx & 0xFF;
	    ret = write(fd_usb,&RF_SW_IDX,1);
	    if(!ret){
	    	printf("Warning: bytes sent to USB failed!\n");
	    }

	    // File name
	    memset(fileName,0,sizeof(fileName));
	    memset(SW_idx_char,0,sizeof(SW_idx_char));
	    sprintf(SW_idx_char,"%d",SW_idx);
	    strcpy(fileName,fileName_1);
	    strcat(fileName,SW_idx_char);
	    strcat(fileName,fileName_2);
	    fp = fopen(fileName,"w");

	    if(fp==NULL){
	    	perror("Open file failed!\n");
		return -1;
	    }else{    
		    send_buf[0] = 0x01;
    		    ret = send(sock,send_buf,1,0);
	    }	
	   total_msg_cnt = 0; 
	    while(quit == 0){
		if (quit == 1){
		    fclose(fp);
		    close(fd);
		    close(fd_usb);
		    close(sock);		    
		    return 0;
		}
		/*  keep listening to the kernel and waiting for the csi report */
		cnt = read_csi_buf(buf_addr,fd,BUFSIZE);
		if(cnt){
		    //printf("cnt is:%d\n",cnt);
		    total_msg_cnt += 1;
		   
		    record_status(buf_addr, cnt, csi_status);
		    //record_csi_payload(buf_addr, csi_status, data_buf, csi_matrix);
		
		    if(print_flag){    
			    printf("Recv %dth msg with rate: 0x%02x | payload len: %d\n",total_msg_cnt,csi_status->rate,csi_status->payload_len);
			    printf("========  Bsic Information of the Transmission ==========\n");
			    printf("csi_len= %d |",csi_status->csi_len);
			    printf("chanBW= %d   |",csi_status->chanBW);
			    printf("num_tones= %d  |",csi_status->num_tones);
			    printf("nr= %d  |",csi_status->nr);
			    printf("nc= %d\n",csi_status->nc);
			    printf("rssi= %d  |",csi_status->rssi);
			    printf("rssi_0= %d  |",csi_status->rssi_0);
			    printf("rssi_1= %d  |",csi_status->rssi_1);
			    printf("rssi_2= %d\n",csi_status->rssi_2);
		    }
		    /*  log the received data for off-line processing */
		    buf_len = csi_status->buf_len;
		    fwrite(&buf_len,1,2,fp);
		    fwrite(buf_addr,1,buf_len,fp);
								    
		}else{
			ret = recv(sock,recv_buf,1,0);
			if(ret){
				if(recv_buf[0]==0x02){
					break;
				}
			}
		}

	    }
	    printf("Received %d packets in total.\n",total_msg_cnt);
	    fclose(fp);
	    memset(command,0,sizeof(command));
	    strcpy(command,"scp ");
	    strcat(command,fileName);
	    strcat(command," project6@155.69.149.12:~/work/Distr-Ant/matlab/code/data/0103/distri/raw/"); 
	    //strcat(command," yanbo@155.69.149.184:~/work/Distr-Ant/data/"); 
	    system(command);

	    memset(command,0,sizeof(command));
	    strcpy(command,"rm ");
	    strcat(command,fileName);
	    system(command);
    }
    send_buf[0] = 0x03;
    ret = send(sock,send_buf,1,0);

    close(fd);
    close(fd_usb);
    close(sock);

    free(csi_status);
    return 0;
}
