/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#include <vector>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

#define PORT "3490" // the port client will be connecting to 

#define MAXDATASIZE 1024 // max number of bytes we can get at once

#define STATUS_CODE_SUCCESS "200" // success status code
#define STATUS_CODE_NOT_FOUND "404" // file not found
#define STATUS_CODE_FAILED "400" // other failures

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *) sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *) sa)->sin6_addr);
}

// parse url, return ip, port and filePath
string *parse_url(string url) {
    int index = url.find_first_of("//");
    // e.g. 12.34.56.78:8888/somefile.txt
    string path = url.substr(index + 2);
    index = path.find_first_of("/");
    // e.g. /somefile.txt
    string filePath = path.substr(index);
    // ip:port
    string socket = path.substr(0, index);
    string port = "";
    string ip = "";
    // check if contains port
    if (socket.find(":") == -1) {
        port = "80";
        ip = socket;
    } else {
        index = socket.find(":");
        port = socket.substr(index + 1);
        ip = socket.substr(0, index);
    }
    printf("ip: %s, port: %s, filePath: %s\n", ip.c_str(), port.c_str(), filePath.c_str());
    static string res[3];
    res[0] = ip;
    res[1] = port;
    res[2] = filePath;
    return res;
}

// check status code
bool check(string head) {
    printf("receive head: %s\n", head.c_str());
    string status_code = "";
    for (int i = 0; i < head.length(); ++i) {
        if (head[i] == ' ') {
            int left = i + 1;
            status_code = head.substr(left, 3);
            break;
        }
    }
    printf("status code: %s\n", status_code.c_str());
    return status_code == STATUS_CODE_SUCCESS;
}

int main(int argc, char *argv[]) {
    int sockfd, numbytes;
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 2) {
        fprintf(stderr, "usage: client hostname\n");
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // parse url
    string *parsed;
    parsed = parse_url(argv[1]);
    string ip = parsed[0];
    string port = parsed[1];
    string filePath = parsed[2];

    if ((rv = getaddrinfo(ip.c_str(), port.c_str(), &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }

    // loop through all the results and connect to the first we can
    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                             p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return 2;
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *) p->ai_addr),
              s, sizeof s);
    printf("client: connecting to %s\n", s);

    freeaddrinfo(servinfo); // all done with this structure

    // send http request GET
    string head = "GET " + filePath + " HTTP/1.1\r\n\r\n"; // basic head
    printf("request head: %s\n", head.c_str());
    send(sockfd, head.c_str(), head.size(), 0);

    // output file
    FILE *output = fopen("./output", "w");
    // find head
    bool foundHead = false;
    while (1) {
        numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0);
        // end of bytes
        if (numbytes <= 0) {
            fclose(output);
            printf("end of file\n");
            break;
        }
        // if did not find head, just search for "\r\n\r\n" in the buffer
        if (!foundHead) {
            string s = buf;
            int index = s.find_first_of("\r\n\r\n");
            if (index != -1) {
                // check if the return code is 200
                if (!check(s.substr(0, index))) {
                    break;
                }
                foundHead = true;
                char *data_begin = strstr(buf, "\r\n\r\n") + 4;
                fwrite(data_begin, strlen(data_begin), 1, output);
            }
        } else {
            fwrite(buf, numbytes, 1, output);
        }
        printf("receive %d bytes\n", numbytes);
    }
    printf("connection finished.\n");
    close(sockfd);

    return 0;
}

