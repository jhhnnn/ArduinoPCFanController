//VERSI0 V1.00


//******Librarys*******
#include <LiquidCrystal.h>
//***Librarys end****

//*****Public Variables******
LiquidCrystal lcd( 13, 12, 3, 4, 5, 7 );
// set keys here. no, really, set them keys. they NEED to be set. *FIXME*
const byte UP = 2;
const byte DOWN = 5;
const byte SET = 6;
const byte BACK = 4;

// menus and their contents
const byte MAINMENUITEMS = 2;
const byte RPMMENUITEMS = 6;
const byte DYNMENUITEMS = 3;
const byte MINTEMPMENUITEMS = 3;
const byte MAXTEMPMENUITEMS = 3;
const byte ALARMTEMPMENUITEMS = 3;

const String mainmenu[ MAINMENUITEMS ] = { "Static mode", "Dynamic mode" };
const String rpmmenu[ RPMMENUITEMS ] = { "0 %", "20 %", "40 %", "60 %", "80 %", "100 %" };
const String dynmenu[ DYNMENUITEMS ] = { "Set min temp", "Set max temp", "Set alarm temp" };
const String mintempmenu[ MINTEMPMENUITEMS ] = { "20 C", "30 C", "40 C" };
const String maxtempmenu[ MAXTEMPMENUITEMS ] = { "50 C", "60 C", "70 C" };
const String alarmtempmenu[ ALARMTEMPMENUITEMS ] = { "60 C", "70 C", "80 C" };

// valuelists that correspond to menuindexes
const byte rpmvaluelist[ RPMMENUITEMS ] = { 30, 75, 120, 165, 210, 255 };  // converted RPM values 0 % - 100 %
const float mintempvaluelist[ MINTEMPMENUITEMS ] = { 20, 30, 40 };
const float maxtempvaluelist[ MAXTEMPMENUITEMS ] = { 50, 60, 70 };
const float alarmtempvaluelist[ ALARMTEMPMENUITEMS ] = { 60, 70, 80 };

// keyboard input
byte input = 255;

// starting menu values
byte rpm = RPMMENUITEMS - 1;
byte mintemp = MINTEMPMENUITEMS - 2;
byte maxtemp = MAXTEMPMENUITEMS - 2;
byte alarmtemp = ALARMTEMPMENUITEMS - 2;

// temporary variables
byte tmainmenu = 0;
byte tdyn = 0;
byte trpm = 0;
byte tmintemp = 0;
byte tmaxtemp = 0;
byte talarmtemp = 0;

// actual values (on valuelists)
float rpmvalue = rpmvaluelist[ RPMMENUITEMS - 1 ];
float mintempvalue = mintempvaluelist[ MINTEMPMENUITEMS - 2 ];
float maxtempvalue = maxtempvaluelist[ MAXTEMPMENUITEMS - 2 ];
float alarmtempvalue = alarmtempvaluelist[ ALARMTEMPMENUITEMS - 2 ];

// Static or dynamic mode
bool isstatic = false;
bool isAlarm = false;

class kp{//keypad class
  
  public:
  byte readNext(void);
  void save(byte value);
  byte update(void);
  
  private:
  bool kpPressed = false;
  bool kpFree = true;
  byte count;
  byte prevVal = 255;
  byte curVal;
  byte keypad[16] =     
    { 1,2,3,0xA,
      4,5,6,0xB, 
      7,8,9,0xC,
      0xD,0,0xE,0xF};;
  volatile byte lastPress = 255;
};

