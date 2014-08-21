+--------------------------------------------------------------------+
+                               README                               +
+--------------------------------------------------------------------+

List of Relevant Files:
-----------------------
	README.txt
	client.cpp
	server.cpp

Compilation Instructions:
-------------------------
	-> open a terminal and go to the "client" directory present in the 
	   root directory. let this terminal be term1.

	-> open a terminal and go to the "server" directory present in the 
	   root directory. let this terminal be term2.

	-> Now execute the following in term1 and term2.

	-> term1 : g++ -o client -std=c++0x client.cpp
	-> term2 : g++ -o server server.cpp

Running Instructions:
---------------------
	-> The server executable is used to intialise a server.
	-> This command should be run from term2.
	-> It should be run before running the client executable.
	-> A file with the name of FileMesh.cfg must be present in the 
	   same folder as the server executable.
	-> It has one command line argument which is the number to be 
	   assigned to the current server.
	-> The command looks like this...
	   term2 : ./server 0

	   Replace the 0 with any number this will be the number of that node.
	-> To generate multiple servers go to another location and copy the 
	   executable there and initialize it using a new node number.

	-> The client executable is used by a user to request a server to 
	   either store a file or retrive a file.
	-> This command should be run in term1.
	-> It should be run after initializing the servers.
	-> It has 3 command line arguments:
		first  : operation to be performed. It can be either str or ret
		second : name of the file on which the operation is to be 
		         performed. In case of store it should be the file name, 
		         in case of retrive it should be the md5 sum of that 
		         particular file.
		third  : IP address and port number of the server it should send 
		         the request to. It is of the form <IP addr>:<port>
	-> The command look like this...
	   term1 : ./client str file 10.0.0.1:5000

	   Replace str with either str or ret and file with file name and 
	   10.0.0.1:5000 with the IP address of the server you wish to send 
	   to and the port used in that server.


Developed by
-------------------------------
Janga Varun
K Vinod Reddy
Sudipto Biswas