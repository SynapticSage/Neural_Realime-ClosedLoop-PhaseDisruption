#ifndef FSSOCKETDEFINES_H
#define FSSOCKETDEFINES_H

/* the size of the buffer used.  */
#define MAX_BUFFER_SIZE			100000
#define MAX_SOCKETS             1000
#define  MAX_SOCKET_WRITE_SIZE          65536 // the largest single write size
/* defines for the three types of sockets */
#define UNIX		1000
#define TCPIP		1001
#define UDP         1002
#define SOCKET_CLIENT_TIMEOUT   5  // 5 second timeout seems to work
#define TCPIP_SOCKET_CLIENT_TIMEOUT     120     // wait up to two minutes

uint16_t ByteSwap(uint16_t x);

#endif // FSSOCKETDEFINES_H
