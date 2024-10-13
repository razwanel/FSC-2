#include <DHT.h>

#define proximityEchoPin 8
#define proximityTrigPin 7
#define gasPin A0
#define motionPin A1
#define tempPin 2
#define fanPin 3

#define proximityThreshold 30
#define gasThreshold 800

enum{
    GASLEAK,
    MOTION,
    INTRUDER
};

DHT dht11(tempPin, DHT11);



void setup(){
    Serial.begin(9600);
    //Serial1.begin(9600);
    
    // Sensor PINS setup //
    pinMode(proximityTrigPin, OUTPUT); // Sets the trigPin as an Output
    pinMode(proximityEchoPin, INPUT); // Sets the echoPin as an Input
    pinMode(gasPin,INPUT);
    pinMode(motionPin,INPUT);
    pinMode(tempPin,INPUT);
    pinMode(fanPin,OUTPUT);
    // ------------------ //
    dht11.begin(); // initialize the temp sensor


    
    delay(1500); // wait for gas sensor to warm up 
}

void loop(){
     
    // read sensors logic //
    bool motion = IsMotion();
    bool proximity = ReadProximity() < proximityThreshold;
    bool gas = IsGas();
    HeatCheck(); 
    // ------------------ //
    
    //SendState( (motion) | (proximity << 1) | (gas << 2) );
    //SendState('X');
    char x;
    if (motion && proximity && gas) {
    x = 'A';  // All true
    } else if (motion && proximity && !gas) {
      x = 'B';  // Motion and Proximity true, Gas false
    } else if (motion && !proximity && gas) {
      x = 'C';  // Motion and Gas true, Proximity false
    } else if (motion && !proximity && !gas) {
      x = 'D';  // Only Motion true
    } else if (!motion && proximity && gas) {
      x = 'E';  // Proximity and Gas true, Motion false
    } else if (!motion && proximity && !gas) {
      x = 'F';  // Only Proximity true
    } else if (!motion && !proximity && gas) {
      x = 'G';  // Only Gas true
    } else {
      x = 'H';  // All false
    }
    SendState(x);

    delay(100);

}

void SendState(char x){
    Serial.print(x);
}

bool IsMotion(){
  return analogRead(motionPin) > 500;
}

float ReadProximity(){
    long duration;
    int distance;

    delayMicroseconds(2);
    // Sets the trigPin on HIGH state for 10 micro seconds
    digitalWrite(proximityTrigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(proximityTrigPin, LOW);
    // Reads the echoPin, returns the sound wave travel time in microseconds
    duration = pulseIn(proximityEchoPin, HIGH);
    // Calculating the distance
    distance = duration * 0.034 / 2;
    // Prints the distance on the Serial Monitor
    return distance;
}

bool IsGas(){
    return analogRead(gasPin) > gasThreshold;
    
}

void HeatCheck(){
    if(dht11.readTemperature() > 30) // start Fan
    {
        digitalWrite(fanPin, HIGH);
    }
    else
    {
        digitalWrite(fanPin, LOW);
    }
}