#include<SPI.h>
#include<Ethernet.h>
int light_sensor=5;
int temp_sensor=4;
int motion_sensor=0;
int led_pin=8;
#define RELAY4 12
#define RELAY3 11
#define RELAY2 10
#define RELAY1 9
int inboard_led=13;
//For serial comm
#define REQUESTED_DATA_LEN 15
String request_byte;
char input[15];

byte mac[]={0xDE,0xAD,0xBE,0xEF,0xFE,0xAB};
//char server[]="52.89.219.153";
char server[]="192.168.0.100";
IPAddress ip(192,168,0,102);
EthernetClient client;
unsigned long lastConnectiontime=0;
const unsigned long delayUpdate = 30L * 1000L;
EthernetServer EthServer(8000);
String data_request;
boolean reading=false;
IPAddress local_ip;

void setup() {
  // put your setup code here, to run once: 
pinMode(light_sensor,INPUT);
pinMode(temp_sensor,INPUT);
pinMode(motion_sensor,INPUT);
pinMode(led_pin,OUTPUT);
pinMode(RELAY1,OUTPUT);
pinMode(RELAY2,OUTPUT);
pinMode(RELAY3,OUTPUT);
pinMode(RELAY4,OUTPUT);
analogReference(DEFAULT);
pinMode(inboard_led, OUTPUT);
Serial.begin(9600);
digitalWrite(RELAY1,HIGH);
digitalWrite(RELAY2,HIGH);
digitalWrite(RELAY3,HIGH);
digitalWrite(RELAY4,HIGH);

if(Ethernet.begin(mac)==0){
  Serial.println("Unable to connect to Ethernet");
  Ethernet.begin(mac,ip);
  EthServer.begin();
  }
  delay(1000);
  EthServer.begin();
  Serial.println("Connecting...");
  Serial.println("Local IP");
  Serial.println(Ethernet.localIP());
  ip = Ethernet.localIP();
  local_ip=ip;
  Serial.println("ip");
  Serial.println(local_ip);
  Ethernet.begin(mac,ip);
}

  
void loop() {
float celcius=analogRead(temp_sensor)-273.15;
int motion  = analogRead(motion_sensor);
int light_value=analogRead(light_sensor);
int ledlevel= map(light_value,200,900, 255, 0);
light_value=constrain(light_value,200,900);
analogWrite(led_pin, ledlevel);
digitalRead(RELAY1);
digitalRead(RELAY2);
digitalRead(RELAY3);
digitalRead(RELAY4);
String final_str = String(light_value) + "," + String(celcius) + "," + String(motion);
Serial.println("finalstr");
Serial.println(final_str);
//client=EthServer.available();
if (EthServer.available()) 
{
    // send http reponse header
    client=EthServer.available();
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println();
    // process request.
    Serial.println("Process_Request");
    processClient(client);
    client.stop();
}

if(Serial.available()>=REQUESTED_DATA_LEN){
  request_byte=Serial.readString();
  Serial.println(request_byte);
  String indexValue=getValue(request_byte,',',3);
  Serial.println(indexValue);
  }

Serial.flush();

if(light_value<=550){
  digitalWrite(led_pin,0);
  }else{
    digitalWrite(led_pin,1);
    }

if(millis()-lastConnectiontime > delayUpdate){
  lastConnectiontime=millis();
  httpRequest(client,celcius,light_value, motion);
  }

delay(2000);
}

void httpRequest(EthernetClient client,float celcius, int light_value, int motion){
  client.stop();
  Ethernet.begin(mac,ip);
  String data ="light1=" + String(digitalRead(RELAY1)) + "&light2=" + String(digitalRead(RELAY2))  + "&light3=" +String(digitalRead(RELAY3))  + "&light4=" + String(digitalRead(RELAY4)) 
                + "&temp="+ String(celcius) +"&light_intensity=" + String(light_value) + "&motion="+ String(motion) + "&ip_address="+ local_ip; 
  Serial.println("data " + data);
  if(client.connect(server,8000)){
    Serial.println("Connecting....");
    client.println("POST /MyApplication/get_arduino_data/ HTTP/1.1");
    client.println("Host: 192.168.0.100");
    client.println("Content-Type: application/x-www-form-urlencoded;");
    client.println("Connection: close");
    client.print("Content-Length: ");
    client.println(data.length());
    client.println();
    client.print(data);
    client.println();
    
    //Printing data for Serial debug
    Serial.println("POST /MyApplication/get_arduino_data/ HTTP/1.1");
    Serial.println("Host: 192.168.0.100");
    Serial.println("Content-Type: application/x-www-form-urlencoded;");
    Serial.println("Connection: close");
    Serial.print("Content-Length: ");
    Serial.println(data.length());
    Serial.println(data);
    Serial.println();
    Serial.println();

//    if(client.available()){
//      char c = client.read();
//      Serial.println("Response "+c);
//      }
    }
    delay(2000);
     //if client is not connect, stop client
    if (!client.connected()) {
    Serial.println();
    Serial.println("disconnecting.");
    client.stop();
    }
 }

void processClient(EthernetClient client)
{
  //In this we check the first instance of character '=' and read the string till we get space
  boolean lineIsBlank = true;
  while (client.connected())
  {
    if (client.available())
    {
      char c = client.read();
      if(reading && c == ' '){ 
      reading = false;
      break;
      }
      //reading true if we get = character
      if(c == '=') {
      reading = true;
      }
            
      if(reading && c != '=') {
      data_request += c;
      }     
    }
  }
  client.print(data_request);
  delay(1);
  client.println("HTTP/1.1 200");
  client.println();
  client.stop();
  Serial.println("req_cl "+data_request);
  set_appliance(data_request);
  data_request = "";
}

void set_appliance(String data){
  if(data.length()!=0){
    if(getValue(data,',',0)=="1"){
      Serial.println("first_is_1");
      digitalWrite(RELAY1,LOW);
      }else if(getValue(data,',',0)=="0"){
        Serial.println("first_is_0");
        digitalWrite(RELAY1,HIGH);
      }
      delay(100);
     if(getValue(data,',',1)=="1"){
      Serial.println("second_is_1");
      digitalWrite(RELAY2,LOW);
      }else if(getValue(data,',',1)=="0"){
        digitalWrite(RELAY2,HIGH);
        digitalWrite(inboard_led,LOW);
        Serial.println("Second_zero");
      }
      delay(100);
      if(getValue(data,',',2)=="1"){
        Serial.println("third_is_1");
        digitalWrite(RELAY3,LOW);
      }else if(getValue(data,',',2)=="0"){
        Serial.println("third_is_0");
        digitalWrite(RELAY3,HIGH);
      }
      delay(100);
      if(getValue(data,',',3)=="1"){
        Serial.println("fourth_is_1");
        digitalWrite(RELAY4,LOW);
      }else if(getValue(data,',',3) == "0"){
        Serial.println("fourth_is_0");
        digitalWrite(RELAY4,HIGH);
      }
    }
  }

String getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1  };
  int maxIndex = data.length()-1;
  for(int i=0; i<=maxIndex && found<=index; i++){
  if(data.charAt(i)==separator || i==maxIndex){
  found++;
  strIndex[0] = strIndex[1]+1;
  strIndex[1] = (i == maxIndex) ? i+1 : i;
  }
 }
  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}
