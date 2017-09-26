#include <sys/types.h>
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
#include <string.h>
#include <dirent.h>

#define MAXBUFSIZE 1000
#define FORLISTINGMSGSIZE 1000


int nbytes;                        			//number of bytes we receive in our message
char buffer[MAXBUFSIZE];           			//a buffer to store our received message
char file_buffer[MAXBUFSIZE];
char ack [20];
char ackr [20];
char str1[MAXBUFSIZE], str2[MAXBUFSIZE], str3[MAXBUFSIZE];

struct packetdata {
	int sequence_number;
	char buffer[MAXBUFSIZE];
	int buffer_length;
} packet_data, filesize;

void remove_old_file(char* file_name);
void unclear_command(int sock, struct sockaddr_in from_addr, unsigned int from_addr_length, char *buffer);
void exit_function(int sock, struct sockaddr_in from_addr, unsigned int from_addr_length, char *value);
void ls_function(int sock, struct sockaddr_in from_addr, unsigned int from_addr_length, char *value);
void put_function(int sock,struct sockaddr_in from_addr, unsigned int from_addr_length, char *file_name, char *value);
void get_function(int sock, struct sockaddr_in from_addr, unsigned int from_addr_length, char *value);
void checking_function(int sock, struct sockaddr_in from_addr, unsigned int from_addr_length, char *value);

void remove_old_file(char* file_name) 
{
	if (access(file_name, F_OK) != -1)
		{
			remove(file_name);
		}
}

void unclear_command(int sock, struct sockaddr_in from_addr, unsigned int from_addr_length, char *buffer)
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
	if (sendto(sock, buffer, sizeof(buffer), 0, (struct sockaddr*) &from_addr, from_addr_length) == -1)
	{
		int errsv = errno;
		printf("Data not sent. Refer to error number: %d of the errno function\n", errsv);
		exit(1);        
	}
}

void delete_function()
{
	// if (access(option, F_OK) != -1) {
	// 	// file exists
	// 	if (remove(option) == 0) {
	// 		char fileDeleted[MAXBUFSIZE] = "File Successfully deleted";
	// 		nbytes = sendto(sock, fileDeleted, sizeof(fileDeleted), 0,
	// 				(struct sockaddr *) &remote, remote_length);
	// 	} else {
	// 		char fileNotDeleted[MAXBUFSIZE] = "Unable to delete file";
	// 		nbytes = sendto(sock, fileNotDeleted, sizeof(fileNotDeleted), 0,
	// 				(struct sockaddr *) &remote, remote_length);
	// 	}
}

void exit_function(int sock, struct sockaddr_in from_addr, unsigned int from_addr_length, char *value)
{
	strcpy(str2, "exit");
	int compare = strcmp(str2, value);

	if(compare == 0)
	{
		filesize.sequence_number = 98765;
		memcpy(filesize.buffer, ack, strlen(ack));
		filesize.buffer_length = 98765;		

		if(sendto(sock, &filesize, sizeof(struct packetdata), 0, (struct sockaddr *)&from_addr, from_addr_length) < 0)    
	  	{
	    	int errsv = errno;
			printf("File not sent. Refer to error number: %d of the errno function\n", errsv);
			exit(1);
	  	}	

		printf ("Going to exit gracefully now\n");

		close(sock);
		exit(0);
	}
	else
	{
		printf(" Incorrect command typed.");
		return;
	}
}

void ls_function(int sock, struct sockaddr_in from_addr, unsigned int from_addr_length, char *value)
{

	strcpy(str2, "ls");
	int compare = strcmp(str2, value);

	if(compare == 0)
	{
		filesize.sequence_number = 98765;
		memcpy(filesize.buffer, ack, strlen(ack));
		filesize.buffer_length = 98765;		

		if(sendto(sock, &filesize, sizeof(struct packetdata), 0, (struct sockaddr *)&from_addr, from_addr_length) < 0)    
	  	{
	    	int errsv = errno;
			printf("File not sent. Refer to error number: %d of the errno function\n", errsv);
			exit(1);
	  	}	

	  	struct dirent *direntry;
		DIR *dir = opendir(".");
		int cn = 0;
		char file_list[FORLISTINGMSGSIZE];

		if(dir != NULL)
		{
			while((direntry = readdir(dir)) != NULL)
			{
				cn += sprintf(file_list + cn, "%s\n",direntry -> d_name);
			}
		}
	  	closedir(dir);
		if(sendto(sock, file_list, sizeof(file_list), 0, (struct sockaddr *)&from_addr, from_addr_length) < 0)    
	  	{
	    	int errsv = errno;
			printf("File not sent. Refer to error number: %d of the errno function\n", errsv);
			exit(1);
	  	}
	}
	else
	{
		printf(" Incorrect command typed.");
		return;
	}
}

