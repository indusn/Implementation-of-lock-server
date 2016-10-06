
#include <string.h>
#include <arpa/inet.h>
#include <stdlib.h>
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

using namespace std;

// Constants
#define ALPHA 0
#define BRAVO 5
#define MAXHOSTNAME 255
int portNumber = 1998;
int bufsize = 1024;
int client;
char buffer[1024];
struct sockaddr_in server_addr;
char hostname[MAXHOSTNAME+1];
struct hostent *hp;

// Templates to be filled

int connectionEstablishment() {
	
	gethostname(hostname,MAXHOSTNAME);
	if((hp=gethostbyname(hostname)) == NULL){ //refered BSD sockets pdf
		errno = ECONNREFUSED;
		return -1;
	}

	memset(&server_addr,0,sizeof(server_addr));
	memcpy((char *)&server_addr.sin_addr,hp->h_addr,hp->h_length);
	server_addr.sin_family= hp->h_addrtype;
	server_addr.sin_port= htons((u_short)portNumber);

	client = socket(AF_INET, SOCK_STREAM, 0);
    if (client < 0) {
        cout << "Error in establishing socket" << endl;
        return -1;
    }
    //cout << "Client socket has been created" << endl;
	if (connect(client,(struct sockaddr *)&server_addr, sizeof(server_addr)) != 0){
		cout<<"Error in connectionEstablishment";
		return -1;
	}
	//cout << "Connection to the server in port " << portNumber << endl;
	return 0;
}

int create_lock(int resource){
	if (connectionEstablishment() == 0){
		cout<<"=>Inside create lock of "<<resource;
		char res[] = "\0";
		char action[] = "createLock$";
		sprintf(res,"%d",resource);
		strcat(action,res);
		send(client,action,bufsize,0);
		recv(client, buffer, bufsize, 0);	
		if (strcmp(buffer,"error") == 0){
			cout << "\nError: Lock already created for " << resource << "\n";		
			return -1;
		}
		else if (strcmp(buffer,"success") == 0){
			cout << "\nLock creation Successful on resource " << resource << "\n";
			return 0;
		}	
	}
}

int read_lock(int resource){
	if(connectionEstablishment() == 0){
		cout<<"=>Inside read lock of "<<resource;
		char res[] = "\0";
		char action[] = "readLock$";	
		sprintf(res,"%d",resource);
		strcat(action,res);
		send(client,action,bufsize,0);
		recv(client, buffer, bufsize, 0);
		if (strcmp(buffer,"error") == 0){
			cout << "\n Error: Lock is not established on resource " << resource << "\n";		
			return -1;
		}
		else if (strcmp(buffer,"success") == 0){
			cout << "\n Read lock is established on resource " << resource << "\n";
			return 0;
		}
		else if (strcmp(buffer,"wait") == 0){
			do
			{
				recv(client, buffer, bufsize,0);
			}while(strcmp(buffer, "success") != 0);
			cout << "\n Read lock is established on resource " << resource << "\n";
			return 0;	
		}			

	}
}

int write_lock(int resource){
	if(connectionEstablishment() == 0){
		cout<<"=>Inside write lock of "<<resource;
		char res[] = "\0";
		char action[] = "writeLock$";		
		sprintf(res,"%d",resource);
		strcat(action,res);
		send(client,action,bufsize,0);
		recv(client, buffer, bufsize, 0);
		if (strcmp(buffer,"error") == 0){
			cout << "\n Error: Lock is not established on resource " << resource << "\n";		
			return -1;
		}
		else if (strcmp(buffer,"success") == 0){
			cout << "\n Write lock is established on resource " << resource << "\n";
			return 0;
		}
		else if (strcmp(buffer,"wait") == 0){
			do
			{
				recv(client, buffer, bufsize,0);
			}while(strcmp(buffer, "success") != 0);
			cout << "\n Write lock is established on resource " << resource << "\n";
			return 0;		
		}		

	}
}

int read_unlock(int resource){
	if(connectionEstablishment() == 0){
		cout<<"=>Inside read unlock of "<<resource;
		char res[] = "\0";
		char action[] = "readUnlock$";		
		sprintf(res,"%d",resource);
		strcat(action,res);
		send(client,action,bufsize,0);
		recv(client, buffer, bufsize, 0);
		if (strcmp(buffer,"error") == 0){
			cout << "\n Error: Read UnLock is not possible on resource " << resource << "\n";		
			return -1;
		}
		else if (strcmp(buffer,"success") == 0){
			cout << "\n Read lock is unlocked Successfully on resource " << resource << "\n";
			return 0;
		}
		else if (strcmp(buffer,"wait") == 0){
			do
			{
				recv(client, buffer, bufsize,0);
			}while(strcmp(buffer, "success") != 0);
			cout << "\n Read lock is unlocked Successfully on resource " << resource << "\n";
			return 0;
		}		

	}
}

