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
char ackr [20];

// Structure for sending data as a packet
struct packetdata {
	int sequence_number;
	char buffer[MAXBUFSIZE];
	int buffer_length;
}packet_data, filesize;

void checking_function(int sock, struct sockaddr_in from_addr, unsigned int addr_length, int packet_number, char *buffer);
void remove_old_file(char* file_name);
void unclear_command(char* buffer);
void exit_function(int sock, char *value);
void ls_function(int sock, char *value);
void get_function(int sock, int packet_number, char *value);
void put_function(int sock, int packet_number, char *value);

// remove file if it already exists
void remove_old_file(char* file_name) 
{
	if (access(file_name, F_OK) != -1)
		{
			remove(file_name);
		}
}


//The function that runs to see if the command is unclear
void unclear_command(char* buffer)
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

	printf("%s \n", buffer);
	// Throw error if data not sent
	if (sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr*) &remote, sizeof(remote)) == -1)
	{
		int errsv = errno;
		printf("Data not sent. Refer to error number: %d of the errno function\n", errsv);
		exit(1);        
	}
}

//The function that exit's the server 
void exit_function(int sock, char *value)
{

	printf("Server received the exit command\n");
	return;
}

// The function that requests server directory contents
void ls_function(int sock, char *value)
{
	char listing_directory[FORLISTINGMSGSIZE];
	if((recvfrom(sock, listing_directory, FORLISTINGMSGSIZE, 0, (struct sockaddr *) &from_addr, (socklen_t *) &addr_length)) == -1)
	{
		int errsv = errno;
		printf("message not received. Refer to error number: %d of the errno function\n", errsv);	
		exit(1);
	}
	printf("Server says the list is as follows:\n\n%s\n\n", listing_directory);
	return;
}

// The function which request a file from server
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
	memset(&filesize, 0, sizeof(struct packetdata));
	filesize.sequence_number = 98765;
	memcpy(filesize.buffer, ack, strlen(ack));
	filesize.buffer_length = 98765;		

	if(sendto(sock, &filesize, sizeof(struct packetdata), 0, (struct sockaddr *)&from_addr, addr_length) < 0)    
  	{
    	int errsv = errno;
		printf("File not sent. Refer to error number: %d of the errno function\n", errsv);
		exit(1);
  	}

	// Loop to receive the file content from server
	for(int i = 0; i<packet_number+1; i++)
	{
		int sequence_number = 0;

		printf("\nBeginning of loop\n");

		memset(&packet_data, 0, sizeof(struct packetdata));

		//Waiting to receive packet data
		if((recvfrom(sock, &packet_data, sizeof(struct packetdata), 0, (struct sockaddr *) &from_addr, (socklen_t *) &addr_length)) == -1)
		{
			int errsv = errno;
			printf("message not received. Refer to error number: %d of the errno function\n", errsv);	
			exit(1);
		}

		// printf("packet and size %s, %d", packet_data.buffer, packet_data.buffer_length);

  		printf("Received packet. Sending ACK\n");

  		//ACK to tell that packet is received.
		memset(&filesize, 0, sizeof(struct packetdata));
		filesize.sequence_number = 98765;
		memcpy(filesize.buffer, ack, strlen(ack));
		filesize.buffer_length = 98765;		

		if(sendto(sock, &filesize, sizeof(struct packetdata), 0, (struct sockaddr *)&from_addr, addr_length) < 0)    
	  	{
	    	int errsv = errno;
			printf("Packet not sent. Refer to error number: %d of the errno function\n", errsv);
			exit(1);
	  	}

	  	bzero(file_buffer,sizeof(file_buffer));

		memset(&filesize, 0, sizeof(struct packetdata));
	  	sequence_number = packet_data.sequence_number;
	  	memcpy(file_buffer,packet_data.buffer,packet_data.buffer_length);
		size_file_buffer = packet_data.buffer_length;

		printf("size_file_buffer:%d\n",size_file_buffer);

		// Writing received contents to a file
	    if(fwrite(packet_data.buffer,1, packet_data.buffer_length, fp) ==0 )
		{
			int errsv = errno;
			printf("packet not written. Refer to error number: %d of the errno function\n", errsv);
			exit(1);
		}

	  	printf("Packet saved. sent ACK\n");
		printf("Finished with Internal Packet Number: %d\n Received Sequence Number: %d\n", i+1 , sequence_number);
		printf("File size received: %d\n", size_file_buffer);

		file_size -=  size_file_buffer;
		printf("file_size left: %lu\n", file_size);

		// Check for if ACK received.
		if(file_buffer == ack)
		{
			printf ("Number of packets sent till %d\n", packet_number);
			return;
		}
	}

	// if((recvfrom(sock, &filesize, sizeof(struct packetdata), 0, (struct sockaddr *) &from_addr, (socklen_t *) &addr_length)) == -1)
	// {
	// 	int errsv = errno;
	// 	printf("message not received. Refer to error number: %d of the errno function\n", errsv);	
	// }
	// if(strcmp(filesize.buffer, ack) != 0)
	// {
	// 	printf("Received: %s\n", filesize.buffer);
	// 	printf("Missed sync. Resend.\n");
	// 	exit(1);
	// }
	// else
	// {
	// 	printf("Received ACK\n");
	// 	printf("\nFile transfer done.\n");

	// }

	fclose(fp);
	return;
}

