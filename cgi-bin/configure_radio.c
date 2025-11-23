#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

#define RADIO_PERIPH_ADDRESS 0x43c00000
#define RADIO_TUNER_FAKE_ADC_PINC_OFFSET    0
#define RADIO_TUNER_TUNER_PINC_OFFSET       1
#define RADIO_TUNER_CONTROL_REG_OFFSET      2
#define RADIO_TUNER_TIMER_REG_OFFSET        3
#define RADIO_TUNER_CTRL_RESET_MASK         0x00000001  // mask for reset bit in the control register
#define RADIO_TUNER_CTRL_RESET_ON           0x1         // reset on value for the control register
#define RADIO_TUNER_CTRL_RESET_OFF          0x0         // reset off value for the control register
#define RADIO_TUNER_CTRL_STREAM_EN_MASK     0x00000002  // mask for stream enable bit in the control register
#define RADIO_TUNER_CTRL_STREAM_EN_ON       0x1         // stream enable on value for the control register
#define RADIO_TUNER_CTRL_STREAM_EN_OFF      0x0         // stream enable off value for the control register

volatile unsigned int * get_a_pointer(unsigned int phys_addr);
volatile unsigned int * get_radio_tuner();
void disable_radio_tuner(volatile unsigned int *radio_base);
void enable_radio_tuner(volatile unsigned int *radio_base);
void set_radio_tuner_stream(volatile unsigned int *radio_base, int enable);
void radioTuner_setMixerFreq(volatile unsigned int *ptrToRadio, float tune_frequency);
void radioTuner_setAdcFreq(volatile unsigned int* ptrToRadio, float freq);
void cgi_callback(void);

float adc_freq    = 0;	// Fake ADC frequency
float tune_freq   = 0;	// Tuner frequency
int stream_en   = 0;	// Stream enable/disable

int main(void)
{
    cgi_callback(); // call the cgi callback function to handle the request

    // get a pointer to the radio tuner peripheral
    volatile unsigned int *radio_base = get_radio_tuner();
    if (radio_base == NULL) {
        fprintf(stderr, "Failed to get radio tuner base address\n");
        return -1;
    }

    enable_radio_tuner(radio_base);	
    radioTuner_setMixerFreq(radio_base, tune_freq);
    radioTuner_setAdcFreq(radio_base, adc_freq);
    set_radio_tuner_stream(radio_base, stream_en);

    munmap((void *)radio_base, 4096);

    return 0;
}

void cgi_callback(void)
{
    printf("Content-Type: text/html\n\n");
    printf("<html>\n");
    printf("<head>\n");
    printf("<title>Initialize SDR</title>\n");
    printf("</head>\n");
    printf("<body>\n");
    printf("<h2>Configuring radio now...</h2>\n");

    char* len_str  = getenv("CONTENT_LENGTH");
    int len = len_str ? atoi(len_str) : 0; // convert content length to an integer

    char* data = malloc(len + 1); // allocate memory for the data
    if (data == NULL) {
        printf("Failed to allocate memory for data<br>\n");
        return;
    }

    if (fread(data, 1, len, stdin) != len) {
        printf("Failed to read POST data<br>\n");
        free(data);
        return;
    }

    data[len] = '\0';

    // printf("POST data: %s<br>\n", data);
    printf("<h3>");

    char* token = strtok(data, "&");
    while (token != NULL) {
        if (sscanf(token, "adc_freq_hz=%f", &adc_freq) == 1) {
            printf("ADC Frequency: %f<br>\n", adc_freq);
        } else if (sscanf(token, "tune_freq_hz=%f", &tune_freq) == 1) {
            printf("Tune Frequency: %f<br>\n", tune_freq);
        } else if (sscanf(token, "streaming=%d", &stream_en) == 1);
        token = strtok(NULL, "&");
    }
    free(data);

    if(stream_en == 1) {
        printf("Streaming enabled<br>\n");
    } else {
        printf("Streaming disabled<br>\n");
    }

    printf("</h3>");
    printf("</body>\n");
    printf("</html>\n");
}

// get pointer to the radio tuner base address in user space
volatile unsigned int * get_radio_tuner()
{
    volatile unsigned int *radio_base = get_a_pointer(RADIO_PERIPH_ADDRESS);
    if (radio_base == NULL) {
        fprintf(stderr, "Failed to map radio tuner base address\n");
        return NULL;
    }
    return radio_base;
}

// disable radio
void disable_radio_tuner(volatile unsigned int *radio_base)
{
    *(radio_base + RADIO_TUNER_CONTROL_REG_OFFSET) |= RADIO_TUNER_CTRL_RESET_MASK;
}

// enable radio
void enable_radio_tuner(volatile unsigned int *radio_base)
{
    *(radio_base + RADIO_TUNER_CONTROL_REG_OFFSET) &= ~RADIO_TUNER_CTRL_RESET_MASK;
}

// control radio udp streaming
void set_radio_tuner_stream(volatile unsigned int *radio_base, int enable)
{
    if (enable) {
        *(radio_base + RADIO_TUNER_CONTROL_REG_OFFSET) |= RADIO_TUNER_CTRL_STREAM_EN_MASK;
    } else {
        *(radio_base + RADIO_TUNER_CONTROL_REG_OFFSET) &= ~RADIO_TUNER_CTRL_STREAM_EN_MASK;
    }
}

// set tuner frequency
void radioTuner_setMixerFreq(volatile unsigned int *ptrToRadio, float tune_frequency)
{
	float pinc = tune_frequency*(float)(1<<27)/125.0e6;
	*(ptrToRadio+RADIO_TUNER_TUNER_PINC_OFFSET)=(int)pinc;
}

// set fake ADC frequency
void radioTuner_setAdcFreq(volatile unsigned int* ptrToRadio, float freq)
{
	float pinc = freq*(float)(1<<27)/125.0e6;
	*(ptrToRadio+RADIO_TUNER_FAKE_ADC_PINC_OFFSET) = (int)pinc;
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
	volatile unsigned int *radio_base = (volatile unsigned int *)map_base; 
    close(mem_fd);
	return (radio_base);
}