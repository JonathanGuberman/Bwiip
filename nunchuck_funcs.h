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


static uint8_t nunchuck_buf[6];   // array to store nunchuck data,

// initialize the I2C system, join the I2C bus,
// and tell the nunchuck we're talking to it
static void nunchuck_init(uint8_t ext_id[])
{ 
    TinyWireM.begin();                // join i2c bus as master
    TinyWireM.beginTransmission(0x52);// transmit to device 0x52
    TinyWireM.send((uint8_t)0xF0);// sends memory address
    TinyWireM.send((uint8_t)0x55);// sends data
    TinyWireM.send((uint8_t)0xFB);// sends memory address
    TinyWireM.send((uint8_t)0x00);// sends data
    TinyWireM.endTransmission();// stop transmitting
    
    _delay_ms(10);
    TinyWireM.beginTransmission(0x52);
    TinyWireM.send((uint8_t)0xFA);                    // extension type register
    TinyWireM.endTransmission();
    _delay_ms(10);
    
    TinyWireM.beginTransmission(0x52);
    TinyWireM.requestFrom((uint8_t)0x52, 6);               // request data from controller
    for (int cnt = 0; cnt < 6; cnt++) {
        if (TinyWireM.available()) {
            ext_id[cnt] = TinyWireM.receive(); // Should be 0x0000 A420 0101 for Classic Controller, 0x0000 A420 0000 for nunchuck
        }
    }
    TinyWireM.endTransmission();
}

// Send a request for data to the nunchuck
// was "send_zero()"
static void nunchuck_send_request()
{
    TinyWireM.beginTransmission(0x52);// transmit to device 0x52
    TinyWireM.send((uint8_t)0x00);// sends one byte
    TinyWireM.endTransmission();// stop transmitting
}

// Receive data back from the nunchuck, 
// returns 1 on successful read. returns 0 on failure
static int nunchuck_get_data()
{
    int cnt=0;
    TinyWireM.requestFrom (0x52, 6);// request data from nunchuck
    while (TinyWireM.available ()) {
        // receive byte as an integer
        nunchuck_buf[cnt] = TinyWireM.receive();
        cnt++;
    }
    nunchuck_send_request();  // send request for next data payload
    // If we recieved the 6 bytes, then go print them
    if (cnt >= 5) {
        return 1;   // success
    }
    return 0; //failure
}


// returns zbutton state: 1=pressed, 0=notpressed
static int nunchuck_zbutton()
{
    return ((nunchuck_buf[5] >> 0) & 1) ? 0 : 1;  // voodoo
}

// returns zbutton state: 1=pressed, 0=notpressed
static int nunchuck_cbutton()
{
    return ((nunchuck_buf[5] >> 1) & 1) ? 0 : 1;  // voodoo
}

// returns value of x-axis joystick
static int nunchuck_joyx()
{
    return nunchuck_buf[0]; 
}

// returns value of y-axis joystick
static int nunchuck_joyy()
{
    return nunchuck_buf[1];
}

// returns value of x-axis accelerometer
static int nunchuck_accelx()
{
    return nunchuck_buf[2];   // FIXME: this leaves out 2-bits of the data
}

// returns value of y-axis accelerometer
static int nunchuck_accely()
{
    return nunchuck_buf[3];   // FIXME: this leaves out 2-bits of the data
}

// returns value of z-axis accelerometer
static int nunchuck_accelz()
{
    return nunchuck_buf[4];   // FIXME: this leaves out 2-bits of the data
}
