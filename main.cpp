/* Name: main.c
 * Author: Jonathan M. Guberman
 * Copyright: <insert your copyright message here>
 * License: <insert your license reference here>
 */

#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "TinyWireM.h"        // I2C library for ATtiny AVR
#include "nunchuck_funcs.h"   // Wii Nunchuck helper functions
#include <avr/pgmspace.h>

volatile uint32_t accumulator, phase, volume, vib_accumulator, vib_phase;
volatile uint16_t counter;
uint8_t outvalue; 

const int8_t wavetable[4][256] PROGMEM = {
  {
    127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 
    127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 
    127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 
    127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 
    127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 
    127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 
    127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 
    127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 127 , 
    -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , 
    -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , 
    -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , 
    -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , 
    -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , 
    -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , 
    -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , 
    -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 , -127 
  },
  {
    -127 , -125 , -123 , -121 , -119 , -117 , -115 , -113 , -111 , -109 , -107 , -105 , -103 , -101 , -99 , -97 , 
    -95 , -93 , -91 , -89 , -87 , -85 , -83 , -81 , -79 , -77 , -75 , -73 , -71 , -69 , -67 , -65 , 
    -63 , -61 , -59 , -57 , -55 , -53 , -51 , -49 , -47 , -45 , -43 , -41 , -39 , -37 , -35 , -33 , 
    -31 , -29 , -27 , -25 , -23 , -21 , -19 , -17 , -15 , -13 , -11 , -9 , -7 , -5 , -3 , -1 , 
    1 , 3 , 5 , 7 , 9 , 11 , 13 , 15 , 17 , 19 , 21 , 23 , 25 , 27 , 29 , 31 , 
    33 , 35 , 37 , 39 , 41 , 43 , 45 , 47 , 49 , 51 , 53 , 55 , 57 , 59 , 61 , 63 , 
    65 , 67 , 69 , 71 , 73 , 75 , 77 , 79 , 81 , 83 , 85 , 87 , 89 , 91 , 93 , 95 , 
    97 , 99 , 101 , 103 , 105 , 107 , 109 , 111 , 113 , 115 , 117 , 119 , 121 , 123 , 125 , 127 , 
    127 , 127 , 125 , 123 , 121 , 119 , 117 , 115 , 113 , 111 , 109 , 107 , 105 , 103 , 101 , 99 , 
    97 , 95 , 93 , 91 , 89 , 87 , 85 , 83 , 81 , 79 , 77 , 75 , 73 , 71 , 69 , 67 , 
    65 , 63 , 61 , 59 , 57 , 55 , 53 , 51 , 49 , 47 , 45 , 43 , 41 , 39 , 37 , 35 , 
    33 , 31 , 29 , 27 , 25 , 23 , 21 , 19 , 17 , 15 , 13 , 11 , 9 , 7 , 5 , 3 , 
    1 , -1 , -3 , -5 , -7 , -9 , -11 , -13 , -15 , -17 , -19 , -21 , -23 , -25 , -27 , -29 , 
    -31 , -33 , -35 , -37 , -39 , -41 , -43 , -45 , -47 , -49 , -51 , -53 , -55 , -57 , -59 , -61 , 
    -63 , -65 , -67 , -69 , -71 , -73 , -75 , -77 , -79 , -81 , -83 , -85 , -87 , -89 , -91 , -93 , 
    -95 , -97 , -99 , -101 , -103 , -105 , -107 , -109 , -111 , -113 , -115 , -117 , -119 , -121 , -123 , -125 
  },
  {
    -127 , -127 , -126 , -125 , -124 , -123 , -122 , -121 , -120 , -119 , -118 , -117 , -116 , -115 , -114 , -113 , 
    -112 , -111 , -110 , -109 , -108 , -107 , -106 , -105 , -104 , -103 , -102 , -101 , -100 , -99 , -98 , -97 , 
    -96 , -95 , -94 , -93 , -92 , -91 , -90 , -89 , -88 , -87 , -86 , -85 , -84 , -83 , -82 , -81 , 
    -80 , -79 , -78 , -77 , -76 , -75 , -74 , -73 , -72 , -71 , -70 , -69 , -68 , -67 , -66 , -65 , 
    -64 , -63 , -62 , -61 , -60 , -59 , -58 , -57 , -56 , -55 , -54 , -53 , -52 , -51 , -50 , -49 , 
    -48 , -47 , -46 , -45 , -44 , -43 , -42 , -41 , -40 , -39 , -38 , -37 , -36 , -35 , -34 , -33 , 
    -32 , -31 , -30 , -29 , -28 , -27 , -26 , -25 , -24 , -23 , -22 , -21 , -20 , -19 , -18 , -17 , 
    -16 , -15 , -14 , -13 , -12 , -11 , -10 , -9 , -8 , -7 , -6 , -5 , -4 , -3 , -2 , -1 , 
    0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 8 , 9 , 10 , 11 , 12 , 13 , 14 , 15 , 
    16 , 17 , 18 , 19 , 20 , 21 , 22 , 23 , 24 , 25 , 26 , 27 , 28 , 29 , 30 , 31 , 
    32 , 33 , 34 , 35 , 36 , 37 , 38 , 39 , 40 , 41 , 42 , 43 , 44 , 45 , 46 , 47 , 
    48 , 49 , 50 , 51 , 52 , 53 , 54 , 55 , 56 , 57 , 58 , 59 , 60 , 61 , 62 , 63 , 
    64 , 65 , 66 , 67 , 68 , 69 , 70 , 71 , 72 , 73 , 74 , 75 , 76 , 77 , 78 , 79 , 
    80 , 81 , 82 , 83 , 84 , 85 , 86 , 87 , 88 , 89 , 90 , 91 , 92 , 93 , 94 , 95 , 
    96 , 97 , 98 , 99 , 100 , 101 , 102 , 103 , 104 , 105 , 106 , 107 , 108 , 109 , 110 , 111 , 
    112 , 113 , 114 , 115 , 116 , 117 , 118 , 119 , 120 , 121 , 122 , 123 , 124 , 125 , 126 , 127 
  },
  {
    0 , 3 , 6 , 9 , 12 , 15 , 18 , 21 , 24 , 27 , 30 , 33 , 36 , 39 , 42 , 45 , 
    48 , 51 , 54 , 57 , 59 , 62 , 65 , 67 , 70 , 73 , 75 , 78 , 80 , 82 , 85 , 87 , 
    89 , 91 , 94 , 96 , 98 , 100 , 102 , 103 , 105 , 107 , 108 , 110 , 112 , 113 , 114 , 116 , 
    117 , 118 , 119 , 120 , 121 , 122 , 123 , 123 , 124 , 125 , 125 , 126 , 126 , 126 , 126 , 126 , 
    127 , 126 , 126 , 126 , 126 , 126 , 125 , 125 , 124 , 123 , 123 , 122 , 121 , 120 , 119 , 118 , 
    117 , 116 , 114 , 113 , 112 , 110 , 108 , 107 , 105 , 103 , 102 , 100 , 98 , 96 , 94 , 91 , 
    89 , 87 , 85 , 82 , 80 , 78 , 75 , 73 , 70 , 67 , 65 , 62 , 59 , 57 , 54 , 51 , 
    48 , 45 , 42 , 39 , 36 , 33 , 30 , 27 , 24 , 21 , 18 , 15 , 12 , 9 , 6 , 3 , 
    0 , -3 , -6 , -9 , -12 , -15 , -18 , -21 , -24 , -27 , -30 , -33 , -36 , -39 , -42 , -45 , 
    -48 , -51 , -54 , -57 , -59 , -62 , -65 , -67 , -70 , -73 , -75 , -78 , -80 , -82 , -85 , -87 , 
    -89 , -91 , -94 , -96 , -98 , -100 , -102 , -103 , -105 , -107 , -108 , -110 , -112 , -113 , -114 , -116 , 
    -117 , -118 , -119 , -120 , -121 , -122 , -123 , -123 , -124 , -125 , -125 , -126 , -126 , -126 , -126 , -126 , 
    -127 , -126 , -126 , -126 , -126 , -126 , -125 , -125 , -124 , -123 , -123 , -122 , -121 , -120 , -119 , -118 , 
    -117 , -116 , -114 , -113 , -112 , -110 , -108 , -107 , -105 , -103 , -102 , -100 , -98 , -96 , -94 , -91 , 
    -89 , -87 , -85 , -82 , -80 , -78 , -75 , -73 , -70 , -67 , -65 , -62 , -59 , -57 , -54 , -51 , 
    -48 , -45 , -42 , -39 , -36 , -33 , -30 , -27 , -24 , -21 , -18 , -15 , -12 , -9 , -6 , -3 
  }
};
#define SQUARE 0
#define TRIANGLE 1
#define SAWTOOTH 2
#define SINE 3

