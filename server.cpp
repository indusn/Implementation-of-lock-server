#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <arpa/inet.h>
#include <cstddef>
using namespace std;

#define MAXHOSTNAME 255

int i,j;
int bufsize =1024;
char buffer[1024];
int resourceCount=0;
string resourceArray[16]={""};
int readLockArray[16]={0};
int readLockCount[16];
int writeLockArray[16]={0};
int readLockQueue[16][50];
int writeLockQueue[16][50];

int get_position(string res)
{
	for(int i = 0; i < resourceCount; i++)
		if(resourceArray[i] == res)
			return i;
	return -1;
}

void handle_operation(string operation, string res, int server) // handling the client requests 
{
	if (operation == "createLock"){

		int position = get_position(res);

		if(position == -1)
		{
			write(server,"success",bufsize);
			//cout<< "Inside createLock of " << res << endl;
			resourceArray[resourceCount] = res;
			resourceCount++;
			return;
		}
		else
		{
			write(server,"error",bufsize);
			return;
		}
	}
		
	if (operation == "readLock"){
		int position = get_position(res);

		if(position == -1)
		{
			write(server,"error",bufsize);
		}

		if (writeLockArray[position]==1){
			for(int j=0;j<50;j++){
				if(readLockQueue[position][j] == -1){
					readLockQueue[position][j] = server;
					break;
				}
			}
			write(server,"wait",bufsize);
		}
		else{
			readLockArray[position] = 1;
			readLockCount[position]++;			
			write(server,"success",bufsize);
		}
		return;
	}
	
	if (operation == "writeLock"){
		int position = get_position(res);

		if(position == -1)
		{
			write(server,"error",bufsize);
			return;
		}

		if (writeLockArray[position] == 1 || readLockArray[position] == 1){
			for(int j=0;j<50;j++){
				if(writeLockQueue[position][j] == -1){
					writeLockQueue[position][j] = server;
					break;
				}
			}
			write(server,"wait",bufsize);
		}
		else{
			writeLockArray[position] = 1;
			write(server,"success",bufsize);
		}
		return;
	}
	
	if (operation == "readUnlock"){
		int position = get_position(res);

		if(position == -1 || readLockArray[position] == 0 || writeLockArray[position] == 1)
		{
			write(server,"error",bufsize);
			return;
		}

		write(server,"success",bufsize);			
		readLockCount[position]--;
		
		if(readLockCount[position] == 0)
			readLockArray[position] = 0;
		else
			return;

		if(writeLockQueue[position][0] != -1)
		{
			writeLockArray[position] = 1;
			write(writeLockQueue[position][0], "success", bufsize);
			for(j=0;j<49;j++){	
				writeLockQueue[position][j]=writeLockQueue[position][j+1];
				writeLockQueue[position][j+1] = -1;
			}
		}
		return;
	}
	
	
	if (operation == "writeUnlock"){
		int position = get_position(res);

		if(position == -1 || writeLockArray[position] == 0 || readLockArray[position] == 1)
		{
			write(server,"error",bufsize);
			return;
		}

		write(server,"success",bufsize);
		writeLockArray[position] = 0;

		if(writeLockQueue[position][0] != -1)
		{
			writeLockArray[position] = 1;
			write(writeLockQueue[position][0], "success", bufsize);
			
			for(j=0;j<49;j++){	
				writeLockQueue[position][j]=writeLockQueue[position][j+1];
				writeLockQueue[position][j+1] = -1;
			}

			return;
		}

		if(readLockQueue[position][0] != -1)
		{
			readLockArray[position] = 1;
			
			for(j=0;j<50;j++){											
				if(readLockQueue[position][j] == -1)
					break;
				write(readLockQueue[position][j], "success", bufsize);
				readLockQueue[position][j] = -1;
				readLockCount[position]++;
			}
			return;
		}
	}
	
	if (operation == "deleteLock"){
		int position = get_position(res);

		if(position == -1)
		{
			write(server,"error",bufsize);
			return;
		}
		
		write(server,"success",bufsize);
		for(int i=position; i < resourceCount - 1; i++)
		{
			resourceArray[i] = resourceArray[i+1];
		}
		resourceArray[resourceCount-1]="";
		resourceCount--;
		return;
	}
}

int main(){

	
	int client,server;
	int portNumber = 1998; 
	
	char myname[MAXHOSTNAME+1];
	struct sockaddr_in socAddr;
	socklen_t size;
	struct hostent *hp;
    
    memset(readLockQueue,-1,3200); //initiallizing queue with default vaue -1
	memset(writeLockQueue,-1,3200);

	memset(&socAddr, 0, sizeof(struct sockaddr_in)); 
	gethostname(myname, MAXHOSTNAME); 	
    hp= gethostbyname(myname); // refered BSD socket pdf

    if (hp == NULL) 
		return(-1);
    socAddr.sin_family = hp->h_addrtype; 
    socAddr.sin_port = htons(portNumber); 
	client = socket(AF_INET, SOCK_STREAM, 0);
	
	if (client < 0){
		cout<<"Error in establishing socket!\n";
		exit(1);
	}
	
	cout<<"Server socket connected\n";
	
	if (bind(client,(struct sockaddr *)&socAddr,sizeof(socAddr)) < 0) {
		cout<<"Error when binding connection!\n";
		exit(1);
	}
	size = sizeof(socAddr);
	
	cout<<"Looking for clients...\n";
	listen(client, 3); // refered BSD socket pdf

	while(true)
	{	
		server = accept( client,0,0);

		if(server<0){
			cout<<"Error while accepting client!\n";
			exit(1);
		}
		memset(buffer,0,sizeof(buffer));
		recv(server,buffer,bufsize,0);	

		string resource = buffer;
		string delimiter = "$";
		size_t pos = 0;
		string request;

		while((pos=resource.find(delimiter)) != string::npos){  //splitting of string using delimiter
  			request=resource.substr(0,pos);
  			resource.erase(0,pos+delimiter.length());
		}
		
		cout << "\nClient requests " << request << " on " << resource <<"\n";
		
		if (request == "killServer"){
			cout << "\nServer terminated by entering " << request << endl ;
			break;
		}

		handle_operation(request, resource, server); //handling client requests 
	}
	
    close(client);
    return 0;
}
