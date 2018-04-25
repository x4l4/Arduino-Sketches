#define LED 13 
#define BAUD_RATE 9600
#define RX_PIN 3
#define TX_PIN 2
#define TASMISSION_PIN 4

#define ID_PIN_1 5
#define ID_PIN_2 6
#define ID_PIN_3 7
#define ID_PIN_4 8


void momatic_setup(unsigned int *registri, unsigned int size);
int momatic_read_id();
void momatic_alarm();