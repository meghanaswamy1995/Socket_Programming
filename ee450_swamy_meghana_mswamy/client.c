#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_NAME_SZ 1000

int main()
{

    char *ip = "127.0.0.1";
    int portTCP = 25173;

    int sock;
    struct sockaddr_in addr;
    socklen_t addr_size;
    char buffer[1024];
    int n;

    /// create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("[-]Socket error");
        exit(1);
    }
    // printf("TCP server socket created.\n");

    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = portTCP; // serverM TCP port number
    // addr.sin_addr.s_addr = inet_addr(ip);
    addr.sin_addr.s_addr = inet_addr(ip);

    connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    printf("The client is up and running.\n");

    // fetch the clients dynamic port number
    unsigned int clientPort;
    struct sockaddr_in clinetAddress;
    bzero(&clinetAddress, sizeof(clinetAddress));
    socklen_t len = sizeof(clinetAddress);
    getsockname(sock, (struct sockaddr *)&clinetAddress, &len);
    clientPort = ntohs(clinetAddress.sin_port);
    // printf("Client's dynamic port number : %u\n", clientPort);

    /// ############################## START USER AUTHENTICATIOn ###########################//

    int passTrials = 3;
    int authenticated = 0;
    char user[100];
    while (passTrials > 0 && !authenticated)
    {
        char username[100];
        char password[100];

        //############# GET USERNAME ##############//
        printf("Please enter the username: ");
        fgets(username, MAX_NAME_SZ, stdin);
        if ((strlen(username) > 0) && (username[strlen(username) - 1] == '\n'))
            username[strlen(username) - 1] = '\0';

        /// send username to the main server
        bzero(buffer, 1024);
        strcpy(buffer, username);
        strcpy(user, username);
        send(sock, buffer, strlen(buffer), 0);

        //############## GET PASSWORD ###############//
        bzero(buffer, 1024);
        recv(sock, buffer, sizeof(buffer), 0);
        printf("%s", buffer);
        fgets(password, MAX_NAME_SZ, stdin);
        if ((strlen(password) > 0) && (password[strlen(password) - 1] == '\n'))
            password[strlen(password) - 1] = '\0';

        /// send password ot the main server
        bzero(buffer, 1024);
        strcpy(buffer, password);
        send(sock, buffer, strlen(buffer), 0);
        printf("%s sent an authentication request to the main server.\n", username);

        //####### RECEIVING AUTHENTICATION FROM SERVERM #######//
        bzero(buffer, 1024);
        recv(sock, buffer, sizeof(buffer), 0);
        // printf("status -%s-\n", buffer);

        char stat[100];
        strcpy(stat, buffer);

        //############# CHECKING AUTHENTICATION Status #############//
        if (!strcmp(stat, "PASS"))
        {
            printf("%s received the result of authentication using TCP over port %d. Authentication is successful.\n", username, clientPort);
            authenticated = 1;
            bzero(buffer, 1024);
            strcpy(buffer, "PASS");
            send(sock, buffer, strlen(buffer), 0);
        }
        else if (!strcmp(stat, "FAIL_NO_PASS"))
        {
            passTrials -= 1;
            printf("%s received the result of authentication using TCP over port %d. Authentication failed: Password does not match\nAttempts remaining: %d\n", username, clientPort, passTrials);
        }
        else
        {
            passTrials -= 1;
            printf("%s received the result of authentication using TCP over port %d. Authentication failed: Username does does not match\nAttempts remaining: %d\n", username, clientPort, passTrials);
        }
    }
    /// if user exhausts the 3 trials for authenticating, close socket
    if (authenticated == 0)
    {
        printf("Authentication Failed for 3 attempts. Client will shut down.\n");
        close(sock);
        printf("Disconnected from the server.\n");
    }
    //// If user properly Authenticated
    /// ######################################## START QUERYING COURSES ######################################///
    else
    {
        while (1)
        {

            ///#################### GETTING COURSE DETAILS ########################//
            char courseCode[1000];
            char category[100];
            char isMultiple[1000];
            printf("Please enter the course code to query: ");
            fgets(courseCode, MAX_NAME_SZ, stdin);
            if ((strlen(courseCode) > 0) && (courseCode[strlen(courseCode) - 1] == '\n'))
                courseCode[strlen(courseCode) - 1] = '\0';

            int field = 0;
            char allCode[100][100];
            char crs[2000];
            strcpy(crs, courseCode);
            char *token = strtok(crs, " ");
            int presentAlrdy = 0;
            while (token)
            {
                /// check to see if there are duplicate entires
                presentAlrdy = 0;

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

            ///// check to see if multiple queries and if the entered courses are less than 10, if more than 10 terminate this request and start a new one
            if (field > 10)
            {
                strcpy(isMultiple, "EXCEEDED");
                printf("Cannot query more than 10 courses at a time. Please enter less than 10 courses to query.\n");
                printf("\n-----Start a new request-----\n");
                continue;
            }
            if (field < 2)
            {
                strcpy(isMultiple, "NOT-MULTIPLE");
                strcpy(courseCode, allCode[0]);
            }
            if (field >= 2 && field <= 10)
            {
                strcpy(isMultiple, "MULTIPLE");
                printf("%s sent a request with multiple CourseCode to the main server.\n", user);
            }

            //// send a message to server saying that the client is ready to send course
            bzero(buffer, 1024);
            strcpy(buffer, "TRUE");
            send(sock, buffer, strlen(buffer), 0);
            bzero(buffer, 1024);
            recv(sock, buffer, sizeof(buffer), 0);

            //// send  sourse code to serverM for query
            bzero(buffer, 1024);
            strcpy(buffer, courseCode);
            send(sock, buffer, strlen(buffer), 0);

            if (!strcmp(isMultiple, "NOT-MULTIPLE")) /// for single entry
            {
                /// get category
                bzero(buffer, 1024);
                recv(sock, buffer, sizeof(buffer), 0);
                printf("%s", buffer);
                fgets(category, MAX_NAME_SZ, stdin);
                if ((strlen(category) > 0) && (category[strlen(category) - 1] == '\n'))
                    category[strlen(category) - 1] = '\0';

                /// SEND CATEGORY TO MAIN SERVER
                bzero(buffer, 1024);
                strcpy(buffer, category);
                send(sock, buffer, strlen(buffer), 0);
                printf("%s sent a request to the main server.\n", user);
            }

            /// Receiving QUERY RESULTS from serverM
            bzero(buffer, 1024);
            recv(sock, buffer, sizeof(buffer), 0);

            printf("The client received the response from the Main server using TCP over port %d.\n", clientPort);
            if (!strcmp(isMultiple, "MULTIPLE"))
            {
                printf("CourseCode: Credits, Professor, Days, Course Name\n%s\n", buffer);
            }
            else
            {
                if (buffer[0] == '!')
                {
                    buffer[0] = ' ';
                    printf("%s.\n", buffer);
                }
                else
                {
                    printf("The %s of %s is %s.\n", category, courseCode, buffer);
                }
            }

            printf("\n-----Start a new request-----\n");
        }
    }

    return 0;
}