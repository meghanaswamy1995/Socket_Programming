#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

#define MAX_LINES 100
#define MAX_LEN 1000

int main(void)
{
    FILE *file;
    char data[MAX_LINES][MAX_LEN];
    file = fopen("ee.txt", "r");
    if (file == NULL)
    {
        printf("Error opening file. \n");
        return 1;
    }
    int line = 0;
    char *entry;
    while (!feof(file) && !ferror(file))
    {
        if (fgets(data[line], MAX_LEN, file) != NULL)
            line++;
    }
    fclose(file);

    char code[100][50], credits[100][20], prof[100][100], time[100][20], name[100][100];

    for (int i = 0; i < line; i++)
    {

        if ((strlen(data[i]) > 0) && (data[i][strlen(data[i]) - 1] == '\n'))
        {
            data[i][strlen(data[i]) - 2] = '\0';
        }
        // printf("----printng all contenst :%s\n", data[i]);
        char info[500];
        strcpy(info, data[i]);

        strcpy(code[i], strtok(info, ","));
        strcpy(credits[i], strtok(NULL, ","));
        strcpy(prof[i], strtok(NULL, ","));
        strcpy(time[i], strtok(NULL, ","));
        strcpy(name[i], strtok(NULL, ","));
    }
    // for (int i = 0; i < line; i++)
    // {
    //     printf("%s | %s | %s |%s |%s \n", code[i], credits[i], prof[i], time[i], name[i]);
    // }

    char *ip = "127.0.0.1";
    int port = 23173;

    int eeSock;
    struct sockaddr_in server_addr, client_addr;
    socklen_t addr_size;
    char buffer[1024];
    int n;

    eeSock = socket(AF_INET, SOCK_DGRAM, 0);
    if (eeSock < 0)
    {
        perror("[-]Socket error");
        exit(1);
    }
    // printf("[+]UDP server socket created.\n");

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = 23173;
    server_addr.sin_addr.s_addr = inet_addr(ip);

    n = bind(eeSock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (n < 0)
    {
        perror("[-]Bind error");
        exit(1);
    }
    printf("The ServerEE is up and running using UDP on port %d.\n", port);

    ////////////// READY TO RECEIVE DATA /////////////

    while (1)
    {
        bzero(buffer, 1024);
        addr_size = sizeof(client_addr);
        // printf("here..");
        recvfrom(eeSock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &addr_size);
        char returnVal[100];

        if (!strcmp(buffer, "MULTIPLE"))
        {
            bzero(buffer, 1024);
            addr_size = sizeof(client_addr);
            printf("here..");
            recvfrom(eeSock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &addr_size);
            char givenCode[100];
            int found = 0;

            strcpy(givenCode, buffer);
            for (int i = 0; i < line; i++)
            {
                if (!strcmp(givenCode, code[i]))
                {
                    strcpy(returnVal, data[i]);
                    found = 1;
                }
            }
            if (found == 0)
            {
                strcpy(returnVal, "Didn't find the course: ");
                strcat(returnVal, givenCode);
            }
            // printf("code received %s-\n", buffer);
        }
        else
        {
            bzero(buffer, 1024);
            addr_size = sizeof(client_addr);
            printf("here..");
            recvfrom(eeSock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &addr_size);
            char givenCode[100];
            strcpy(givenCode, buffer);
            // printf("code received %s-\n", buffer);

            bzero(buffer, 1024);
            recvfrom(eeSock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &addr_size);
            char givenCtgry[100];
            strcpy(givenCtgry, buffer);
            // printf("code received %s-\n", buffer);
            printf("The ServerEE received a request from the Main Server about the %s of %s.\n", givenCtgry, givenCode);

            int idx = -1;

            for (int i = 0; i < line; i++)
            {
                if (!strcmp(givenCode, code[i]))
                {
                    idx = i;
                }
            }
            if (idx != -1)
            {

                if (!strcmp(givenCtgry, "Credit"))
                {
                    strcpy(returnVal, credits[idx]);
                }
                else if (!strcmp(givenCtgry, "Professor"))
                {
                    strcpy(returnVal, prof[idx]);
                }
                else if (!strcmp(givenCtgry, "Days"))
                {
                    strcpy(returnVal, time[idx]);
                }
                else if (!strcmp(givenCtgry, "CourseName"))
                {
                    strcpy(returnVal, name[idx]);
                }
                else
                {
                    printf("Didn't find the category: %s.\n", givenCtgry);
                    strcpy(returnVal, "Didn't find the category: ");
                    strcat(returnVal, givenCtgry);
                }
            }
            else
            {
                printf("Didn't find the course: %s.\n", givenCode);
                strcpy(returnVal, "Didn't find the course: ");
                strcat(returnVal, givenCode);
            }
        }
        bzero(buffer, 1024);
        strcpy(buffer, returnVal);
        // printf("sending buffer %s\n", buffer);
        sendto(eeSock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
        printf("The ServerEE finished sending the response to the Main Server.\n");
    }
    return 0;
}