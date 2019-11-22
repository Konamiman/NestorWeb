#ifndef __BUFFERS_H
#define __BUFFERS_H

/*
These buffers are defined in page 2 because TCP/IP UNAPI implementations
can (and often will) refuse to exchange data with buffers in page 1.
It's ugly but the alternative would be moving the entire data area
to page 2 when compiling, making the resulting program file
unnecessarily big.
*/

#define OUTPUT_DATA_BUFFER_LENGTH 4096
#define OUTPUT_DATA_BUFFER_START (void*)(0xC000-OUTPUT_DATA_BUFFER_LENGTH)

#define TCP_INPUT_DATA_BUFFER_SIZE 2048
#define TCP_INPUT_DATA_BUFFER_START (void*)((int)OUTPUT_DATA_BUFFER_START-TCP_INPUT_DATA_BUFFER_SIZE)


#endif