uint8_t ext_id[6];

#define BASENOTE 10955816L
#define CENTS_LUT_SIZE 13
#define CENTS_COMPRESS 10
uint32_t cents_lut[CENTS_LUT_SIZE] = {8197, 8201,8211,8230,8268,8345,8501,8821,9498,11011,14800,26739,87278};

int16_t atan2_int(int16_t y, int16_t x);
int16_t radius(int16_t y, int16_t x);
int32_t square_scale(int32_t x);

 
ISR (TIMER0_COMPA_vect){
    accumulator += phase;
    vib_accumulator += vib_phase;
    
    outvalue = volume*((accumulator >> 31) ? 1 : -1) + 127;
    
    OCR1B = outvalue;
    
    counter++;
}

int main(void){  
    PLLCSR |= _BV(PLLE);               // Enable 64 MHz PLL
    _delay_ms(10);            // Stabilize
    while(!(PLLCSR & _BV(PLOCK)));     // Wait for it...
    PLLCSR |= _BV(PCKE);               // Timer1 source = PLL

    // Set up Timer/Counter1 for PWM output
    TIMSK  = 0;                        // Timer interrupts OFF
    TCCR1  = _BV(CS10);                // 1:1 prescale
    GTCCR  = _BV(PWM1B) | _BV(COM1B1); // PWM B, clear on match
    OCR1C  = 255;                      // Full 8-bit PWM cycle
    OCR1B  = 127;                      // 50% duty at start

    DDRB = _BV(PORTB4);                // Enable PWM output pin
    
    // INTERRUPT SETUP
    TCCR0B = _BV(CS01); 	//CLK_IO/8
    TCCR0A = _BV(WGM01);	// clear on match
    OCR0A = 40; // Approximately 24kHz sampling rate
    TIMSK = (1<<OCIE0A);
    
    // init the DDS phase increment
    // for a 32-bit DDS accumulator, running at Fclock:
    // phase = 2^32*Fout/Fclock (where Fclock is the refresh rate)
    // phase = (long)(167503.724544*660.0);    
    int16_t vib_offset, cents_from_basenote;

    uint16_t angle; 
    int16_t accel_x, accel_y, accel_z, joy_x, joy_y, roll;

    do{
      _delay_ms(100);
      nunchuck_init(ext_id);
    } while(ext_id[2] != 0xA4);
    //if(ext_id[0] == 0 && ext_id[1] == 0 && ext_id[2] == 0xA4 && ext_id[3] == 0x20 && ext_id[4] == 0x01 && ext_id[5] == 0x01)
    {
      // Enable interrupts for sound generation;
      // do this after nunchuck init, otherwise sometimes things go funny (for timing reasons, I assume).
      sei();
      for(;;){
        vib_offset = ((int8_t)pgm_read_byte(&wavetable[joy_y > 0 ? SINE : SAWTOOTH][vib_accumulator >> 24])*square_scale(radius(joy_y,joy_x))) >> 7;
        // Counter replaces the 10ms delay to ensure nunchuck isn't polled too often
        if(counter > 250){
          if(nunchuck_get_data()){
            // TODO modify atan2 to use full 10-bit data
            accel_x = (nunchuck_accelx() >> 2) - 127;
            accel_y = (nunchuck_accely() >> 2) - 127;
            accel_z = (nunchuck_accelz() >> 2) - 127;
            joy_x = nunchuck_joyx() - 127;
            joy_y = nunchuck_joyy() - 127;
            angle = atan2_int(abs(joy_y), joy_x);
            // TODO adjust vibrato range
            vib_phase = 125000L + 7400L * square_scale(angle);

            // TODO make tapping the z button toggle lock-to-semitone
            if(nunchuck_zbutton() == 0){
              roll = atan2_int(accel_x, (accel_z < 0 ? -1 : 1) * (abs(accel_z) +abs(accel_y >> 3)));
            }
            
            // TODO adjust note range
            cents_from_basenote = 4*(roll + 512 + vib_offset);
            uint32_t temp_phase = BASENOTE;
            for(int i = 0; i < CENTS_LUT_SIZE; ++i){
              if(cents_from_basenote & 0x01){
                temp_phase >>= CENTS_COMPRESS;
                temp_phase *= (uint32_t)cents_lut[i];
                temp_phase >>= 3;
              }
              cents_from_basenote >>= 1;
            }
            phase = temp_phase;

            if(nunchuck_cbutton()){
              volume = 0;
            } else {
              volume = (255-75) - (accel_y + 127); // Reversed so that "down" is mute and "up" is loud
            }
          } else {
            volume = 0;
          }
          counter = 0; // Reset counter to ensure that nunchuck isn't polled too often
        }
      }
    };
    
    while(true){
      volume = 0;
    };
    
    return 0;   /* never reached */
}

int16_t atan2_int(int16_t y, int16_t x){
  /* 
  *  Returns an integer estimate of atan2 scaled to between -255 and 255
  *  TODO modify to accept and return full 10-bit data (or more generally, user-specified data size)
  */
  if(x == 0 && y == 0){
    return 0;
  }

  int16_t abs_y = abs(y);
  int16_t unscaled_angle;
  if(x >= 0){
    unscaled_angle = (1 << 8) - ((x-abs_y) << 8)/(x+abs_y);
  } else {
    unscaled_angle = 3*(1 << 8) - ((x+abs_y) << 8)/(-x+abs_y);
  }
  unscaled_angle >>= 2;
  return y < 0 ? -unscaled_angle : unscaled_angle;
}

int16_t radius(int16_t y, int16_t x){
  // Quick and dirty "radius" estimate
  return abs(x) + abs(y);
}

//TODO convert this to 16 bit data types without messing it all up
int32_t square_scale(int32_t x){
  return (x * x) >> 8;
}
