/*
 * fsSockets.cpp: Shared code for messaging between fsData, fsGUI and trodes
 *
 * Copyright 2014 Loren M. Frank
 *
 * This program is part of the trodes data acquisition package.
 * nspike is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * nspike is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with nspike; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <fsDefines.h>
#include <errno.h>
#include <ctype.h>

static char tmpdata[MAX_BUFFER_SIZE];

int GetServerSocket(const char *name)
/* returns the file descriptor for a blocking server socket */
{
    struct sockaddr_un address;
    int socketid;
    int conn;
    socklen_t addrlen;

    //fd_set            servercon;
    //struct timeval        timeout;

    /* get the socket */
    if ((socketid = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Error in GetServerSocket: unable to get socket");
        return -1;
    }

    /* make this a UNIX domain socket */
    address.sun_family = AF_UNIX;
    /* unlink the socket */
    unlink(name);
    /* assign the path */
    strcpy(address.sun_path, name);
    /* the total length of the address is the sum of the lengths of the family
     * and path elements */
    addrlen = sizeof(address.sun_family) + sizeof(address.sun_path);
    /* bind the address to the socket */
    if (bind(socketid, (struct sockaddr *)&address, addrlen)) {
        fprintf(stderr, "GetServerSocket: Error binding server socket to address\n");
        return -1;
    }
    if (listen(socketid, 5)) {
        fprintf(stderr, "Error listening to server socket");
        return -1;
    }
    /* get the socket (waits for "connect" from client)*/
    if ((conn = accept(socketid, (struct sockaddr * )&address, &addrlen)) < 0) {
        fprintf(stderr, "GetServerSocket: Error connecting to socket");
        return -1;
    }

    return conn;
}

int GetServerSocket(const char *name, int timeoutsec, int timeoutusec)
/* returns the file descriptor for a non-blocking server socket */
{
    struct sockaddr_un address;
    int socketid;
    int conn;
    socklen_t addrlen;

    //fd_set            servercon;
    //struct timeval        timeout;

    /* get the socket */
    if ((socketid = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Error in GetServerSocket: unable to get socket %s\n", name);
        return -1;
    }

    /* make this a UNIX domain socket */
    address.sun_family = AF_UNIX;
    /* unlink the socket */
    unlink(name);
    /* assign the path */
    strcpy(address.sun_path, name);
    /* the total length of the address is the sum of the lengths of the family
     * and path elements */
    addrlen = sizeof(address.sun_family) + sizeof(address.sun_path);
    /* bind the address to the socket */
    if (bind(socketid, (struct sockaddr *)&address, addrlen)) {
        fprintf(stderr, "GetServerSocket: Error binding server socket %s to address\n", name);
        return -1;
    }
    if (listen(socketid, 5)) {
        fprintf(stderr, "Error listening to server socket %s\n", name);
        return -1;
    }

    /* set the socket to be non-blocking */
    fcntl(socketid, F_SETFL, O_NONBLOCK);

    timeoutusec += timeoutsec * 1000000;
    do {
        conn = accept(socketid, (struct sockaddr * )&address, &addrlen);
        if (conn < 0) {
            usleep(50000);
            timeoutusec -= 50000;
        }
    } while ((conn < 0) && (timeoutusec > 0));


    return conn;
}


int GetClientSocket(const char *name)
/* returns the file descriptor for a non-blocking client socket */
{
    struct sockaddr_un address;
    int socketid;
    size_t addrlen;
    fd_set servercon;
    struct timeval timeout;

    /* set a timeout */
    timeout.tv_sec = SOCKET_CLIENT_TIMEOUT;
    timeout.tv_usec = 0;

    /* get the socket */
    if ((socketid = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "GetClientSocket: unable to get socket %s\n", name);
        return 0;
    }

    /* make this a UNIX domain socket */
    address.sun_family = AF_UNIX;

    /* assign the path */
    strcpy(address.sun_path, name);

    /* the total length of the address is the sum of the lengths of the family
     * and path elements */
    addrlen = sizeof(address.sun_family) + sizeof(address.sun_path);

    /* set up fd_set to wait for a connection from the server */
    FD_ZERO(&servercon);
    FD_SET(socketid, &servercon);

    /* wait for a connection on the socket (blocks on server executing "accept")*/
    if (select(socketid + 1, NULL, &servercon, NULL, &timeout) == 0) {
        fprintf(stderr, "GetClientSocket: Error waiting for socket to be ready\n");
        return -1;
    }

    /* connect to the socket */
    while (connect(socketid, (struct sockaddr *)&address, addrlen) < 0) {
        //fprintf(stderr,"GetClientSocket: Waiting for server on socket %s\n", name);
        usleep(100000);
    }

    return(socketid);
}

int GetClientSocket(const char *name, int timeoutsec, int timeoutusec)
/* returns the file descriptor for a non-blocking client socket */
{
    struct sockaddr_un address;
    int socketid;
    size_t addrlen;
    fd_set servercon;
    struct timeval timeout;

    /* set a timeout */
    timeout.tv_sec = timeoutsec;
    timeout.tv_usec = timeoutusec;

    /* get the socket */
    if ((socketid = socket(PF_UNIX, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "GetClientSocket: unable to get socket %s\n", name);
        return -1;
    }

    /* make this a UNIX domain socket */
    address.sun_family = AF_UNIX;

    /* assign the path */
    strcpy(address.sun_path, name);

    /* the total length of the address is the sum of the lengths of the family
     * and path elements */
    addrlen = sizeof(address.sun_family) + sizeof(address.sun_path);

    /* set up fd_set to wait for a connection from the server */
    FD_ZERO(&servercon);
    FD_SET(socketid, &servercon);

    /* wait for a connection on the socket */
    if (select(socketid + 1, NULL, &servercon, NULL, &timeout) == 0) {
        return -1;
    }

    timeoutusec += timeoutsec * 1000000;
    /* connect to the socket */
    while ((timeoutusec > 0) &&
           (connect(socketid, (struct sockaddr *)&address, addrlen) < 0)) {
        usleep(100000);
        timeoutusec -= 100000;
    }
    if (timeoutusec <= 0) {
        close(socketid);
        unlink(name);
        return -1;
    }
    return(socketid);
}

int SendMessage(int fd, uint8_t message, const char *data, uint32_t datalen)
// send a message to a qt socket.
{
    u32 dlen;
    int nwritten;
    int totalwritten;
    int writesize;

    int tmp;
    char messageout[1024], *mptr;


    // create a single array to send.  This is necessary for UDP datagrams
    mptr = messageout;
    *mptr = message;
    mptr++;
    memcpy(mptr, &datalen, sizeof(uint32_t));
    mptr += sizeof(uint32_t);
    if (datalen > 0) {
        memcpy(mptr, data, datalen);
    }

    if (write(fd, messageout, 5 + datalen) != 5 + datalen) {
        fprintf(stderr, "Error: unable to write message in SendMessage message %d, fd %d \n", message, fd);
        return -1;
     }

//    /* send the message */
//    if (write(fd, (char *) &message, sizeof(uint8_t)) != sizeof(uint8_t)) {
//        fprintf(stderr, "Error: unable to write message in SendMessage message %d, fd %d \n", message, fd);
//        return -1;
//    }


//    write(fd, (char *) &datalen, sizeof(uint32_t));
//    //fprintf(stderr, "message datalen %d\n", datalen);

//    nwritten = 0;
//    totalwritten = 0;
//    while (datalen > 0) {
//        writesize = datalen > MAX_SOCKET_WRITE_SIZE ? MAX_SOCKET_WRITE_SIZE : datalen;
//        nwritten = write(fd, data + totalwritten, writesize);
//        if (nwritten == -1) {
//            fprintf(stderr, "Error: unable to write data in SendMessage\n");
//            return -1;
//        }
//        else {
//            totalwritten += nwritten;
//            datalen -= nwritten;
//        }
//    }
    return 1;
}

int SendHardwareMessage(int fd, const char *data, u32 datalen, struct sockaddr_in socketAddress)
/* Send a state script message to the SpikeGadgets hardware */
{
    int nwritten;
    u32 totalwritten;
    u32 writesize;
    u32 maxwritesize = 1000;  // for UDP packets

    /* send the message */
    /* while (datalen > 0) {
        writesize = datalen > maxwritesize ? maxwritesize : datalen;
        nwritten = write(fd, data + totalwritten, writesize);
        if (nwritten == -1) {
            fprintf(stderr, "fsSockets: Error unable to write complete buffer "
                    "in SendHardwareMessage, datalen: %d writesize: %d totalwritten: %d\n", datalen, writesize, totalwritten);
            return -1;
        }
        else {
            totalwritten += nwritten;
            datalen -= nwritten;
        }
    } */

    if(sendto(fd, data, datalen, 0,(struct sockaddr*) &socketAddress, sizeof(struct sockaddr_in)) == -1) {
        fprintf(stderr, "fsSockets: SendHardwareMessage unable to sendto datalen: %d errno %d\n", datalen, errno);
        return -1;
    }
    return 1;
}

int GetMessage(int fd, char *messagedata, u32 *datalen, int block)
/* gets a message, if one is present.  */
{
    static int noMessageCounter; // automatically initialized to 0
    uint8_t message;
    int ntoread, nread, totalread, readsize;

    /* read the message */
    if (block) {
        while (read(fd, &message, sizeof(uint8_t)) < 0) ;
    }
    else if (read(fd, &message, sizeof(uint8_t)) <= 0) {
        printf("fsSockets: GetMessage(): No message\n");
        if (++noMessageCounter > 10) {
            printf("FSData exiting...\n");
            fsdataexit(-1);
        }
        return -1;
    }
    read(fd, datalen, sizeof(u32));

    //fprintf(stderr, "Get Message datalen = %d\n", *datalen);

    if (*datalen > 0) {
        totalread = 0;
        /* read into messagedata */
        ntoread = *datalen;
        while (ntoread > 0) {
            readsize = ntoread > MAX_SOCKET_WRITE_SIZE ? MAX_SOCKET_WRITE_SIZE : ntoread;
            /*if (sysinfo.program_type == SPIKE_DAQ) {
               readsize = 1000;
               } */
            nread = read(fd, messagedata + totalread, readsize);
            if (nread != -1) {
                totalread += nread;
                ntoread -= nread;
            }
            if (block && nread == 0) {
                /* somehow we've lost some data */
                //printf("fsSockets: GetMessage(): Expected data lost\n");

                return -1;
            }
        }
    }
    return message;
}

int GetUDPMessage(int fd, char *messagedata, u32 *datalen, int block)
/* gets a message, if one is present.  */
{
    uint8_t message;
    char data[1024], *dataptr;
    int nread;

    /* read the message */
    nread = read(fd, data, 1024);
    if (nread == 0) {
        fprintf(stderr, "fsSockets: GetUDPMessage(): No message\n");
        return -1;
    }
    //fprintf(stderr, "UDP bytes read: %d\n", nread);

    dataptr = data;
    message = (uint8_t) *data;
    dataptr++;
    memcpy(datalen, dataptr, sizeof(u32));
    dataptr += sizeof(u32);
    memcpy(messagedata, dataptr, *datalen);

    return message;
}

int WaitForMessage(int fd, uint8_t message, float sec)
/* wait up to about sec seconds for a message on the specified socket and return 1
 * if the message was recieved and 0 otherwise */
{
    fd_set readfds;       // the set of readable fifo file descriptors
    struct timeval timeout;
    u32 tmp[1];

    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);
    timeout.tv_sec = (int)sec;
    timeout.tv_usec = (long)((sec - (int)sec) * 1e6);
    while (1) {
        select(fd + 1, &readfds, NULL, NULL, &timeout);
        if (FD_ISSET(fd, &readfds) && (GetMessage(fd, tmpdata, tmp, 0)) == message) {
            return 1;
        }
        else if ((timeout.tv_sec == 0) && (timeout.tv_usec == 0)) {
            return 0;
        }
    }
}

int WaitForMessage(int fd, uint8_t message, float sec, char *data, u32 *datalen)
/* wait up to about sec seconds for a message on the specified socket and return 1
 * if the message was recieved and 0 otherwise */
{
    fd_set readfds;       // the set of readable fifo file descriptors
    struct timeval timeout;

    FD_ZERO(&readfds);
    FD_SET(fd, &readfds);
    timeout.tv_sec = (int)sec;
    timeout.tv_usec = (long)((sec - (int)sec) * 1e6);
    while (1) {
        select(fd + 1, &readfds, NULL, NULL, &timeout);
        if (FD_ISSET(fd, &readfds) && (GetMessage(fd, data, datalen, 0)) == message) {
            return 1;
        }
        else if ((timeout.tv_sec == 0) && (timeout.tv_usec == 0)) {
            return 0;
        }
    }
}


void CloseSockets(SocketInfo *s)
{
    int i;

    for (i = 0; i < MAX_SOCKETS; i++) {
        close(s[i].fd);
        if (strlen(s[i].name)) {
            unlink(s[i].name);
        }
    }
    return;
}

int GetTCPIPServerSocket(unsigned short port)
/* returns the file descriptor for a network server socket */
{
    struct sockaddr_in address;
    int socketid;
    int conn;
    int i;
    socklen_t addrlen;


    /* get the socket */
    if ((socketid = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "Error in GetServerSocket: unable to get socket");
        return -1;
    }

    i = 1;
    /* bind the address to the socket */
    setsockopt(socketid, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));

    addrlen = sizeof(struct sockaddr_in);

    /* make this a network domain socket */
    address.sin_family = AF_INET;

    /* assign the port */
    address.sin_port = htons(port);

    memset(&address.sin_addr, 0, sizeof(address.sin_addr));

    if (bind(socketid, (struct sockaddr *)&address, sizeof(address))) {
        fprintf(stderr, "GetTCPIPServerSocket: Error binding server socket to address, port %d\n", port);
        return -1;
    }
    if (listen(socketid, 5)) {
        fprintf(stderr, "Error listening to server socket");
        return -1;
    }

    /* get the socket */
    if ((conn = accept(socketid, (struct sockaddr * )&address, &addrlen)) < 0) {
        fprintf(stderr, "GetServerSocket: Error connecting to socket");
        return -1;
    }

    return conn;
}


int GetTCPIPClientSocket(const char *name, unsigned short port)
/* returns the file descriptor for a client socket */
{
    struct sockaddr_in address;
    struct hostent        *host;
    int socketid;
    fd_set con;
    struct timeval timeout;

    /* set a timeout of 120 seconds */
    //timeout.tv_sec = TCPIP_SOCKET_CLIENT_TIMEOUT;
    timeout.tv_sec = 5;
    timeout.tv_usec = 0;


    host = NULL;
    if (!(inet_aton(name, &address.sin_addr))) {
        host = gethostbyname(name);
        if (!host) {
            fprintf(stderr, "Unable to lookup host %s\n", name);
            return -1;
        }
    }

    /* get the socket */
    if ((socketid = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        fprintf(stderr, "GetTCPIPClientSocket: unable to get socket to %s\n", name);
        return -1;
    }


    /* make this a network socket */
    address.sin_family = AF_INET;
    /* set the port */
    address.sin_port = htons(port);

    if (!(host == NULL)) {
        /* Get the first ip address associated with this machine */
        memcpy(&address.sin_addr, host->h_addr_list[0], sizeof(address.sin_addr));
    }

    /* set up fd_set to wait for a connection from the server */
    FD_ZERO(&con);
    FD_SET(socketid, &con);

    /* connect to the socket */
//    fprintf(stderr, "&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&  Connecting to address %s on port %d\n", name, port);
    while (connect(socketid, (struct sockaddr *)&address, sizeof(address)) < 0) {
        int errorsv = errno;
        sleep(1);
        fprintf(stderr, "GetTCPIPClientSocket: waiting for socket server for connection to %s on port %d\n", name, port);
        fprintf(stderr, "  Error number: %d",errorsv);
    }

    return(socketid);
}

int GetUDPServerSocket(unsigned short port)
/* returns the file descriptor for a UDP network server socket */
/* used only in spike_daq.c */
{
    struct sockaddr_in address;
    int socketid;
    //int           conn;
    int i;
    size_t addrlen;

    /* get the socket */
    if ((socketid = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket");
        return -1;
    }


    i = 1;
    /* bind the address to the socket */
    setsockopt(socketid, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));

    /* make sure the address is clear */
    bzero(&address, sizeof(address));

    addrlen = sizeof(struct sockaddr_in);

    /* make this a network domain socket */
    address.sin_family = AF_INET;

    /* assign the port */
    address.sin_port = htons(port);

    address.sin_addr.s_addr = INADDR_ANY;

    if (bind(socketid, (struct sockaddr *)&address, sizeof(address)) != 0) {
        perror("bind");
        return -1;
    }

//  fcntl(socketid, F_SETFL, O_NONBLOCK);
    return socketid;
}

int GetUDPHardwareClientSocket(const char *name, unsigned short port, struct sockaddr_in *address)
{
    // This gets a UDP connection with flags set to communicate successfully to the MCU or ECU
    //int                *data;
    struct in_addr inaddr;
    struct hostent        *host;
    int socketid;

    if (inet_aton(name, &inaddr)) {
        //fprintf(stderr, "fsSockets: hostname is IPV4 address: %s\n", inet_ntoa(inaddr));
        //host = gethostbyaddr((char*)&inaddr, sizeof(inaddr), AF_INET);
    }
    else {
        //fprintf(stderr, "fsSockets: hostname is: %s\n", name);
        host = gethostbyname(name);

        if (!host) {
            char host_err_label[20];
            switch(h_errno) {
            case HOST_NOT_FOUND:
                sprintf(host_err_label,"HOST_NOT_FOUND");
                break;
            case NO_ADDRESS:
                sprintf(host_err_label,"NO_ADDRESS");
                break;
            case NO_RECOVERY:
                sprintf(host_err_label,"NO_RECOVERY");
                break;
            case TRY_AGAIN:
                sprintf(host_err_label,"TRY_AGAIN");
            default:
                sprintf(host_err_label,"");
            }

            fprintf(stderr, "Unable to find hostname %s, error: %s\n", name, host_err_label);
            return -1;
        }

        memcpy(&inaddr, host->h_addr_list[0], sizeof(struct in_addr));
    }
    /* get the socket */
    if ((socketid = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        fprintf(stderr, "GetUDPClientSocket: unable to get socket");
        return -1;
    }
    /* make this a network socket */
    address->sin_family = AF_INET;
    /* set the port */
    address->sin_port = htons(port);

    /* Get the first ip address associated with this machine */
    memcpy(&address->sin_addr, &inaddr, sizeof(struct in_addr));
    /* connect to the socket */
    if (bind(socketid, (struct sockaddr *)address, sizeof(struct sockaddr_in)) == -1) {
        fprintf(stderr, "fsSockets: Error connecting UDP client socket on port %d, addr: %s, errno: %d\n",
                port, inet_ntoa(address->sin_addr), errno);

        return -1;
    }
    /* set this to be non-blocking.  This is done to resolve a bug in the DSP
     * code where some messages are received but the proper acknowledgement
     * is not sent */
    fcntl(socketid, F_SETFL, O_NONBLOCK);

    int on = 1;
    if(setsockopt(socketid, SOL_SOCKET, SO_BROADCAST, (int *)&on, sizeof(on)) < 0) {
        printf("fsSockets: GetUDPClientSocket, failed to set UDP as broadcast.");
        return -1;
    }
    return(socketid);
}

int GetUDPDataClientSocket(const char *name, unsigned short port, struct sockaddr_in *address)
{
    //int                *data;
    struct in_addr inaddr;
    struct hostent        *host;

    int socketid;

    if (inet_aton(name, &inaddr)) {
        fprintf(stderr, "fsSockets: hostname is IPV4 address: %s\n", inet_ntoa(inaddr));
        //host = gethostbyaddr((char*)&inaddr, sizeof(inaddr), AF_INET);
    }
    else {
        fprintf(stderr, "fsSockets: hostname is: %s\n", name);
        host = gethostbyname(name);

        if (!host) {
            char *host_err_label;
            switch(h_errno) {
            case HOST_NOT_FOUND:
                host_err_label = "HOST_NOT_FOUND";
                break;
            case NO_ADDRESS:
                host_err_label = "NO_ADDRESS";
                break;
            case NO_RECOVERY:
                host_err_label = "NO_RECOVERY";
                break;
            case TRY_AGAIN:
                host_err_label = "TRY_AGAIN";
            default:
                host_err_label = "";
            }

            fprintf(stderr, "Unable to find hostname %s, error: %s\n", name, host_err_label);
            return -1;
        }

        memcpy(&inaddr, host->h_addr_list[0], sizeof(struct in_addr));
    }
    /* get the socket */
    if ((socketid = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
        fprintf(stderr, "GetUDPClientSocket: unable to get socket");
        return -1;
    }
    /* make this a network socket */
    address->sin_family = AF_INET;
    /* set the port */
    address->sin_port = htons(port);

    /* Get the first ip address associated with this machine */
    memcpy(&address->sin_addr, &inaddr, sizeof(struct in_addr));
    /* connect to the socket */
    if (connect(socketid, (struct sockaddr *) address, sizeof(struct sockaddr_in)) == -1) {
        fprintf(stderr, "fsSockets: Error connecting UDP client socket on port %d, addr: %s, errno: %d\n",
                port, inet_ntoa(address->sin_addr), errno);

        return -1;
    }
    return(socketid);
}

void ClearData(int fd)
{
    /* set the socket to be non blocking temporarily */
    fcntl(fd, F_SETFL, O_NONBLOCK);
    /* read in and discard all of the data in the specified socket */
    while (read(fd, tmpdata, MAX_SOCKET_WRITE_SIZE) > 0) ;
    /* set it back to blocking */
    fcntl(fd, F_SETFL, 0);
}


void ErrorMessage(const char *errorstring, SocketInfo *client_message)
/* display an error message */
{
    SendMessage(client_message[0].fd, TRODESMESSAGE_ERRORMESSAGE, errorstring, strlen(errorstring) * sizeof(char) + 1);
    return;
}

void StatusMessage(const char *message, SocketInfo *client_message)
{
    SendMessage(client_message[0].fd, TRODESMESSAGE_STATUSMESSAGE, message,
                strlen(message) * sizeof(char) + 1);
    return;
}





void SetupFDList(fd_set *readfds, int *maxfds, SocketInfo *message, SocketInfo *data)
/* set up the list of file descriptors to watch for incoming data or
 * messages and for outgoing */
{
    int i;

    *maxfds = 0;

    FD_ZERO(readfds);
    /* go through module file descriptors and add each one to the list */
    i = 0;
    while ((message[i].fd != -1) && (i < MAX_SOCKETS)) {
        FD_SET(message[i].fd, readfds);
        *maxfds = MAX(*maxfds, message[i].fd);
        i++;
    }
    i = 0;
    while ((data[i].fd != -1) && (i < MAX_SOCKETS)) {
        FD_SET(data[i].fd, readfds);
        *maxfds = MAX(*maxfds, data[i].fd);
        i++;
    }
    (*maxfds)++;
}

void AddFD(int fd, SocketInfo *s, int *fdlist)
/* add the file descriptor to the socket info structure and the fdlist */
{
    int i, j;

    /* find the first available element of the s outside the MAX_SOCKETS range
     * */
    i = MAX_SOCKETS - 1;
    while (s[++i].fd) ;
    /* set this element to be the fd */
    s[i].fd = fd;
    /* add the index to the fdlist */
    j = -1;
    while (fdlist[++j] != -1) ;
    fdlist[j] = i;
    fdlist[j + 1] = -1;
    return;
}


void RemoveFD(int fd, SocketInfo *s, int *fdlist)
/* remove the file descriptor to the socket info structure and the fdlist */
{
    int i, j;

    /* find the fd in s */
    i = -1;
    while (s[++i].fd != fd) ;
    /* remove this fd */
    s[i].fd = 0;
    /* remove the index for this fd */
    j = -1;
    while (fdlist[++j] != i) ;
    while (fdlist[j] != -1) {
        fdlist[j] = fdlist[j + 1];
        j++;
    }
    return;
}

uint16_t ByteSwap(uint16_t x)
{
    return ((x & 0xff) << 8) | ((x & 0xff00) >> 8);
}







