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

#ifndef DSETTINGS_H
#define DSETTINGS_H

#include "DuinoPort.h"

//notification
const bool default_mobile_enabled = false;
const bool default_email_enabled = false;

//temperature
const bool default_temperature_is_on = false;    
const int default_min_temperature = -50;
const int default_max_temperature = 100;

const bool default_ports_enabled = false;
//Normally opened(NO - true) or Normally closed(NC - false)
const bool default_ports_opened = false;

//default auth settings
const char* default_username = "user";
const char* default_password = "user";
const char* default_serial_number = "aa00";

//default event server settings
const char* default_server_domain = "www.notify.tom.ru";
const int default_server_port = 80;

//default network settings
const int default_port = 80;
const uint8_t default_mac     [6] = { 0xFC , 0xC2 , 0x3D , 0x4C , 0x4B , 0x40};
const uint8_t default_gateway [4] = { 192, 168, 10, 1 };
const uint8_t default_mask    [4] = { 255, 255, 255, 0 };
const uint8_t default_dns     [4] = {8,8,8,8};
const uint8_t default_ip      [4] = { 192, 168, 10, 113 };

//func for validate interval
template<typename T>
  static bool isInBounds(const T& value , const T& low  , 
                                 const T& high){
     if (value >= low && value <= high) return true;
     else return false;
}

class SerialNumber{
public:
  SerialNumber(){}
  static SerialNumber getDefaultSerialNumber () {
    SerialNumber serial_obj;
    String serial = generateSerial();
    char buff[30];
    
    serial.toCharArray(buff,30);
    memcpy(serial_obj.m_serial , buff , 30);
    
    return serial_obj;
}

String getData() const{
  String data (this->m_serial);
  return data;
}

bool isValid() const{
  if (m_serial[4] == '-' && m_serial[9] == '-' && m_serial[14] == '-' &&
                           m_serial[19] == '-' && m_serial[24] == '-')
  return true;
  else return false;                        
}

static String generateSerial(){
   String serial;
   while (serial.length() < 30){
    
      randomSeed(analogRead(0));
      long randNumber = random(100000);
      serial.concat(randNumber);
      delay(500);
     
   }
   serial = serial.substring(0,29);
   serial.setCharAt(4, '-');
   serial.setCharAt(9, '-');
   serial.setCharAt(14, '-');
   serial.setCharAt(19, '-');
   serial.setCharAt(24, '-');
   Serial.println(serial);
   return serial;
}

private:  
  char m_serial[30];
};

//Autorization for device username , password
class Autorization{
public:
  Autorization(){}
  
  Autorization(const char* username , const char* password){
    strcpy(m_username , username);
    strcpy(m_password , password);
  }
  void setUsername(char* username){
     strcpy(m_username , username);
  }
  void setPassword(char* password){
     strcpy(m_password , password);
  }
  static Autorization getDefaultAutorization(){
  return Autorization (default_username, default_password);
  }
  String getPassword() const{
    return String(m_password);
  }
  String getUsername() const{
    return String(m_username);
  }
private:
  char m_username [17];
  char m_password [17];
};

//network settings (ip,mac,dns,gateway,mask)
class Network{
public:  
  Network(){}
  static Network getDefaultNetwork()
  {
    Network network;
    network.m_port = default_port;
    memcpy(network.m_mac , default_mac , 6);
    memcpy(network.m_ip , default_ip , 4);
    memcpy(network.m_dns , default_dns , 4);
    memcpy(network.m_gateWay , default_gateway , 4);
    memcpy(network.m_mask , default_mask , 4);
    return network;
  }
  
  void setMac(char* mac){
    int buffSize = sizeof(m_mac);
    char* pch = strtok (mac , ":");
    unsigned int _index = 0 ; 
    while (pch != NULL)
    {
        if (_index >= buffSize) break;
        m_mac [_index] = strtol(pch, NULL, 16);
        pch = strtok (NULL, ":");
        _index++;
    }
  }

  void setIp(char* ip){
    splitToNumbers(ip,'.', m_ip , sizeof(m_ip));
  }

  void setPort(const int& port){
    m_port = port;
  }