byte kp::readNext()
{
  byte val = lastPress;
  lastPress = 255;
  return val;
}
void kp::save(byte value)
{
 if(value < 16);
 lastPress = keypad[value];
}
byte kp::update()
{
kpPressed = false;//resets keypress boolean
byte col = 0;//keeps track of columns
  for(byte sweep = B00001110;!(sweep == B11101111);col++)//Sweeps lines to ground one by one
  {  
    PORTB = (PINB & B11110000) | (sweep & B00001111);//masks off port pins that arn't needes and sets port values
    byte row = 0;//keeps track of rows
    for(byte mask = B00000010;!(mask == B00100000);mask <<= 1)//checks rows one by one;
    {      
      if(mask != (mask & PINC))//gives true if tested key is pressed
      {
        curVal = row+col*4;//saves current keypress value
        kpPressed = true;//sets Key is pressed boolean so we know some key is pressed
        if(prevVal == curVal)// counts time
        {
          count++;
        }
        else // if no key or different key was pressed previously saves new value to prevVal and reset count
        {
          prevVal = curVal;
            count = 0;
        }
        if(kpFree && count > 1)// these conditions must be met to key to be saved 
        {
          save(curVal);//Save key press
            kpFree = false; //set kpFree to false so we know not take same keypress twice
        }
        
        break;// some key was pressed so breaks out to make function faster
      }
      row++;
    }
    
    if(kpPressed)//breaks the other loop when a key is pressed
    {
        break;
    }
    
    sweep <<= 1;// some bitwise magic
    sweep++; 
  }
  if(!kpPressed){ //reset key press values when no key press is detected
    prevVal == 255;
    kpFree = true;
    count = 0;
  }

  
}

kp keypad;

volatile unsigned long firstPulseTime;
volatile unsigned long lastPulseTime;
volatile unsigned long numPulses;
volatile unsigned long prev = 0;
volatile float temp;
volatile float sensor;

void isr()  //interrupt service routine
{
  
 unsigned long now = micros();    
 if( 1700 < now - prev )
 {
   if (numPulses == 1)
   {
     firstPulseTime = now;
   }
   else
   {
     lastPulseTime = now;
   }
   ++numPulses;
   prev = now;
 }
}

//****Public Variables end****

// Measure the frequency over the specified sample time in milliseconds, returning the frequency in Hz
float readFrequency()
{
 float readVal = (numPulses < 3) ? 0 : (1000000.0 * (float)(numPulses - 2))/(float)(lastPulseTime - firstPulseTime);//if (numpulses<3){return 0}; else{1000000*pulssien määrä - 2/kuinka kauan pulsseja on mitattu mikrosekunneissa=taajuus}
 numPulses = 0;                      // prime the system to start a new reading
 return readVal; 
}

ISR(TIMER1_OVF_vect){//Timer1 OVERFLOW callback
  keypad.update();
  if(isstatic)
  {
    analogWrite(6,rpmvalue);
  }else{
    temp = (23.0/212.0)*sensor;
    if(temp > alarmtempvalue)
      {
        isAlarm = true;
        analogWrite(6,255);
      }else
      {
       if(temp < mintempvalue)
       {
        analogWrite(6,0); 
       }else{
         if(temp > maxtempvalue)
         {
          analogWrite(6,255);
         }else
         {
          analogWrite(6,90); 
         }
       }
      }
      
  }
}


void setup() {
  lcd.begin( 16, 2 );
  lcd.setCursor( 0, 0 );
  Serial.begin(9600);//Serial fo debugging
  attachInterrupt(digitalPinToInterrupt(2), isr,FALLING);
  //****PORT SETTINGS********
  DDRC = B00000000;//PortC inputs 
  DDRB = B11111111;//PortB outputs
  PORTB = B00000000;//PortB output 0
  PORTC = B11111111;//PortC pullups on
  //****PORT SETTINGS END****
  
  //****TIMER SETTINGS*******
  noInterrupts();//stop interrupts
  //set timer2 interrupt at 61Hz(0,6ms)
  TCCR1A = 0;// set entire TCCR2A register to 0
  TCCR1B = 0;// same for TCCR2B
  TCNT1  = 0;//initialize counter value to 0
  TCCR1B |= (1 << CS10);//CS10 bit for no prescaler   
  TIMSK1 |= (1 << TOIE2);// enable overflow interrupt
  interrupts();//allow interrupts
  //****TIMER SETTINGS END***
}

