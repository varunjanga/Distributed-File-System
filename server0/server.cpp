/* server.cpp
 * 
 * Node in the FileMesh is implemented in this file.
 */

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string>
#include <cstring>
#include <sys/types.h>
#include <time.h>
#include <openssl/md5.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>

using namespace std;

#define UDP_PORT 5000
#define MAX_MSG_LEN 50000000

int md5sumModN(string,int); // Function to compute the node index using md5sum


// The inputs given while executing the program are node no.
// ./server node
int main(int argc, char *argv[]){

  int current_node = atoi(argv[1]); // Serial number of node 
	int udp_socket; // Variable used to store the socket file descritper for udp
	int tcp_socket; // Variable used to store the socket file descritper for tcp
	int udp_port; // Variable used to store the port number used for udp
	struct sockaddr_in my_addr_udp; // Variable used to store the IP address and port of server for udp.
	struct sockaddr_in my_addr_tcp; // Variable used to store the IP address and port of server for tcp.
	struct sockaddr_in client_addr; // Variable used to store the IP address and port of client of incomming message.

	ifstream config; // Variable used to open the configuration file.
	string line;
	config.open("FileMesh.cfg"); // Opening the configuration file.
	for(int i=0; i<=current_node; i++){ // Getting the nth line where n is the node number specified while initiazing the server.
		getline(config, line); // Getting the ith line and storing it in variable "line".
	}
	config.close(); // Closing the configuration file
	config.open("FileMesh.cfg"); // Opening the configuration file.
	int total_nodes; // Variable used to store the number of nodes in the mesh
	total_nodes = count(std::istreambuf_iterator<char>(config), std::istreambuf_iterator<char>(), '\n'); // Counting the number of lines in the configuration file.
	config.close(); // Closing the configuration file
 

  // Parsing the nth line obtained from the configuration file to obtain IP address and port
	int a = line.find(':');
	int b = line.find(' ');
	string my_ip;
	my_ip = line.substr(0,a); // IP address used by the server.
	udp_port = atoi((line.substr(a+1,b-a-1)).c_str()); // Port number used by the server.


	udp_socket = socket(PF_INET, SOCK_DGRAM, 0); // Getting a socket file descriptor for server's udp
	my_addr_udp.sin_family = AF_INET; // Setting it to Internet Socket
	my_addr_udp.sin_port = htons(udp_port); // Setting the port to be udp_port. udp_port has been obtained from the configuration file.
	inet_aton(my_ip.c_str(),&(my_addr_udp.sin_addr));// Setting the IP address to be my_ip. my_ip has been btained from the configuration file.
	memset(&(my_addr_udp.sin_zero), '\0', 8); // Filling the remaining space with zeros.

	cout<< "NodeId       : " << current_node << endl;
	cout<< "IP           : " << my_ip << endl;
	cout<< "Port         : " << udp_port << endl;
	cout<< "No. of Nodes : " << total_nodes << endl;

	bind(udp_socket, (struct sockaddr *)&my_addr_udp, sizeof(struct sockaddr)); // Binding the socket file descripter to the IP address and port.

  int maxsize = 1000; // Maxsize of the message to be received from through udp.
	char* msg; // Variable used to store the message obtained through udp
  msg = new char[maxsize]; // Allocating space for the message to be received.
  unsigned int sin_len = sizeof(struct sockaddr); // The size of the address of the client from whom the message is to be received.
  
  while(true){ // The whole process is set in a while loop so that the server continously processes the requests.
    int received = recvfrom(udp_socket, msg, maxsize, 0, (struct sockaddr *)&client_addr, &sin_len); // Receiving message from the client or other servers.
    
    string message; // Variable used to store the message obtained.
    message = string(msg); // converting the char* to string
    
    // Parsing the message to obtain operation required to be performed, md5 sum of the file, IP address and the TCP port of client.
    int pos  = message.find(',');
    int pos1 = message.find(',', pos+1);
    int pos2 = message.find(':');
    string operation = message.substr(0,pos);
    string md5sum;
    md5sum = message.substr(pos+1, pos1-pos-1); // md5 sum of the file.
    string client_ip = message.substr(pos1+1, pos2-pos1-1); // IP address of the client
    string client_port = message.substr(pos2+1); // Port number of TCP of client

    cout << "UDP Message received from "+client_ip+":"+client_port+".\n" ;
    //cout<<message<<endl;
    //cout<<operation<<endl;
    //cout<<md5sum<<endl;
    //cout<<client_ip<<endl;
    //cout<<client_port<<endl;

    int proper_node = md5sumModN(md5sum,total_nodes); // Obtaining the node number of the server that is required to perform the operation
    cout<<"current_node:" << current_node << " ";
    cout<<"proper_node:"  << proper_node  << endl;

    if(proper_node == current_node){ // If the current server is required to perform the operation then...
      cout<<"Reached Proper Node...\nEstablishing TCP Connection... ";
      tcp_socket = socket(PF_INET, SOCK_STREAM, 0); // Getting a socket file descripter for tcp

      client_addr.sin_family = AF_INET; // Setting it to Internet Socket
      client_addr.sin_port = htons(atoi(client_port.c_str()));// Setting the port to be client_port. client_port has been obtained from message through udp.
      inet_aton(client_ip.c_str(),&(client_addr.sin_addr));// Setting the IP address to be client_ip. client_ip has been obtained from message through udp.
      memset(&(client_addr.sin_zero), '\0', 8); // Filling the remaining space with zeros.

      int err = connect(tcp_socket, (struct sockaddr *)&client_addr, sizeof(struct sockaddr)); // Establishing a TCP connection with the client
      if (err >= 0) cout << " done.\n";
      else cout << " failed.\n";

      if(operation == "str"){ // If the operation required to perform is store then...
        int bytes_recv; // Variable used to store the number of bytes received from the connection
        string file="";
        char* rec_msg; // Variable used to store the incomming connection
        rec_msg = new char[MAX_MSG_LEN]; // Allocating the space for the receiving message.
        do {
          bytes_recv = recv(tcp_socket, rec_msg, MAX_MSG_LEN, 0); // Receiving the message from the tcp connection established with the client.
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
      else{ // If the operation required to be performed is retrive then...
        ifstream filestream;
        string file="";
        filestream.open(md5sum.c_str());
        while (filestream.good()) {
          char c = filestream.get();
          if (filestream.good())
            file += c;
        }
        filestream.close();
        int len = file.length();
        //cout<<file<<" sent"<<endl;
        int bytes_sent = send(tcp_socket, file.c_str(), len+1, 0);
        if (bytes_sent!=-1) cout << "No of bytes sent in TCP Packet = "<< bytes_sent<<  endl;
        else cout << "Sending TCP Packet failed."<< endl;
      }
      close(tcp_socket); // closing the tcp connection
    }
    else{ // If the current server is not the server required to perform the operation then...
      cout << "Forwarding the UDP packet from node " << current_node
           << " to node " << proper_node << "... ";
      
      config.open("FileMesh.cfg"); // Opening the configuration file.
      for(int i = 0; i <= proper_node; i++){ // Getting the nth line from configuration file where n is node no. of the server that is required to perform the operation.
        getline(config, line); // Getting the ith line and storing it in the variable "line"
      }
      config.close();// Closing the configuration file

      // Parsing the nth line obtained from the configuration file to obtain IP address and port
      a = line.find(':');
      b = line.find(' ');
      string server_ip;
      server_ip = line.substr(0,a); // IP address of the server which has to perform the operation.
      int server_udp_port;
      server_udp_port = atoi((line.substr(a+1,b-a-1)).c_str()); // Port number of th server which has to perform the operation.

      struct sockaddr_in server_addr; // Variable used to store the IP address and port of the server which has to perform the operation.
      server_addr.sin_family = AF_INET; // Setting it to Internet Socket
      server_addr.sin_port = htons(server_udp_port);// Setting the port to be server_udp_port. server_udp_port has been obtained from the configuration file.
      inet_aton(server_ip.c_str(),&(server_addr.sin_addr));// Setting the IP address to be server_ip. server_ip has been obtained from the configuration file.
      memset(&(server_addr.sin_zero), '\0', 8); // Filling the remaining space with zeros.


      // Sending the message received from the client to the server which has to perform the operation.
      int sent = sendto(udp_socket, msg, message.size()+1, 0, (struct sockaddr *)&server_addr, sizeof(struct sockaddr));
      if(sent != -1) cout << "done.\n";
      else cout << "failed.\n"; 
    }
    cout << "Request processed. Waiting for next request...\n";
  }
	return 0;
}

// Function to compute the node index using md5sum
int md5sumModN(string sum,int n){
  bool val[128];
  for (int i = 0; i < sum.length(); i++)
  {
    char cur = sum[i];
    if (cur<='9'&&cur>='0') cur-='0';
    else if (cur>='a'&&cur<='f')cur+=10-'a';
    else cur+=10-'A';
    val[i*4+3] = cur & 1; cur >>=1;
    val[i*4+2] = cur & 1; cur >>=1;
    val[i*4+1] = cur & 1; cur >>=1;
    val[i*4] = cur & 1; 
  }
  int expmod=1,ans=0;
  
  for (int i = 0; i < 128; i++)
  {
    if(val[127-i]){
      ans += expmod;
      ans %= n;
    }
    expmod <<= 1;
    expmod %= n;
  }
  return ans;
}
