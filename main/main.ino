//library to communicate with ethernet shield
#include <SPI.h>
//library to connect to internet
#include <Ethernet.h>

//stores trigger pin position
int triggerPin = 2;
//stores echo pin position
int echoPin = 3;
int time;
int distance;
//server that Arduino will use
char serverName[] = "covid-mask-detector.herokuapp.com";
//mac address for ethernet shield
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };
//creates Ethernet client
EthernetClient twilioClient;
String answer;

void setup() {
  
  Serial.begin(9600);
  //if Ethernet is not configured, theprogram does not continue
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet");
    while(true);
  }
  else {
    Serial.println("Arduino connected to Internet");
  }
  pinMode(triggerPin, OUTPUT);
  pinMode(echoPin, INPUT);

}

void loop() {
  
  digitalWrite(triggerPin, HIGH);
  delay(1);
  digitalWrite(triggerPin, LOW);
  time = pulseIn(echoPin, HIGH);
  distance = (time * 0.034) / 2;

  if(distance <=10) {
    sendSMS();
  }
}

void sendSMS()
{
  //check if Arduino can open connection to theserver
  if (twilioClient.connect(serverName, 80)) {
    Serial.println("Sending text message...");
    //call to /sendsms endpoint, this will send the sms to the user
    twilioClient.println("GET /sendsms HTTP/1.1");
    twilioClient.println("Host: covid-mask-detector.herokuapp.com");
    twilioClient.println("Connection: close");
    twilioClient.println();
    twilioClient.stop();
    Serial.println("Text message sent.");
    //stop program for two minutes
    delay(120000);
    //get user reply
    getSMS();
  }
}

void getSMS()
{ 
  //check if Arduino can open connection to theserver
  if (twilioClient.connect(serverName, 80)) {
    Serial.println("Getting SMS...");
    //call to /readsms endpoint, this will return the body of the SMS that the user has sent
    twilioClient.println("GET /readsms HTTP/1.1");
    twilioClient.println("Host: covid-mask-detector.herokuapp.com");
    twilioClient.println("Connection: close");
    twilioClient.println();
    //wait to start reading data
    while(twilioClient.connected() && !twilioClient.available()) delay(1);
    //while data is available, keep reading data
    while (twilioClient.connected() || twilioClient.available()) {
          //serverCall stores the data fetched from the endpoint
          char serverCall = twilioClient.read();
          answer += serverCall;
    }
    //stop running the Ethernet client
    twilioClient.stop();
    //if the reply from the user is equal to yes, stop the program
    if(answer.indexOf("Yes") > 0) {
      Serial.println("You are wearing a mask!");
    }
    //if the reply from the user is not equal to yes, send SMS again
    else {
      Serial.println("You are not wearing a mask. Repeating...");
      sendSMS();
    }
    
  }
}
