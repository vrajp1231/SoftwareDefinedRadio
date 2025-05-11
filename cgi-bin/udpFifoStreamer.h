#ifndef _UDP_FIFO_STREAMER_H_
#define _UDP_FIFO_STREAMER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <time.h>
#include <pthread.h>
#include <signal.h>
#include <semaphore.h>
#include <stdbool.h>

#define AXI4_STREAM_FIFO_BASE_ADDR       0x43c10000 // Base address of the AXI FIFO

// https://docs.amd.com/r/en-US/pg080-axi-fifo-mm-s/Register-Space
#define AXI4_STREAM_FIFO_RDFR_OFFSET     0x18 // Receive data FIFO reset offset
#define AXI4_STREAM_FIFO_RDFO_OFFSET     0x1C // Receive data FIFO occupancy offset
#define AXI4_STREAM_FIFO_RDFD_OFFSET     0x20 // Receive data FIFO data offset
#define AXI4_STREAM_FIFO_RLR_OFFSET      0x24 // Receive Length offset

#define AXI4_STREAM_FIFO_RDFR_RESET_MASK 0x000000FF // Reset mask for receive data FIFO
#define AXI4_STREAM_FIFO_RDFR_RESET_KEY  0x000000A5 // Reset key for receive data FIFO

#define DEFAULT_UDP_DEST_PORT   25344           // Default UDP port
#define DEFAULT_DEST_IP         "192.168.50.205"   // Default destination IP address

#define NUM_SAMPLES 256 // Number of samples in the packet

typedef struct dataPacket
{
    uint32_t packetID;                  // Packet ID
    int32_t  sdrData[NUM_SAMPLES];      // SDR data (256 samples)
} dataPacket;

#define PACKET_SIZE (sizeof(dataPacket)) // 1028 bytes

volatile unsigned int * get_a_pointer(unsigned int phys_addr);
volatile unsigned int * get_pointer_to_axi_fifo();
void fifo_reset(volatile unsigned int *axi_fifo_base);
unsigned int fifo_get_current_occupancy(volatile unsigned int *axi_fifo_base);
unsigned int fifo_get_data(volatile unsigned int *axi_fifo_base);
void usage(const char *executableName);

#endif