/*
 *  Arduino Camera Turret Control
 
    This file is part of CameraTurret.

    CameraTurret is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    CameraTurret is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with CameraTurret.  If not, see <https://www.gnu.org/licenses/>.

 *  Charles Shapiro Nov 2018
 */
#include <Servo.h>
#include <string.h>

#define CHS_DEBUG

#define HSERVOPIN 10
#define VSERVOPIN 9

// These may need adjustment for your turret.
#define HSERVOMIN 0
#define HSERVOMAX 180
#define VSERVOMIN 0
#define VSERVOMAX 112
#define HSERVOCENTER ((HSERVOMAX - HSERVOMIN) / 2)
#define VSERVOCENTER ((VSERVOMAX - VSERVOMIN) / 2)

Servo Hservo;
Servo Vservo;
int CurrentHPosition;
int CurrentVPosition;

#define CMDSTRSIZE 65 // Max number of chars the UART will hold
char Cmdstr[CMDSTRSIZE];
char CmdBuffer[CMDSTRSIZE];
char *InCursor;
typedef enum cmdSendingStates {None,Sending,Received};
cmdSendingStates CmdSendingState;
//                        0   1     2  3     4
typedef enum directions {Left,Right,Up,Down,Center};


typedef struct command {
   int distance;
   directions theDirection;
};


/*
   Wait for a command from UART
*/
int checkForCommand(char *outBuffer) {
   int retVal=0;
   int numAvail;
   
   while(Serial.available()) {
      if( (InCursor - Cmdstr) > (sizeof(Cmdstr)-1) ) {
         memset(Cmdstr,0,(sizeof(Cmdstr)-1) );
         InCursor=Cmdstr;
      }
      *InCursor=Serial.read();
      if('^' == *InCursor)
         CmdSendingState=Sending;
      else if ('/' == *InCursor)
         CmdSendingState=Received;
      if('\n' != *InCursor)
        ++InCursor;      
   }

   if(Received == CmdSendingState) {
      strncpy(CmdBuffer,Cmdstr+1,strlen(Cmdstr)-2);      
      memset(Cmdstr,0,sizeof(Cmdstr));
      InCursor = Cmdstr;
      retVal=1;
   }
   return retVal;   
}
/*
  Parse command string we got from serial interface.
*/
command parseCommand(char *cmdbuf)
{
   command retVal;
   
   char *thetok=strtok(cmdbuf," ");
#ifdef CHS_DEBUG
   Serial.print("thetok: [");
   Serial.print(thetok);
   Serial.println("]");
#endif
  if(0 == strcasecmp(thetok,"LEFT"))
      retVal.theDirection=Left;
   else if (0 == strcasecmp(thetok,"RIGHT"))
      retVal.theDirection=Right;
   else if (0 == strcasecmp(thetok,"UP"))
      retVal.theDirection=Up;
   else if (0 == strcasecmp(thetok,"DOWN"))
      retVal.theDirection=Down;
   else if (0 == strcasecmp(thetok,"CENTER"))
      retVal.theDirection=Center;
   if(retVal.theDirection != Center)
   {
      thetok=strtok(0," ");
      retVal.distance=atoi(thetok);
   }
   
   return retVal;
}

/*
Setup part of main 
*/ 
void setup() {
  Hservo.attach(HSERVOPIN);
  Vservo.attach(VSERVOPIN);
  Serial.begin(9600);
  memset(Cmdstr,0,(sizeof(Cmdstr)-1) );
  InCursor=Cmdstr;
  CmdSendingState=None;
  CurrentHPosition=HSERVOCENTER;
  CurrentVPosition=VSERVOCENTER;
  
}
/*
Move horizontal and vertical servos to new position specified in command.
*/
void moveServo(command whichWhere)
{

   switch(whichWhere.theDirection) {
      case Left:
      	 CurrentHPosition -= whichWhere.distance;
	 break;
      case Right:
         CurrentHPosition += whichWhere.distance;
	 break;
      case Up:
         CurrentVPosition -= whichWhere.distance;
	 break;
      case Down:
         CurrentVPosition += whichWhere.distance;
	 break;
      // case Center:
      default:
         CurrentHPosition=HSERVOCENTER;
	 CurrentVPosition=VSERVOCENTER;
         break;
   }
   // Limit current position.
   if(CurrentHPosition > HSERVOMAX)
      CurrentHPosition=HSERVOMAX;
   if(CurrentHPosition < HSERVOMIN)
      CurrentHPosition = HSERVOMIN;
   if(CurrentVPosition > VSERVOMAX)
      CurrentVPosition=VSERVOMAX;
   if(CurrentVPosition < VSERVOMIN)
      CurrentVPosition=VSERVOMIN;
      
#ifdef CHS_DEBUG
   Serial.print("theDirection: ");
   Serial.print(whichWhere.theDirection);
   Serial.print(" Distance: ");
   Serial.print(whichWhere.distance);
   Serial.print(" Writing HPosition: ");
   Serial.print(CurrentHPosition);
   Serial.print(" VPosition: " );
   Serial.print(CurrentVPosition);
   Serial.println(" ");
#endif

   Hservo.write(CurrentHPosition);
   delay(15);
   Vservo.write(CurrentVPosition);
   delay(15);
   
}
/*
  Loop part of main
*/
void loop() {

   command currentCommand;
   
   if(checkForCommand(CmdBuffer)) {
#ifdef CHS_DEBUG   
      Serial.print("Cmd Received: [");
      Serial.print(CmdBuffer);
      Serial.println(']');
#endif
      currentCommand = parseCommand(CmdBuffer);
#ifdef CHS_DEBUG
      Serial.print("currentCommand Direction: ");
      Serial.print(currentCommand.theDirection);
      Serial.print(" Distance: ");
      Serial.println(currentCommand.distance);
#endif      
      CmdSendingState=None;
      memset(CmdBuffer,0,sizeof(CmdBuffer));
      moveServo(currentCommand);
   }
   
}
