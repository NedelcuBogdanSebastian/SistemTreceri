/*
 * SOFT ZERO CROSS 
 * 
 * FUSES
 * --------
 * EFUSE = 0xff     // = 0x07
 * HFUSE = 0xde
 * LFUSE = 0xe2
 * 
 */

#define PCINT_PIN 8         // PB0 - PCINT0
#define PCINT_MODE CHANGE
#define PCINT_FUNCTION      getZeroCross

volatile uint8_t oldPort = 0x00;

#define PCMSK *digitalPinToPCMSK(PCINT_PIN)
#define PCINT digitalPinToPCMSKbit(PCINT_PIN)
#define PCIE  digitalPinToPCICRbit(PCINT_PIN)
#define PCPIN *portInputRegister(digitalPinToPort(PCINT_PIN))

#if (PCIE == 0)
#define PCINT_vect PCINT0_vect
#elif (PCIE == 1)
#define PCINT_vect PCINT1_vect
#elif (PCIE == 2)
#define PCINT_vect PCINT2_vect
#else
#error This board doesnt support PCINT ?
#endif

#define INPUT_PIN_LOW       !(PINB & (1<<PB0))
#define INPUT_PIN_HIGH      (PINB & (1<<PB0))

#define SIGHIGH             PORTC |= (1<<PC5)
#define SIGLOW              PORTC &= ~(1<<PC5)

#define LEDON               PORTB |= (1<<PB5)
#define LEDOFF              PORTB &= ~(1<<PB5)
#define LEDTOGGLE           PORTB ^= (1<<PB5)

volatile uint8_t pulse_counter = 0;

void setup() {
  // led pin as output
  DDRB |= (1<<PB5);
  DDRC |= (1<<PC5); // Output pin
     
  // ==== attachPinChangeInterrupt
  // update the old state to the actual state
  oldPort = PCPIN;
  // pin change mask registers decide which pins are enabled as triggers
  PCMSK |= (1 << PCINT);
  // PCICR: Pin Change Interrupt Control Register - enables interrupt vectors
  PCICR |= (1 << PCIE);
}

void loop() {
}

void getZeroCross(void) {
  if (INPUT_PIN_HIGH) {
    SIGHIGH;
    if (pulse_counter++ == 50) {
      pulse_counter = 0;
      LEDTOGGLE; 
    }
  }

  if (INPUT_PIN_LOW) {
    SIGLOW;
  }
}

ISR(PCINT_vect) {  
  PCINT_FUNCTION();
}
