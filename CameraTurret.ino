/*
 *  Arduino Camera Turret Control
 *
 *  Charles Shapiro Mar 2018
 */
#include <Servo.h>
#include <string.h>

#define CHS_DEBUG

#define HSERVOPIN 10
#define VSERVOPIN 9
#define HSERVOMIN 10
#define HSERVOMAX 170
#define VSERVOMIN 60
#define VSERVOMAX 170
#define HSERVOCENTER ((HSERVOMAX - HSERVOMIN) / 2)
#define VSERVOCENTER ((VSERVOMAX - VSERVOMIN) / 2)

Servo Hservo;
Servo Vservo;

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
}
void moveServo(command whichWhere)
{
   Servo *servoP;
   static int currentHDistance;
   static int currentVDistance;
   int mappedDistance;
   
   if((whichWhere.theDirection == Right) || (whichWhere.theDirection == Left))
      servoP=&Hservo;
   else
      servoP=&Vservo;
      
   if((whichWhere.theDirection == Left) || (whichWhere.theDirection == Down))
      whichWhere.distance=-whichWhere.distance;
   switch(whichWhere.theDirection) {
      case Left:      
      case Right:
         currentHDistance += whichWhere.distance;
	 break;
      case Up:
      case Down:
         currentVDistance += whichWhere.distance;
	 break;
      // case Center:
      default:
         break;
   }
#ifdef CHS_DEBUG
   if (servoP == &Hservo)
      Serial.print("servo: Hservo ");
   else
      Serial.print("servo: Vservo ");
   Serial.print("theDirection: ");
   Serial.print(whichWhere.theDirection);
   Serial.print(" Distance: ");
   Serial.print(whichWhere.distance);
   Serial.print(" currentHDistance: ");
   Serial.print(currentHDistance);
   Serial.print(" currentVDistance: ");
   Serial.print(currentVDistance);
   Serial.println(" ");
#endif
   if(whichWhere.theDirection != Center)
      servoP->write(whichWhere.distance);
   else
   {
#ifdef CHS_DEBUG
   Serial.print("Writing HDistance: ");
   Serial.print(-currentHDistance);
   Serial.print(" VDistance: " );
   Serial.print(-currentVDistance);
   Serial.println(" ");
#endif
      Hservo.write(HSERVOCENTER);
      Vservo.write(VSERVOCENTER);

      currentHDistance = currentVDistance = 0;
   }
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
