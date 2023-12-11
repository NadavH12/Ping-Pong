#include <IRremote.h>
#include <LiquidCrystal.h>


/*
Nadav Horowitz CS351 Assignment 3 pingpongNH.ino 3/4/2023

This program simulates a ping pong match.

This program tracks the score of each player during the game and displays the scores on an LCD screen.
The program tracks which player is serving at any given time and displays this information on an LCD screen.
The program tracks game score and set score and displays them on 2 seperate rows on the LCD.
The program has control over 2 LEDs which it uses to show which player is currently serving and which player has achieved victory.
The program has control over 4 buttons, 1 score incrementing button and 1 score decrementing button for each player.
The program switches the serving player according to the official rules of ping pong.
After each set, as the players switch sides of the table, the LCD monitor switches the scores and points as well. 
The program ensures the victorius player is the first to 5 sets, and that each set must be won by a point differential of 2. 
*/


//Constant ints to specify various pin numbers of hardware components
const int led5 = 10,
					led6 = 13;		

const int button7 = 6,
					button8 = 7,
					button9 = 8,
					button10 = 9;

const int lcd_rs = 12,
				  lcd_en = 11,
					lcd_d4 = 5,
					lcd_d5 = 4,
					lcd_d6 = 3,
					lcd_d7 = 2;
const int ir_pin = A0;


//Scores, who is serving, who is playing top part
int	gamescoreA;			//A's game score
int gamescoreB;			//B's game score
int setpointA;			//A's set point
int setpointB;			//B's set point
int	servingA;				//True if A is serving
int topfieldA;			//True if A is playing on the top part
int serves;					//Number of serves presented since last switch
int	setfirstserveA;	//True if A is serving first in this set


//Button states
int	button7state;
int button8state;
int button9state;
int button10state;


//Button debounce (When a button is pressed or released it will make noisy contact initially. We ignore state changes for a bit)
unsigned long lastDebounceTime = 0;  //last time input pin was toggled
long debounceDelay = 150;    //the debounce time


//Method header for LCD screen
LiquidCrystal lcd(lcd_rs, lcd_en, lcd_d4, lcd_d5, lcd_d6, lcd_d7);
//Method header for IR Sensor
IRrecv irrecv(ir_pin);


//This method formats and prints scoring information on the LCD screen
void formatLine(char *buf, int score1, int serve1, int score2, int serve2) {
	char s1[10], s2[10];

	sprintf(s1, "%d", score1);
	sprintf(s2, "%d", score2);
	char *s = "-Serves-";
	if(serve1 == serve2)
		s = "";
	int dashes = 16 - strlen(s1) - strlen(s2) - strlen(s);

	strcpy(buf, s1);
	if(serve1)
			strcat(buf, s);
	for(int i = 0; i < dashes; i++)
			strcat(buf, "-");
	if(serve2)
			strcat(buf, s);
	strcat(buf, s2);
}


//This method draws the current state of the match on the LCD and LEDs
void drawState() {
	lcd.clear();
	char buf[17];

	//row 0
	if(topfieldA)
		formatLine(buf, gamescoreA, servingA, gamescoreB, !servingA);
	else
		formatLine(buf, gamescoreB, !servingA, gamescoreA, servingA);

	lcd.setCursor(0, 0);
	lcd.print(buf);

  //row 1
	if(topfieldA)
		formatLine(buf, setpointA, 0, setpointB, 0);
	else
		formatLine(buf, setpointB, 0, setpointA, 0);
	lcd.setCursor(0, 1);
	lcd.print(buf);

	//Serving LED
	if(servingA) {
		digitalWrite(led6, LOW);
		digitalWrite(led5, HIGH);
	}
	else {
		digitalWrite(led5, LOW);
		digitalWrite(led6, HIGH);
	}
}


//This method is called when the game is over. Blinks winning player's LED, turns losing player's LED off. Also responds to IR Sensor input to restart game
void gameover(char *s, int winled, int loseled) {

	//Game is over, blink winning led
	drawState();
	digitalWrite(loseled, LOW);	//Make sure lose led is off
	for(;;) {
		digitalWrite(winled, HIGH); 
   	delay(1000);
    if (irrecv.decode()){
      irrecv.resume();
      break;
    }
   	digitalWrite(winled, LOW); 
    delay(1000);
    if (irrecv.decode()){
      irrecv.resume();  
      break;
    }
	}
  clearBoard();
  drawState();
}


