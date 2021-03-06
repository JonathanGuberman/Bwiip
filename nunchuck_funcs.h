/*
 * Nunchuck functions  -- Talk to a Wii Nunchuck
 * 
 * Adapted to work with TinyWire and ATtiny85 by Arnaud Boudou
 *
 * This library is from the Bionic Arduino course : 
 *                          http://todbot.com/blog/bionicarduino/
 *
 * 2007-11 Tod E. Kurt, http://todbot.com/blog/
 *
 * The Wii Nunchuck reading code originally from Windmeadow Labs
 *   http://www.windmeadow.com/node/42
 */

#include "TinyWireM.h"
#include <util/delay.h>

#define WII_I2C_ADDR 0x52

#define DRUM_BASS 0
#define DRUM_BLUE 1
#define DRUM_GREEN 2
#define DRUM_YELLOW 3
#define DRUM_RED 4
#define DRUM_ORANGE 5

static uint8_t nunchuck_buf[6];   // array to store nunchuck data,

// initialize the I2C system, join the I2C bus,
// and tell the nunchuck we're talking to it
static void extension_init(uint8_t ext_id[])
{
    // Initialize device to send unencrypted data
    // See http://wiibrew.org/wiki/Wiimote/Extension_Controllers#The_New_Way for more information
     
    TinyWireM.begin();                // join i2c bus as master
    TinyWireM.beginTransmission(WII_I2C_ADDR);// transmit to device 0x52
    TinyWireM.send((uint8_t)0xF0);// sends memory address
    TinyWireM.send((uint8_t)0x55);// sends data
    TinyWireM.send((uint8_t)0xFB);// sends memory address
    TinyWireM.send((uint8_t)0x00);// sends data
    TinyWireM.endTransmission();// stop transmitting
    
    // Request the extension type code
    // The delay before and after appear to be necessary, for unknown reasons
    _delay_ms(10);
    TinyWireM.beginTransmission(WII_I2C_ADDR);
    TinyWireM.send((uint8_t)0xFA);
    TinyWireM.endTransmission();
    _delay_ms(10);
    
    TinyWireM.beginTransmission(WII_I2C_ADDR);
    TinyWireM.requestFrom((uint8_t)WII_I2C_ADDR, 6);
    for (int cnt = 0; cnt < 6; cnt++) {
        if (TinyWireM.available()) {
            ext_id[cnt] = TinyWireM.receive();
        }
    }
    TinyWireM.endTransmission();
}

// Send a request for data to the nunchuck
// was "send_zero()"
static void extension_send_request()
{
    TinyWireM.beginTransmission(WII_I2C_ADDR);// transmit to device 0x52
    TinyWireM.send((uint8_t)0x00);// sends one byte
    TinyWireM.endTransmission();// stop transmitting
}

// Receive data back from the nunchuck, 
// returns 1 on successful read. returns 0 on failure
static uint8_t extension_get_data()
{
    uint8_t cnt=0;
    TinyWireM.requestFrom (WII_I2C_ADDR, 6);// request data from nunchuck
    while (TinyWireM.available ()) {
        // receive byte as an integer
        nunchuck_buf[cnt] = TinyWireM.receive();
        cnt++;
    }
    extension_send_request();  // send request for next data payload
    if (cnt >= 5) {
        for (int i = 0; i < 6; i++){
            if (nunchuck_buf[i] > 0 && nunchuck_buf[i] < 255){
                return 1; //success
            }
        }
    }
    return 0; //failure
}


// returns zbutton state: 1=pressed, 0=notpressed
static uint8_t nunchuck_zbutton()
{
    return ((nunchuck_buf[5] >> 0) & 1) ? 0 : 1;  // voodoo
}

// returns zbutton state: 1=pressed, 0=notpressed
static uint8_t nunchuck_cbutton()
{
    return ((nunchuck_buf[5] >> 1) & 1) ? 0 : 1;  // voodoo
}

// returns value of x-axis joystick
static uint8_t nunchuck_joyx()
{
    return nunchuck_buf[0]; 
}

// returns value of y-axis joystick
static uint8_t nunchuck_joyy()
{
    return nunchuck_buf[1];
}

// returns value of x-axis accelerometer
static uint16_t nunchuck_accelx()
{
    return (nunchuck_buf[2] << 2) | ((nunchuck_buf[5] >> 2) & 3);
}

// returns value of y-axis accelerometer
static uint16_t nunchuck_accely()
{
    return (nunchuck_buf[3] << 2) | ((nunchuck_buf[5] >> 4) & 3);
}

// returns value of z-axis accelerometer
static uint16_t nunchuck_accelz()
{
    return (nunchuck_buf[4] << 2) | ((nunchuck_buf[5] >> 6) & 3);
}

static uint8_t extension_classic_byax()
{
    return ((~nunchuck_buf[5]) >> 3) & 0xF;
}

static uint8_t extension_classic_dpad()
{
    return (((~nunchuck_buf[4] >> 4) & 0xC) | (~nunchuck_buf[5] & 0x3)) & 0xF;
}

static uint8_t extension_classic_ljoyx()
{
    return nunchuck_buf[0] & 0x3F;
}

static uint8_t extension_classic_ljoyy()
{
    return nunchuck_buf[1] & 0x3F;
}

static uint8_t extension_classic_rjoyx()
{
    return ((nunchuck_buf[0] & 0xC0) >> 3) | ((nunchuck_buf[1] & 0xC0) >> 5) | (nunchuck_buf[2] >> 7);
}

static uint8_t extension_classic_rjoyy()
{
    return nunchuck_buf[2] & 0x1F;
}

static uint8_t extension_classic_rtrig_analogue(){
    return nunchuck_buf[3] & 0x1F;
}

static uint8_t extension_classic_ltrig_analogue(){
    return (((nunchuck_buf[2] & 0x60) >> 2) | (nunchuck_buf[3] >> 5) ) & 0x1F;
}

static uint8_t extension_classic_zl()
{
    return ((~nunchuck_buf[5]) >> 7);
}

static uint8_t extension_classic_zr()
{
    return ((~nunchuck_buf[5]) >> 2) & 1;
}

static uint8_t extension_classic_plus()
{
    return ((~nunchuck_buf[4]) >> 2) & 1;
}
static uint8_t extension_classic_minus()
{
    return ((~nunchuck_buf[4]) >> 4) & 1;
}
static uint8_t extension_classic_home()
{
    return ((~nunchuck_buf[4]) >> 3) & 1;
}

static uint8_t extension_drums()
{
    return ((~nunchuck_buf[5]) >> 2) & 0x3F;
}