  void setDns(char* dns){
    splitToNumbers(dns,'.', m_dns , sizeof(m_dns));
  }

  void setGateWay(char* gateWay){
    splitToNumbers(gateWay,'.', m_gateWay , sizeof(m_gateWay));
  }

  void setMask(char* mask){
    splitToNumbers(mask,'.', m_mask , sizeof(m_mask));
  }
  
  String getIp() const {
    String ip = String(m_ip[0],DEC) + '.' + String(m_ip[1],DEC) + '.' + 
                String(m_ip[2],DEC) + '.' + String(m_ip[3],DEC);
    return ip;
  }
  
String getMac() const {
    String mac = String(m_mac[0],HEX) + ':' + String(m_mac[1],HEX) + ':' + String(m_mac[2],HEX) + ':' +
                 String(m_mac[3],HEX) + ':' + String(m_mac[4],HEX) + ':' + String(m_mac[5],HEX);
    return mac;
  }

int getPort() const{
    return m_port;
  }

String getDns() const{
    String dns = String(m_dns[0],DEC) + '.' + String(m_dns[1],DEC) + '.' + 
                 String(m_dns[2],DEC) + '.' + String(m_dns[3],DEC);
    return dns;
}

String getGateWay() const{
    String gateWay = String(m_gateWay[0],DEC) + '.' + String(m_gateWay[1],DEC) + '.' + 
                     String(m_gateWay[2],DEC) + '.' + String(m_gateWay[3],DEC);
    return gateWay;
}

String getMask() const {
    String mask = String(m_mask[0],DEC) + '.' + String(m_mask[1],DEC) + '.' + 
                  String(m_mask[2],DEC) + '.' + String(m_mask[3],DEC);
    return mask;
}

const uint8_t*  macData() const{
    return  m_mac;
}

 const uint8_t*  ipData() const{
    return  m_ip;
} 

const uint8_t*  dnsData() const{
    return  m_dns;
} 

const uint8_t*  gateData() const{
    return  m_gateWay;
} 

const uint8_t*  maskData() const{
    return  m_mask;
} 

private:

  void splitToNumbers(const char* str, char separator, uint8_t* const buff , size_t buffSize){
    char* copy = strdup(str);
    char* pch = strtok (copy , &separator);
    unsigned int _index = 0 ; 
    while (pch != NULL)
    {
        if (_index >= buffSize) break;
        buff [_index] = atoi(pch);
        Serial.println(pch);
        Serial.println(buff [_index]);
        pch = strtok (NULL, &separator);
        _index++;
    }
    free(copy);
  }
  
  int     m_port;
  uint8_t m_mac     [6];
  uint8_t m_ip      [4];
  uint8_t m_dns     [4];
  uint8_t m_gateWay [4];
  uint8_t m_mask    [4];
};

//notification settings 
class Notification{
public:  
  Notification(){}
  static Notification getDefaultNotification(){
    Notification defaultNotification;
    memset(defaultNotification.m_email, 0, 48);
    defaultNotification.setEmailEnabled(default_email_enabled);
    defaultNotification.setMobileEnabled(default_mobile_enabled);
    return defaultNotification;
  }

  void setEmail(String email){
    strcpy(m_email , email.c_str());
  }
  
  void setMobileEnabled(const bool& enabled){
    m_mobileOn = enabled;
  }
  
  bool mobileEnabled()const{
    return m_mobileOn;
  }
  
  bool emailEnabled() const{
    return m_emailOn;
  }
  String getEmail()   const{
    
    return String(m_email);
  }

  void setEmailEnabled(const bool& enabled){
    m_emailOn = enabled;
 }
  
private:
  bool m_emailOn;
  char m_email[48];
  bool m_mobileOn;
};

class ServerNetwork{
public:  

  ServerNetwork(){
     m_port = default_server_port;
     memset(m_domain,0,40);
     strcpy(m_domain, default_server_domain);
   }
   
  static ServerNetwork getDefaultServerNetwork(){
    ServerNetwork serverNetwork;
    return serverNetwork;
  } 

  void setPort(const int& port){
    this->m_port = port;
  }

  int getPort() const{
    return this->m_port;
  }
  
