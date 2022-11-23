#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAX_LINES 100
#define MAX_LEN 1000

int main(void)
{

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
    while (!feof(file) && !ferror(file))
    {
        // if (fgets(entry, MAX_LEN, file) != NULL)
        //     line++;
        if (fgets(data[line], MAX_LEN, file) != NULL)
            line++;
        // data[line][strcspn(data[line], "\n")] = 0;
        //  data[line] = *entry;
    }
    fclose(file);

    for (int i = 0; i < line; i++)
    {
        if ((strlen(data[i]) > 0) && (data[i][strlen(data[i]) - 1] == '\n'))
        {
            // printf("last char-%d", strlen(data[i]));
            data[i][strlen(data[i]) - 2] = '\0';
        }
        // printf("----printng all contenst :%s", data[i]);
    }

    char *ip = "127.0.0.1";
    int port = 21173;

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
    // printf("[+]UDP server socket created.\n");

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = port;
    server_addr.sin_addr.s_addr = inet_addr(ip);

    n = bind(cSock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (n < 0)
    {
        perror("[-]Bind error");
        exit(1);
    }
    // printf("[+]Bind to the port number: %d\n", port);
    printf("The ServerC is up and running using UDP on port %d.\n", port);
    while (1)
    {
        int count1 = 3;
        int authenticated1 = 0;
        while (count1 > 0 && !authenticated1)
        {
            bzero(buffer, 1024);
            addr_size = sizeof(client_addr);
            recvfrom(cSock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &addr_size);
            if (!strcmp(buffer, "PASS"))
            {
                break;
            }
            char givenUser[100];
            // printf("Username received %s-\n", buffer);
            strcpy(givenUser, buffer);
            printf("The ServerC received an authentication request from the Main Server.\n");

            bzero(buffer, 1024);
            recvfrom(cSock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &addr_size);
            // printf("passreceived %s-\n", buffer);
            char givenPass[100];
            strcpy(givenPass, buffer);
            // printf("given pass - %s\n", buffer);

            int authStat = 0;
            char authStatus[100];
            strcpy(authStatus, "FAIL_NO_USER");

            for (int i = 0; i < line; i++)
            {
                char *gt = givenUser;
                char username[100];
                strcpy(username, data[i]);
                char *passwd;
                // printf("UU - %s PASSS - %s", ll, ptr);
                strtok_r(username, ",", &passwd);
                if (!strcmp(username, gt))
                {
                    authStat = 1;
                    strcpy(authStatus, "FAIL_NO_PASS");
                    if (!strcmp(passwd, givenPass))
                    {
                        strcpy(authStatus, "PASS");
                        break;
                    }
                }
            }

            bzero(buffer, 1024);
            strcpy(buffer, authStatus);
            // printf("sending buffer %s\n", buffer);
            sendto(cSock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));

            count1 -= 1;
            printf("The ServerC finished sending the response to the MainServer.\n");
        }
    }
    return 0;
}