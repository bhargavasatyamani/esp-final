/*web page to control: http://18.218.109.88/anurag/home/DeviceControl.html */
/*-----( Calling of Header File)-----*/
#include<AltSoftSerial.h>
#include <LiquidCrystal.h>

#define IP "185.151.28.142"   //Server IP 


#define Relay1 10
#define Relay2 11
#define Relay3 12

String SSID_Name = "belkin.18f";
String SSID_PSWD = "ERF1234_0001";

String Response,cmp="";
String DATA;
String MSG = "", CMD;
bool Data_Rx = false, Change = false;
unsigned long Ptime_Post = 0, Ptime_get = 0;
char Receive_Channel;

AltSoftSerial ESP_Serial(8,9);
LiquidCrystal lcd(A0,A1,A2,A3,A4,A5);

void setup()
{
  Serial.begin(9600);
  ESP_Serial.begin(9600);
  pinMode(Relay1, OUTPUT);
  pinMode(Relay2, OUTPUT);
  pinMode(Relay3, OUTPUT);
  lcd.begin(16, 2);
  lcd.print(F(" Web Base Home  "));
  lcd.setCursor(0, 1);
  lcd.print(F(" Automation Sys "));
  delay(3000);
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(F("Connecting to   "));
  lcd.setCursor(0,1);
  lcd.print(F("WIFI............")); 
  delay(1000);
  ESP_Setup();
  lcd.clear();
  Ptime_Post = Ptime_get = millis();
}

void loop()
{
  if (cmp.length() > 1)
  {
    Serial.println("Hello world");
    if (cmp=="on1")
    {
      Serial.println(F("Relay1 On"));
      digitalWrite(Relay1, HIGH);
      lcd.setCursor(0,0);
      lcd.print(F("Relay1 On      "));
    }
     if (cmp=="off1")
    {
      Serial.println(F("Relay1 Off"));
      digitalWrite(Relay1, LOW);
      lcd.setCursor(0,0);
      lcd.print(F("Relay1 OFF     "));
    }

    if (cmp=="on2")
    {
      Serial.println(F("Relay2 On"));
      digitalWrite(Relay2, HIGH);
      lcd.setCursor(0,0);
      lcd.print(F("Relay2 On      "));
    }
    if (cmp=="off2")
    {
      Serial.println(F("Relay2 Off"));
      digitalWrite(Relay2, LOW);
      lcd.setCursor(0,0);
      lcd.print(F("Relay2 OFF      "));
    }
    if (cmp=="on3")
    {
      Serial.println(F("Relay3 ON"));
      digitalWrite(Relay3, HIGH);
      lcd.setCursor(0,0);
      lcd.print(F("Relay3 ON     "));
    }
    if (cmp=="off3")
    {
      Serial.println(F("Reay3 OFF"));
      digitalWrite(Relay3, LOW);
      lcd.setCursor(0,0);
      lcd.print(F("Relay3 Off     "));
    }

    CMD = "";
    cmp="";
  }

  if (millis()-Ptime_get >= 3000)
  {
    get_server();
    Ptime_get = millis();
  }
}

/*
      This function set the ESP8266 Mode 3 And on Server
*/
void ESP_Setup()
{
  ESP_Serial.println(F("AT+CWMODE=3"));
  if (ESP_Receive("OK", 2000))
    Serial.println(F("Mode Set"));
  ESP_Serial.println(F("ATE0"));
  if (ESP_Receive("OK", 2000))
    Serial.println(F("Echo Stop"));

  ESP_Serial.println(F("AT+CIPMUX=1"));
  if (ESP_Receive("OK", 2000))
    Serial.println(F("CIP MUX SET"));
    G1:
  ESP_Serial.print(F("AT+CWJAP=\""));
  ESP_Serial.print(SSID_Name);
  ESP_Serial.print(F("\",\""));
  ESP_Serial.print(SSID_PSWD);
  ESP_Serial.println("\"");
  if (ESP_Receive("OK", 7000))
  {
    Serial.println(F("Connected to Wifi"));
    lcd.clear();
    lcd.print(F("Connected 2 Wifi"));
    delay(2500);
    lcd.clear();
  }
  else
  {
    Serial.println(F("Wifi not Connect"));
    Serial.println(F("Going to loop"));
    goto G1;
  }
  ESP_Serial.println(F("AT+CIPSERVER=1,80"));
  if (ESP_Receive("OK", 4000))
    Serial.println(F("Server Started"));
  ESP_Serial.println(F("AT+CIPMUX=1"));
  if (ESP_Receive("OK", 2000))
    Serial.println(F("CIP MUX SET"));
}

