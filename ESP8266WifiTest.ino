#include <SoftwareSerial.h>

SoftwareSerial WIFI(7, 8); //RX, TX
#define SSID "SSID"
#define PASS "password"
int Wifi_State = 0, mStep = 0;;
char val, q;
boolean redo, flag, wait;
String cmd, res;

/*----------*/
int connectionId, pinNumber;
String closeCommand;
/*----------*/

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  Serial.println(F("Wifi is ready!"));
  WIFI.begin(9600);
}

void loop() {
  if (Serial.available()) {
    val = Serial.read();
  }
  if (WIFI.available()) {
    res = get_response();
    Serial.print("Result: ");
    Serial.println(res);
  }
  switch (val) {
    case 'A': //--------------------------------------------------測試 esp8266 AT
    case 'a':
      Serial.println(F("-----------Case A-----------"));
      sendDebug("AT");
      if (Loding("Sent AT")) {
        redo = false;
      } else {
        redo = true;
      }
      Serial.println(F("----------------------------"));
      break;
    case 'B': //--------------------------------------------------查看 esp8266 狀態
    case 'b':
      Serial.println(F("-----------Case B-----------"));
      flag = !flag;
      while (flag) {
        switch (Wifi_State) {
          case 0:
            Serial.println(F("------Firmware Version------"));
            sendDebug("AT+GMR");
            if (Loding("Get firmware version")) { //delay 1 sec
              Wifi_State++; //next setting wifi mode
            }
            break;
          case 1: //--------------------------------------------------查詢鮑率
            Serial.println(F("----------Baud Rate---------"));
            sendDebug("AT+CIOBAUD?");
            if (Loding("Get BAUD rate")) { //delay 1 sec
              Wifi_State++; //next setting wifi mode
            }
            break;
          case 2: //--------------------------------------------------查詢目前工作模式
            Serial.println(F("---------Work Mode----------"));
            Serial.println(F("- 1: STA - 2: AP - 3: BOTH -"));
            sendDebug("AT+CWMODE?"); // 1 = STA (網卡模式), 2 = AP (基地台模式), 3 = BOTH (AP+STA).
            if (Loding("Get work mode")) { //delay 1 sec
              Wifi_State++; //next setting wifi mode
            }
            break;
          case 3: // Get ip address 數據機"http://192.168.xxx.xxx" 手機熱點" http://xx.xx.xx.xx:port/pin=" <== ip
            Serial.println(F("-------------IP-------------"));
            sendDebug("AT+CIFSR");
            if (Loding("Get ip address")) { //delay 1 sec
              Wifi_State++; //next setting wifi mode
            }
            break;
          case 4: //--------------------------------------------------查詢目前連線到哪一個基地台
            Serial.println(F("---Connected Base Station---"));
            sendDebug("AT+CWJAP?");
            if (Loding("Get connected base station")) { //delay 1 sec
              Wifi_State++;
            }
            break;
          case 5: //---------------------------------查詢有哪些 Client 連線至此 AP (只在 AP/BOTH 模式有效)
            Serial.println(F("-----Connected Devices------"));
            sendDebug("AT+CWLIF");
            if (Loding("Get connected devices")) { //delay 1 sec
              Wifi_State++;
            }
            break;
          case 6: //--------------------------------------------------詢問目前 TCP/UDP 連線模式
            Serial.println(F("-----------TCP/UDP----------"));
            sendDebug("AT+CIPMUX?");
            if (Loding("Get TCP/UDP mode")) { //delay 1 sec
              Wifi_State++;
            }
            Serial.println(F("----------------------------"));
            break;
          case 7: //--------------------------------------------------查看連線狀態
            Serial.println(F("------Connection Status-----"));
            sendDebug("AT+CIPSTATUS");
            if (Loding("Get Connection Status")) { //delay 1 sec
              Wifi_State = 0;
              flag = !flag;
            }
            Serial.println(F("----------------------------"));
            break;
          default :
            break;
        }
      }

      break;
    case 'T': //--------------------------------------------------測試發送訊號
    case 't':
      Serial.println(F("-----------Case T-----------"));
      flag = !flag;
      while (flag) {
        switch (mStep) {
          case 0:
            Serial.println(F("-----Connect To WIFI AP-----"));
            cmd = "AT+CWJAP=\"";
            cmd += SSID;
            cmd += "\",\"";
            cmd += PASS;
            cmd += "\"";
            sendDebug(cmd);
            if (Loding("Wifi connect")) {
              mStep++;
            } else {
              mStep = 0;
            }
            Serial.println(F("----------------------------"));
            break;
          case 1:
            Serial.println(F("------Set TCP/UDP mode-----"));
            sendDebug("AT+CIPMUX=1");
            if (Loding("Set TCP/UDP mode")) {
              mStep++;
            }
            break;
          case 2:
            Serial.println(F("-------Enable TCP Server------"));
            sendDebug("AT+CIPSERVER=1,8888");
            if (Loding("Enable TCP Server")) {
              mStep++;
            } else {
              mStep--;
            }
            break;
          case 3:
            Serial.println(F("---------Getting msg----------"));
            if (getMsg("Getting msg")) {
              mStep++;
            }
            break;
          case 4:// wait for the serial buffer to fill up (read all the serial data)
            delay(10);
            connectionId = WIFI.read() - 48; // subtract 48 because the read() function returns
            // the ASCII decimal value and 0 (the first decimal number) starts at 4
            WIFI.find("pin="); // advance cursor to "pin="
            pinNumber = (WIFI.read() - 48) * 10; // get first number i.e. if the pin 13 then the 1st number is 1, then multiply to get 10
            pinNumber += (WIFI.read() - 48); // get second number, i.e. if the pin number is 13 then the 2nd number is 3, then add to the first number
//            digitalWrite(pinNumber, !digitalRead(pinNumber)); // toggle pin
            // make close command
            closeCommand = "AT+CIPCLOSE=";
            closeCommand += connectionId; // append connection id
            closeCommand += "\r\n";
            Serial.print("Turn Pin");
            Serial.print(pinNumber);
            Serial.print(":");
//            Serial.print(digitalRead(pinNumber));
            Serial.print("ON");
            Serial.println("!");
            sendDebug(closeCommand); // close connection
            mStep--; //next setting wifi mode
            break;
          case 5:
            Serial.println(F("-----Connect Server By TCP----"));
            cmd = "AT+CIPSTART=\"";
            cmd += "TCP";
            cmd += "\",\"";
            cmd += "www.google.com";
            cmd += "\",";
            cmd += "8080";
            sendDebug(cmd);
            //            if (Loding("Connect Server By TCP")) {
            //              mStep++;
            //            } else {
            //              mStep = 3;
            //            }
            if (gg("Connect Server By TCP")) {
              mStep++;
            } else {
              mStep = 5;
            }
            break;
          case 6:
            Serial.println(F("---Wait For Server Response---"));
            sendDebug("AT+CIPSEND=7");
            mStep++;
            break;
          case 7:
            if (test("Wait For Server Response")) {
              mStep++;
            } else {
              mStep = 7;
            }
            break;
          case 8:
            Serial.println(F("---------Send Message---------"));
            sendDebug("GET /");
            if (Loding("Wait For Server Response")) {
              mStep++;
            } else {
              mStep = 8;
            }
            break;
          case 9:
            Serial.println(F("----Disable TCP Connection----"));
            sendDebug("AT + CIPCLOSE = 0");
            if (Loding("Disable TCP Connection")) {
              mStep = 3;
              flag = !flag;
            } else {
              mStep = 9;
            }
            break;
          default:
            break;
        }
      }
      break;
    default:
      break;
  }

  if (!redo)
    val = ' ';
}

