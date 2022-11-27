#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_LINES 1000
#define MAX_LEN 5000

int main(void)
{
    //########################## Reading the cred.txt file and storing all the entries into the datastructure ######################
    FILE *file;
    char data[MAX_LINES][MAX_LEN];
    file = fopen("cred.txt", "r");
    if (file == NULL)
    {
        printf("Error opening file. \n");
        return 1;
    }
    int line = 0;
    char *entry;
    while (!feof(file) && !ferror(file)) // Read all the lines and store it in the array data[]
    {
        if (fgets(data[line], MAX_LEN, file) != NULL)
            line++;
    }
    fclose(file);

    for (int i = 0; i < line; i++)
    {
        if ((strlen(data[i]) > 0) && (data[i][strlen(data[i]) - 1] == '\n'))
        {
            data[i][strlen(data[i]) - 2] = '\0';
        }
    }

    /////#################################### Creating socket  #######################################
    char *ip = "127.0.0.1";
    int credPort = 21173;

    int cSock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    char buffer[1024];
    int n;

    cSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (cSock < 0)
    {
        perror("[-]Socket error");
        exit(1);
    }
    // printf("UDP server socket created.\n");

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = credPort;
    server_addr.sin_addr.s_addr = inet_addr(ip);

    n = bind(cSock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (n < 0)
    {
        perror("[-]Bind error");
        exit(1);
    }

    printf("The ServerC is up and running using UDP on port %d.\n", credPort);

    //////############################### READY TO RECEIVE DATA ###############################

    while (1)
    {
        int count1 = 3;
        int authenticated1 = 0;
        while (count1 > 0 && !authenticated1)
        {

            /// Receive username
            bzero(buffer, 1024);
            addr_size = sizeof(client_addr);
            recvfrom(cSock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &addr_size);
            if (!strcmp(buffer, "PASS"))
            {
                break;
            }
            char givenUser[100];

            strcpy(givenUser, buffer);
            printf("The ServerC received an authentication request from the Main Server.\n");

            /// Reveive password
            bzero(buffer, 1024);
            recvfrom(cSock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &addr_size);

            char givenPass[100];
            strcpy(givenPass, buffer);
            int authStat = 0;
            char authStatus[100];
            strcpy(authStatus, "FAIL_NO_USER");

            for (int i = 0; i < line; i++)
            {
                char *gt = givenUser;
                char username[100];
                strcpy(username, data[i]);
                char *passwd;
                strtok_r(username, ",", &passwd);
                if (!strcmp(username, gt)) /// check if the given username matches the credentials list
                {
                    authStat = 1;
                    strcpy(authStatus, "FAIL_NO_PASS");
                    if (!strcmp(passwd, givenPass)) /// check if ppassword matches if the username matches
                    {
                        strcpy(authStatus, "PASS");
                        break;
                    }
                }
            }

            /// send the status of the authentication to the main Server
            bzero(buffer, 1024);
            strcpy(buffer, authStatus);
            sendto(cSock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));

            count1 -= 1;
            printf("The ServerC finished sending the response to the MainServer.\n");
        }
    }
    return 0;
}