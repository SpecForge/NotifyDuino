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

#ifndef SERVERAPI_H
#define SERVERAPI_H

#include "AES.h"
#include "Dsettings.h"

#ifndef SERVER_API_SERIAL_DEBUGGING
  #define SERVER_API_SERIAL_DEBUGGING 0
#endif

#if SERVER_API_SERIAL_DEBUGGING
  #include <HardwareSerial.h>
#endif

//aes key 
static  byte key [32]  =
{
0x6c, 0x7c, 0x77, 0x7b,
0xf2, 0x6b, 0x6f, 0x76, 
0x30, 0x01, 0x67, 0x2b,
0xfe, 0x76, 0xab, 0x76,
0xca, 0x82, 0xc9, 0x7d,
0xfa, 0x59, 0x47, 0xf0, 
0xad, 0xd4, 0xa2, 0x6c,
0x9c, 0xa4, 0x72, 0xc0,
};  

class ServerApi{
  public:
  
    ServerApi(const ServerNetwork& serverNetwork);
    bool postEvent(const Notification& notification ,  String message_p , String serial);
    void changeServer(const ServerNetwork& network);
    
  private:
  
    String toHexString(const byte* encrypted , int  length);
    String alignString(const String& inputString);
    EthernetClient client;
    ServerNetwork m_serverNetwork;
    AES m_aes;
};

ServerApi::ServerApi(const ServerNetwork& serverNetwork){
    m_serverNetwork = serverNetwork;
    m_aes.set_key(key,32);
}

//string to multiple 16 for encrypt
String ServerApi::alignString(const String& inputString){
     String alignedString = inputString;
     if (alignedString.length()%16!=0){
       int count = 16 - (alignedString.length()%16);
       for (int i=0;i<count;i++){
           alignedString.concat(' ');
       }
     }
     return alignedString;
}

//convert to hex string
String ServerApi::toHexString(const byte* encrypted , int  length){
  String outString;
  for (int i=0;i<length;i++){
        String hexadecimalNum = String(encrypted[i], HEX);
        if (hexadecimalNum.length()<2) {
         String str ('0');
         str.concat(hexadecimalNum);
         hexadecimalNum = str;
      }
      outString.concat(hexadecimalNum);
     }
  return outString;
}

void ServerApi::changeServer(const ServerNetwork& network){
    m_serverNetwork = network;
}

//posting event to EventServer 
bool ServerApi::postEvent(const Notification& notification ,  String message_p , String serial ){

  if (notification.emailEnabled() == false && notification.mobileEnabled() == false){
#if SERVER_API_SERIAL_DEBUGGING > 1
  Serial.println("Notification methods are disabled !!!");
#endif 
    return false;
  }
  String encryptedRequest;
     byte IV[16] = {0x6c, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 
                    0x6f, 0x76, 0x30, 0x01, 0x67, 0x2b, 
                    0xfe, 0x76, 0xab,0x76};
     
     if (notification.emailEnabled()){
        encryptedRequest.concat("&email=1");
     }
     else{
        encryptedRequest.concat("&email=0");
     }
     
     if (notification.emailEnabled()){
            String email = alignString(notification.getEmail());
            byte email_encrypted [email.length()];
            byte email_buff[email.length()];
            email.getBytes(email_buff,email.length());
            m_aes.cbc_encrypt(email_buff,email_encrypted,email.length()/16,IV);
            encryptedRequest.concat("&email_address=");
            String encodedEmail = this->toHexString(email_encrypted,email.length());
            encryptedRequest.concat(encodedEmail);
     } 

     if (notification.mobileEnabled()){
        encryptedRequest.concat("&mobile_app=1");
     }
     else{
        encryptedRequest.concat("&mobile_app=0");
     }
     encryptedRequest.concat("&serial=");
     encryptedRequest.concat(serial);
     encryptedRequest.concat("&message=");
     encryptedRequest.concat(message_p);

#if SERVER_API_SERIAL_DEBUGGING > 1

     Serial.println("<RequestString>");
     Serial.println(encryptedRequest);
     Serial.print(m_serverNetwork.m_domain); 
     Serial.println("</RequestString>"); 
     Serial.print("Connecting to event server with domain - ");
     Serial.print(m_serverNetwork.m_domain); 
     Serial.print(" and port - "); 
     Serial.println(m_serverNetwork.m_port);
     
#endif  
        
     if (client.connect(m_serverNetwork.m_domain,m_serverNetwork.m_port))
     {           
        //send request with parametrs to EventServer            
        client.println("POST /api/notify HTTP/1.1");
        client.println("Host:" + String(m_serverNetwork.m_domain));
        client.println("User-Agent: Arduino/1.0");
        client.println("Connection: close");
        client.println("Content-Type: application/x-www-form-urlencoded");
        client.print  ("Content-Length: ");
        client.println(encryptedRequest.length());
        client.println();
        client.println(encryptedRequest);
        client.flush();
        return true;
     }
    else {
      
#if SERVER_API_SERIAL_DEBUGGING > 1

     Serial.println("Cannot connect to Server");  
     
#endif   

      client.flush();
      client.stop();
      return false;
    }
}

#endif // SERVERAPI_H
