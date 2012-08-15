//http://www.engblaze.com/microcontroller-tutorial-avr-and-arduino-timer-interrupts/
#include "beerButton.h"


static int counter = 0;
static int c1 = 0, c2 = 0;
static int oneortwo = 0;

void setup() {
    pinMode(13, OUTPUT);

    pinMode(2, OUTPUT);
    pinMode(3, OUTPUT);
    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);
    pinMode(6, OUTPUT);
    pinMode(7, OUTPUT);

    digitalWrite(6, LOW);
    digitalWrite(7, LOW);
    
    setupBrejaCounter();
    
    /* timer1 = 100Hz */
    cli();
    TCCR1A = 0;
    TCCR1B = 0;
    OCR1A = 625;
    TCCR1B |= (1 << WGM12);
    TCCR1B |= (1 << CS12); // prescaler 256
    TIMSK1 |= (1 << OCIE1A);
    sei();
}

void loop() {
    if (++counter == 10)
        counter = 0;
    c1 = counter;
    c2 = 9 - counter;
    delay(1000);
    
    brejaButton();
}

ISR(TIMER1_COMPA_vect)
{
    digitalWrite(6, LOW);
    digitalWrite(7, LOW);

    int c = oneortwo ? c1 : c2;

    digitalWrite(2, c&1);
    c >>= 1;
    digitalWrite(3, c&1);
    c >>= 1;
    digitalWrite(4, c&1);
    c >>= 1;
    digitalWrite(5, c&1);

    digitalWrite(6, oneortwo ? HIGH : LOW);
    digitalWrite(7, oneortwo ? LOW : HIGH);

    oneortwo = !oneortwo;
}

