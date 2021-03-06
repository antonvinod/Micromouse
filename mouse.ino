//test code for kit mouse
#include <avr/io.h>
#include <avr/interrupt.h>
//These are the assigned pins for the teensy 3.1
#define NW_EMIT 		      20
#define N_EMIT 	          	      19
#define NE_EMIT 		      20
#define NW_RECV 		      23
#define N_RECV 		              21
#define NE_RECV 		      22

//#define W_EMIT                19
//#define E_EMIT                19

//#define W_RECV  10
//#define E_RECV  11

//check if left is less than 125
//check if right is less than 100

//these values need to be calibrated
#define FRONT_WALL_DETECT     750
#define LEFT_WALL_STRGHT      600
#define RIGHT_WALL_STRGHT     550 //was 600

#define NW_SET              360
#define NE_SET              135
#define N_SET               285

#define DEBUG_PRINT     1

enum {
	IR_LEFT = 0,
        IR_RIGHT,
        IR_MID,
	NUM_IRLED
} number_of_IRled;

//led connected to pins 3 and 4
//const int Led1 = 20;   //this is for left and right leds
//const int Led2 = 21;	//this is for middle led
//Proportinoal control for mouse (l = left, r = right etc...)


//////////////////////////////////////////////////
/*		NEWLY ADDED CODE		*/
//////////////////////////////////////////////////

volatile unsigned long leftTick = 0;
volatile unsigned long rightTick = 0;

//velocity of each wheel in meters per millisecond
const double rightVelocity = 0;
const double leftVelocity = 0;

//current speed values
double currentrightSpeed = 0;
double currentleftSpeed = 0;

//derivative motor control variables
unsigned long prev_encRight = 0;
unsigned long prev_encLeft = 0;
unsigned long diff_encRight = 0;
unsigned long diff_encLeft = 0;
double velocity_encRight = 0;
double velocity_encLeft = 0;
double velocity_errorRight = 0;
double velocity_errorLeft = 0;
double set_velocity = 60;
double Kd_speed = .125;

int l_wheel_ticks = 102; 
int r_wheel_ticks = 19;
int l_turn_ticks = 0;
int r_turn_ticks = 0; 
////////////////////////////////////////////////// *****KProportional ONLY***********//////////////////////
//PWM values translated from velocity
double Kp_speed = .6;
int rightspeed = 40;
int leftspeed = 40;



const int RencoderA = 14;
const int LencoderA = 16;
const int rightPWMA = 5;
const int rightPWMB = 6;
const int leftPWMA = 3;
const int leftPWMB = 4;



void set_ticks(void){
 
 l_wheel_ticks = 102; 
 r_wheel_ticks = 19;
 l_turn_ticks = leftTick + l_wheel_ticks;
 r_turn_ticks = rightTick + r_wheel_ticks;
 //right_turn();
}

void stopboth_motors(void) {
   analogWrite(leftPWMA,0);
   analogWrite(leftPWMB,0);
   analogWrite(rightPWMA,0);
   analogWrite(rightPWMB,0);
}
//may need to switch the left_speed and right_speed
void move_forward(unsigned char leftspeed, unsigned char rightspeed) {
        analogWrite(leftPWMA,leftspeed);
	analogWrite(leftPWMB,0);
	analogWrite(rightPWMA,rightspeed);
	analogWrite(rightPWMB,0);
	//digitalWrite(Led1,LOW);
	//digitalWrite(Led2,HIGH);
}
void move_backward(unsigned char leftspeed, unsigned char rightspeed){
        analogWrite(leftPWMA,0);
	analogWrite(leftPWMB,leftspeed);
	analogWrite(rightPWMA,0);
	analogWrite(rightPWMB,rightspeed);
	//digitalWrite(Led1,LOW);
	//digitalWrite(Led2,HIGH);
}
void turn_right(unsigned char leftspeed, unsigned char rightspeed) {
	analogWrite(leftPWMA,leftspeed);
	analogWrite(leftPWMB,0);
	analogWrite(rightPWMA,0);
	analogWrite(rightPWMB,rightspeed);
        //digitalWrite(Led1,HIGH);
	//digitalWrite(Led2,LOW);
}
void turn_left(unsigned char leftspeed, unsigned char rightspeed) {
	analogWrite(leftPWMA,0);
	analogWrite(leftPWMB,leftspeed);
	analogWrite(rightPWMA,rightspeed);
	analogWrite(rightPWMB,0);
        //digitalWrite(Led1,LOW);
	//digitalWrite(Led2,HIGH);
}
//////////////////////////////////////////////////
/*		END OF NEW CODE			*/
//////////////////////////////////////////////////

bool left_empty = false;
bool right_empty = false;

int no_l_wall = 0;
int no_r_wall = 0;


int kp_left = 0;
int ki_left = 0;
int kd_left = 0;
int error_left = 0;
int prev_err_left = 0;
int err_sum_left = 0;
int Output_left = 0;
int Offset_left = 100;

