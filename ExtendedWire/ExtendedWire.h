//============================================================================
//Weather Station Data Logger : Weather Shield for Arduino
//Revised by Weber Anderson, August 2010
// 
//This application is free software; you can redistribute it and/or
//modify it under the terms of the GNU Lesser General Public
//License as published by the Free Software Foundation; either
//version 3 of the License, or (at your option) any later version.
//
//This application is distributed in the hope that it will be useful,
//but WITHOUT ANY WARRANTY; without even the implied warranty of
//MERCHANTABILITY or FITNESS FOR When PARTICULAR PURPOSE. See the GNU
//Lesser General Public License for more details.
//
//You should have received a copy of the GNU Lesser General Public
//License along with this library; if not, see <http://www.gnu.org/licenses/>
//
//=============================================================================
// this library has been enhanced for the needs of 
// the weather shield project for WSDL.
//
/*
  TwoWire.h - TWI/I2C library for Arduino & Wiring
  Copyright (c) 2006 Nicholas Zambetti.  All right reserved.
  Revised 31 May 2009 Christopher K. Johnson.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef ExtendedWire_h
#define ExtendedWire_h

#include "Arduino.h"
#include <inttypes.h>

#define BUFFER_LENGTH 32

#define STANDARD 100000L
#define FAST 400000L

#define WIRE_STATUS_BUSY          0
#define WIRE_STATUS_SUCCESS       1
#define WIRE_STATUS_ADDR_NACK     2
#define WIRE_STATUS_DATA_NACK     3
#define WIRE_STATUS_OTHER_ERROR   4
#define WIRE_STATUS_BAD_ARGS      5

class ExtendedWire
{
  private:
    static uint8_t* rxBuffer;
    static uint8_t rxBufferIndex;
    static uint8_t rxBufferLength;
    static uint8_t asyncRxActive;

    static uint8_t txAddress;
    static uint8_t* txBuffer;
    static uint8_t txBufferIndex;
    static uint8_t txBufferLength;

    static uint8_t transmitting;
    static void (*user_onRequest)(void);
    static void (*user_onReceive)(int);
    static void onRequestService(void);
    static void onReceiveService(uint8_t*, int);
    
    void asyncBufferUpdate();
  public:
    ExtendedWire();
    void begin();
    void begin(uint8_t);
    void begin(int);
    void enablePullupResistors(uint8_t);
    long unsigned int setSpeed(long unsigned int);
    void beginTransmission(uint8_t);
    void beginTransmission(int);
    void beginTransmissionAt(uint8_t,uint8_t);
    void beginTransmissionAt(int,int);
    void beginTransmissionAt2(uint8_t,int);
    void beginTransmissionAt2(int,int);

    uint8_t endTransmission();
    uint8_t kickoffTransmission();

    uint8_t requestFrom(uint8_t, uint8_t);
    uint8_t requestFrom(int, int);
    uint8_t requestFromAt(uint8_t, uint8_t, uint8_t);
    uint8_t requestFromAt(int, int, int);
    uint8_t requestFromAt2(uint8_t, int, uint8_t);
    uint8_t requestFromAt2(int, int, int);

    uint8_t kickoffRequestFrom(uint8_t, uint8_t);
    uint8_t kickoffRequestFrom(int, int);
    uint8_t kickoffRequestFromAt(uint8_t, uint8_t, uint8_t);
    uint8_t kickoffRequestFromAt(int, int, int);
    uint8_t kickoffRequestFromAt2(uint8_t, int, uint8_t);
    uint8_t kickoffRequestFromAt2(int, int, int);

    uint8_t status();

    void send(uint8_t);
    void send(uint8_t*, uint8_t);
    void send(int);
    void send(char*);
    uint8_t available(void);
    uint8_t receive(void);
    void onReceive( void (*)(int) );
    void onRequest( void (*)(void) );

    // compatibiilty with standard Wire library
    inline void write(uint8_t b) { send(b); }
    inline uint8_t read() { return receive(); }
};

extern ExtendedWire Wire;

#endif

