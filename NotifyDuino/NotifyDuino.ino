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

#define UIP_ATTEMPTS_ON_WRITE 2
#define WEBDUINO_SE), INPUT_PULLUP);#define ENC28J60DEBUG 10
#define WEBDUINO_AUTH_REALM "SpecForge"
#define WEBDUINO_FAVICON_DATA ""
#include <UIPEthernet.h>
#include "WebServer.h"
#include "EEPROM2.h"
#include <Base64.h>
#include <Streaming.h>
#include <OneWire.h>
#include "DSettings.h"
#include "AES.h"
#include "ServerApi.h"
#include "DuinoPort.h"
#include <avr/interrupt.h>
#include <DallasTemperature.h>

int portValues[event_port_count] = {1,1,1,1};

// Messages for events
const String event1Message = "Pin 18 changed ...";
const String event2Message = "Pin 19 changed ...";
const String event3Message = "Pin 20 changed ...";
const String event4Message = "Pin 21 changed ...";

const String tempEvent1Message = "Temperature out of range !!!";

// Temperature and output pins
const int TempSensor1Pin = A0;
const int TempSensor2Pin = A1;
const int outputPin = 7;

// Objects of setting data
DSettings* settings = DSettings::getInstance();
Autorization autorization;
ServerNetwork serverNetwork;
Network network;
Notification notification;
ControlEvents controlEvents;
String serialNumber;
PortEvents portEvents;

// Flag which  describes is event generated 
volatile bool event1 = false;
volatile bool event2 = false;
volatile bool event3 = false;
volatile bool event4 = false;

ServerApi* api = NULL;

void(* resetFunc) (void) = 0;

// Form types of post request
enum FormType{
  UnknowForm = 0x00,
  TemperatureForm,
  NotificationForm,
  NetworkForm,
  AutorizationForm,
  PortEventsForm
};

#define PREFIX ""
// WebServer initialization
WebServer webserver(PREFIX, 80);

OneWire  OWTempSensor1(TempSensor1Pin);
OneWire  OWTempSensor2(TempSensor2Pin);
DallasTemperature TempSensor1(&OWTempSensor1);
DallasTemperature TempSensor2(&OWTempSensor2);
// Start temperature value
static const float initTemp = -200;
float TempC1 = initTemp;
float TempC2 = initTemp;
unsigned long temp_timer_start, temp_timer_stop;

// UrlPathCommand contains pointers to the seperate parts of the URL path where '/' was used as the delimiter.
void UrlPathCommand(WebServer &server, WebServer::ConnectionType type,
                              char **url_path, char *url_tail, bool tail_complete)
{
  String auth = autorization.getUsername() + ':' + autorization.getPassword();
  char   input[auth.length()];
  auth.toCharArray(input,auth.length()+1);
  int inputLen = sizeof(input);
  int encodedLen = base64_enc_len(inputLen);
  char encoded[encodedLen];
 
  base64_encode(encoded, input, inputLen); 
  
  if (server.checkCredentials(encoded))
  {
    if (url_path[0] == 0 || url_path[1] != 0) return;                             
    String path (url_path[0]);
    String tail(url_tail);   
    if (path == String("api"))
    {
        int index =  tail.indexOf('=');
        if (index == -1 ) return;
        String getParamName = tail.substring(0,index);
        
        if (tail.length() < index+1) return;
        
        String value = tail.substring(index + 1,tail.length());
        if (getParamName == "type")
        {
          if (value == serialNumber){
              server << F("HTTP/1.1 200 OK");
              server << F("\n");
              server << F("Content-Type: application/json;charset=utf-8");
              server << F("\n\n");
              server << F("{\"status\":\"successfully\",");
              if (TempC1 != initTemp){
                  server << F("\"tmp1\":")<<F("\"")<<TempC1<<F("\",");
              }
              if (TempC2 != initTemp){
                  server << F("\"tmp2\":")<<F("\"")<<TempC2<<F("\",");
              }
              server << F("\"e_temp_is_on\":")<<F("\"")<<controlEvents.temperatureEnabled()<<F("\",");
              server << F("\"e_temp_min\":")<<F("\"")<<controlEvents.getMinTemperature()<<F("\",");
              server << F("\"e_temp_max\":")<<F("\"")<<controlEvents.getMaxTemperature()<<F("\",");
                       
              server << F("\"web_login\":") <<'\"'<< autorization.getUsername()<<F("\",");; 
              server << F("\"web_pass\":") <<'\"'<< autorization.getPassword() <<F("\",");
              server << F("\"web_port\":") <<'\"'<<network.getPort()<<F("\",");
              server << F("\"web_ip\":") <<'\"'<<network.getIp()<<F("\",");
              server << F("\"web_mac\":") <<'\"'<< network.getMac()<<F("\",");
              server << F("\"web_host\":")<<F("\"")<<network.getGateWay()<<F("\",");
              server << F("\"web_mask\":")<<F("\"")<<network.getMask()<<F("\",");
              server << F("\"web_dns\":")<<F("\"")<<network.getDns()<<F("\",");

              for (int i = 0 ; i < event_port_count ; i++ ){
                  DuinoPort::EventPort port = static_cast<DuinoPort::EventPort>(i);
                  server << F("\"e_port")<<i+1<<F("_on\":")<<F("\"")<<portEvents.isEnabled(port)<<F("\",");
                  server << F("\"e_port")<<i+1<<F("_opened\":")<<F("\"")<<portEvents.isOpened(port)<<F("\",");
                  server << F("\"e_port")<<i+1<<F("_value\":")<<F("\"")<<portValues[i]<<F("\",");
              }
              server << F("\"web_output\":")<<F("\"")<<portEvents.outputState()<<F("\",");
              server << F("\"emailIsOn\":")<<F("\"")<<notification.emailEnabled()<<F("\",");
              server << F("\"web_email\":")<<F("\"")<<notification.getEmail()<<F("\",");
              server << F("\"mobileIsOn\":")<<F("\"")<<notification.mobileEnabled()<<F("\"");
              server <<F("}"); 
           }

//if value == "serial" send serial number of device in json
           else if (value == "serial"){
              server << F("HTTP/1.1 200 OK");
              server << F("\n");
              server << F("Content-Type: application/json;charset=utf-8");
              server << F("\n\n");
              server << F("{\"status\":\"successfully\" , \"serial\":\"")<< serialNumber << F("\" }");
           }

//else send json with status failed
           else {
              server << F("HTTP/1.1 200 OK");
              server << F("\n");
              server << F("Content-Type: application/json;charset=utf-8");
              server << F("\n\n");
              server << F("{\"status\":\"failed\" , \"error_message\":\"Invalid serial number !!!\" }");
           }
        }
   }
  }
  else {
    // output headers and a message indicating "401 Unauthorized"
    server.httpUnauthorized();
  }
}

