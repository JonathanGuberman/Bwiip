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
#include <avr/eeprom.h>

volatile uint32_t accumulator, phase, vib_accumulator, vib_phase;
volatile uint16_t counter, env_accumulator, env_phase;
volatile uint8_t volume;
volatile bool is_pressed;

uint32_t lfsr = 0xAB5A55AA;
uint8_t bit0, bit1;
volatile int8_t lfsr_out;
bool noise_on;
#define BIT31 ((uint32_t)1 << 31)
#define BIT28 ((uint32_t)1 << 28)

uint8_t outvalue; 

#define TOTALWAVES 4
const int8_t wavetable[TOTALWAVES][256] PROGMEM = {
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
volatile uint8_t currentwave;
uint8_t vibwaveup;
uint8_t vibwavedown;

uint8_t EEMEM stored_currentwave = SQUARE;
uint8_t EEMEM stored_vibwaveup = SINE;
uint8_t EEMEM stored_vibwavedown = SAWTOOTH;

uint8_t ext_id[6];
const int8_t button_intervals[15] = {4, 7, 5, 0, 2, 1, 3, 11, 8, 9, 6, 12, 13, 10, -1};

#define SAMPLE_RATE 12000
#define COUNTER_OVERFLOW ((F_CPU/8)/SAMPLE_RATE)
#define PHASESCALE (((uint64_t)1 << 32)/(1000000.0/(COUNTER_OVERFLOW-1.0)))
#define BASENOTE (uint32_t)(PHASESCALE*65.4064 + 0.5)
#define VIB_BASE (uint32_t)(PHASESCALE*0.75 + 0.5)
#define VIB_SCALE (uint32_t)(PHASESCALE*0.044 + 0.5)
#define CENTS_LUT_SIZE 13
#define CENTS_COMPRESS 10
#define DEBOUNCE 5

// TODO calculate from sample rate and clock speed
uint32_t cents_lut[CENTS_LUT_SIZE] = {8197, 8201,8211,8230,8268,8345,8501,8821,9498,11011,14800,26739,87278};

// Function prototypes
int16_t atan2_int(int16_t y, int16_t x);
int16_t radius(int16_t y, int16_t x);
int32_t square_scale(int32_t x);
uint32_t phase_from_cents(int16_t cents);

 
ISR (TIMER0_COMPA_vect){
    accumulator += phase;
    vib_accumulator += vib_phase;
    env_accumulator += env_phase;
    
    //outvalue = volume*((accumulator >> 31) ? 1 : -1) + 127;
    outvalue = (volume*(noise_on ? lfsr_out : (int8_t)pgm_read_byte(&wavetable[currentwave][accumulator >> 24])) >> 8) + 127;
    
    OCR1B = outvalue;
    
    counter++;
    
    // implement the shift register
    lfsr = lfsr << 1 ;
    bit0 = (lfsr & BIT31) > 0;
    bit1 = (lfsr & BIT28) > 0;
    lfsr = lfsr + (bit0 ^ bit1);
    lfsr_out = (lfsr & 0xFF)-127;
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
    OCR0A = COUNTER_OVERFLOW; // Approximately 24kHz sampling rate
    TIMSK = (1<<OCIE0A);
    
    // init the DDS phase increment
    // for a 32-bit DDS accumulator, running at Fclock:
    // phase = 2^32*Fout/Fclock (where Fclock is the refresh rate)
    // phase = (long)(167503.724544*660.0);    
    int16_t vib_offset;

    uint16_t angle, last_byax, debounce_byax; 

    currentwave = eeprom_read_byte(&stored_currentwave);
    vibwaveup = eeprom_read_byte(&stored_vibwaveup);
    vibwavedown = eeprom_read_byte(&stored_vibwavedown);

    for(;;){
      cli();
      loop_until_bit_is_set(PINB,PB3);

      sei();
      do{
        _delay_ms(100);
        extension_init(ext_id);
      } while(ext_id[2] != 0xA4);
      //if(ext_id[0] == 0 && ext_id[1] == 0 && ext_id[2] == 0xA4 && ext_id[3] == 0x20 && ext_id[4] == 0x01 && ext_id[5] == 0x01)
      // Enable interrupts for sound generation;
      // do this after nunchuck init, otherwise sometimes things go funny (for timing reasons, I assume).
      //sei();

      while(bit_is_set(PINB,PB3)){
        int16_t accel_x, accel_y, accel_z, joy_x, joy_y, roll, env_subtract, env_add, pitch_bend;
        
        vib_offset = ((int8_t)pgm_read_byte(&wavetable[joy_y > 0 ? vibwaveup : vibwavedown][vib_accumulator >> 24])*square_scale(radius(joy_y,joy_x))) >> 7;
        
        if((env_accumulator & (1<<15))){
          env_accumulator = 0; 
          if(volume && !is_pressed){
            volume -= volume > env_subtract ? env_subtract : volume;
            if(pitch_bend) {phase = (15*phase)/16;}
          }
          if(env_add && is_pressed){
            volume = (volume + env_add) < 128 ? volume + env_add : 128;
          }
        }
        
            
        // Counter replaces the 10ms delay to ensure nunchuck isn't polled too often (10ms = 1/100s, hence the div by 100)
        if(counter > SAMPLE_RATE/100){
          counter = 0; // Reset counter to ensure that nunchuck isn't polled too often
          if(extension_get_data()){
            if(ext_id[2] == 0xA4 && ext_id[3] == 0x20 && ext_id[4] == 0x00 && ext_id[5] == 0x00)
            { // Nunchuck
              env_phase = 0;
              env_accumulator = 0;
              is_pressed = true;
              pitch_bend=0;
              noise_on = false;
              
              // TODO modify atan2 to use full 10-bit data
              accel_x = (nunchuck_accelx() >> 2) - 127;
              accel_y = (nunchuck_accely() >> 2) - 127;
              accel_z = (nunchuck_accelz() >> 2) - 127;
              joy_x = nunchuck_joyx() - 127;
              joy_y = nunchuck_joyy() - 127;
              angle = atan2_int(abs(joy_y), joy_x);
              // TODO adjust vibrato range, calculate from constants
              vib_phase = VIB_BASE + VIB_SCALE * square_scale(angle);

              // TODO make tapping the z button toggle lock-to-semitone
              if(nunchuck_zbutton() == 0){
                /* 
                 * Roll from three access accelerometer equation adapted from Freescale Semiconductors application
                 * note "Tilt Sensing Using a Three-Axis Accelerometer" (AN3461, rev 6), equation #38
                 * http://www.freescale.com/files/sensors/doc/app_note/AN3461.pdf
                 */
                roll = atan2_int(accel_x, (accel_z < 0 ? -1 : 1) * (abs(accel_z) +abs(accel_y >> 3)));
              }
            
              // TODO adjust note range
              phase = phase_from_cents(4*(roll + 512 + vib_offset));

              if(nunchuck_cbutton()){
                volume = 0;
              } else {
                volume = (255-75) - (accel_y + 127); // Reversed so that "down" is mute and "up" is loud
              }
            } else if (ext_id[2] == 0xA4 && ext_id[3] == 0x20 && ext_id[4] == 0x01 && ext_id[5] == 0x01)
            { //Classic or Pro
              noise_on = false;
              pitch_bend=0;
              env_phase = 322;
              uint8_t rtrig_temp = 31-extension_classic_rtrig_analogue();
              uint8_t ltrig_temp = 31-extension_classic_ltrig_analogue();
              env_subtract = ((rtrig_temp*rtrig_temp*rtrig_temp) >> 10) + 1;
              env_add = ((ltrig_temp*ltrig_temp*ltrig_temp) >> 10) + 1;
              
              uint8_t classic_byax = extension_classic_byax();
              uint8_t classic_dpad = extension_classic_dpad();
              if(classic_byax^last_byax){
                debounce_byax = 0;
                last_byax = classic_byax;
              } else {
                ++debounce_byax;
                if(debounce_byax > DEBOUNCE){
                  if(classic_byax){
                    //TODO Some way to keep vibrato going during the release phase
                    phase = phase_from_cents(2048 + 100*(button_intervals[classic_byax-1] - extension_classic_zl() + extension_classic_zr()) + vib_offset);
                    is_pressed = true;
                  } else {
                    is_pressed = false;
                  }
                }
              }
              if(classic_dpad){
                for(int dpadcount = 0; dpadcount < 4; ++dpadcount){
                  if(classic_dpad & 1){
                    if(extension_classic_plus()){
                      vibwaveup = dpadcount;
                    } else if (extension_classic_minus()){
                      vibwavedown = dpadcount;
                    } else {
                      currentwave = dpadcount;
                    }
                    break;
                  }
                  classic_dpad >>= 1;
                }
              }
              if(extension_classic_home()){
                eeprom_update_byte(&stored_currentwave, currentwave);
                eeprom_update_byte(&stored_vibwaveup, vibwaveup);
                eeprom_update_byte(&stored_vibwavedown, vibwavedown);
              }
              joy_x = (extension_classic_ljoyx() << 2) - 127;
              joy_y = (extension_classic_ljoyy() << 2) - 127;
              angle = atan2_int(abs(joy_y), joy_x);
              // TODO adjust vibrato range, calculate from constants
              vib_phase = VIB_BASE + VIB_SCALE * square_scale(angle);
            }   else if (ext_id[0] == 0x01 && ext_id[2] == 0xA4 && ext_id[3] == 0x20 && ext_id[4] == 0x01 && ext_id[5] == 0x03)
            { //Drums
              env_subtract = 16;
              env_add = 0;
              pitch_bend = true;
              is_pressed = false;
              
              uint8_t drums = extension_drums();
              if(drums){
                int whichdrum = 0;
                for(; whichdrum < 6; ++whichdrum){
                  if(drums & 1){
                    break;
                  }
                  drums >>= 1;
                }
                switch(whichdrum){
                  case DRUM_GREEN:
                  env_phase = 320;
                  noise_on = false;
                  phase = phase_from_cents(1200);
                  break;
                  case DRUM_BLUE:
                  env_phase = 320;
                  noise_on = false;
                  phase = phase_from_cents(2400);
                  break;
                  case DRUM_RED:
                  env_phase = 320;
                  noise_on = false;
                  phase = phase_from_cents(3600);
                  break;
                  case DRUM_ORANGE:
                  env_phase = 50;
                  noise_on = true;
                  break;
                  case DRUM_YELLOW:
                  env_phase = 600;
                  noise_on = true;
                  break;
                }
                volume = 255;
              }
            }
          }
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

uint32_t phase_from_cents(int16_t cents){
  uint32_t temp_phase = BASENOTE;
  for(int i = 0; i < CENTS_LUT_SIZE; ++i){
    if(cents & 0x01){
      temp_phase >>= CENTS_COMPRESS;
      temp_phase *= (uint32_t)cents_lut[i];
      temp_phase >>= 3;
    }
    cents >>= 1;
  }
  return temp_phase;
}
