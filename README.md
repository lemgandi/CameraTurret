# Camera Turret Control 

This is code to control a 2-servo camera turret with the Arduino from a serial interface.

## Protocol

```
Move ::= '^' Direction ' ' Size '/'
Direction ::= {'left' || 'right' || 'up' || 'down' || 'center'}
Size ::= [0-9]+
```
So "^center/" will center the turret.  "^up 20/" moves it up 20 servo steps.  "^left 40/" moves it
left 40 servo steps.

Serial interface is currently defined as the serial on pins 1 & 2 8-1-N 9600 baud.  You can test
this code by writing commands through the Arduino GUI serial monitor tool.