void loop() {
  sensor = ReadADC(0); //käydään kääntämässä signaali  
  Serial.println(sensor*(23.0/212.0));
  int a7Value = analogRead(A7)/4;
  //analogWrite(6,a7Value);
  float freq = readFrequency();  //mittaa taajuutta sekunnin ajan
  Serial.print(a7Value);
  Serial.print(" ");
  Serial.print((freq*60)/2);
  Serial.print("\n");
  //LCD PRINT START
  lcd.clear();
  lcd.print("TEMP: ");
  lcd.print(sensor*(23.0/212.0));
  lcd.setCursor(14,0);
  if(isstatic)
  {
    lcd.print("SM");  
  }else{
    lcd.print("DM");
  }
  lcd.setCursor(0,1);
  lcd.print("RPM: ");
  lcd.print(round((freq*60)/2));
  if(isAlarm)
  {
    lcd.setCursor(11,1);
    lcd.print("ALARM");  
  }
  //LCD PRINT ENDS
  
  
  input = keypad.readNext();
  delay(100);
  Serial.println(input == SET);
  if(input == SET){
  // Main menu
  input = '0';
  while( input != BACK && input != SET )
  {
    lcd.clear();
    lcd.print( "Main menu" );
    lcd.setCursor( 0, 1 );
    lcd.print( mainmenu[ tmainmenu ] );
    input = keypad.readNext();
    delay(100);
    if( input == UP || input == DOWN )
    {
      if( tmainmenu == 0 )        // if one, show other (duh)
        tmainmenu = 1;
      else
        tmainmenu = 0;
    }

    if( input == SET )
    {
      if( tmainmenu == 0 )        // descend to static (RPM) menu
      {
        // RPM menu
        input = '0';        // reset input variable so while doesn't immediately exit
        trpm = rpm;
        while( input != BACK && input != SET )
        {
          lcd.clear();
          lcd.print( "Set static RPM" );
          lcd.setCursor( 0, 1 );
          lcd.print( rpmmenu[ trpm ] ); // set this somewhere
          input = keypad.readNext();
          delay(100);
          if( input == UP )
          {
            if( trpm == 0 )       // if at top, go to bottom
              trpm = RPMMENUITEMS - 1;
            else
              trpm--;
          }

          if( input == DOWN )
            if( trpm == RPMMENUITEMS - 1 )    // if at bottom, go to top
              trpm = 0;
            else
              trpm++;

          if( input == SET )
          {
            rpm = trpm; // save trpm to rpm variable (set new RPM) and set isstatic to true
            isstatic = true;
            rpmvalue = rpmvaluelist[ rpm ];
          }

          if( input == BACK )
            trpm = rpm; // discard changes (do nothing to RPM)
        }
        input = '0';        // reset input so parent while() doesn't get upset
      }

      if( tmainmenu == 1 )        // descend to dynamic menu
      {
        // Dynamic menu
        input = '0';        // reset input variable so while doesn't immediately exit
        while( input != BACK && input != SET )
        {
          lcd.clear();
          lcd.print( "Dynamic mode" );
          lcd.setCursor( 0, 1 );
          lcd.print( dynmenu[ tdyn ] ); // set this somewhere
          
          input = keypad.readNext();
          delay(100);
          if( input == UP )
          {
            if( tdyn == 0 )       // if at top, go to bottom
              tdyn = DYNMENUITEMS - 1;
            else
              tdyn--;
          }

          if( input == DOWN )
            if( tdyn == DYNMENUITEMS - 1 )    // if at bottom, go to top
              tdyn = 0;
            else
              tdyn++;

          if( input == SET )
          {
            if( tdyn == 0 )     // descend to min temp menu
            {
              // Min temp menu
              input = '0';        // reset input variable so while doesn't immediately exit
              tmintemp = mintemp;
              while( input != BACK && input != SET )
              {
                lcd.clear();
                lcd.print( "Set min temp" );
                lcd.setCursor( 0, 1 );
                lcd.print( mintempmenu[ tmintemp ] ); // set this somewhere
            
                input = keypad.readNext();
                delay(100);
                if( input == UP )
                {
                  if( tmintemp == 0 )       // if at top, go to bottom
                    tmintemp = MINTEMPMENUITEMS - 1;
                  else
                    tmintemp--;
                }

                if( input == DOWN )
                  if( tmintemp == MINTEMPMENUITEMS - 1 )    // if at bottom, go to top
                    tmintemp = 0;
                  else
                    tmintemp++;

                if( input == SET )
                {
                  mintemp = tmintemp; // save tmintemp to mintemp variable (set new min temperature)
                  isstatic = false;
                  mintempvalue = mintempvaluelist[ mintemp ];
                }

                if( input == BACK )
                  tmintemp = mintemp; // discard changes (do nothing to min temperature)
              }
              input = '0';        // reset input so parent while() doesn't get upset
            }

            if( tdyn == 1 )
            {
              // Max temp menu
              input = '0';        // reset input variable so while doesn't immediately exit
              tmaxtemp = maxtemp;
              while( input != BACK && input != SET )
              {
                lcd.clear();
                lcd.print( "Set max temp" );
                lcd.setCursor( 0, 1 );
                lcd.print( maxtempmenu[ tmaxtemp ] ); // set this somewhere
            
                input = keypad.readNext();
                delay(100);
                if( input == UP )
                {
                  if( tmaxtemp == 0 )       // if at top, go to bottom
                    tmaxtemp = MAXTEMPMENUITEMS - 1;
                  else
                    tmaxtemp--;
                }

                if( input == DOWN )
                  if( tmaxtemp == MAXTEMPMENUITEMS - 1 )    // if at bottom, go to top
                    tmaxtemp = 0;
                  else
                    tmaxtemp++;

                if( input == SET )
                {
                  maxtemp = tmaxtemp; // save tmaxtemp to maxtemp variable (set new max temperature)
                  isstatic = false;
                  maxtempvalue = maxtempvaluelist[ maxtemp ];
                }

                if( input == BACK )
                  tmaxtemp = maxtemp; // discard changes (do nothing to max temperature)
              }
              input = '0';        // reset input so parent while() doesn't get upset
            }

            if( tdyn == 2 )
            {
              // Alarm temp menu
              input = '0';        // reset input variable so while doesn't immediately exit
              talarmtemp = alarmtemp;
              while( input != BACK && input != SET )
              {
                lcd.clear();
                lcd.print( "Set alarm temp" );
                lcd.setCursor( 0, 1 );
                lcd.print( alarmtempmenu[ talarmtemp ] ); // set this somewhere
            
                input = keypad.readNext();
                delay(100);
                if( input == UP )
                {
                  if( talarmtemp == 0 )       // if at top, go to bottom
                    talarmtemp = ALARMTEMPMENUITEMS - 1;
                  else
                    talarmtemp--;
                }

                if( input == DOWN )
                  if( talarmtemp == ALARMTEMPMENUITEMS - 1 )    // if at bottom, go to top
                    talarmtemp = 0;
                  else
                    talarmtemp++;

                if( input == SET )
                {
                  alarmtemp = talarmtemp;   // save talarmtemp to alarmtemp variable (set new alarm temperature)
                  isstatic = false;
                  alarmtempvalue = alarmtempvaluelist[ alarmtemp ];
                }

                if( input == BACK )
                  talarmtemp = alarmtemp;   // discard changes (do nothing to alarm temperature)
              }
              input = '0';        // reset input so parent while() doesn't get upset
            }
          }
          if( input == BACK );
        }
        input = '0';        // reset input so parent while() doesn't get upset
      }
    }
    if( input == BACK );
  }
  }
}

int ReadADC(int vayla)
{
  ADMUX = B00000001 & vayla;       // Väylän valinta muxista
  ADCSRA |= _BV(ADSC);                //Asetetaan ADSC-bitti = 1 eli aloitetaan kääntäminen
  while(!bit_is_set(ADCSRA,ADIF));    // Silmukassa kunnnes ADIF = 1 
  ADCSRA |= _BV(ADIF);                // Asetetaan ADIF = 1, nollataan
  return(ADC);
}



