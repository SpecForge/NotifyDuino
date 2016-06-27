/*
 * Free NotifyDuino project it’s simple way to get live message(GCM) 
 * from Arduino board to your mobile Android - anywhere & anytime!

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to deal
   in the Software without restriction, including without limitation the rights
   to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
   copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
   OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
   THE SOFTWARE.
*/

#ifndef DUINOPORT_H
#define DUINOPORT_H

#include <WString.h>
#include <Streaming.h>
#include <OneWire.h>

const int event_port_count = 4;

typedef struct{
  String m_sPortName; // pin name
  byte   m_iPin; //value of pin
} EventPortData;

class DuinoPort
{
public:
    enum  EventPort{
         Port_1 = 0x00,
         Port_2,
         Port_3,
         Port_4
    };
   DuinoPort();
   static bool   readDigital  (const EventPort& port); 
   static byte   getDigitalPin(const EventPort& port);
   static String getPortName  (const EventPort& port);
private:
    static const EventPortData     m_ports[event_port_count];
};
#endif // DUINOPORT_H

