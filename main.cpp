//STL headers
#include <iostream>
#include <sstream>
#include <cstring>

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

#define BUFLEN 512

//UDP Message defines
//Broken into two bytes, giving a total of 65k different options. Not that we need that
//many, this is more about giving say 1 for file browsers and then 2 for current
//directory. We also avoid all the big and little endian shit.
//
//In the event of a request needing more information, like ls of a certain directory,
//than a thrift request will be made first to get that information.

//First, defining type of message
#define DEBUG 			0x00 //0x01 as well
#define HELLOWORLD	0x02
#define FILEBROWSER	0x03
//others
#define CONNECTION  0xFE //IE, Vesta connecting to Mercury

//reserve 0x03-0x06 to FILEBROWSER

//Second, defining the message
//0x01 or HelloWorld
#define SENDHELLO 0x00

//0x02 FILEBROWSER
#define FB_RETURNDIRECTORY

using namespace std;

//Information from the Vesta clients.
void vestaCommunication_cb(int);

int main(void){

	//Insert startup for the Postgre Database.
	//the string was a test, and it worked nicely!
	string s = "user=mercury password=windy dbname=mercury";
	conn = PQconnectdb(s.c_str());

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

	//Start the looping
	//event_base_dispatch(eb);
	printf("Got here\n");
	vestaCommunication_cb(udpSocket);
	//Kill and free everything
	//don't really have to anymore. OS will wipe out everything
	return 0;
}


template <typename T>
std::string tostring(const T& t)
{
    std::ostringstream ss;
    ss << t;
    return ss.str();
}

//All communication between
void vestaCommunication_cb(int socket){
	//create the return socket info
	sockaddr si_other;
	socklen_t slen = sizeof(sockaddr);
	std::memset((char *) &si_other, 0, sizeof(si_other));

	char buf[BUFLEN];
	for (;;){
		if (recvfrom(socket, buf, BUFLEN, 0,  &si_other, &slen) ==-1){
			printf("Broken");
			exit(-3);
		}
		uint8_t type = buf[0]; //the first part
		uint8_t message = buf[1]; //second address

		/* TODO
		place buf 2 to 9 into a union with uint8_t and uint64_t and just use that
		This will give us the easiest way to denetwork it.

		Will be completed when I have the Vesta client up, no point otherwise
		*/

		uint64_t vid = 0x01; //vesta id 1. Will be replaced with a better one.
												 //vesta ids are unique, so if it reconnects under new
												 //details we will overwrite
		//At the moment we will kill anything not a connection
		if (type != CONNECTION){
			exit(-10);
		}

		sockaddr_in* details = (sockaddr_in*) &si_other;

		//TODO check to see if vid is in the user database. For now assume that it is
		//SELECT EXISTS FROM ventus_users WHERE vid = our_vid_we_made;

		string s = "INSERT INTO Connections VALUES(" + tostring(vid) + ", " +
			inet_ntoa(details->sin_addr) + ", " + tostring(ntohs(details->sin_port)) + ")";

		PGresult *res = PQexec(conn, s.c_str());

		if (PQresultStatus(res) != PGRES_COMMAND_OK) {
			printf("unable to write");
		  exit(-4);
		}

		//clear out buffer and si_other
		std::memset((char *) &si_other, 0, sizeof(si_other));
		std::memset((char *) &buf, 0, sizeof(char)*BUFLEN);
	}
}
