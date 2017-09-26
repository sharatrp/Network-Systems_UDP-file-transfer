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
} packet_data;

void remove_old_file(char* file_name);
void exit_function(int sock, char *value);
void ls_function(int sock, struct sockaddr_in from_addr, unsigned int from_addr_length, char *value);
void put_function(int sock, char *value);
void get_function(int sock, struct sockaddr_in from_addr, unsigned int from_addr_length, char *value);
void checking_function(int sock, struct sockaddr_in from_addr, unsigned int from_addr_length, char *value);

void remove_old_file(char* file_name) 
{
	if (access(file_name, F_OK) != -1)
		{
			remove(file_name);
		}
}

void exit_function(int sock, char *value)
{
	strcpy(str2, "exit");
	int compare = strcmp(str2, value);

	if(compare == 0)
	{
		printf ("Going to exit now");
	}
}

void ls_function(int sock, struct sockaddr_in from_addr, unsigned int from_addr_length, char *value)
{
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
  	return;
}

void put_function(int sock, char *value)
{

}

void get_function(int sock,struct sockaddr_in from_addr, unsigned int from_addr_length, char *file_name)
{
	

	size_t sub_file_size = 1000;
	int size_file_buffer;
	int size_sent = 0;

	FILE *fp;
	fp = fopen(file_name, "r");
	// {
 //    	int errsv = errno;
	// 	printf("File not opened. Refer to error number: %d of the errno function\n", errsv);
	// 	exit(1);
 //  	}

	if(fp != NULL)
	{

		fseek(fp,0,SEEK_END);
		size_t file_size = ftell(fp);
		fseek(fp,0,SEEK_SET);

		int sub_file_number = file_size/sub_file_size;

		printf("No. of packets: %d\n", sub_file_number);

		memset(file_buffer, '\0', MAXBUFSIZE);

		char ToSend[10];
		sprintf(ToSend, "%lu", file_size);

		//Sending file size
		if(sendto(sock, ToSend, MAXBUFSIZE, 0, (struct sockaddr *)&from_addr, from_addr_length) < 0)    
	  	{
	    	int errsv = errno;
			printf("File not sent. Refer to error number: %d of the errno function\n", errsv);
			exit(1);
	  	}

		printf("File size to be sent: %lu\n", file_size);

		for(int i = 0; i < sub_file_number+1; i++)
		{
			printf("\nBeginning of loop\n");
			bzero(file_buffer,sizeof(file_buffer));

			int bytes_Read = fread(file_buffer,1, sub_file_size,fp);

		   // sub_file_size = file_size - sub_file_size;

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
			strcpy(packet_data.buffer, file_buffer);
			packet_data.buffer_length = bytes_Read;

		    printf("Sending data now\n");
			
			if(sendto(sock, &packet_data, sizeof(struct packetdata), 0, (struct sockaddr *)&from_addr, from_addr_length) == -1)    
		  	{
		    	int errsv = errno;
				printf("File not sent. Refer to error number: %d of the errno function\n", errsv);
				exit(1);
		  	}
		  	else
		  	{
		  		printf("File part %d sent\n", i);
		  	}

			printf("File size sent till now: %d\n", size_sent);
		  	size_sent += bytes_Read;


		}
		
		if(sendto(sock, ack, sizeof(ack), 0, (struct sockaddr *)&from_addr, sizeof(from_addr_length)) == -1)    
	  	{
	    	int errsv = errno;
			printf("ACK not sent. Refer to error number: %d of the errno function\n", errsv);
			exit(1);
	  	}


		fclose(fp);
	  	return;
	}
}


void checking_function(int sock, struct sockaddr_in from_addr, unsigned int from_addr_length, char *value)
{
	strcpy(str1, value);

	printf("str1's first letter: %c \n", value[0]);

	if(value[0] == 'e')
	{
		printf("Going to exit function\n");
		exit_function(sock, buffer);
	}
	else if(value[0] == 'l')
	{
		printf("Going to ls function\n");
		ls_function(sock, from_addr, from_addr_length, buffer);
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
		get_function(sock,from_addr, from_addr_length, file_name);
	}
	else if(value[0] == 'p')
	{
		printf("Going to put function\n");
		put_function(sock, buffer);
	}
	else	
	{
		// struct sockaddr_in sin;
		// unsigned int remote_length;        			//length of the sockaddr_in structure
		char st1[MAXBUFSIZE], st2[MAXBUFSIZE], st3[MAXBUFSIZE];

		strcpy(st1, "Command given:");
		strcpy(st2, value);
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
		exit(1);
	}

	printf("One round done.\n");


	// printf("%d \n",check);
	// printf("%lu \n",sizeof(check));

	// if(check1 == 0) exit_function();
	// else if(check2 == 0) ls_function();
	// else
	// {
	// 	//Do Nothing
	// }
	return;
}

int main (int argc, char * argv[] )
{
	struct sockaddr_in remote, from_addr;    						//"Internet socket address structure"
	unsigned int remote_length = sizeof(remote);        			//length of the sockaddr_in structure
	unsigned int from_addr_length  = sizeof(from_addr);
	int sock; 

	strcpy(ack, " ACK for file. ");

	struct sockaddr_in sin;

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
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
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

		// if(sendto(sock, ack, sizeof(ack), 0, (struct sockaddr *)&from_addr, from_addr_length) < 0)    
	 //  	{
	 //    	int errsv = errno;
		// 	printf("File not sent. Refer to error number: %d of the errno function\n", errsv);
		// 	exit(1);
	 //  	}


		checking_function(sock,from_addr, from_addr_length, buffer);

/*		 
		if((FILE *fp = fopen(buffer, 'r')) == 0)
		{
			int errsv = errno;
			printf("File doesn't exist. Please check file name and try again");
			exit(1);
		}

*/
	}	
	close(sock);
}