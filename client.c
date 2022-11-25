#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_NAME_SZ 1000

int main()
{

    char *ip = "127.0.0.1";
    int port = 25173;

    int sock;
    struct sockaddr_in addr;
    socklen_t addr_size;
    char buffer[1024];
    int n;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror("[-]Socket error");
        exit(1);
    }
    // printf("[+]TCP server socket created.\n");

    memset(&addr, '\0', sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = port;
    addr.sin_addr.s_addr = inet_addr(ip);

    connect(sock, (struct sockaddr *)&addr, sizeof(addr));
    // printf("Connected to the server.\n");
    printf("The client is up and running.\n");

    int passTrials = 3;
    int authenticated = 0;
    char user[100];
    while (passTrials > 0 && !authenticated)
    {
        char username[100];
        char password[100];

        //########################## GET USERNAME ########################//
        printf("Please enter the username: ");
        fgets(username, MAX_NAME_SZ, stdin);
        if ((strlen(username) > 0) && (username[strlen(username) - 1] == '\n'))
            username[strlen(username) - 1] = '\0';
        // printf("username - %s\n", username);
        bzero(buffer, 1024);
        strcpy(buffer, username);
        strcpy(user, username);
        send(sock, buffer, strlen(buffer), 0);
        // printf("username sent to server\n");

        //########################## GET PASSWORD ########################//
        bzero(buffer, 1024);
        recv(sock, buffer, sizeof(buffer), 0);
        printf("%s", buffer);
        fgets(password, MAX_NAME_SZ, stdin);
        if ((strlen(password) > 0) && (password[strlen(password) - 1] == '\n'))
            password[strlen(password) - 1] = '\0';

        bzero(buffer, 1024);
        strcpy(buffer, password);
        // printf("Client: %s\n", buffer);
        send(sock, buffer, strlen(buffer), 0);
        printf("%s sent an authentication request to the main server.\n", username);

        //###################### RECEIVING AUTHENTICATION FROM SERVERM #####################//
        bzero(buffer, 1024);
        recv(sock, buffer, sizeof(buffer), 0);
        // printf("status -%s-\n", buffer);

        char stat[100];
        strcpy(stat, buffer);

        //########################## CHECKING AUTHENTICATION ########################//
        if (!strcmp(stat, "PASS"))
        {
            printf("%s received the result of authentication using TCP over port %d. Authentication is successful.\n", username, port);
            authenticated = 1;
            bzero(buffer, 1024);
            strcpy(buffer, "PASS");
            send(sock, buffer, strlen(buffer), 0);
        }
        else if (!strcmp(stat, "FAIL_NO_PASS"))
        {
            passTrials -= 1;
            printf("%s received the result of authentication using TCP over port %d. Authentication failed: Password does not match\nAttempts remaining: %d\n", username, port, passTrials);
        }
        else
        {
            passTrials -= 1;
            printf("%s received the result of authentication using TCP over port %d. Authentication failed: Username does does not match\nAttempts remaining: %d\n", username, port, passTrials);
        }
    }
    if (authenticated == 0)
    {
        printf("Authentication Failed for 3 attempts. Client will shut down.\n");
        close(sock);
        printf("Disconnected from the server.\n");
    }
    else
    {
        while (1)
        {
            printf("Starting newwwwww");
            ///#################### GETTING COURSE DETAILS ########################//
            char courseCode[1000];
            char category[100];
            char isMultiple[1000];
            printf("Please enter the course code to query: ");

            fgets(courseCode, MAX_NAME_SZ, stdin);
            if ((strlen(courseCode) > 0) && (courseCode[strlen(courseCode) - 1] == '\n'))
                courseCode[strlen(courseCode) - 1] = '\0';

            // printf("coursecode --- %s", courseCode);
            bzero(buffer, 1024);
            strcpy(buffer, "TRUE");
            send(sock, buffer, strlen(buffer), 0);

            bzero(buffer, 1024);
            recv(sock, buffer, sizeof(buffer), 0);
            // printf("%s", buffer);

            bzero(buffer, 1024);
            strcpy(buffer, courseCode);
            send(sock, buffer, strlen(buffer), 0);

            bzero(buffer, 1024);
            recv(sock, buffer, sizeof(buffer), 0);
            if (!strcmp(buffer, "MULTIPLE"))
            {
                printf("Receiving the ...");
                bzero(buffer, 1024);
                recv(sock, buffer, sizeof(buffer), 0);
                if (buffer[0] == '!')
                {
                    buffer[0] = ' ';
                    printf("%s", buffer);
                    goto newQ;
                }

                printf("%s sent a request with multiple CourseCode to the main server.\n", user);
                strcpy(isMultiple, buffer);
                bzero(buffer, 1024);
                strcpy(buffer, "OK");
                send(sock, buffer, strlen(buffer), 0);
            }
            else
            {
                strcpy(isMultiple, buffer);

                bzero(buffer, 1024);
                strcpy(buffer, "OK");
                send(sock, buffer, strlen(buffer), 0);

                bzero(buffer, 1024);
                recv(sock, buffer, sizeof(buffer), 0);
                printf("%s", buffer);
                fgets(category, MAX_NAME_SZ, stdin);
                if ((strlen(category) > 0) && (category[strlen(category) - 1] == '\n'))
                    category[strlen(category) - 1] = '\0';

                bzero(buffer, 1024);
                strcpy(buffer, category);
                send(sock, buffer, strlen(buffer), 0);
                printf("%s sent a request to the main server.\n", user);
            }
            // printf("Please enter the category(Credit/ Professor/ Days/ CourseName): ");

            /// Receiving course info from serverM
            bzero(buffer, 1024);
            recv(sock, buffer, sizeof(buffer), 0);
            // printf("%c----", buffer[0]);
            printf("The client received the response from the Main server using TCP over port %d.\n", port);
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

        newQ:
            printf("\n-----Start a new request-----\n");
        }
    }

    return 0;
}