//This method is responsible for starting a new set. The method initializes setpoints to 0, the topfield player, and first serving player.
void newset() {
	//Start a new set
	setpointA = 0;
	setpointB = 0;

	//switch fields and starting serve
	topfieldA = !topfieldA;
	setfirstserveA = !setfirstserveA;
	servingA = setfirstserveA;
	serves = 0;
}


//This method controls what happens in the program whenever various buttons are pressed.
//The method increments and decrements points appropriately according to which buttons are pressed.
//The method checks if the set is over and if the game is over.
//The method also checks which player is serving.
void buttonevent(int button, int newstate) {
	if(newstate == LOW) {
		//Don't do anything on button release
		return;
	}

	switch(button) { 
		case 7:
			//The button increments Player-A’s set points
			setpointA++;
			serves++;
			break;
		case 8:
			//The button increments Player-B’s set points
			setpointB++;
			serves++;
			break;
		case 9:
			//The button decrements Player-A’s set points
			//Don't allow going beyond the begining of the set
			if(setpointA > 0) {
				serves--;
				setpointA--;
			}
			break;
		case 10:
			//The button decrements Player-B’s set points
			//Don't allow going beyond the begining of the set
			if(setpointB > 0) {
				setpointB--;
				serves--;
			}
	}

	//Set the serve direction
	if(setpointA >= 10 && setpointB >= 10) {
		//Flip at every point
		serves = 0;
		servingA = !servingA;
	} else {
		if(serves == -1) {
			//we went back need to flip
			serves = 1;
			servingA = !servingA;
		} else if(serves == 2) {
			serves = 0;
			servingA = !servingA;
		}
	}

	//Check if if anyone has won a set
	if(setpointA >= 11) {
		if(setpointA - setpointB >= 2) {
			//A won set
			gamescoreA++;
			newset();
		}
	}
	if(setpointB >= 11) {
		if(setpointB - setpointA >= 2) {
			// B won set
			gamescoreB++;
			newset();
		}
	}

	//Check if anyone has one the game
	if(gamescoreA >= 5)
		gameover("A WINS", led5, led6);
	if(gamescoreB >= 5)
		gameover("B WINS", led6, led5);
	
	//draw new state
	drawState();
}


//This method initializes various fields like gamescores and setpoints to 0 at the start of the simulation. 
//It also sets player A to be the first serving player
void clearBoard() {
	//Initialize score
	gamescoreA = 0;
	gamescoreB = 0;
	setpointA = 0;
	setpointB = 0;

	//Setup who is playing top field and who is serving
	topfieldA = 1;			// A is playing top field
	servingA = 1;				// A is serving
	serves = 0;					// zero serves thus far
	setfirstserveA = 1;	// A is serving the first serve in this set
}


//setup method initializes pinModes and buttonstates, also initializes LCD and IR Reciever
void setup() {
	//Setup serial for debugging
	Serial.begin(9600);

	//Setup pin modes
	pinMode(led5, OUTPUT);
	pinMode(led6, OUTPUT);
	pinMode(button7, INPUT);
	pinMode(button8, INPUT);
	pinMode(button9, INPUT);
	pinMode(button10, INPUT);

  //Setup ir reciever
  irrecv.enableIRIn();

	//Setup lcd
	lcd.begin(16, 2);
	lcd.noBlink();
	lcd.noCursor();
	lcd.noAutoscroll();
	lcd.print("Welcome!");
  delay(3000);

	//initialize button states
	button7state = LOW;
	button8state = LOW;
	button9state = LOW;
	button10state = LOW;

  clearBoard();
	drawState();
}


//This method sets the lastDebounceTime field. This method ensures that although buttons make noisy contact, each contact is only counted as 1 press.
void setDebounce() {
	lastDebounceTime = millis();
}


//Helper method to check if signal should be counted as button press or as noise
int notDebouncing() {
  return (millis() - lastDebounceTime) > debounceDelay;
}


//Check button state, generate buttonevent if required
int getstate(int pin, int oldstate, int button) {
	//filter out any noise by using debounce delay
	if(notDebouncing()) {
		int newstate = digitalRead(pin);
		if(newstate != oldstate) {
			//Button state change, start debouncing and do buttonevent
			setDebounce();
			buttonevent(button, newstate);
		}
		return(newstate);
	}
	return(oldstate);
}


//loop method checks for button presses and IR input. Runs continously throughout program 
void loop() {
  if (irrecv.decode()){
    irrecv.resume();
    clearBoard();
    drawState();
  } else {
	  button7state = getstate(button7, button7state, 7);
	  button8state = getstate(button8, button8state, 8);
	  button9state = getstate(button9, button9state, 9);
	  button10state = getstate(button10, button10state, 10);
  }
}