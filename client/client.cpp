/* client.cpp
 * 
 * User sending packets is implemented in this file.
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <fstream>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <time.h>
#include <iostream>

using namespace std;

#define UDP_PORT 6000 // The port used by client for udp
#define TCP_PORT 6001 // The port used by client to listen for tcp connections
#define MAX_MSG_LEN 50000000
#define MY_IP "10.8.12.164" // The IP used by the client

//Function which runs a command and returns output as a string.
string exec(const char* cmd){
	FILE* pipe = popen(cmd,"r");
	if(!pipe) return "ERROR";
	char buffer[128];
	string result = "";
	while(!feof(pipe)){
		if(fgets(buffer, 128, pipe) != NULL)
			result += buffer;
	}
	pclose(pipe);
	return result;
}

// The inputs given while executing the program are req/str, filename, node no..
// ./client str filename server_ip:server_port
// ./client req md5sum   server_ip:server_port
int main(int argc, char *argv[]){
  if (argc != 4) { // Checking input format
    printf("Input format:\n  ./client str filename server_ip:server_port\n  OR\n  ./client req md5sum   server_ip:server_port\n");
    return -1;
  }

  string operation = string(argv[1]); // The operation to be performed store or retrive.
  string filename = string(argv[2]); // The filename on which the operation is to be performed.
  string server_ip_port = string(argv[3]); // The 3rd argument which is server_ip:server_port

	int colon_pos = server_ip_port.find(':');
  string server_ip = server_ip_port.substr(0,colon_pos); // The IP of the server to which the the request is to be sent.
  int server_port = stoi(server_ip_port.substr(colon_pos+1,server_ip_port.size())); // The port in the server to which the request is to be sent.

	int udp_socket; // Socket descripter for udp to be used by the client
	udp_socket = socket(PF_INET, SOCK_DGRAM, 0); // Getting a socket descripter for udp.

	struct sockaddr_in my_addr_udp; // This is used for storing the IP address and port of the client and is used for udp.
	my_addr_udp.sin_family = AF_INET; // Setting it to Internet Socket
	my_addr_udp.sin_port = htons(UDP_PORT); // Setting the port to be UDP_PORT. UDP_PORT has been defined at the top.
	inet_aton(MY_IP,&(my_addr_udp.sin_addr)); // Setting the IP address to be MY_IP. MY_IP has been defined at the top.
	memset(&(my_addr_udp.sin_zero), '\0', 8); // Filling the remaining space with zeros.

  bind(udp_socket, (struct sockaddr *)&my_addr_udp, sizeof(struct sockaddr)); // Binding the socket descripter to IP address and port.

	struct sockaddr_in server_addr; // This is used for storing the IP address and port of the server to which the request is to be sent.
	server_addr.sin_family = AF_INET; // Setting it to Internet Socket
	server_addr.sin_port = htons(server_port); // Setting the port to be server_port. server_port has been obtained from user input.
	inet_aton(server_ip.c_str(),&(server_addr.sin_addr)); // Setting the IP address to be server_addr. server_addr has been obtained from user input.
	memset(&(server_addr.sin_zero), '\0', 8); // Filling the remaining space with zeros.

	string md5sum; // Variable used to store the md5 sum of the file specified by user.
	if(operation == "str"){
		md5sum = exec(("md5sum " + filename + " | awk '{print $1}'").c_str()); // Using the function exec() to get the md5 sum of the file.
    md5sum = md5sum.substr(0,md5sum.size()-1); // This removes the '\n' character at the end of the obtained result.
	}
	else{
    md5sum = filename; // This is the case of retriving the file in which case the user specifies the md5 sum of the file to be retrived.
	}

  string message; // Variable used to store the message sent to the server.
	message = operation + "," + md5sum + "," + MY_IP 
            + ":" + to_string(TCP_PORT); // Genrating the message.
            // The message is of the format "operation,md5sum,client_ip:client_port".
	
    // Sending the message to server whose IP and port are given by the user.
	int sent = sendto(udp_socket, message.c_str(), message.size()+1, 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));

	cout<<message<<" sent\n" ;
  if (sent!=-1) cout << "No of bytes sent in UDP Packet = "<< sent<<  endl;
  else cout << "Sending UDP Packet failed."<< endl;

	int tcp_socket; // Variable used to store the socket descripter for tcp to listen for tcp connections.
  tcp_socket = socket(PF_INET, SOCK_STREAM, 0); // Getting a socket descripter for tcp

	struct sockaddr_in my_addr_tcp; // This is used for storing the IP address and port of the client and is used for tcp.
  my_addr_tcp.sin_family = AF_INET; // Setting it to Internet Socket
	my_addr_tcp.sin_port = htons(TCP_PORT); // Setting the port to be TCP_PORT. TCP_PORT has been defined at the top.
	inet_aton(MY_IP,&(my_addr_tcp.sin_addr)); // Setting the IP address to be MY_IP. MY_IP has been defined at the top.
	memset(&(my_addr_tcp.sin_zero), '\0', 8); // Filling the remaining space with zeros.

	bind(tcp_socket, (struct sockaddr *)&my_addr_tcp, sizeof(struct sockaddr)); // Binding the tcp socket descripter to IP address and port.

	listen(tcp_socket, 5); // Listening for tcp connections with a backlog of 5.
	unsigned int sin_size = sizeof(struct sockaddr_in); // The size of the struct sockaddr_in. Variable used to store the size of the address for the incomming connection.
	struct sockaddr_in server_addr_tcp; // Variable used to store the IP address and port of incomming tcp connection.
	int tcp_socket_conn; // Variable used to store the socket descripter for tcp when a connection is established with the server.
	tcp_socket_conn = accept(tcp_socket, (struct sockaddr *)&server_addr_tcp, &sin_size); // Accept the incomming tcp connection.

	if(operation == "str"){// If the operation requested by the user is store then...
    ifstream filestream; // Varaible used to open the file specifed by the user.
    string file=""; // String used to store the data before sending.Initialising the string to zero.
    filestream.open(filename.c_str()); // Opening the file
    while (filestream.good()) {
      char c = filestream.get(); // Get char by char from the file.
      if (filestream.good())
        file += c; // Add the charecters to the string.
    }
    filestream.close(); // Closing the file.
		int len = file.length(); // Length of the file string is stored in this variable.
		int bytes_sent = send(tcp_socket_conn, file.c_str(), len, 0); // Sending the message to the server to store it.
    if (bytes_sent!=-1) cout << "No of bytes sent in TCP Packet = "<< bytes_sent<<  endl;
    else cout << "Sending TCP Packet failed."<< endl;
	}
	else{ // If the operation requested by the user is retrive then...
    int bytes_recv;
    string file="";
    char* rec_msg;
    rec_msg = new char[MAX_MSG_LEN];
    do {
      bytes_recv = recv(tcp_socket_conn, rec_msg, MAX_MSG_LEN, 0);
      cout<<"No of bytes received in TCP packet: " << bytes_recv <<endl;
      cout<<"Message Content:\n";
      cout<<"---------------------------------------------------------\n";
      for (int i = 0; i < bytes_recv; i++)
      {
        file += rec_msg[i]; 
      }
      cout<<"\n---------------------------------------------------------\n";
    } while(bytes_recv != -1 && bytes_recv != 0) ;
    ofstream myfile;
    myfile.open (md5sum.c_str());
    myfile << file;
    myfile.close();
	}
	return 0;
}