// The function that sends file to the server
void put_function(int sock, int packet_number, char *value)
{
	size_t sub_file_size = 1000;

	int size_file_buffer;
	int size_sent = 0;

	char file_name[MAXBUFSIZE];

	strcpy(file_name,value+4);

	FILE *fp;
	fp = fopen(file_name, "rb+");

	if(fp != NULL)
	{
		fseek(fp,0,SEEK_END);
		size_t file_size = ftell(fp);
		fseek(fp,0,SEEK_SET);

		int packet_number = file_size/sub_file_size;

		printf("No. of packets: %d\n", packet_number);

		memset(&filesize, 0, sizeof(struct packetdata));
		filesize.sequence_number = packet_number;
		memcpy(filesize.buffer, file_name, sizeof(file_name));
		filesize.buffer_length = file_size;		

		printf("%d\n", filesize.buffer_length);

		//Sending file size in a packet
		if(sendto(sock, &filesize, sizeof(struct packetdata), 0, (struct sockaddr *)&from_addr, addr_length) < 0)    
	  	{
	    	int errsv = errno;
			printf("File not sent. Refer to error number: %d of the errno function\n", errsv);
			exit(1);
	  	}

	  	//Waiting for ack
	  	if((recvfrom(sock, &filesize, sizeof(struct packetdata), 0, (struct sockaddr *) &from_addr, (socklen_t *) &addr_length)) == -1)
		{
			int errsv = errno;
			printf("message not received. Refer to error number: %d of the errno function\n", errsv);	
		}

		if(strcmp(filesize.buffer, ack) != 0)
		{
			printf("Received: %s\n", filesize.buffer);
			printf("Missed sync. Resend.\n");
			exit(1);
		}
		else
		{
			printf("Received ACK\n");
		}


		printf("Size of the file to be sent: %lu\n", file_size);

		// Loop to send file contents to server
		for(int i = 0; i < packet_number+1; i++)
		{
			memset(&packet_data, 0, sizeof(struct packetdata));
			printf("\nBeginning of loop\n");
			bzero(file_buffer,sizeof(file_buffer));

			int bytes_Read = fread(file_buffer,1, sub_file_size,fp);

		    printf("%d", bytes_Read);

			memset(&packet_data, 0, sizeof(struct packetdata));
			packet_data.sequence_number = (i+1);
			memcpy(packet_data.buffer,file_buffer,bytes_Read);
			packet_data.buffer_length = bytes_Read;

		    printf("Sending data now\n");
			
			// Sending contents of the file
			if(sendto(sock, &packet_data, sizeof(struct packetdata), 0, (struct sockaddr *)&from_addr, addr_length) == -1)    
		  	{
		    	int errsv = errno;
				printf("packet_data not sent. Refer to error number: %d of the errno function\n", errsv);
				exit(1);
		  	}
		  	else
		  	{
		  		printf("File part %d sent\n", (i+1));
		  	}

		  	size_sent += bytes_Read;
			printf("File size sent till now: %d\n", size_sent);

			//Waiting for ack
		  	if((recvfrom(sock, &filesize, sizeof(struct packetdata), 0, (struct sockaddr *) &from_addr, (socklen_t *) &addr_length)) == -1)
			{
				int errsv = errno;
				printf("message not received. Refer to error number: %d of the errno function\n", errsv);	
			}
			printf("Received ACK\n");
			if(strcmp(filesize.buffer, ack) != 0)
			{
				printf("Missed sync. Resend.");
				exit(1);
			}

		}
		
		// Receiving ACK
    	if((recvfrom(sock, &filesize, sizeof(struct packetdata), 0, (struct sockaddr *) &from_addr, (socklen_t *) &addr_length)) == -1)
		{
			int errsv = errno;
			printf("message not received. Refer to error number: %d of the errno function\n", errsv);	
		}

		if(strcmp(filesize.buffer, ack) != 0)
		{
			printf("Missed sync. Resend.");
			exit(1);
		}
	  	printf("\nReceived ACK. Hence file sent.\n");
		fclose(fp);
	  	return;
	}
}