int kp_right = 0;
int ki_right = 0;
int kd_right = 0;
int error_right = 0;
int prev_err_right = 0;
int err_sum_right = 0;
int Output_right = 0;
int Offset_right = 100;

//int leftspeed = 40 *.9;//was 50
//int rightspeed = 37 *.9;//was 45

int l_error = 0;
int r_error = 0;
int m_error = 0;

/*
//motor driver logic pins
const int leftPWMA = 3;
const int leftPWMB = 4;
const int rightPWMA = 5;
const int rightPWMB = 6;
*/

//global variables
int ir_raw[3]={0, 0, 0}; //this initializes each arr elem to 0
int sides[2] = {0,0};
int led_state = 0;
int turn_flag = 0;
unsigned long timer = 0;

/*
psuedo code for speed control

unsigned long ENC = 0

unsigned long prev_ENC = 0

long dif_ENC = ENC - prev_ENC

speed_one = dif_ENC / 0.001

int set_val = 10

error_val = set_val - speed_one

speed_control = kp*error_val

*/

/*
void check_empty(){
	
	if(//sensor left is really low)
	{	
		left_empty = true;
	}
	if(//sensor right is really low)
	{
		right_empty = true;
	}
	
}
*/

// Encoder functions
void rightENCODER(void){
  rightTick++;  
}


void leftENCODER(void){
  leftTick++;
}

void resetValues(void){
  stopboth_motors();
  rightTick = 0;
  leftTick = 0;
  prev_encRight = 0;
  prev_encLeft = 0;
}


//RIGHT TURNING 
void right_turn(void){
 stopboth_motors();
 //int left_turntick = leftTick;
 //int right_turntick = rightTick;

 //turn_right(rightSpeed, leftSpeed);
 set_ticks();
 while(leftTick < l_turn_ticks){
 
   //leftTick++;
   //rightTick++;
   if(leftTick > l_turn_ticks) {stopboth_motors();}
   Serial.print("\nleftTick is: ");
   Serial.print(leftTick);
   Serial.print("\nl_turn_ticks is: ");
   Serial.print(l_turn_ticks);
   Serial.print("\nrightTick is: ");
   Serial.print(rightTick);
   Serial.print("\nr_turn_Ticks is: ");
   Serial.print(r_turn_ticks);   
  turn_right(rightspeed, leftspeed);
 } 
 move_forward(0,0);
 delay(1000);
 
}

//LEFT TURNING
void left_turn(void){
 stopboth_motors();
 //int left_turntick = leftTick;
 //int right_turntick = rightTick;

 //turn_right(rightSpeed, leftSpeed);
 set_ticks();
 while(rightTick < r_turn_ticks){
 
   //leftTick++;
   //rightTick++;
   if(rightTick > r_turn_ticks) {stopboth_motors();}
   Serial.print("\nleftTick is: ");
   Serial.print(rightTick);
   Serial.print("\nl_turn_ticks is: ");
   Serial.print(r_turn_ticks);
   Serial.print("\nrightTick is: ");
   Serial.print(leftTick);
   Serial.print("\nr_turn_Ticks is: ");
   Serial.print(l_turn_ticks);   
  turn_left(rightspeed, leftspeed);
 } 
 move_forward(0,0);
 delay(1000);
 
}



void checkVelocity(void){

  diff_encRight = rightTick - prev_encRight;
  
  prev_encRight = rightTick;
  
  velocity_encRight = (diff_encRight/.001);
  
  velocity_errorRight = set_velocity - velocity_encRight;
  
  rightspeed = (Kd_speed*velocity_errorRight);
  
}

void switch_sensors() {
	switch(led_state) {
		case 0:
                        //ACTIONS
			ir_raw[0] = analogRead(NW_RECV);
			ir_raw[2] = analogRead(NE_RECV);

			digitalWrite(N_EMIT,HIGH);
			digitalWrite(NW_EMIT,LOW);
			digitalWrite(NE_EMIT,LOW);
                     //   digitalWrite(W_EMIT,HIGH);
                     //   digitalWrite(E_EMIT,HIGH);
			if (DEBUG_PRINT) {

				Serial.print("\nCase 0:  ");
				Serial.print("\n         ---->left recv: ");
                                Serial.print(ir_raw[0]);
                                Serial.print("<---->mid recv: ");
				Serial.print(ir_raw[1]);
                                Serial.print("---->right recv: ");
				Serial.print(ir_raw[2]);
                                Serial.print("\nWest recv: ");
                                Serial.print(sides[0]);
                                Serial.print("<--------->East recv: ");
                                Serial.print(sides[1]);
                                //delay(500);
			}
                        //TRANSIOTIONS
                        led_state = 1;
			break;
		case 1:
                        //ACTIONS
			ir_raw[1] = analogRead(N_RECV);
              //          sides[0] = analogRead(W_RECV);
              //          sides[1] = analogRead(E_RECV);
		        digitalWrite(N_EMIT,LOW);
			digitalWrite(NW_EMIT,HIGH);
			digitalWrite(NE_EMIT,HIGH);
                //        digitalWrite(W_EMIT, LOW);
                //        digitalWrite(E_EMIT,LOW);
			if (DEBUG_PRINT) {
				Serial.print("\nCase 1:  ");
                                Serial.print("\n       ---->left recv: ");
                                Serial.print(ir_raw[0]);
                                Serial.print("<---->mid recv: ");
				Serial.print(ir_raw[1]);
                                Serial.print("---->right recv: ");
				Serial.print(ir_raw[2]);
                                Serial.print("\nWest recv: ");
                                Serial.print(sides[0]);
                                Serial.print("<--------->East recv: ");
                                Serial.print(sides[1]);
                                //delay(500);
			}
                        //TRANSITIONS
                        led_state = 0;
			break;
		default: break;
	}
}