// main handler for responce of WebServer
void WebCmd(WebServer &server, WebServer::ConnectionType type, char * data, bool)
{
  String auth = autorization.getUsername() + ':' + autorization.getPassword();
  char   input[auth.length()];
  auth.toCharArray(input,auth.length()+1);
  int inputLen = sizeof(input);
  int encodedLen = base64_enc_len(inputLen);
  char encoded[encodedLen];
  bool mobile_app = false;
 
  base64_encode(encoded, input, inputLen); 

  //check authorization
  if (server.checkCredentials(encoded))
  {
    if (type == WebServer::POST)
    {
      bool repeat;
      FormType formType = UnknowForm;
      char name[40], value[40];
      do
      {
        /* readPOSTparam returns false when there are no more parameters
         * to read from the input.  We pass in buffers for it to store
         * the name and value strings along with the length of those
         * buffers. */
        repeat = server.readPOSTparam(name, 40, value, 40);
        
        if (strcmp_P(name, PSTR("web_login")) == 0)
        {
          autorization.setUsername(value);  
        }
        else if (strcmp_P(name, PSTR("web_pass")) == 0)
        { 
          formType = AutorizationForm;
          autorization.setPassword(value);
        }
        else if (strcmp_P(name, PSTR("web_port")) == 0)
        {
           network.setPort(atoi(value));
           formType = NetworkForm;  
        }
        else if (strcmp_P(name, PSTR("web_ip")) == 0)
        {
          network.setIp(value); 
        }
        else if (strcmp_P(name, PSTR("web_mac")) == 0)
        {
          network.setMac(value); 
        }
        else if (strcmp_P(name, PSTR("web_host")) == 0)
        {
          network.setGateWay(value);   
        }
        else if (strcmp_P(name, PSTR("web_mask")) == 0)
        {
          network.setMask(value);     
        }
        else if (strcmp_P(name, PSTR("web_dns")) == 0)
        {
          network.setDns(value);    
        }
        else if (strcmp_P(name, PSTR("web_reboot")) == 0)
        {
          resetFunc(); // reboot board
        }
        else if (strcmp_P(name, PSTR("web_setoutpin")) == 0)
        {
          int outpin_value = atoi(value);
          portEvents.setOutputState(outpin_value);
          formType = PortEventsForm;
        }
        else if (strcmp_P(name, PSTR("web_email")) == 0)
        {
          String email(value);
          notification.setEmail(email);
        }
        else if (strcmp_P(name, PSTR("emailIsOn")) == 0)
        {
          notification.setEmailEnabled(atoi(value));
          formType =  NotificationForm;  
        }
        else if (strcmp_P(name, PSTR("mobileIsOn")) == 0)
        {
          notification.setMobileEnabled(atoi(value));  
        }
        else if (strcmp_P(name, PSTR("temperature_warning")) == 0)
        {
          controlEvents.setTemperatureEnabled (atoi(value));  
          settings->save(DSettings::ControlEventsType,controlEvents);   
        }
        else if (strcmp_P(name, PSTR("min_temperature_lvl")) == 0)
        {
          controlEvents.setMinTemperature(atoi(value));  
          settings->save(DSettings::ControlEventsType,controlEvents);   
        } 
        else if (strcmp_P(name, PSTR("max_temperature_lvl")) == 0)
        {
          controlEvents.setMaxTemperature(atoi(value));  
          settings->save(DSettings::ControlEventsType,controlEvents);   
        }  
        else if (strcmp_P(name, PSTR("e_port1_on")) == 0)
        {
          portEvents.setEnabled( DuinoPort::Port_1 , atoi(value) );  
        }
        else if (strcmp_P(name, PSTR("e_port2_on")) == 0)
        {
          portEvents.setEnabled( DuinoPort::Port_2 , atoi(value) );    
        }
        else if (strcmp_P(name, PSTR("e_port3_on")) == 0)
        {
          portEvents.setEnabled( DuinoPort::Port_3 , atoi(value) ); 
        }
        else if (strcmp_P(name, PSTR("e_port4_on")) == 0)
        {
          portEvents.setEnabled( DuinoPort::Port_4 , atoi(value) );
          formType = PortEventsForm;
        }
        else if (strcmp_P(name, PSTR("e_port1_opened")) == 0)
        {
          portEvents.setOpened( DuinoPort::Port_1 , atoi(value) );  
        }
        else if (strcmp_P(name, PSTR("e_port2_opened")) == 0)
        {
          portEvents.setOpened( DuinoPort::Port_2 , atoi(value) );  
        }
        else if (strcmp_P(name, PSTR("e_port3_opened")) == 0)
        {
          portEvents.setOpened( DuinoPort::Port_3 , atoi(value) ); 
        }
         else if (strcmp_P(name, PSTR("e_port4_opened")) == 0)
        {
          portEvents.setOpened( DuinoPort::Port_4 , atoi(value) ); 
        }
        else if (strcmp_P(name, PSTR("mobile_app")) == 0)
        {
          mobile_app = true;
        }
      } while (repeat);

      if (mobile_app)
      {
        server.httpSuccess();
      } else
      {
        server.httpSeeOther("/");
      }

      switch (formType){
            case UnknowForm:
                  break;
            case AutorizationForm:{
              settings->save(DSettings::AutorizationType , autorization);  
              break;
            }  
            case NetworkForm:{
              settings->save(DSettings::NetworkType , network);
              webserver.reset();
              webserver.setPort(network.getPort());
              Ethernet.begin(network.macData() , network.ipData() , network.dnsData() , network.gateData() , network.maskData());
              webserver.begin();
              break;
            }    
            case NotificationForm:{
              settings->save(DSettings::NotificationType , notification); 
              break;
            }   
            case PortEventsForm:{
              settings->save(DSettings::PortEventsType , portEvents); 
              SetOutPin(portEvents.outputState());
              break;
            } 
      }
      // after procesing the POST data, tell the web browser to reload
      // the page using a GET method. 
      return;
    }
    
    /* for a GET or HEAD, send the standard "it's all OK headers" */
    server.httpSuccess();
  
    /* we don't output the body for a HEAD request */    
    if (type == WebServer::GET)
    {
      server << F("<html lang='ru'><title>SpecForge</title><head>");
      //style 
      server << F("<style>");
      server << F("body{height:100%;}");
      server << F(".page_content{height:100%;width:480px;margin:auto;}");
      server << F("h2{font-size:15px;}");
      server << F(".b_content{display:flex;flex-direction:row;justify-content:space-between;}");
      server << F(".b_content.status {border:2px;border-style: solid;border-color: gray;flex:}");
      server << F(".b_content.notification {border:2px;border-style: solid;border-color: gray;margin-top:7px;}");
      server << F(".page_content p{padding:3px; margin:0}");
      server << F(".b_content.header h1{font-size:18px;padding:3px; margin:0}");
      server << F(".controll_row{display:flex;flex-direction:row;}");
      server << F(".separator{height:10px;}");
      //->tabwid style
      server << F(".tabs {position: relative;clear: both;height:500px; margin: 25px 0;}");
      server << F(".tab { float: left;}");
      server << F(".tab label {background: #eee; padding: 10px; border: 1px solid #ccc; margin-left: -1px; position: relative;left: 1px;}");
      server << F(".tab [type=radio] {display: none;}");
      server << F(".content {height:600px;display:flex;flex-direction:column;position: absolute;top: 28px;left: 0;background: white;right: 0;bottom: 0;padding: 20px ; border: 1px solid #ccc;}");
      server << F(" [type=radio]:checked ~ label {background: white; border-bottom: 1px solid white;z-index: 2;}");
      server << F("[type=radio]:checked ~ label ~ .content {z-index: 1;}");
      server << F("</style>");
   
      //shortcut icon 
      server << F("<link rel='shortcut icon' type='image/x-icon' href='");
      server << F("data:image/x-icon;base64,AAABAAEAFBAAAAEAIABoBQAAFgAAACgAAAAUAAAAIAAAAAEAIAAAAAAAAAUAAIy4AACMuAAAAAAAAAAAAAA1JA4xlmgp17uBM/+5fzL/uX8y/7l/Mv+5fzL");
      server << F("/uX8y/7l/Mv+5fzL/uH8y/7h/Mv+4fzL/uH8y/7h/Mv+4fzL/uH8y/7d+Mv+WaCnqAAAAH41hJty2fTH/tn82/7eANv+3gDb/t4A2/7eANv+3gDb/t4A2/7e");
      server << F("ANv+3gDb/t4A2/7eANv+3gDb/toA2/7N9NP6yezH9s3sx/rV9Mf+cbCrro3Et6at1Le6ihFzUqIld2KiJXtioiV7YqIle2KiJXtioiV7YqIle2KiJXtioiV7YqIl");
      server << F("e2KiJXtmpil/XrIJK6qJrIfK2fjL/s3sw/qt2Lvelci/rmmgl4vf7//L+/v7+///////////////////////////////////////////////////////////9/Pz/4OHi5aBuLeW0f");
      server << F("TL/q3Yv+KVyL+uaaCXi9vv/8v/////Ly83au7Wtzrm0rc65tK3OubStzrm0rc65tK3OubStzrm0rc65tK3OubSszrWxrcr/////4eTo4Z9oH/CqdS73pXIv65poJeL1+f/x9Pb687F9N/G6g");
      server << F("TX/uoAy/7qAMv+6gDL/uoAy/7qAMv+6gDL/uoAy/7qAMv+6gDL/wIMy/62oosf/////oXc/6at2MPelci/rmmgl4vX5//H09/r1sH477Ld/Nf+3fjL/t34y/7");
      server << F("d+Mv+3fjL/t34y/7d+Mv+3fjL/t34y/7d+Mv+8gTH/sa2oyf////+XaSzfqnYv96VyL+uaaCXi9/v/8v/////d3+Hl09DL3NLPzNvSz8zb0s/M29LPzNvSz8zb0s/M29L");
      server << F("PzNvSz8zb0s/M287My9n/////rKac2650J/mqdS74pXIv65poJeL3+//y//////T19vfv7u3w7+7t8e/u7fHv7u3x7+7t8e/u7fHv7u3x7+7t8e/u7fHv7u3x7enk8JeIc8q3fTD/s");
      server << F("Hkw/Kt2Lvmlci/rmmkm3vX5/vP4+v35pn1H4K55M/asdy/1rHcv9ax3L/Wsdy/1rHcv9ax3L/Wsdy/1rHcv9a13MPWpcin3tHsv/7R8Mf+yezH+q3Yv+aJwLum0ey324efv6fb5/PWfbi7kuX4w/7Z7K/");
      server << F("+2eyv/tnsr/7Z7K/+2eyv/tnsr/7Z7K/+2eyv/tnsr/7Z6Kv+2eir/un4v/7J6Mf2rdi/4oW8s6Ld/M//AtKXc/////+Tl6PGusrm9s7e9wbO3vcGzt");
      server << F("73Bs7e9wbO3vcGzt73Bs7e9wbO3vcGzt73Bs7e9wbi8wMVlbnqauH0u/6t2L/iibyzptHwx/6l0Leyurq7K//////////////////////////////////////////////////////////////");
      server << F("///////7q6usq0eSn/q3Yv+KJwLOu0fDH/s3wy/7Z7Lf+JXSHQo4di16KIZtGiiWbQoolm0KKJZtCiiWbQoolm0KKJZtCiiWbQoolm0KKJZtCli2jShGlFv7d9MP");
      server << F("+rdi/4iV4lz7V9Mf+0fDH/tHwx/7R8Mf+0fDL/tHwy/7R8Mv+0fDL/tHwy/7R8Mv+0fDL/tHwy/7R8Mv+0fDL/tHwy/7R8Mv+0fDH+snsw/aVxLfE3Jg8jhVwkx6BuLPGfbivt");
      server << F("n24r7Z9uK+2fbivtn24r7Z9uK+2fbivtn24r7Z9uK+2fbivtn24r7Z9uK+2fbivtn24r7Z9uLO2PYifeOigQPAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
      server << F("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA='/>");

      server << F("</head><body>\n");
      server << F("<div class='page_content'>");
      server << F("<div class='b_content header'>");
      server << F("<div class = 'header_label'>");
      server << F("<h1>NotifyDuino</h1>");
      server << F("<p>Supporting : Arduino UNO and MEGA 2560</p>");
      server << F("</div>");
      server << F("<div class='header_logo'>");

      server << F("<p style='color:black'><a href='http://www.specforge.com/'>www.specforge.com</a></p>");
      server << F("</div>");
      server << F("</div>");
      server << F("<div class='b_content status'>");
      server << F("<div class='status_temp' style='vertical-align:top'>");
      server << F("<p>Temperature : ")<<TempC1<<("°C</p>");
      server << F("<p>Port ") << DuinoPort::getDigitalPin(DuinoPort::Port_1) << F(" : ") <<portValues[0]<<F("</p>");
      server << F("<p>Port ") << DuinoPort::getDigitalPin(DuinoPort::Port_2) << F(" : ") <<portValues[1]<<F("</p>");

      server << F("</div>");
      server << F("<div class='status_power'>");
      server << F("<p>Port ") << DuinoPort::getDigitalPin(DuinoPort::Port_3) << F(" : ") <<portValues[2]<<F("</p>");
      server << F("<p>Port ") << DuinoPort::getDigitalPin(DuinoPort::Port_4) << F(" : ") <<portValues[3]<<F("</p>");
      server << F("</div>");
      server << F("</div>");
      server << F("<div class='b_content notification'>");
      server << F("<div class = 'notification_conditions'>");
      if(controlEvents.temperatureEnabled())
      {
          server << F("<p>T : ≤ ")<<controlEvents.getMinTemperature()<<F("°C;≥ ")<<controlEvents.getMaxTemperature()<<F("°C</p>");
      } else
      {
          server << F("<p>T : OFF</p>");
      }

      for (int i = 0 ; i < event_port_count ; i++)
      {
        if (portEvents.isEnabled(static_cast<DuinoPort::EventPort>(i)))
        {
            server << F("<p>Event ")<<String(i+1,DEC) << (" : ON</p>");
        } else
        {
           server << F("<p>Event ")<<String(i+1,DEC) << (" : OFF</p>");
        }
      }
      
      server << F("</div>");
      server << F("<div class='notification_types'>");
      server << F("<p>Email : ");
      if (notification.emailEnabled())
          server << F(" ON ");
      else 
          server << F(" OFF ");   
      server << F("</p>");
      server << F("<p>Mobile : ");
      if (notification.mobileEnabled())
          server << F(" ON ");
      else 
          server << F(" OFF ");   
      server << F("</p>");
      server << F("</div>");
      server << F("</div>");
      server << F("<div class='tabs'>");
      //<tab1>
      server << F(" <div class='tab'>");
      server << F("<input type='radio' id='tab-1' name='tab-group-2' checked>");
      server << F("<label for='tab-1'>Power</label>");
      server << F("<div class='content'>");
      server << F("<form action='' method='POST'>");
      server << F("<button name='web_reboot'>Reboot</button>");
      server << F("</form>");
      server << F("</div>");
      server << F("</div>");
      //</tab1>
      
      //<tab3>
      server << F("<div class='tab'>");
      server << F("<input type='radio' id='tab-2' name='tab-group-2' checked>");
      server << F("<label for='tab-2'>Settings</label>");
      server << F("<div class='content'>");
      server << F("<form action='' method='POST'>\n");
      server << F("<table>");
      server << F("<tr><td colspan = '3'><h2>Authorization</h2></td></tr>");
      server << F("<tr><td>Username</td><td><input type='text' maxlength = '16' name='web_login' value='")<<autorization.getUsername()<<F("' required></td></tr>");
      server << F("<tr><td>Password</td><td><input type='password' maxlength = '16' name='web_pass' value='")<< autorization.getPassword() <<("' required></td></tr>");
      server << F("</table>");
      server << F("<p><button>Save</button></p>\n");
      server << F("<hr>"); 
      server << F("</form>\n");
      server  << F("<form action='' method='POST'>\n"); 
      server  << F("<table>");
      server  << F("<tr><td>IPv4</td>");
      server  << F("<td><input type='text' name='web_ip'  required pattern='^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$' value='");
      server  << network.getIp()<<F("'title='192.168.1.2'></td>");
      server  << F("</tr>");
      server  << F("<tr><td>PORT</td>");
      server  << F("<td><input type='number' required name='web_port' value='");
      server  << network.getPort() <<F("' min='1' max='65535'></td>");
      server  << F("</tr>");
      server  << F("<tr><td>GATE WAY</td>");
      server  << F("<td><input type='text' name='web_host' title = '192.168.1.2' pattern='^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$' value='"); 
      server  << network.getGateWay() << ("'></td>");
      server  << F("</tr>");
      server  << F("<tr><td>DNS</td>");
      server  << F("<td><input type='text' name='web_dns' pattern='^(([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])\\.){3}([0-9]|[1-9][0-9]|1[0-9]{2}|2[0-4][0-9]|25[0-5])$' value='");
      server  << network.getDns() << F("'></td>");
      server  << F("</tr>");
      server  << F("<tr><td>MASK</td>");
      server  << F("<td><input type='text' name='web_mask' value = '");
      server  << network.getMask() << F("' pattern='^(254|252|248|240|224|192|128)\\.0\\.0\\.0|255\\.(254|252|248|240|224|192|128|0)\\.0\\.0|255\\.255\\.(254|252|248|240|224|192|128|0)\\.0|255\\.255\\.255\\.(254|252|248|240|224|192|128|0)'></td>");
      server  << F("</tr>");   
      server  << F("<tr><td>MAC</td>");
      server  << F("<td><input type='text' name='web_mac' value='");
      server  << network.getMac() << F("'></td>");
      server  << F("</tr>");          
      server  << F("</table>");
      server  << F("<p><button>Save</button></p>\n");
      server  << F("</form>\n");
      server << F("</div>");
      server << F("</div>");
      //<tab3>
      
      //tab4
      server << F("<div class='tab'>");
      server << F("<input type='radio' id='tab-3' name='tab-group-2' checked>");
      server << F("<label for='tab-3'>Events</label>");
      server << F("<div class='content'>");
      server << F("<form action='' method='POST'>\n");
      server << F("<h2>Notification methods</h2>");
      server << F("<table>");

      //tr
      server << F("<tr><td>Email</td>");
      server << F("<td><select class='switcher' data-id='1' name='emailIsOn' >");
      if (notification.emailEnabled())
      {
          server << F("<option value='1' selected>ON</option> <option value='0'>OFF</option></select></td>");
          server << F("<td><input type='email' class = 'hidable' data-id='1' data-email='")<< notification.getEmail() <<F("'name='web_email' maxlength = '40'  value='");
          server <<  notification.getEmail()<<(" ' required></td>");
      } else
      {
          server << F("<option value='1'>ON</option> <option value='0' selected>OFF</option></select></td>");
          server << F("<td><input type='email' class = 'hidable' data-id='1' data-email='")<< notification.getEmail() <<F("'name='web_email' maxlength = '40'  value='' required></td>");
      }
      server << F("</tr>");
      //tr
      
      server << F("<tr><td>Mobile</td>");
      if (notification.mobileEnabled())
      {
      server << F("<td> <select id='mobile_select' name='mobileIsOn'>");
      server << F("<option value='1' selected='selected'>ON</option> <option value='0'>OFF</option>");
      } else 
      {
        server << F("<td> <select id='mobile_select' name='mobileIsOn'>");
        server << F("<option value='1'>ON</option> <option value='0' selected='selected'>OFF</option>");
      }
      server << F("</select></td>");
      server << F("</tr>");
      server << F("</table>");
      server << F("<p><button>Save</button></p>\n");
      server << F("<hr>");
      server << F("</form>\n");      
      server  << F("</table>");
      server << F("<h2>Events control<h2>");
      server << F("<form action='' method='POST' oninput='min_temperature_lvl_out.value=min_temperature_lvl.value+\"°C\";max_temperature_lvl_out.value=max_temperature_lvl.value+\"°C\"'>\n"); 
      server << F("<p>Temperature T : </p>");
      server << F("<table><tr><td>");
      server << F("<select name='temperature_warning' id = 'temperature_warning_select' class='switcher' data-id='2'>");
      if (controlEvents.temperatureEnabled())
      {
        server << F("<option value='1' selected='selected'>ON</option> <option value='0'>OFF</option>");
      } else 
      {
        server << F("<option value='1'>ON</option> <option value='0' selected='selected'>OFF</option>");
      }
      server << F("</select'></td>");
      server << F("<td><output size='3' style='display:inline-block;width:3em;' class='hidable' data-id='2' id = 'min_temperature_lvl_out'>")<<controlEvents.getMinTemperature()<< F("°C</output></td>");
      server << F("<td><input type='range' class='hidable' data-id='2' min='-50' max = '0' name='min_temperature_lvl' id = 'min_temperature_lvl' style='width:100px;' value = '")<<controlEvents.getMinTemperature()<< F("'></td>");
      server << F("<td><input type='range' class='hidable' data-id='2' min='0' max = '100' name='max_temperature_lvl' id = 'max_temperature_lvl' style='width:100px;' value = '")<<controlEvents.getMaxTemperature()<< F("'></td>");
      server << F("<td><output id = 'max_temperature_lvl_out' class='hidable' data-id='2' >") <<controlEvents.getMaxTemperature()<< F("°C</output></td></tr>");
      server << F("<tr><td><button>Save</button><td></tr>");
      server << F("</table>");
      server << F("</form>");

      //<PortEvents>
      server << F("<form action='' method='POST'>");
      server << F("<hr>");
      server << F("<p>Port events</p>");
      server << F("<table>");
      
      for (int i = 0 ; i < event_port_count ; i++)
      {
         server << F("<tr>");
         server << F("<td><p>") << String(i+1,DEC) <<( ")</p></td>");
         server << F("<td>");

         server << F("<td>");
         server << F("<p>")<< DuinoPort::getPortName (static_cast <DuinoPort::EventPort> (i))<<("</p>");
         server << F("<td>");

         server << F("<select class='switcher' name='e_port")<<String(i+1,DEC)<<("_on' data-id = '")<< String (i+3 , DEC)<< ("'>"); 
         if (portEvents.isEnabled(static_cast<DuinoPort::EventPort>(i)))
         {
           server << F("<option value='1' selected>ON</option>");
           server << F("<option value='0'>OFF</option>");
         } else 
         {
           server << F("<option value='1'>ON</option>");
           server << F("<option value='0' selected>OFF</option>");
         }
         server << F("</select>");
         server << F("</td>");
         server << F("<td>");
         server << F("<select class='hidable' name='e_port")<<String(i+1,DEC)<<("_opened'  data-id = '")<< String (i+3 , DEC) << ("'>"); 
         if (portEvents.isOpened(static_cast<DuinoPort::EventPort>(i)))
         {
           server << F("<option value='1' selected>Key Open</option>");
           server << F("<option value='0'>Key Close</option>");
         } else 
         {
           server << F("<option value='1'>Key Open</option>");
           server << F("<option value='0' selected>Key Close</option>");
         }
         server << F("</select>");
         server << F("</td>");
         server << F("</tr>");
      }
      server << F("<tr><td><button>Save</button></td></tr>");
      server << F("</table>");
      server << F("</form>");
      //</PortEvents>

      server << F("</div>");
      server << F("</div>");
      
      //tab4
      server << F("</div>");
      server << F("</div>");

      // <script>
      server << F("<script>"); 
      server << F("(function () {");
      server << F("console.log('script started');");
      server << F("var switchers = document.getElementsByClassName('switcher');");
      server << F("for (var i = 0 ; i < switchers.length ; i++){");
      server << F("switchers[i].onchange = function(){");
      server << F("console.log('turned');");
      server << F("var value =  this.options[this.selectedIndex].value;");
      server << F("var id = this.dataset['id'];");
      server << F("var hidables = document.querySelectorAll('.hidable[data-id=\"'+id+'\"]');");
      server << F("if (value == '1'){");
      server << F("for (var j = 0 ; j < hidables.length ; j++){");
      server << F("hidables[j].style.visibility = 'visible';");
      server << F("hidables[j].disabled = false;");
      server << F("}");
      server << F("}");
      server << F("else{");
      server << F("for (var j = 0 ; j < hidables.length ; j++){");
      server << F("hidables[j].style.visibility = 'hidden';");
      server << F("hidables[j].disabled = true;");
      server << F("}");
      server << F("}");
      server << F("};");
      server << F("switchers[i].onchange();");
      server << F("}");
      server << F("})();");
      server << F("</script>");
      //</script>
      server  << F("</body></html>\n");
    }
  } else
  {
    /* send a 401 error back causing the web browser to prompt the user for credentials */
    server.httpUnauthorized();
  }
}

void ISRPort1()
{
  bool state = DuinoPort::readDigital(DuinoPort::Port_1);
  portValues[0] = (int) state;
  if (portEvents.isEnabled(DuinoPort::Port_1))
  {
    if (state != portEvents.isOpened(DuinoPort::Port_1))event1 = true;
  }
}

void ISRPort2()
{
  bool state = DuinoPort::readDigital(DuinoPort::Port_2);
  portValues[1] = (int) state;  
  if (portEvents.isEnabled(DuinoPort::Port_2))
  {
    if (state != portEvents.isOpened(DuinoPort::Port_2)) event2 = true;
  }
}
 
void ISRPort3()
{
  bool state = DuinoPort::readDigital(DuinoPort::Port_3);
  portValues[2] = (int) state;
  if (portEvents.isEnabled(DuinoPort::Port_3))
  {
    if (state != portEvents.isOpened(DuinoPort::Port_3)) event3 = true;
  }
}
 
void ISRPort4()
{
  bool state = DuinoPort::readDigital(DuinoPort::Port_4);
  portValues[3] = (int) state;
  if (portEvents.isEnabled(DuinoPort::Port_4))
  {
    if (state != portEvents.isOpened(DuinoPort::Port_4)) event4 = true;
  }
}

//process of incomming events if has event send data to EventServer
void processEvents()
{
  if (event1) 
  {
    api->postEvent(notification, event1Message, serialNumber); 
    event1 = false;
  }
  if (event2)
  {
    api->postEvent(notification, event2Message, serialNumber);
    event2 = false;
  }
  if (event3) 
  {
    api->postEvent(notification, event3Message, serialNumber);
    event3 = false;
  }
  if (event4) 
  {
    api->postEvent(notification, event4Message, serialNumber);
    event4 = false;
  }
}

void checkTemperature()
{
  TempSensor1.requestTemperatures();
  TempC1 = TempSensor1.getTempCByIndex(0);
  if ((TempC1 < -50) || (TempC1 > 120))
  {
    TempC1 = initTemp;
  }
  
  TempSensor2.requestTemperatures();
  TempC2 = TempSensor2.getTempCByIndex(0);
  if ((TempC2 < -50) || (TempC2 > 120))
  {
    TempC2 = initTemp;
  }
}

// Ouput pin to Low or Hight state
void SetOutPin(int state)
{
    switch (state)
    {
      case 0:
        digitalWrite(outputPin, LOW);
        break;
      case 1:
        digitalWrite(outputPin, HIGH);
        break;  
      case 2:
        digitalWrite(outputPin, LOW);
        delay(1000);
        digitalWrite(outputPin, HIGH);
        break;
      default:  
        digitalWrite(outputPin, LOW);
        break;    
    }   
}

// Main Setup
void setup()
{ 
  Serial.begin (9200);
  
  //getting pins
  byte port1 = DuinoPort::getDigitalPin(DuinoPort::Port_1);
  byte port2 = DuinoPort::getDigitalPin(DuinoPort::Port_2);
  byte port3 = DuinoPort::getDigitalPin(DuinoPort::Port_3);
  byte port4 = DuinoPort::getDigitalPin(DuinoPort::Port_4);
  
  // Set pin state
  pinMode(port1, INPUT_PULLUP);
  pinMode(port2, INPUT_PULLUP);
  pinMode(port3, INPUT_PULLUP);
  pinMode(port4, INPUT_PULLUP);
  pinMode(outputPin, OUTPUT);

  SerialNumber serial;

  settings->read(DSettings::SerialNumberType,serial);

  //if serial not valid reset all settings to default
  if (!serial.isValid())
  {
    serial = SerialNumber::getDefaultSerialNumber();
    settings->save(DSettings::SerialNumberType,serial);
    serialNumber = serial.getData();
    settings->resetToDefault();
  }
  
  settings->read(DSettings::NetworkType, network);
  settings->read(DSettings::AutorizationType, autorization);
  settings->read(DSettings::NotificationType ,notification);
  settings->read(DSettings::ControlEventsType ,controlEvents);
  settings->read(DSettings::PortEventsType,portEvents);
  
  Ethernet.begin(network.macData() , network.ipData() , network.dnsData() , network.gateData() , network.maskData());  


  serialNumber = serial.getData();
  api = new ServerApi (serverNetwork); 

  webserver.setPort(network.getPort());
  webserver.setDefaultCommand(&WebCmd); 
  webserver.setUrlPathCommand(&UrlPathCommand);
  webserver.begin();
    
  attachInterrupt(digitalPinToInterrupt(port1), ISRPort1 , CHANGE );
  attachInterrupt(digitalPinToInterrupt(port2), ISRPort2 , CHANGE );
  attachInterrupt(digitalPinToInterrupt(port3), ISRPort3 , CHANGE );
  attachInterrupt(digitalPinToInterrupt(port4), ISRPort4 , CHANGE );
  int output_state = portEvents.outputState();
  
  if (output_state != 0) 
      SetOutPin(1);
  else 
      SetOutPin(0);

  temp_timer_start = millis();
  temp_timer_stop = millis();
  
  TempSensor1.begin();
  TempSensor2.begin();
  checkTemperature();
}

// Main Loop
void loop()
{
  // process incoming connections one at a time forever
  char request[50];
  int  request_len = 50;
  webserver.processConnection(request,&request_len);
  
  // process incoming events one at a time forever
  processEvents(); 
  
  // measure temperature
  temp_timer_stop = millis();
  if ((long)(temp_timer_stop - temp_timer_start) > 60000)  // 1 min
  {
    checkTemperature();
    temp_timer_start = temp_timer_stop;
    if (controlEvents.temperatureEnabled()){
      // if temperature out of range send notification
      if (TempC1 < controlEvents.getMinTemperature() || TempC1 > controlEvents.getMaxTemperature()){
        api->postEvent(notification, tempEvent1Message, serialNumber);
       }
    }
  }  
}