void put_function(int sock,struct sockaddr_in from_addr, unsigned int from_addr_length, char *file_name, char *value)
{
	int size_file_buffer = 0;

	strcpy(file_name,value+4);

	remove_old_file(file_name);

	FILE *fp = fopen(file_name,"w");

	if (fp == NULL) 
	{
		int errsv = errno;
		printf("File not opened. Refer to error number: %d of the errno function\n", errsv);	
		exit(1);
	}

	filesize.sequence_number = 98765;
	memcpy(filesize.buffer, ack, strlen(ack));
	filesize.buffer_length = 98765;		

	//Sending ack to confirm number of bytes received and send files
	if(sendto(sock, &filesize, sizeof(struct packetdata), 0, (struct sockaddr *)&from_addr, from_addr_length) < 0)    
  	{
    	int errsv = errno;
		printf("File not sent. Refer to error number: %d of the errno function\n", errsv);
		exit(1);
  	}

  	//Receiving file size and file name
	if((recvfrom(sock, &filesize, sizeof(struct packetdata), 0, (struct sockaddr *) &from_addr, (socklen_t *) &from_addr_length)) == -1)
  	{
    	int errsv = errno;
		printf("File not sent. Refer to error number: %d of the errno function\n", errsv);
		exit(1);
  	}

  	int file_size = filesize.buffer_length;
  	int packet_number = filesize.sequence_number;

	filesize.sequence_number = 98765;
	memcpy(filesize.buffer, ack, strlen(ack));
	filesize.buffer_length = 98765;		

	//Sending ack to confirm number of bytes received and send files
	if(sendto(sock, &filesize, sizeof(struct packetdata), 0, (struct sockaddr *)&from_addr, from_addr_length) < 0)    
  	{
    	int errsv = errno;
		printf("File not sent. Refer to error number: %d of the errno function\n", errsv);
		exit(1);
  	}

	printf("File size to be recieved: %d\n", file_size);

	for(int i = 0; i<packet_number+1; i++)
	{
		int sequence_number = 0;

		printf("\nBeginning of loop\n");

		memset(&packet_data, 0, sizeof(struct packetdata));
		//bzero(&packet_data,sizeof(struct packetdata));

		//Waiting to receive packet data
		if((recvfrom(sock, &packet_data, sizeof(struct packetdata), 0, (struct sockaddr *) &from_addr, (socklen_t *) &from_addr_length)) == -1)
		{
			int errsv = errno;
			printf("message not received. Refer to error number: %d of the errno function\n", errsv);	
			exit(1);
		}

  		printf("Received packet. Sending ACK\n");

  		//ACK to tell that packet is received.
		filesize.sequence_number = 98765;
		memcpy(filesize.buffer, ack, strlen(ack));
		filesize.buffer_length = 98765;		

		if(sendto(sock, &filesize, sizeof(struct packetdata), 0, (struct sockaddr *)&from_addr, from_addr_length) < 0)    
	  	{
	    	int errsv = errno;
			printf("File not sent. Refer to error number: %d of the errno function\n", errsv);
			exit(1);
	  	}	

	  	bzero(file_buffer,sizeof(file_buffer));

	  	sequence_number = packet_data.sequence_number;
	  	memcpy(file_buffer,packet_data.buffer,packet_data.buffer_length);
		size_file_buffer = packet_data.buffer_length;

		printf("size_file_buffer:%d\n",size_file_buffer);

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
		printf("file_size left: %d\n", file_size);

	}
	printf("\nFile transfer done. Sending ACK\n");

	filesize.sequence_number = 98765;
	memcpy(filesize.buffer, ack, strlen(ack));
	filesize.buffer_length = 98765;		

	//Sending ack to confirm number of bytes received and send files
	if(sendto(sock, &filesize, sizeof(struct packetdata), 0, (struct sockaddr *)&from_addr, from_addr_length) < 0)    
  	{
    	int errsv = errno;
		printf("File not sent. Refer to error number: %d of the errno function\n", errsv);
		exit(1);
  	}



	fclose(fp);
	return;
}