/*
   This function Received the data from ESP_Module
   In this Function we can pass the Argument to cross check starswith
   If argument satisfied with startwith then it return 1 and store the string in "MSG"
*/
bool ESP_Receive(String Argument, int time_delay)
{
  unsigned long Rx_Time = millis();
  String Rx_MSG;
  while (millis() - Rx_Time < time_delay)   // it working fine with delay 520 milisec
  {
    if (ESP_Serial.available())
    {
      char Byte = ESP_Serial.read();
      Rx_MSG += Byte;
      if (Byte == '\n')
      {
        if (Rx_MSG.startsWith(F("check")))
        {
          Response += Rx_MSG;
        }
        else if (Rx_MSG.startsWith(Argument))
        {
          Response = Rx_MSG;
          if(Argument == "Content-Type")
          {
            Byte ='\0'; Rx_MSG = "";
          Rx_Time = millis();
          while(millis()- Rx_Time < 1000)
          {
            if (ESP_Serial.available())
            {
               Byte = ESP_Serial.read();
               Rx_MSG += Byte;
            }
          }
          Serial.println("Msg is:");
          Serial.println(Rx_MSG);
          unsigned char Count=0;
          for(int i=0; Rx_MSG[i] !='\0'; i++)
          {
            if(Rx_MSG[i]=='\n')
            {
              Count++;
              Serial.println(i);
            }
          }
          CMD = Rx_MSG.substring(2,MSG.indexOf(","));
          int a=CMD.length();
          CMD=CMD.substring(0,a-10);
         Serial.println(CMD);
         cmp=CMD.substring(CMD.indexOf("o"),a);
         Serial.print("Cmp is:");
         Serial.println(cmp);
          return 1;
          }
          if (Argument == "<html>")
          {
            Serial.println(Rx_MSG);
            Get_Control_CMD(Rx_MSG);
          }
          else if (Argument == "+IPD")
          {
            get_data(Rx_MSG);
          }
          return 1;
        }

        else
          Rx_MSG = "";
      } Rx_Time = millis();
    }
  }
  return 0;
}

/*
   This Below function will get the Control Comand from the Received String
   This function we will call once we get return 1 from ESP_Receive()
*/

bool Get_Control_CMD(String MSG)
{
  //  String CMD_Byte;
  CMD = "";
  CMD = MSG.substring((MSG.indexOf("<html>") + 6), MSG.indexOf("</html>"));
   // Serial.println(MSG);
   // Serial.print(F("Command is: ")); Serial.println(CMD);
  MSG = "";
}

boolean get_data(String MSG)
{
  CMD = "";
  Receive_Channel = MSG[5];
  CMD = MSG.substring(9, MSG.indexOf("\r\n"));
  Serial.print(F("Command is: ")); Serial.println(CMD);
  MSG = "";
}

/*
   This Function we used to send the response to user
*/



void get_server()
{
  String POST = "GET /test/status.txt HTTP/1.0\r\nHost: smartsocket-org.stackstaging.com\r\nAccept: */*\r\nContent-Type: application/x-www-form-urlrncoded\r\n\r\n\r\n";
  ESP_Serial.println(F("AT+CIPSTART=2,\"TCP\",\"185.151.28.142\",80"));
  if (ESP_Receive("OK", 2000));
  Serial.println("OK");
  ESP_Serial.print(F("AT+CIPSEND=2,"));
  ESP_Serial.println(POST.length());
  Serial.println(POST.length());
  if (ESP_Receive("OK", 200));
  {
    delay(200);                //with 250 milisec delay its working fine
    Serial.println("OK");
    ESP_Serial.print(POST);
    Serial.print(POST);
//    if (ESP_Receive("+IPD", 1000));
    if (ESP_Receive("Content-Type", 1000));
//    Serial.println(Response);
//    Response="";
  }
}

