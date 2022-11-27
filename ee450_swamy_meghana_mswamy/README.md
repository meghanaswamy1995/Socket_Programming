1. Name: Meghana Swamy
2. Student Id : 6565038173
3. Project summary:
   I have built a straightforward course web registration system for USC. Student can use the client to submit their request to the central main server whihch inturn forwards the request to the respective department servers after authenticating the user using a credential server. The department server will keep track of the information about the courses that are provided in each department.
   The user has 3 chances to authenticate themselves, if they fail to do so the socket closes and the user has to run the clinet again to start querying. If the queried courses are not present in the department servers, it will simply return "course not found". Also, the user is able to submit multiple query requests for the courses, the program will output the results/information for all the courses entered. Only a maximum of 10 courses can be queried at a time, if more than 10 is entered the user request will be terminated and prompts for a new request.
   Note: I have implemented the extra credit phase. Please asses me on that.
4. Code files:
   a. client.c:
   The client is the user interface for the web registration system. It prompts for the username and password and forwards it to the main server for authentication. It gets the status of the authentication from the main server and if the authentication is successful, it prompts for the course to query. User has 3 chances to get authenticated, if fails the client socket closes.
   client.c asks the user for the course code and category for querying (for single course ) and just the course code for multiple query, it prints the result of the query information.
   b. serverM.c:
   The serveM is the main server that connects the client with credential server, EE/CS deaprtment servers. This server forwards the authentication request from the client to the credential server after encrypting the username and password. The authentication status form the credential server is then forwarded back to the client.
   The serverM then takes the request for querying the course. The request from the client is forwarded to the respective department servers and the result of the query is sent back to the client.
   c. serverC.c:
   serverC is the credential server which does the authentication of the user. It reads the data given in cred.txt file, stores all the entries from the file into its data array which acts as a database of username and password for registered students.
   It takes the request from the main server and then checks its database if the given username is present or not and sends back the status of the authentication to the main server.
   d. serverEE.c:
   serverEE is the backened department server for EE. It first reads and stores all the information of all the courses of EE department in its data array
   It takes the requested course code from the main server, searches its database and returns the result to the main server. If the course is not found, it just returns course not found.
   e. serverCS.c:
   serverCS is the backened server for CS department. It first reads and stores all the information of all the courses of CS department in its data array
   It takes the requested course code from the main server, searches its database and returns the result to the main server. If the course is not found, it just returns course not found.

5. format of messages:

   1. client.c:
      ./client
      The client is up and running.
      Please enter the username: james
      Please enter the password: 2kAnsa7s)
      james sent an authentication request to the main server.
      james received the result of authentication using TCP over port 60548. Authentication is successful.
      Please enter the course code to query: ee450
      Please enter the category (Credit / Professor / Days / CourseName): Professor
      james sent a request to the main server.
      The client received the response from the Main server using TCP over port 60548.
      The Professor of ee450 is Ali Zahid.

      -----Start a new request-----
      Please enter the course code to query: ee450 cs100
      james sent a request with multiple CourseCode to the main server.
      The client received the response from the Main server using TCP over port 60548.
      CourseCode: Credits, Professor, Days, Course Name
      EE450: 4, Ali Zahid, Tue;Thu, Introduction to Computer Networks
      CS100: 4, Sathyanaraya Raghavachary, Tue;Thu, Explorations in Computing

      -----Start a new request-----
      Please enter the course code to query:

   2. serverM.c:
      ./serverM
      The main server is up and running.
      The main server received the authentication for james using TCP over port 25173.
      The main server sent an authentication request to serverC.
      The main server received the result of the authentication request from ServerC using UDP over port 24173
      The main server sent the authentication result to the client.
      The main server received from james to query course ee450 about Professor.
      The main server sent a request to serverEE.
      The main server received the response from serverEE using UDP over port 24173.
      The main server sent the query information to the client.

      The main server sent a request to serverEE.
      The main server received the response from serverEE using UDP over port 24173.
      The main server sent a request to serverCS.
      The main server received the response from serverCS using UDP over port 24173.
      The main server sent the query information to the client.

   3. serverC.c:
      The ServerC is up and running using UDP on port 21173.
      The ServerC received an authentication request from the Main Server.
      The ServerC finished sending the response to the MainServer.

   4. serverEE.c:
      ./serverEE
      The ServerEE is up and running using UDP on port 23173.
      The ServerEE received a request from the Main Server about the Professor of EE450.
      The course information has been found: The Professor of EE450 is Ali Zahid.
      The ServerEE finished sending the response to the Main Server.
      The ServerEE received a request from the Main Server for the course EE450.
      The course info has been found:
      EE450: 4, Ali Zahid, Tue;Thu, Introduction to Computer Networks
      The ServerEE finished sending the response to the Main Server.

   5. serverCS.c
      ./serverCS
      The ServerCS is up and running using UDP on port 22173.
      The ServerCS received a request from the Main Server for the course CS100.
      The course info has been found:
      CS100: 4, Sathyanaraya Raghavachary, Tue;Thu, Explorations in Computing
      The ServerCS finished sending the response to the Main Server.

6. Idiosyncrasies:
   Since the output format varies slightly for MULTIPLE and single queries. I have used a code to distinguish between the two all over my code. Many of the times in my code you can see messages being sent from serverM to all other servers saying "MULTIPLE" and "NOT-MULTIPLE", these messages are used to tell the respective server whether it a single request or a multiple request.
   aprat from the onscreen messages given in the project document, my code prints:
   a. an error statement if the category is not found
   b. an error statement if the user exceeds more than 10 courses while querying multiple code
   c. an error statemen and closes the socket if the 3 chances to authenticate is expired
   d. the course information in CS/EE department server output when multiple course are queried, for all the courses.
7. Reused code:
   1. Beej's Code: http://www.beej.us/guide/bgnet/ for TCP and UDP implementation
   2. Tutorials on Socket Programming Geeks for Geeks
      https://www.geeksforgeeks.org/socket-programming-cc/