void move_robot() {

        error_left = ir_raw[0] - NW_SET;
        err_sum_left += error_left;
        kp_left = 2 * error_left;
        ki_left = 2 * err_sum_left;
        kd_left = 2 * (error_left - prev_err_left);

        Output_left = (kp_left + ki_left + kd_left)/3;

        error_right = ir_raw[2] - NE_SET;
        err_sum_right += error_right;
        kp_right = 2 * error_right;
        ki_right = 2 * err_sum_right;
        kd_right = 2 * (error_right - prev_err_right);

        Output_right = (kp_right + ki_right +kd_right)/3;
//////////////////////////////////////        
        //l_error = 16;
        //r_error = 10;
//////////////////////////////////////
  	if(ir_raw[1] < FRONT_WALL_DETECT){
            //go forward if we dont have a wall in front
            move_forward(leftspeed, rightspeed);
            //proportional control
            if(error_left > 30){

              move_forward(leftspeed + Output_left, rightspeed);
            }
            //else if((ir_raw[0] + Offset - ir_raw[2]) < 0){

            if(error_right > 30){
            //else if(((ir_raw[0] + Offset - ir_raw[2]) < -20)){    //too close to right
              move_forward(leftspeed, rightspeed + Output_right);
            }

            else{

                  if (ir_raw[0] < no_l_wall && ir_raw[2] > no_r_wall ){
                    Serial.print("                  GOING LEFT");
                     //move forward x number of cells and
                     //call go left instead of this stuff
                     delay(150);
                     stopboth_motors();
                     delay(100);
                     move_forward(leftspeed, rightspeed);
                     delay(250);
                     //turn_left(leftspeed, rightspeed);
                     delay(400);
                     move_forward(leftspeed, rightspeed);
                     delay(350);
                  }
                  else if (ir_raw[0] > no_l_wall && ir_raw[2] < no_r_wall ){
                     Serial.print("                  GOING RIGHT");
                     //move forward x number of cells and
                     //call go right instead of this stuff
                     delay(150);
                     stopboth_motors();
                     delay(100);
                     move_forward(leftspeed, rightspeed);
                     delay(250);
                     turn_right(leftspeed, rightspeed);
                     delay(400);
                     move_forward(leftspeed, rightspeed);
                     delay(350);
                  }

            }

           prev_err_left = error_left;
           prev_err_right = error_right;
	}
        else if(ir_raw[1] >= FRONT_WALL_DETECT){
                    Serial.print("                  DETECT FRONT WALL");
                    
                    //do two turn rights or call 180
                    stopboth_motors();
                    delay(5000);
                    move_backward(leftspeed, rightspeed);
                    delay(500);

	}
}
void setup() {
  Serial.begin(9600);
  
 //IR sensor emitter and receiver setup
  pinMode(NW_EMIT,OUTPUT);
  pinMode(NW_RECV,INPUT);

  pinMode(N_EMIT,OUTPUT);
  pinMode(N_RECV,INPUT);
  
  pinMode(NE_EMIT,OUTPUT);
  pinMode(NE_RECV,INPUT);

//IR digital write setup
  digitalWrite(NW_EMIT,HIGH);
  digitalWrite(N_EMIT,LOW);
  digitalWrite(NE_EMIT,HIGH);

//Encoder Setup
  pinMode(RencoderA,INPUT);
  pinMode(LencoderA,INPUT);
  attachInterrupt(14,rightENCODER,RISING);
  attachInterrupt(16,leftENCODER,RISING);
  stopboth_motors();
}

void loop() {
	timer = micros();
	Serial.print("\nswitch begin");
        //delay(500);
	switch_sensors();
	//delay(500);
        Serial.print("\nswitch ended");

	Serial.print("\nmove begin");
        //delay(500);
	move_robot();
	//delay(500);
        Serial.print("\nmove ended");
	while((micros()-timer)<1000) {delayMicroseconds(250);}	//was i microsecond
}
