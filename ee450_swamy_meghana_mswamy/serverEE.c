#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <ctype.h>

#define MAX_LINES 1000
#define MAX_LEN 5000

int main(void)
{
    //################# Reading the ee.txt file and storing all the entries into the datastructure #################
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
    while (!feof(file) && !ferror(file)) // read all the lines from the file and store it in an array data[]
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
        char info[500];
        strcpy(info, data[i]);
        //// from data array copy all the different category in their respective array
        strcpy(code[i], strtok(info, ","));
        strcpy(credits[i], strtok(NULL, ","));
        strcpy(prof[i], strtok(NULL, ","));
        strcpy(time[i], strtok(NULL, ","));
        strcpy(name[i], strtok(NULL, ","));
    }

    ///  //################# Creating socket  #################

    char *ip = "127.0.0.1";
    int eePort = 23173;

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

    memset(&server_addr, '\0', sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = eePort;
    server_addr.sin_addr.s_addr = inet_addr(ip);

    n = bind(eeSock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (n < 0)
    {
        perror("[-]Bind error");
        exit(1);
    }
    printf("The ServerEE is up and running using UDP on port %d.\n", eePort);

    /// //################# READY TO RECEIVE DATA #################

    while (1)
    {
        bzero(buffer, 1024);
        addr_size = sizeof(client_addr);
        recvfrom(eeSock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &addr_size);
        char returnVal[100];

        if (!strcmp(buffer, "MULTIPLE")) // If multiple courses are queried
        {

            bzero(buffer, 1024);
            addr_size = sizeof(client_addr);
            // Receeive course code
            recvfrom(eeSock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &addr_size);

            char givenCode[100];
            int found = 0;
            strcpy(givenCode, buffer);
            printf("The ServerEE received a request from the Main Server for the course %s.\n", givenCode);

            for (int i = 0; i < line; i++)
            {
                if (!strcmp(givenCode, code[i])) /// finding the course index
                {
                    strcpy(returnVal, code[i]);
                    strcat(returnVal, ": ");
                    strcat(returnVal, credits[i]);
                    strcat(returnVal, ", ");
                    strcat(returnVal, prof[i]);
                    strcat(returnVal, ", ");
                    strcat(returnVal, time[i]);
                    strcat(returnVal, ", ");
                    strcat(returnVal, name[i]);
                    found = 1;
                }
            }
            if (found == 0) // if course not found
            {
                printf("Didn't find the course: %s.\n", givenCode);
                strcpy(returnVal, "Didn't find the course: ");
                strcat(returnVal, givenCode);
            }
            else
            {
                printf("The course info has been found:\n %s\n", returnVal);
            }
        }
        else // if this course code is part of single course query
        {
            bzero(buffer, 1024);
            addr_size = sizeof(client_addr);

            // Receeive course code
            recvfrom(eeSock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &addr_size);
            char givenCode[100];
            strcpy(givenCode, buffer);

            // Receeive category
            bzero(buffer, 1024);
            recvfrom(eeSock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, &addr_size);
            char givenCtgry[100];
            strcpy(givenCtgry, buffer);

            printf("The ServerEE received a request from the Main Server about the %s of %s.\n", givenCtgry, givenCode);

            int idx = -1;
            int cat = 1;

            // finding out the index of the course from the codes array
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
                else /// if the entered category is not found
                {
                    cat = -1;
                    printf("Didn't find the category: %s.\n", givenCtgry);
                    strcpy(returnVal, "!Didn't find the category: ");
                    strcat(returnVal, givenCtgry);
                }
            }
            else // if index is not greater or equal to zero then the course was not found
            {
                printf("Didn't find the course: %s.\n", givenCode);
                strcpy(returnVal, "!Didn't find the course: ");
                strcat(returnVal, givenCode);
            }
            if (idx != -1 && cat != -1)
            {
                printf("The course information has been found: The %s of %s is %s.\n", givenCtgry, givenCode, returnVal);
            }
        }
        // send the query results to the main server
        bzero(buffer, 1024);
        strcpy(buffer, returnVal);
        sendto(eeSock, buffer, sizeof(buffer), 0, (struct sockaddr *)&client_addr, sizeof(client_addr));
        printf("The ServerEE finished sending the response to the Main Server.\n");
    }
    return 0;
}