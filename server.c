// Server side C/C++ program to demonstrate Socket programming

//including libraries
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>
#include <netinet/in.h>
#include <string.h>

//including libraries for ls
#include <fcntl.h>
#include <pwd.h>
#include <grp.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/param.h>

//dirent struct used in ls
struct dirent **namelist;

//defining port
#define PORT 8080

//driver code
int main(int argc, char const *argv[])
{
	//declaring data structures
	int server_fd, new_socket, valread;
	struct sockaddr_in address;  // sockaddr_in - references elements of the socket address. "in" for internet
	int opt = 1;
	int addrlen = sizeof(address);
	char buffer[1024] = {0};
	char *hello = "Hello from server";

	char *cmd1 = "listall";
	char ls[1000]={0};
	char commands[1000]={0};

	// Creating socket file descriptor
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)  // creates socket, SOCK_STREAM is for TCP. SOCK_DGRAM for UDP
	{
		perror("socket failed");
		exit(EXIT_FAILURE);
	}

	// This is to lose the pesky "Address already in use" error message
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,
				&opt, sizeof(opt))) // SOL_SOCKET is the socket layer itself
	{
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	address.sin_family = AF_INET;  // Address family. For IPv6, it's AF_INET6. 29 others exist like AF_UNIX etc.
	address.sin_addr.s_addr = INADDR_ANY;  // Accept connections from any IP address - listens from all interfaces.
	address.sin_port = htons( PORT );    // Server port to open. Htons converts to Big Endian - Left to Right. RTL is Little Endian

	// Forcefully attaching socket to the port 8080
	if (bind(server_fd, (struct sockaddr *)&address,
				sizeof(address))<0)
	{
		perror("bind failed");
		exit(EXIT_FAILURE);
	}

	// Port bind is done. You want to wait for incoming connections and handle them in some way.
	// The process is two step: first you listen(), then you accept()
	if (listen(server_fd, 3) < 0) // 3 is the maximum size of queue - connections you haven't accepted
	{
		perror("listen");
		exit(EXIT_FAILURE);
	}

	//declaring chilg pid for forking
	pid_t childpid;

	/////////////////////////////////////////////////////////////////////////////////
	while(1)
	{
		//cleaning buffer
		memset(buffer,0,sizeof(buffer));

		// returns a brand new socket file descriptor to use for this single accepted connection. Once done, use send and recv
		if ((new_socket = accept(server_fd, (struct sockaddr *)&address,
						(socklen_t*)&addrlen))<0)
		{
			perror("accept");
			exit(EXIT_FAILURE);
		}

		//forking for every client
		childpid=fork();
		if(childpid==0)
		{

			valread = read( new_socket , buffer, 1024);  // read infromation received into the buffer

			//listall command
			if(!(strcmp(buffer,cmd1)))
			{
				//clearing ls buffer
				memset(ls,0,sizeof(ls));

				//scan '.' directory and store all files in namelist(coz filter=null and sorts using alphasort)
				int nof=scandir(".",&namelist,NULL,alphasort);
				if(nof==-1)
					perror("SCANDIR");
				else
				{
					int count;
					for(count=0;count<nof;count++)
					{	
						if(strncmp(namelist[count]->d_name,".",1) && strncmp(namelist[count]->d_name,"..",2))
						{
							strcat(ls,namelist[count]->d_name);
							strcat(ls,"\n");//storing in ls buffer
						}
					}
				}
				send(new_socket,ls,strlen(ls),0);  // use sendto() and recvfrom() for DGRAM
				return 0;
			}

////////////////////////////////////////////////////////////////////////////////////////////////

			//for send command
			else
			{
				char sendbuffer[10000]={0};
				FILE* file;
				file=fopen(buffer,"r");

				//if error in file
				if(file==NULL)
				{
					strcpy(sendbuffer,"Cannot download the file\n");
					send(new_socket,sendbuffer,strlen(sendbuffer),0);  // use sendto() and recvfrom() for DGRAM
					return 0;
				}
				else
				{
					char ch;
					ch=fgetc(file);
					while(ch!=EOF)
					{
						strcat(sendbuffer,&ch);
						ch=fgetc(file);
					}
					fclose(file);
					send(new_socket,sendbuffer,strlen(sendbuffer),0);  // use sendto() and recvfrom() for DGRAM
					return 0;
				}
			}
		}
	}
	return 0;
}