  int m_port;
  char m_domain[40];
};

//temperature of sensor min , max
class ControlEvents{
public: 
  ControlEvents(){
  }
  
  static ControlEvents getDefaultControlEvents(){
    
    ControlEvents control;
    
    control.m_temperatureIsOn =  default_temperature_is_on;
    control.m_minTemperature  =  default_min_temperature;
    control.m_maxTemperature  =  default_max_temperature;
    
    return control;
  }

  void setMinTemperature(const int8_t& temp){
    m_minTemperature = temp;
  }

  void setMaxTemperature(const uint8_t& temp){
    m_maxTemperature = temp;
  }

  int8_t getMaxTemperature() const{
    return m_maxTemperature;
  }

  int8_t getMinTemperature() const{
    return m_minTemperature;
  }

  void setTemperatureEnabled(const bool& enabled){
    m_temperatureIsOn = enabled;
  }

  bool temperatureEnabled() const{
    return m_temperatureIsOn;
  }

private:  
  bool    m_temperatureIsOn;
  int8_t  m_minTemperature;
  uint8_t m_maxTemperature;
};
  
struct PortData{
  friend class PortEvents;
private:
  bool m_bEnabled;//port is enabled for event 
  bool m_bNO; // NC - normally closed (false) , NO - normally opened (true)
};

class PortEvents{
public:  
   PortEvents(){}
   static PortEvents getDefaultPortEvents(){
      PortEvents portEvents;
      for (int i = 0 ; i < event_port_count ; i++){
        portEvents.m_ports[i].m_bEnabled = default_ports_enabled;
        portEvents.m_ports[i].m_bNO      = default_ports_opened;
      }
      portEvents.m_outputPinState = 0;
      return portEvents;
   }
   
   void setEnabled(const DuinoPort::EventPort& port , const bool& enabled){
      int index = static_cast<int>(port);
      m_ports[index].m_bEnabled = enabled;
   }
   
   bool isEnabled(const DuinoPort::EventPort& port) const {
     int index = static_cast<int>(port);
     return m_ports[index].m_bEnabled;
   }
   
   void setOpened(const DuinoPort::EventPort& port , const bool& opened){
      int index = static_cast<int>(port);
      m_ports[index].m_bNO = opened;
   }

   bool isOpened (const DuinoPort::EventPort& port) const{
      int index = static_cast<int>(port);
      return m_ports[index].m_bNO;
   }

   void setOutputState(int value){//hight or low
    m_outputPinState = value;
   }

   int outputState(){
    return m_outputPinState;
   }
   
private:
   PortData m_ports[event_port_count];
   int m_outputPinState;
};

class DSettings{
  public:
  // enum of start address in memory 
  enum SettingType{
    SerialNumberType  = 0x00,
    NetworkType       = 0x64,
    AutorizationType  = 0xC8,
    NotificationType  = 0x12C,
    ControlEventsType = 0x190,
    PortEventsType    = 0x1F4
  };

  static DSettings* getInstance(){
    if(!m_pInstance){
      m_pInstance = new DSettings();
    }
    return m_pInstance;
  }

  // reset settings to default and save them
  void resetToDefault(){
    save(DSettings::AutorizationType, Autorization::getDefaultAutorization());
    save(DSettings::NotificationType , Notification::getDefaultNotification());
    save(DSettings::NetworkType ,Network::getDefaultNetwork());
    save(DSettings::ControlEventsType ,ControlEvents::getDefaultControlEvents());
    save(DSettings::SerialNumberType,SerialNumber::getDefaultSerialNumber());
    save(DSettings::PortEventsType,PortEvents::getDefaultPortEvents());
  }
  
  //read data from EEPROM
  template<typename T>
  void read (const  SettingType& type, T& data) const{
    EEPROM_read_mem(type , &data , sizeof(data));
  }
  
  //save data to EEPROM
  template<typename T>
  void  save(const  SettingType& type,  const T& data){
    EEPROM_write_mem(type,&data,sizeof(data));
  }
  
  private:
    DSettings(){}
    static DSettings* m_pInstance;
    
};
DSettings* DSettings::m_pInstance = NULL;

#endif // DSETTINGS_H