void get_function(int sock,struct sockaddr_in from_addr, unsigned int from_addr_length, char *file_name)
{
	
	size_t sub_file_size = 1000;
	int size_file_buffer;
	int size_sent = 0;

	FILE *fp;
	fp = fopen(file_name, "rb+");

	if(fp != NULL)
	{

		fseek(fp,0,SEEK_END);
		int file_size = ftell(fp);
		fseek(fp,0,SEEK_SET);

		int packet_number = file_size/sub_file_size;

		printf("No. of packets: %d\n", packet_number);

		filesize.sequence_number = 98765;
		memcpy(filesize.buffer, file_name, sizeof(file_name));
		filesize.buffer_length = file_size;		

		printf("%d\n", filesize.buffer_length);

		//Sending file size in a packet
		if(sendto(sock, &filesize, sizeof(struct packetdata), 0, (struct sockaddr *)&from_addr, from_addr_length) < 0)    
	  	{
	    	int errsv = errno;
			printf("File not sent. Refer to error number: %d of the errno function\n", errsv);
			exit(1);
	  	}

		// memset(file_buffer, 0, MAXBUFSIZE);


		printf("Size of the file to be sent: %d\n", file_size);

		for(int i = 0; i < packet_number+1; i++)
		{
			memset(&packet_data, 0, sizeof(struct packetdata));
			printf("\nBeginning of loop\n");
			bzero(file_buffer,sizeof(file_buffer));

			int bytes_Read = fread(file_buffer,1, sub_file_size,fp);

		    printf("%d", bytes_Read);

		    //Wait for ACK before sending files
		    if(recvfrom(sock, ackr, sizeof(ackr), 0, (struct sockaddr *) &from_addr, (socklen_t *) &from_addr_length) == -1)
			{
				int errsv = errno;
				printf("ACK not received. Refer to error number: %d of the errno function\n", errsv);
				exit(1);
			}

			printf("Received ACK\n");

			if(ackr == ack)
			{
				printf("\n\nMissed a packet\n\n\n");
				i--;
				continue;
			}

			packet_data.sequence_number = (i+1);
			memcpy(packet_data.buffer,file_buffer,bytes_Read);
			packet_data.buffer_length = bytes_Read;

		    printf("Sending data now\n");
			
			if(sendto(sock, &packet_data, sizeof(struct packetdata), 0, (struct sockaddr *)&from_addr, from_addr_length) == -1)    
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
		}
		
	    if(recvfrom(sock, ackr, sizeof(ackr), 0, (struct sockaddr *) &from_addr, (socklen_t *) &from_addr_length) == -1)
	  	{
	    	int errsv = errno;
			printf("ACK not sent. Refer to error number: %d of the errno function\n", errsv);
			exit(1);
	  	}
	  	printf("\nFile sent.\n");
		fclose(fp);
	  	return;
	}
}


void checking_function(int sock, struct sockaddr_in from_addr, unsigned int from_addr_length, char *value)
{
	strcpy(str1, value);

	printf("str1's first letter: %c \n", value[0]);

	// if()
	if(value[0] == 'e')
	{
		printf("Going to exit function\n");
		exit_function(sock, from_addr, from_addr_length, value);
	}
	else if(value[0] == 'l')
	{
		printf("Going to ls function\n");
		ls_function(sock, from_addr, from_addr_length, value);
		printf("came back!");

	}
	else if(value[0] == 'g')
	{
		char file_name[MAXBUFSIZE];
		bzero(file_name,sizeof(MAXBUFSIZE));
		// printf("Value received: %s\n", value);
		char *command;
		command = strtok(value, " ");
		strcpy(file_name ,strtok(NULL, " "));
		// printf("Converted values: %s ; %s \n", value, file_name);
		//printf("File name: %s\n", file_name);
		printf("Going to get function\n");
		get_function(sock, from_addr, from_addr_length, file_name);
	}
	else if(value[0] == 'p')
	{
		char file_name[MAXBUFSIZE];
		bzero(file_name,sizeof(MAXBUFSIZE));
		char *command;
		command = strtok(value, " ");
		strcpy(file_name ,strtok(NULL, " "));
		printf("Going to put function\n");
		put_function(sock, from_addr, from_addr_length, file_name, command);
	}
	else	
	{
		unclear_command(sock, from_addr, from_addr_length, value);	
	}
	// return;
}

int main (int argc, char * argv[] )
{
	struct sockaddr_in sin, from_addr;    						//"Internet socket address structure"
	unsigned int from_addr_length  = sizeof(from_addr);
	int sock; 

	strcpy(ack, " ACK for file. ");

	if (argc != 2)
	{
		printf ("Type Port number!\n" );
		exit(1);
	}

	/******************
	  This code populates the sockaddr_in struct with
	  the information about our socket
	 ******************/
	bzero(&sin,sizeof(sin));                    //zero the struct
	sin.sin_family = AF_INET;                   //address family
	sin.sin_port = htons(atoi(argv[1]));        //htons() sets the (short) host byte order to (short) network byte order. atoi() converts the number in a string to an int.
	sin.sin_addr.s_addr = INADDR_ANY;    //Binding socket to all interfaces. Supplies the IP address of the local machine. htonl() sets the (long) host byte order to (long) network byte order


	//Causes the system to create a generic socket of type UDP (datagram)
	if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		int errsv = errno;
		printf("Socket not created. Refer to error number: %d of the errno function\n", errsv);
		exit(1);
	}


	/******************
	  Binding the created socket structure to an address specified by the second argument. Once we've created a socket, we must bind that socket to the 
	  local address and port we've supplied in the sockaddr_in struct
	 ******************/
	if (bind(sock, (struct sockaddr *)&sin, sizeof(sin)) == -1)
	{
		int errsv = errno;
		printf("Socket not bound. Refer to error number: %d of the errno function\n", errsv);
		exit(1);
	}

	printf("Server setup complete. Waiting for command.\n");

	while(1)
	{
		//waits for an incoming message
		bzero(buffer,sizeof(buffer));


		//Receive the command and (if required) file name
		if(recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *) &from_addr, (socklen_t *) &from_addr_length) == -1)
		{
			int errsv = errno;
			printf("Data not received. Refer to error number: %d of the errno function\n", errsv);
			exit(1);
		}

		printf("The client says: %s \n", buffer);

		checking_function(sock,from_addr, from_addr_length, buffer);

		printf("\n\nWaiting for the next command\n\n");
	}
}