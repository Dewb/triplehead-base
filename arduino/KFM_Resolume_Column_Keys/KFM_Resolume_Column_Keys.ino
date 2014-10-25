const int button1 = 3;        
const int button2 = 4;
const int button3 = 5;

void setup() { // initialize the buttons' inputs:
  pinMode(button1, INPUT);  
  pinMode(button2, INPUT);  
  pinMode(button3, INPUT);  

  Keyboard.begin();
}

void loop() {
  if (digitalRead(button1) == HIGH) {
    Keyboard.write('!'); 
    delay(250);
  }
  if (digitalRead(button2) == HIGH) {
    Keyboard.write('@'); 
    delay(250);
  }
  if (digitalRead(button3) == HIGH) {
    Keyboard.write('#'); 
    delay(250);
  }
}