// The function that checks the received command from the user
void checking_function(int sock, struct sockaddr_in from_addr, unsigned int addr_length, int packet_number, char *buffer)
{
	// Check for different commands like delete, get, put etc.
	if(buffer[0] == 'd')
	{
  		printf("\nReceived ACK. Hence file sent.\n");		printf("file deleted.\n");
	}
	else if(buffer[0] == 'e')
	{
		printf("Going to exit function\n");
		exit_function(sock, buffer);
		printf("came back!");
	}
	else if(buffer[0] == 'l')
	{
		printf("Going to ls function\n");
		ls_function(sock, buffer);
		printf("came back!");

	}
	else if(buffer[0] == 'g')
	{
		printf("Going to get function\n");
		get_function(sock, packet_number, buffer);
		printf("came back!");
	}
	else if(buffer[0] == 'p')
	{
		printf("Going to put function\n");
		put_function(sock, packet_number, buffer);
		printf("came back!");
	}
	else	
	{
		unclear_command(buffer);
	}
}

// Main function definition for UDP protocol
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

	while(1)
	{
		printf("\n\nCommand Menu is as follows:\nls\nget <filename>\nput <filename>\ndel <filename> \nSay something to server: ");

		bzero(buffer,sizeof(buffer));

		// Taking the command from the console
		fgets(buffer, sizeof(buffer), stdin);
		buffer[strlen(buffer)-1] = '\0';

		//Sending the command and (if required) the file name to Server.
		if( sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *) &remote, sizeof(remote)) == -1)
		{
			int errsv = errno;
			printf("message not sent. Refer to error number: %d of the errno function\n", errsv);
			exit(1);
		}

		memset(&filesize, '\0', sizeof(struct packetdata));

		//Waiting for packet containing filesize(get) or ACK(ls, exit, delete, put)
		if((recvfrom(sock, &filesize, sizeof(struct packetdata), 0, (struct sockaddr *) &from_addr, (socklen_t *) &addr_length)) == -1)
		{
			int errsv = errno;
			printf("message not received. Refer to error number: %d of the errno function\n", errsv);	
		}

		// Check for if ACK received
		if(strcmp(filesize.buffer, ack) != 0)
		{

			printf("Reciving file of size: %d with file size", filesize.buffer_length);
			
			file_size = filesize.buffer_length;

			packet_number = file_size/PACKETSIZE;

			printf("No. of packets: %d\n", packet_number);
		}
		else
		{
			printf("ACK Received\n");
		}

		checking_function(sock, from_addr, addr_length, packet_number, buffer);
	}

}