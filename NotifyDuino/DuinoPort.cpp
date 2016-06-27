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

#include "DuinoPort.h"

//port name and port pin number
//to add a port add his name and pin value 
const EventPortData DuinoPort::m_ports[] = {{"Digital pin 18" , 18}, 
                                            {"Digital pin 19" , 19},
                                            {"Digital pin 20" , 20}, 
                                            {"Digital pin 21" , 21}};

DuinoPort::DuinoPort(){}

String DuinoPort::getPortName(const DuinoPort::EventPort &port)
{
    String portName;
    
    switch (port){
      case Port_1:
        portName = m_ports[0].m_sPortName;
        break;
        
       case Port_2:
        portName = m_ports[1].m_sPortName;
        break;
       
       case Port_3:
        portName = m_ports[2].m_sPortName;
        break;
        
       case Port_4:
        portName = m_ports[3].m_sPortName;
        break;
        
    }
    return portName;
}

// return pin number
byte DuinoPort::getDigitalPin(const DuinoPort::EventPort &port){
    byte portPin = 0x00;
    
    switch (port)
    {
       case Port_1:
        portPin = m_ports[0].m_iPin;
        break;
        
       case Port_2:
        portPin = m_ports[1].m_iPin;
        break;
       
       case Port_3:
        portPin = m_ports[2].m_iPin;
        break;
        
       case Port_4:
        portPin = m_ports[3].m_iPin;
        break;        
    }
    return portPin;
}

//read value in pin 
bool DuinoPort::readDigital(const EventPort& port){
  int pin = getDigitalPin(port);
  int pinValue =  digitalRead(pin);
  return (pinValue == 0) ? false : true;
}

