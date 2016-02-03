//STL headers
#include <iostream>
#include <cstring>

//Libevent headers
#include <event2/event.h>

//Socket headers + others that are needed
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

//Database Headers
#include <libpq-fe.h>

//Database globals for Libevent (who is only one thread)
//Only located in main.cpp scope for libevent

PGconn *conn;

//Thrift headers

//Socket defines
#define BUFLEN 512
#define NPACK 10
#define UDPPORT 32512
#define THRPORT 32513 //thrift port

//UDP Message defines
//Broken into two bytes, giving a total of 65k different options. Not that we need that
//many, this is more about giving say 1 for file browsers and then 2 for current
//directory. We also avoid all the big and little endian shit.
//
//In the event of a request needing more information, like ls of a certain directory,
//than a thrift request will be made first to get that information.

//First, defining type of message
#define DEBUG				0x00
#define HELLOWORLD	0x01
#define FILEBROWSER	0x02
//reserve 0x03-0x06 to FILEBROWSER

//Second, defining the message
//0x01 or HelloWorld
#define SENDHELLO 0x00

//0x02 FILEBROWSER
#define FB_RETURNDIRECTORY

//Initialise and removal things
event* init(event_base*, int);
void end(event_base*, int, event*);


//Information from the Vesta clients.
void vestaCommunication_cb(evutil_socket_t, short, void *);

int main(void){

	//Insert startup for the Postgre Database.
	conn = PQconnectdb("user=mercury password=windy dbname=mercury");

  if (PQstatus(conn) == CONNECTION_BAD) {
    fprintf(stderr, "Connection to database failed: %s\n",
            PQerrorMessage(conn));
    exit(-3);
  }

	//Remove unnecessary crap from the Postgre Database. In case the shutdown was poor.
	//We don't want the old one. I could very well be wrong, but I'm sure Delan
	//or Tom will say yes/no on this
	PGresult *res = PQexec(conn, "DROP TABLE IF EXISTS Connections");

	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    exit(-4);
  }

	//insert Postgre connections table
	res = PQexec(conn, "CREATE TABLE Connections(vid INTEGER PRIMARY KEY," \
							"addr INET, port INT)");
							//vid = vesta client id, addr and port are the return info
							//when I get better at this database, i'll sanitise port to be
							//only valid addresses. Until then, we will have unlimited range

	if (PQresultStatus(res) != PGRES_COMMAND_OK) {
    exit(-5);
  }

	//Insert code here to create the thread for the Thrift portion. 1000% must be thread.
	//If we create a new process we cannot share file descriptors.

	//Start the UDP socket
	int udpSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (udpSocket == -1){
		exit(-1);
	}

	//Set up socket address information
	sockaddr_in si_me;
	std::memset((char *) &si_me, 0, sizeof(si_me));

	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(UDPPORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);

	//Bind the socket to listen for everything on the port
	if(bind(udpSocket, (struct sockaddr*) &si_me, sizeof(si_me)) == -1){
		exit(-2);
	}

	//Start the Libevent for the event bases.
	event_base* eb = NULL;
	event* e = init(eb, udpSocket);

	//Start the looping
	//event_base_dispatch(eb);
	printf("Got here\n");
	//Kill and free everything
	end(eb, udpSocket, e);
	return 0;
}

//All communication between
void vestaCommunication_cb(evutil_socket_t s, short d, void* p){

}

//Init the event stuff
event* init(event_base* eb, int socket){
	eb = event_base_new();
	event* e = event_new(eb, socket, EV_READ|EV_PERSIST, &vestaCommunication_cb, 0);
	event_add(e,NULL);
	return e;
}

//Again, only a function because we may need to change how the event base dies
void end(event_base* eb, int socket, event *e){
	event_free(e);
	event_base_free(eb);
	close(socket);
}