String get_response() {  //get esp responce without "Serial.find()".
  String response = "";
  char c;
  while (WIFI.available()) {
    c = WIFI.read();
    response.concat(c);
    delay(10);
  }
  response.trim();
  return response;
}

boolean Loding(String state) {
  String response = get_response();
  for (int timeout = 0 ; timeout < 30 ; timeout++) {
    if (response.indexOf("OK") != -1 || response.indexOf("no change") != -1) {
      if (timeout >= 2) {
        Serial.print(F(" OK! "));
        Serial.print(F("This commend waste "));
        Serial.print(timeout / 2);
        Serial.println(F(" sec."));
      } else {
        Serial.print(F("Get:  "));
        Serial.println(response);
      }
      return 1;
      break;
    } else if (timeout == 29) { //after 15 sec for wait esp without responce.
      Serial.print(state);
      Serial.println(F(" fail...\nExit2"));
      return 0;
    } else {
      response = get_response(); //reget
      if (timeout == 0)
        Serial.print(F("Wifi Loading."));
      else
        Serial.print(F("."));
      delay(500);
    }
  }
}

void sendDebug(String sent_cmd) {
  Serial.print("SEND: ");
  Serial.println(sent_cmd);
  WIFI.println(sent_cmd);
}

boolean getMsg(String state) {
  String response = get_response();
  q = Serial.read();
  for (int timeout = 0 ; timeout < 30 ; timeout++) {
    if (q == 'q') {
      q = ' ';
      mStep = 5;
      break;
    }
    if (response.indexOf("+IPD,") != -1) {
      if (timeout >= 2) {
        Serial.print(F(" OK! "));
        Serial.print(F("This commend waste "));
        Serial.print(timeout / 2);
        Serial.println(F(" sec."));
        Serial.println("Get webpage signal, analyzing...");
        Serial.print(F("Get:  "));
        Serial.println(response);
      } else {
        Serial.print(F("Get:  "));
        Serial.println(response);
      }
      return 1;
      break;
    } else if (timeout == 29) { //after 15 sec for wait esp without responce.
      Serial.print(state);
      Serial.println(F(" fail...\nExit2"));
      return 0;
    } else {
      response = get_response(); //reget
      if (timeout == 0)
        Serial.print(F("Wifi Loading."));
      else
        Serial.print(F("."));
      delay(500);
    }
  }
}

boolean gg(String state) {
  String response = get_response();
  if (!response.equals("")) {
    Serial.println(response);
    return 1;
  } else {
    return 0;
  }
}

boolean test(String state) {
  String response = get_response();
  Serial.println(response);
  for (int timeout = 0 ; timeout < 30 ; timeout++) {
    if (response.indexOf(">") != -1) {
      if (timeout >= 2) {
        Serial.print(F(" OK! "));
        Serial.print(F("This commend waste "));
        Serial.print(timeout / 2);
        Serial.println(F(" sec."));
        Serial.println(response);
      } else {
        Serial.println(response);
      }
      return 1;
      break;
    } else if (timeout == 29) { //after 15 sec for wait esp without responce.
      Serial.print(state);
      Serial.println(F(" fail...\nExit2"));
      return 0;
    } else {
      response = get_response(); //reget
      if (timeout == 0)
        Serial.print(F("Wifi Loading."));
      else
        Serial.print(F("."));
      delay(500);
    }
  }
}
