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
/* You will have to modify the program below */

#define MAXBUFSIZE 1000
#define FORLISTINGMSGSIZE 1000

extern int errno;
	// 	printf("File not opened. Refer to error number: %d of the errno function\n", errsv);
	// 	exit(1);
struct sockaddr_in from_addr;    			//"Internet socket address structure"
unsigned int from_addr_length  = sizeof(from_addr);

int nbytes;                        			//number of bytes we receive in our message
char buffer[MAXBUFSIZE];           			//a buffer to store our received message
char file_buffer[MAXBUFSIZE];
int sock;                          			//This will be our socket

char str1[MAXBUFSIZE], str2[MAXBUFSIZE], str3[MAXBUFSIZE];

void remove_old_file(char* file_name);
void exit_function(int sock, char *value);
void ls_function(int sock, char *value);
void put_function(int sock, char *value);
void get_function(int sock, char *value);
void checking_function(int sock, char *value);

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

void ls_function(int sock, char *value)
{
	// strcpy(str3, "ls");
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
	printf("%s", file_list);
  	closedir(dir);
	if(sendto(sock, &file_list, sizeof(file_list), 0, (struct sockaddr *)&from_addr, from_addr_length) < 0)    
  	{
    	int errsv = errno;
		printf("File not sent. Refer to error number: %d of the errno function\n", errsv);
		exit(1);
  	}
}

void put_function(int sock, char *value)
{

}

void get_function(int sock, char *value)
{
	char file_name[MAXBUFSIZE];
	long sub_file_size = 1000;
	char ack [15];

	strcpy(file_name, value+4);
	FILE *fp = fopen(file_name, "r");

	if(fp != NULL)
	{
		fseek(fp,0,SEEK_END);
		long file_size = ftell(fp);
		fseek(fp,0,SEEK_SET);

		printf("File size %ld , sub_file_size : %ld\n",file_size,sub_file_size);
		
		long sub_file_number = file_size / sub_file_size;

		printf("No. of packets: %ld\n", ++sub_file_number);

		// file_buffer = (char*) malloc (file_size);
		// if(file_buffer == NULL) {fputs("Memory error",stderr); exit(2);}

		memset(file_buffer, '\0', MAXBUFSIZE);

		char ToSend[10];

		sprintf(ToSend, "%ld", sub_file_number);

		printf("Sending number of packets: %s\n", ToSend);

		if( (nbytes = sendto(sock, ToSend, sizeof(ToSend), 0, (struct sockaddr *)&from_addr, from_addr_length)) < 0)    
	  	{
	    	int errsv = errno;
			// printf("File count not sent. Refer to error number: %d of the errno function\n", errsv);
			fprintf(stderr, "Nbytes : %d .File count not sent : %s\n",nbytes,
				strerror(errno));
			close(sock);
			exit(1);
	  	}

	  	printf("Sent the file count\n");
		for(int i = sub_file_number+1; i>0; i--)
		{
			printf("\nBeginning of loop\n");

			if(fread(file_buffer, sub_file_size, 1,fp) == 0)
		    {
		   //    int errsv = errno;
			  // printf("File not read. Refer to error number: %d of the errno function\n", errsv);
			  // fprintf(stderr, "%s\n", strerror(errno));
			  // // printf("unable to copy file into buffer\n");
		   //    exit(1);
		    }

		    printf("Sending data now\n");
			
			if(sendto(sock, &file_buffer, strlen(file_buffer), 0, (struct sockaddr *)&from_addr, from_addr_length) == -1)    
		  	{
		    	int errsv = errno;
				printf("File not sent. Refer to error number: %d of the errno function\n", errsv);
				exit(1);
		  	}

		  	else

		  	{
		  		printf("File part %d sent.\n", i);
		  	}
		  	// file_buffer += sub_file_size;

		  	if(recvfrom(sock, ack, sizeof(ack), 0, (struct sockaddr *) &from_addr, &from_addr_length) == -1)
			{
				int errsv = errno;
				printf("Data not received. Refer to error number: %d of the errno function\n", errsv);
				exit(1);
			}

			printf("%s\n", ack);
		}
		fclose(fp);

	  	return;
	}else{
		int errsv = errno;
		printf("File not opened. Refer to error number: %d of the errno function\n", errsv);
		close(sock);
		exit(1);
	}
}


void checking_function(int sock, char *value)
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
		ls_function(sock, buffer);
	}
	else if(value[0] == 'g')
	{
		printf("Going to get function\n");
		get_function(sock, buffer);
	}
	else if(value[0] == 'p')
	{
		printf("Going to put function\n");
		put_function(sock, buffer);
	}
	else	
	{
		// struct sockaddr_in sin;
		// unsigned int from_addr_length;        			//length of the sockaddr_in structure
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
		if (sendto(sock, &buffer, sizeof(buffer), 0, (struct sockaddr*) &from_addr, from_addr_length) == -1)
		{
			int errsv = errno;
			printf("Data not sent. Refer to error number: %d of the errno function\n", errsv);
			exit(1);        
		}
	}

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
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		int errsv = errno;
		printf("Socket not created. Refer to error number: %d of the errno function\n", errsv);
		exit(1);
	}


	/******************
	  Binding the created socket structure to an address specified by the second argument. Once we've created a socket, we must bind that socket to the 
	  local address and port we've supplied in the sockaddr_in struct
	 ******************/
	if (bind(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		int errsv = errno;
		printf("Socket not bound. Refer to error number: %d of the errno function\n", errsv);
		exit(1);
	}

	while(1)
	{
		//waits for an incoming message
		bzero(buffer,sizeof(buffer));

		if(recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *) &from_addr, &from_addr_length) == -1)
		{
			int errsv = errno;
			printf("Data not received. Refer to error number: %d of the errno function\n", errsv);
			exit(1);
		}

		printf("The client says: %s \n", buffer);


		checking_function(sock, buffer);

/*		 
		if((FILE *fp = fopen(buffer, 'r')) == 0)
		{
			int errsv = errno;
			printf("File doesn't exist. Please check file name and try again");
			exit(1);
		}

*/
		printf("One round done.\n");
	}	
	close(sock);
}