int write_unlock(int resource){
	if(connectionEstablishment() == 0){
		cout<<"=>Inside write unlock of "<<resource;
		char res[] = "\0";
		char action[] = "writeUnlock$";		
		sprintf(res,"%d",resource);
		strcat(action,res);
		send(client,action,bufsize,0);
		recv(client, buffer, bufsize, 0);
		if (strcmp(buffer,"error") == 0){
			cout << "\n Error: Write UnLock is not possible on resource " << resource << "\n";		
			return -1;
		}
		else if (strcmp(buffer,"success") == 0){
			cout << "\n Write lock is unlocked Successfully on resource " << resource << "\n";
			return 0;
		}
		else if (strcmp(buffer,"wait") == 0){
			do
			{
				recv(client, buffer, bufsize,0);
			}while(strcmp(buffer, "success") != 0);
			cout << "\n Write lock is unlocked Successfully on resource " << resource << "\n";
			return 0;
		}		

	}
}

int delete_lock(int resource){
	if(connectionEstablishment() == 0){
		cout<<"=>Inside delete lock of "<<resource;
		char res[] = "\0";
		char action[] = "deleteLock$";
		sprintf(res,"%d",resource);
		strcat(action,res);
		send(client,action,bufsize,0);
		recv(client, buffer, bufsize, 0);
		if (strcmp(buffer,"error") == 0){
			cout << "\nError: No lock is established on the resource " << resource << "\n";		
			return -1;
		}
		else if (strcmp(buffer,"success") == 0){
			cout << "\nLock is deleted Successfully on resource " << resource << "\n";
			return 0;
		}		

	}
}

int kill_server(){
	if(connectionEstablishment() == 0){
		send(client,"killServer$", bufsize, 0);
	}
}

main () {
	int pid; // child's pid

        // Before the fork
        cout << "Create lock ALPHA\n";
	create_lock(ALPHA);
        cout << "Create lock BRAVO\n";
	create_lock(BRAVO);
        cout << "Parent requests write permission on lock BRAVO\n";
	write_lock(BRAVO);
        cout << "Write permission on lock BRAVO was granted\n";
        cout << "Parent requests read permission on lock ALPHA\n";
	read_lock(ALPHA);
        cout << "Read permission on lock ALPHA was granted\n";
	sleep(1);
	
	// Fork a child
	if ((pid = fork()) == 0) {
		// Child process
	        cout << "Child requests read permission on lock ALPHA\n";
		read_lock(ALPHA); // This permission should be granted
        	cout << "Read permission on lock ALPHA was granted\n";
		sleep(1);
	        cout << "Child releases read permission on lock ALPHA\n";
		read_unlock(ALPHA);
		sleep(1);
        	cout << "Child requests write permission on lock BRAVO\n";
		write_lock(BRAVO); // Should wait until parent relases its lock
        	cout << "Write permission on lock BRAVO was granted\n";
		sleep(1);
	        cout << "Child releases write permission on lock BRAVO\n";
		write_unlock(BRAVO);
		cout << "Child terminates\n";
                _exit(0);
	} // Child

	// Back to parent
        cout << "Parent releases read permission on lock ALPHA\n";
	read_unlock(ALPHA);
        cout << "Parent requests write permission on lock ALPHA\n";
	write_lock(ALPHA); // Should wait until child removes its read lock
        cout << "Write permission on lock ALPHA was granted\n";
	sleep(1);
        cout << "Parent releasesweite permission on lock ALPHA\n";
	write_unlock(ALPHA);
	sleep(1);
        cout << "Parent releases write permission on lock BRAVO\n";
	write_unlock(BRAVO);

	// Child and parent join
        while (pid != wait(0));  // Busy wait
	delete_lock(ALPHA);
        delete_lock(BRAVO);
        // We assume that failed operations return a non-zero value
        if (write_lock(ALPHA) != 0) {
		cout << "Tried to access a deleted lock\n";
	}
	kill_server();
} // main
