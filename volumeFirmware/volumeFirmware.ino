int volumeValue = -1;

void setup(){
  Serial.begin(9600);
}

void loop(){
   int analogInput = analogRead(A0);
   if(volumeValue != analogInput){
       volumeValue = analogInput;
       Serial.println(volumeValue);
   }
}
