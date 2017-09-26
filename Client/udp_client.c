#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>

#define MAXBUFSIZE 1000
#define FORLISTINGMSGSIZE 1000

int PACKETSIZE = 1000;
int nbytes;                             	// number of bytes send by sendto()
int sock;                               	//this will be our socket
char buffer[MAXBUFSIZE];
char packetnumber[10];
char file_buffer[MAXBUFSIZE];
int size_file_buffer;
size_t file_size; 
struct sockaddr_in remote, from_addr;              	//"Internet socket address structure"
unsigned int addr_length = sizeof(struct sockaddr);
char ack[20];

struct packetdata {
	int sequence_number;
	char buffer[MAXBUFSIZE];
	int buffer_length;
}packet_data;

void remove_old_file(char* file_name);
void exit_function(int sock, char *value);
void ls_function(int sock, char *value);
void get_function(int sock, int packet_number, char *value);
void put_function(int sock, char *value);

void remove_old_file(char* file_name) 
{
	if (access(file_name, F_OK) != -1)
		{
			remove(file_name);
		}
}

void exit_function(int sock, char *value)
{

}

void ls_function(int sock, char *value)
{
	char listing_directory[FORLISTINGMSGSIZE];
	if(sendto(sock, value, sizeof(value), 0, (struct sockaddr *) &remote, sizeof(remote)) == -1)
	{	
		int errsv = errno;
		printf("message not received. Refer to error number: %d of the errno function\n", errsv);	
		exit(1);
	}

	if((recvfrom(sock, listing_directory, FORLISTINGMSGSIZE, 0, (struct sockaddr *) &from_addr, (socklen_t *) &addr_length)) == -1)
	{
		int errsv = errno;
		printf("message not received. Refer to error number: %d of the errno function\n", errsv);	
		exit(1);
	}
	printf("Server says the list is as follows:\n%s\n\n", listing_directory);
	exit(0);
}


void get_function(int sock, int packet_number, char *value)
{
	char file_name[MAXBUFSIZE];

	strcpy(file_name,value+4);

	remove_old_file(file_name);

	FILE *fp = fopen(file_name,"w");

	if (fp == NULL) 
	{
		int errsv = errno;
		printf("File not opened. Refer to error number: %d of the errno function\n", errsv);	
		exit(1);
	}

	//Sending ack to confirm number of bytes received and send files
	if(sendto(sock, ack, strlen(ack), 0, (struct sockaddr *)&remote, sizeof(remote)) == -1)    
	{
	   	int errsv = errno;
		printf("File not sent. Refer to error number: %d of the errno function\n", errsv);
		exit(1);
	}

	printf("File size to be recieved: %lu", file_size);

	for(int i = 0; i<packet_number+1; i++)
	{
		int sequence_number = 0;

		printf("\nBeginning of loop\n");

		// char new_file[]="Received file";
		// strcat(new_file,file_buffer);
	
		memset(&packet_data, '\0', sizeof(struct packetdata));
		//bzero(&packet_data,sizeof(struct packetdata));

		//Waiting to receive packet data
		if((recvfrom(sock, &packet_data, sizeof(struct packetdata), 0, (struct sockaddr *) &from_addr, (socklen_t *) &addr_length)) == -1)
		{
			int errsv = errno;
			printf("message not received. Refer to error number: %d of the errno function\n", errsv);	
			exit(1);
		}

  		printf("Received packet. Sending ACK\n");

  		//ACK to tell that packet is received.
  		if(sendto(sock, ack, strlen(ack), 0, (struct sockaddr *)&remote, sizeof(remote)) == -1)    
	  	{
	    	int errsv = errno;
			printf("File not sent. Refer to error number: %d of the errno function\n", errsv);
			exit(1);
	  	}
	  	bzero(file_buffer,sizeof(file_buffer));
	  	sequence_number = packet_data.sequence_number;
		strcpy(file_buffer, packet_data.buffer);
		size_file_buffer = packet_data.buffer_length;
		printf("size_file_buffer:%d\n",size_file_buffer);


	    if(fwrite(file_buffer,1, size_file_buffer, fp) == 0)
		{
			int errsv = errno;
			printf("packet not written. Refer to error number: %d of the errno function\n", errsv);
			exit(1);
		}

	  	printf("Packet saved. sent ACK\n");

		printf("Finished with Internal Packet Number: %d\n Received Sequence Number: %d\n", i+1 , sequence_number);

		printf("File size received: %d\n", size_file_buffer);

		printf("file_size left: %lu\n", file_size);
		file_size -=  size_file_buffer;

		
		// if(file_buffer == ack)
		// {
		// 	printf ("Number of packets sent till %d\n", packet_number);
		// 	return;
		// }
	}

	printf("File transfer done.");
	fclose(fp);
	return;
}

