#include "udpFifoStreamer.h"

// Global Variables
volatile unsigned int *axi_fifo_base = NULL;    // Pointer to AXI FIFO base address
char *dest_ip = DEFAULT_DEST_IP;                // Destination IP address
int dest_port = DEFAULT_UDP_DEST_PORT;          // Destination port
int timeout = 0;

int main(int argc, char const *argv[])
{
    int opt = 0;
    while ((opt = getopt(argc, (char * const *)argv, "i:p:t:h")) != -1) {
        switch (opt) {
            case 'i': dest_ip = optarg; break;
            case 'p': dest_port = atoi(optarg); break;
            case 't': timeout = atoi(optarg); break;
            case 'h': usage(argv[0]); return 0;
            default: usage(argv[0]); return -1;
        }
    }

    // check arguments
    if(timeout < 0) {
        fprintf(stderr, "Invalid timeout value: %d\n", timeout);
        return -1;
    }
    if(dest_port < 0 || dest_port > 65535) {
        fprintf(stderr, "Invalid port value: %d\n", dest_port);
        return -1;
    }
    struct sockaddr_in sa;
	
    // check IP address
    if(inet_pton(AF_INET, dest_ip, &sa) <= 0) {
        fprintf(stderr, "Invalid IP address: %s\n", dest_ip);
        return -1;
    }

    // display arguments
    printf("Summary:\n");
    printf("Destination IP: %s\n", dest_ip);
    printf("Destination Port: %d\n", dest_port);
    if(timeout > 0) {
        printf("Timeout: %d seconds\n", timeout);
    } else {
        printf("Timeout: infinite\n");
    }
    printf("Packet size: %d bytes\n", PACKET_SIZE);
    printf("Number of samples per packet: %d\n", NUM_SAMPLES);

    // get AXI FIFO base address
    axi_fifo_base = get_pointer_to_axi_fifo();
    if (axi_fifo_base == NULL) {
        fprintf(stderr, "Failed to get AXI FIFO base address\n");
        return -1;
    }

    fifo_reset(axi_fifo_base);

    // create UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Error creating socket");
        return -1;
    }

    // create a sockaddr_in structure to hold server address
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(dest_port);
    if (inet_pton(AF_INET, dest_ip, &server_addr.sin_addr) <= 0) {
        perror("Invalid IP address or IP Address not supported");
        close(sockfd);
        return -1;
    }

    dataPacket packet;
    uint32_t packetID = 0;
    unsigned int numSamplesRead = 0; // number of samples read for current packet
    unsigned int targetSamples = NUM_SAMPLES; // target number of samples to read for each packet

    while(true)
    {
        // process reading data from the FIFO to fill the packet
        while(numSamplesRead < targetSamples) {
            // get current occupancy of the FIFO
            unsigned int occupancy = fifo_get_current_occupancy(axi_fifo_base);
            if (occupancy > 0) {
                // read data from the FIFO
                for (unsigned int i = 0; i < occupancy; i++) {
                    uint32_t temp = fifo_get_data(axi_fifo_base); 
                    // swap bytes inside each int16
                    packet.sdrData[numSamplesRead] = ((temp & 0x0000FFFF) << 16) | ((temp & 0xFFFF0000) >> 16);
                    ++numSamplesRead; // increment the number of samples read
                    if (numSamplesRead >= targetSamples) {
                        break;
                    }
                }
            }
            usleep(1000);
        }

        // check if we have read enough samples for the current packet
        if (numSamplesRead >= targetSamples) {
            packet.packetID = packetID++;

            // send the packet over UDP
            ssize_t bytes_sent = sendto(sockfd, &packet, PACKET_SIZE, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
            if (bytes_sent < 0) {
                perror("Error sending packet");
                close(sockfd);
                break;
            }
            if (packetID % 1000 == 0) {
                printf("Sent %u packets to %s:%d\n", packetID, dest_ip, dest_port);
            }
        }
        numSamplesRead = 0;
    }
    // report the number of packets sent
    printf("Total:sent %d packets to %s : %d\n", packetID, dest_ip, dest_port);

    return 0;
}

// get a pointer to a physical address using /dev/mem
volatile unsigned int * get_a_pointer(unsigned int phys_addr)
{
	int mem_fd = open("/dev/mem", O_RDWR | O_SYNC); 
    if (mem_fd < 0) {
        perror("Failed to open /dev/mem");
        return NULL;
    }
	void *map_base = mmap(0, 4096, PROT_READ | PROT_WRITE, MAP_SHARED, mem_fd, phys_addr); 
    if (map_base == MAP_FAILED) {
        perror("mmap failed");
        close(mem_fd);
        return NULL;
    }
	volatile unsigned int *peripheral_base = (volatile unsigned int *)map_base; 
    close(mem_fd);
	return (peripheral_base);
}

// get a pointer to the AXI FIFO base address in user space
volatile unsigned int * get_pointer_to_axi_fifo()
{
    volatile unsigned int *axi_fifo_base = get_a_pointer(AXI4_STREAM_FIFO_BASE_ADDR);
    if (axi_fifo_base == NULL) {
        fprintf(stderr, "Failed to map AXI FIFO base address\n");
        return NULL;
    }
    return axi_fifo_base;
}

// write reset to reset register
void fifo_reset(volatile unsigned int *axi_fifo_base)
{
    axi_fifo_base[AXI4_STREAM_FIFO_RDFR_OFFSET/4] = AXI4_STREAM_FIFO_RDFR_RESET_KEY;
}

// get the current occupancy of the FIFO by reading occupancy register
unsigned int fifo_get_current_occupancy(volatile unsigned int *axi_fifo_base)
{
    unsigned int occupancy = axi_fifo_base[AXI4_STREAM_FIFO_RDFO_OFFSET/4];
    printf("FIFO occupancy: %u\n", occupancy);
    return occupancy;
}

// get the data from the FIFO by reading the data register
unsigned int fifo_get_data(volatile unsigned int *axi_fifo_base)
{
    return axi_fifo_base[AXI4_STREAM_FIFO_RDFD_OFFSET/4];
}

void usage(const char *executableName)
{
    fprintf(stderr, "Usage: %s -i <IP address> -p <port> -t <timeout_second>\n\n", executableName);
    fprintf(stderr, "  -i <IP address>      : Destination IP address (default: %s)\n\n", DEFAULT_DEST_IP);
    fprintf(stderr, "  -p <port>            : Destination UDP port (default: %d)\n\n", DEFAULT_UDP_DEST_PORT);
    fprintf(stderr, "  -t <timeout_second>  : Timeout (seconds) (default: 0)\n\n");
    fprintf(stderr, "  -h                   : Show help message\n\n");
}
