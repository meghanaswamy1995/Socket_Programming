#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

int main()
{

    char *ip = "127.0.0.1";
    int portTCP = 25173;
    int portUdp = 24173;
    int eePort = 23173;
    int csPort = 22173;
    int credPort = 21173;

    int server_sock, client_sock, serverC_sock, serverEE_sock, serverCS_sock;
    struct sockaddr_in server_addr, client_addr, serverC_addr, serverEE_addr, serverCS_addr;
    socklen_t addr_size;
    char buffer[1024];
    int n;

    //######################## CREATE and INITIALIZE UDP SOCKETS ############################//
    /// SERVERC SOCK//////
    serverC_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverC_sock < 0)
    {
        perror("cred Socket error");
        exit(1);
    }
    memset(&serverC_addr, '\0', sizeof(serverC_addr));
    serverC_addr.sin_family = AF_INET;
    serverC_addr.sin_port = credPort;
    serverC_addr.sin_addr.s_addr = inet_addr(ip);

    ////SERVER EE SOCKET/////
    serverEE_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverEE_sock < 0)
    {
        perror("[-]EE Socket error");
        exit(1);
    }
    memset(&serverEE_addr, '\0', sizeof(serverEE_addr));
    serverEE_addr.sin_family = AF_INET;
    serverEE_addr.sin_port = eePort;
    serverEE_addr.sin_addr.s_addr = inet_addr(ip);

    ////SERVER CS SOCKET/////
    serverCS_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (serverCS_sock < 0)
    {
        perror("[-]CS Socket error");
        exit(1);
    }
    memset(&serverCS_addr, '\0', sizeof(serverCS_addr));
    serverCS_addr.sin_family = AF_INET;
    serverCS_addr.sin_port = csPort;
    serverCS_addr.sin_addr.s_addr = inet_addr(ip);

    //######################## TCP SOCKET ############################//
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock < 0)
    {
        perror("[-]main server Socket error");
        exit(1);
    }

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = portTCP;
    server_addr.sin_addr.s_addr = inet_addr(ip);

    n = bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (n < 0)
    {
        perror("[-]Bind error");
        exit(1);
    }

    printf("The main server is up and running.\n");
    listen(server_sock, 5);

    //////############################### LISTENING.. and  READY TO RECEIVE DATA ###############################
    char username[100];

    while (1)
    {
        addr_size = sizeof(client_addr);

        client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &addr_size);
        int count = 3;
        int authenticated = 0;

        while (count > 0 && !authenticated)
        {

            /// Receive username from client
            bzero(buffer, 1024);
            recv(client_sock, buffer, sizeof(buffer), 0);
            if (strlen(buffer) <= 0)
            {
                break;
            }
            if (!strcmp(buffer, "PASS"))
            {
                bzero(buffer, 1024);
                strcpy(buffer, "PASS");
                sendto(serverC_sock, buffer, strlen(buffer), 0, (struct sockaddr *)&serverC_addr, sizeof(serverC_addr));
                break;
            }

            char user[strlen(buffer)];
            strcpy(user, buffer);
            strcpy(username, buffer);

            ////// ENCRYPTION STRATEGY

            printf("The main server received the authentication for %s using TCP over port %d.\n", user, portTCP);
            for (int i = 0; i < sizeof(user); i++)
            {
                char c = user[i];
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
                {

                    int val = c;
                    val += 4;
                    if (c >= 'a' && c <= 'z' && val > 122)
                    {
                        val = val - 122 + 96;
                    }
                    if (c >= 'A' && c <= 'Z' && val > 90)
                    {
                        val = val - 90 + 64;
                    }
                    if (c >= 47 && c <= 57 && val > 57)
                    {
                        val = val - 57 + 47;
                    }
                    user[i] = val;
                }
            }

            /// Asking for password from client

            bzero(buffer, 1024);
            strcpy(buffer, "Please enter the password: ");
            send(client_sock, buffer, strlen(buffer), 0);

            //// Receive password
            bzero(buffer, 1024);
            recv(client_sock, buffer, sizeof(buffer), 0);
            char passwd[strlen(buffer)];
            strcpy(passwd, buffer);

            /// ENCRYPT PASSWORD
            for (int i = 0; i < sizeof(passwd); i++)
            {
                char c = passwd[i];
                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))
                {

                    int val = c;
                    val += 4;
                    if (c >= 'a' && c <= 'z' && val > 122)
                    {
                        val = val - 122 + 96;
                    }
                    if (c >= 'A' && c <= 'Z' && val > 90)
                    {
                        val = val - 90 + 64;
                    }
                    if (c >= 47 && c <= 57 && val > 57)
                    {
                        val = val - 57 + 47;
                    }
                    passwd[i] = val;
                }
            }

            ///######## send to Cred server #########///
            bzero(buffer, 1024);
            strcpy(buffer, user);
            sendto(serverC_sock, buffer, strlen(buffer), 0, (struct sockaddr *)&serverC_addr, sizeof(serverC_addr));

            bzero(buffer, 1024);
            strcpy(buffer, passwd);
            sendto(serverC_sock, buffer, strlen(buffer), 0, (struct sockaddr *)&serverC_addr, sizeof(serverC_addr));
            printf("The main server sent an authentication request to serverC.\n");

            /// ######## check authorization ######### ///
            socklen_t add_size = sizeof(serverC_addr);
            bzero(buffer, 1024);
            recvfrom(serverC_sock, buffer, 1024, 0, (struct sockaddr *)&serverC_addr, &add_size);
            printf("The main server received the result of the authentication request from ServerC using UDP over port %d\n", portUdp);
            // printf("STATUS!!%s!!\n", buffer);

            // sending state back to client
            send(client_sock, buffer, strlen(buffer), 0);
            count -= 1;
            printf("The main server sent the authentication result to the client.\n");
        }

        /// ##################################### GETTING COURSE DETAILS ################################ ///
        char repeatCheck[100];
        strcpy(repeatCheck, "TRUE");

        while (1)
        {
            //// Receive new Request

            char returnResult[1000];
            strcpy(returnResult, "");
            bzero(buffer, 1024);
            recv(client_sock, buffer, sizeof(buffer), 0);

            /// check to see if the client is ready to send the details
            if (strlen(buffer) <= 0)
            {
                break;
            }
            if (!strcmp(buffer, "FALSE"))
            {
                break;
            }
            if (!strcmp(buffer, " "))
            {
                break;
            }
            // authenticate
            bzero(buffer, 1024);
            strcpy(buffer, "ok");
            send(client_sock, buffer, strlen(buffer), 0);

            //******* GETCOURSE CODE ********//
            bzero(buffer, 1024);
            recv(client_sock, buffer, sizeof(buffer), 0);

            char course[strlen(buffer)];
            char crs[strlen(buffer)];
            strcpy(course, buffer);
            strcpy(crs, course);

            int field = 0;
            char allCode[100][100];
            char *token = strtok(crs, " ");
            int num = 0;
            int presentAlrdy = 0;
            while (token)
            {
                /////// check for duplicate entries and if it is a single entry or multiple entries
                presentAlrdy = 0;
                num = sizeof(allCode) / sizeof(allCode[0]);
                for (int i = 0; i < field; i++)
                {
                    if (!strcmp(token, allCode[i]))
                    {
                        presentAlrdy = 1;
                    }
                }
                if (presentAlrdy == 0)
                {
                    strcpy(allCode[field++], token);
                }
                token = strtok(NULL, " ");
            }

            // ################################  IF SINGLE ENTRY ################################
            if (field < 2)
            {
                char ctgry[100];

                // ********* GET CATEGORY ***********

                // ******* Ask Category
                bzero(buffer, 1024);
                strcpy(buffer, "Please enter the category (Credit / Professor / Days / CourseName): ");
                send(client_sock, buffer, strlen(buffer), 0);

                //  ****** Authenticate
                bzero(buffer, 1024);
                recv(client_sock, buffer, sizeof(buffer), 0);

                strcpy(ctgry, buffer);
                printf("The main server received from %s to query course %s about %s.\n", username, course, ctgry);

                ////********** get deptartment
                char dept[3];
                size_t j;
                for (j = 0; j < 2; j++)
                {
                    course[j] = toupper(course[j]);
                    dept[j] = course[j];
                }
                dept[2] = '\0';

                if (!strcmp(dept, "EE"))
                {
                    /// *************** SEND TO EE SERVER ****************

                    /// ******** send Not Multiple
                    bzero(buffer, 1024);
                    strcpy(buffer, "NOT-MULTIPLE");
                    sendto(serverEE_sock, buffer, strlen(buffer), 0, (struct sockaddr *)&serverEE_addr, sizeof(serverEE_addr));

                    ///*************** send course
                    bzero(buffer, 1024);
                    // strcpy(buffer, userPass);
                    strcpy(buffer, course);
                    sendto(serverEE_sock, buffer, strlen(buffer), 0, (struct sockaddr *)&serverEE_addr, sizeof(serverEE_addr));
                    // sendto(sockfd, buffer, 1024, 0, (struct sockaddr*)&addr, sizeof(addr));

                    ///*************** send category
                    bzero(buffer, 1024);
                    strcpy(buffer, ctgry);
                    sendto(serverEE_sock, buffer, strlen(buffer), 0, (struct sockaddr *)&serverEE_addr, sizeof(serverEE_addr));
                    printf("The main server sent a request to serverEE.\n");

                    /// Receive data from EE server
                    socklen_t add_size = sizeof(serverEE_addr);
                    bzero(buffer, 1024);
                    recvfrom(serverEE_sock, buffer, 1024, 0, (struct sockaddr *)&serverEE_addr, &add_size);
                    // printf("%s\n", buffer);

                    strcpy(returnResult, buffer);

                    printf("The main server received the response from serverEE using UDP over port %d.\n", portUdp);
                }
                else if (!strcmp(dept, "CS"))
                {
                    /// *************** SEND TO CS SERVER ****************

                    /// ******** send Not Multiple
                    bzero(buffer, 1024);
                    strcpy(buffer, "NOT-MULTIPLE");
                    sendto(serverCS_sock, buffer, strlen(buffer), 0, (struct sockaddr *)&serverCS_addr, sizeof(serverCS_addr));

                    // printf("sending to CS server\n");

                    ///*************** send course to ee server
                    bzero(buffer, 1024);
                    strcpy(buffer, course);
                    sendto(serverCS_sock, buffer, strlen(buffer), 0, (struct sockaddr *)&serverCS_addr, sizeof(serverCS_addr));

                    ///*************** send category
                    bzero(buffer, 1024);
                    strcpy(buffer, ctgry);
                    sendto(serverCS_sock, buffer, strlen(buffer), 0, (struct sockaddr *)&serverCS_addr, sizeof(serverCS_addr));
                    printf("The main server sent a request to serverCS.\n");

                    /// Receive data from CS server
                    socklen_t add_size = sizeof(serverCS_addr);
                    bzero(buffer, 1024);
                    recvfrom(serverCS_sock, buffer, 1024, 0, (struct sockaddr *)&serverCS_addr, &add_size);
                    printf("The main server received the response from serverCS using UDP over port %d.\n", portUdp);
                    strcpy(returnResult, buffer);
                }
                else
                {
                    printf("Invalid dept\n");
                    strcpy(returnResult, "!Didn't find the course: ");
                    strcat(returnResult, course);
                }
            }
            // ################################  MULTIPLE ENTRY ################################
            else
            {
                strcpy(returnResult, "");
                for (int i = 0; i < field; i++) //// for the multiple code entries
                {
                    // printf("%s ----", allCode[i]);

                    char dept[3];
                    size_t j;
                    for (j = 0; j < 2; j++)
                    {
                        allCode[i][j] = toupper(allCode[i][j]);
                        dept[j] = allCode[i][j];
                    }
                    dept[2] = '\0';

                    // printf("dept is %s\n", dept);

                    if (!strcmp(dept, "EE"))
                    {
                        ///################### send to EE server ###################///
                        /// *************** send multiple
                        bzero(buffer, 1024);
                        strcpy(buffer, "MULTIPLE");
                        sendto(serverEE_sock, buffer, strlen(buffer), 0, (struct sockaddr *)&serverEE_addr, sizeof(serverEE_addr));

                        ///***** send course
                        bzero(buffer, 1024);
                        strcpy(buffer, allCode[i]);
                        sendto(serverEE_sock, buffer, strlen(buffer), 0, (struct sockaddr *)&serverEE_addr, sizeof(serverEE_addr));
                        printf("The main server sent a request to serverEE.\n");

                        /// Receive data from EE server
                        socklen_t add_size = sizeof(serverEE_addr);
                        bzero(buffer, 1024);
                        recvfrom(serverEE_sock, buffer, 1024, 0, (struct sockaddr *)&serverEE_addr, &add_size);
                        // printf("%s\n", buffer);

                        strcat(returnResult, buffer);
                        strcat(returnResult, "\n");
                        printf("The main server received the response from serverEE using UDP over port %d.\n", portUdp);
                    }
                    else if (!strcmp(dept, "CS"))
                    {
                        ///################ send to CS server ################///
                        /// *************** send multiple
                        bzero(buffer, 1024);
                        strcpy(buffer, "MULTIPLE");
                        sendto(serverCS_sock, buffer, strlen(buffer), 0, (struct sockaddr *)&serverCS_addr, sizeof(serverCS_addr));

                        /// *************** send course code to CS servre
                        bzero(buffer, 1024);
                        strcpy(buffer, allCode[i]);
                        sendto(serverCS_sock, buffer, strlen(buffer), 0, (struct sockaddr *)&serverCS_addr, sizeof(serverCS_addr));
                        printf("The main server sent a request to serverCS.\n");

                        /// Receive data from CS server
                        socklen_t add_size = sizeof(serverCS_addr);
                        bzero(buffer, 1024);
                        recvfrom(serverCS_sock, buffer, 1024, 0, (struct sockaddr *)&serverCS_addr, &add_size);
                        printf("The main server received the response from serverCS using UDP over port %d.\n", portUdp);
                        strcat(returnResult, buffer);
                        strcat(returnResult, "\n");
                    }
                    else
                    {
                        printf("invalid dept\n");
                        strcat(returnResult, "Didn't find the course: ");
                        strcat(returnResult, allCode[i]);
                    }
                }
            }

            /// send the result of query to the client

            bzero(buffer, 1024);
            strcpy(buffer, returnResult);
            send(client_sock, buffer, strlen(buffer), 0);
            printf("The main server sent the query information to the client.\n");
            printf("\n");

            /// ################### Close client socket ###################### ///
            // close(client_sock);
            // printf("Client disconnected.\n\n");
        }
    }

    return 0;
}