void put_function(int sock, char *value)
{

}


/* You will have to modify the program below */

int main (int argc, char * argv[])
{
	int packet_number;
	strcpy(ack, " ACK for file. ");

	if (argc < 3)
	{	
		printf("USAGE: <executable> <server_ip> <server_port>\n");
		exit(1);
	}

	/******************
	  Here we populate a sockaddr_in struct with
	  information regarding where we'd like to send our packet 
	  i.e the Server.
	 ******************/
	bzero(&remote,sizeof(remote));               //zero the struct
	remote.sin_family = AF_INET;                 //address family
	remote.sin_port = htons(atoi(argv[2]));      //sets port to network byte order
	remote.sin_addr.s_addr = inet_addr(argv[1]); //sets remote IP address

	//Causes the system to create a generic socket of type UDP (datagram)
	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		int errsv = errno;
		printf("Socket not created. Refer to error number: %d of the errno function\n", errsv);
		exit(1);
	}

	printf("Say something to server: ");

	bzero(buffer,sizeof(buffer));

	//taking the command from the console
	fgets(buffer, sizeof(buffer), stdin);
	buffer[strlen(buffer)-1] = '\0';

	//Sending the command and (if required) the file name to Server.
	if( sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *) &remote, sizeof(remote)) == -1)
	{
		int errsv = errno;
		printf("message not sent. Refer to error number: %d of the errno function\n", errsv);
		exit(1);
	}

	char filesize[20];

	//Waiting for number of packets or ACK
	if((recvfrom(sock, filesize, MAXBUFSIZE, 0, (struct sockaddr *) &from_addr, (socklen_t *) &addr_length)) == -1)
	{
		int errsv = errno;
		printf("message not received. Refer to error number: %d of the errno function\n", errsv);	
	}

	if(strcmp(filesize, ack) != 0)
	{
		printf("Reciving file of size: %s", filesize);
		
		file_size = atoi(filesize);

		packet_number = file_size/PACKETSIZE;

		printf("No. of packets: %d\n", packet_number);
	}
	else
	{
		printf("ACK Received");
	}

	if(buffer[0] == 'e')
	{
		printf("Going to exit function\n");
		exit_function(sock, buffer);
	}
	else if(buffer[0] == 'l')
	{
		printf("Going to ls function\n");
		ls_function(sock, buffer);
	}
	else if(buffer[0] == 'g')
	{
		printf("Going to get function\n");
		get_function(sock, packet_number, buffer);
	}
	else if(buffer[0] == 'p')
	{
		printf("Going to put function\n");
		put_function(sock, buffer);
	}
	else	
	{
		char st1[MAXBUFSIZE], st2[MAXBUFSIZE], st3[MAXBUFSIZE];

		strcpy(st1, "Command given:");
		strcpy(st2, buffer);
		strcpy(st3, ". Command not clear.\n");

		char buf[100] = "";
		strcat(buf, st1);
		strcat(buf, st2);
		strcat(buf, st3);

		bzero(buffer, sizeof(buffer));

		strcat(buffer,buf);

		printf("Statement to be sent to client: %s \n", buffer);
		if (sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr*) &remote, sizeof(remote)) == -1)
		{
			int errsv = errno;
			printf("Data not sent. Refer to error number: %d of the errno function\n", errsv);
			exit(1);        
		}
	}

	close(sock);
}

