//MakeIdea Hemant
#include <avr/io.h>
#include <avr/interrupt.h>

#define LookupEntries (512)

static int microMHz = 16;
static int freq; 
static long int period;  
static unsigned int lookUp[LookupEntries];
static char theTCCR1A = 0b10000010;
static unsigned long int phaseinc, switchFreq;
static double phaseincMult;

int setFreq(int _freq);
int setSwitchFreq(int sfreq);
void registerInit(void);


void setup(){ 
  Serial.begin(9600);
  setSwitchFreq(10000);  
  setFreq(50);
  registerInit();
}

void loop(){
  
  int sensorValue = analogRead(A0);
  static int sensorValue2;
  if(sensorValue > sensorValue2*1.01 || sensorValue < sensorValue2*0.99)
  {
    sensorValue2 = sensorValue;
    setFreq(map(sensorValue, 0, 1023, 5, 300));
    
    Serial.print(phaseinc>>23);
    Serial.print(".");
    Serial.print(phaseinc&0x007FFFFF);
    Serial.print("\n");   
  }
}

ISR(TIMER1_OVF_vect){
  static unsigned long int phase, lastphase;
  static char delay1, trig = LOW;

  phase += phaseinc;

  if(delay1 > 0)
  {
    theTCCR1A ^= 0b10100000;
    TCCR1A = theTCCR1A;
    delay1 = 0;  
  } 
  else if((phase>>31 != lastphase>>31) && !(phase>>31)){ 
    delay1++;      
    trig = !trig;
    digitalWrite(13,trig); 
  }
  lastphase = phase;
  
  OCR1A = OCR1B = lookUp[phase >> 23];
}

int setFreq(int _freq){
  if(_freq < 0 || _freq > 1000)
  { 
    return 0;
  } else {
    freq = _freq;
    phaseinc = (unsigned long int) phaseincMult*_freq;
    return 1; 
  }
}

int setSwitchFreq(int sfreq){
  double temp;
  
  if(sfreq <= 0 || sfreq > 20000){
    return 0;
  }
  else
  {
    switchFreq = sfreq;
    period = microMHz*1e6/sfreq;
    cli(); 
    TCCR1A = 0b00000010; 
    ICR1   = period;
    for(int i = 0; i < LookupEntries; i++)
    { 
      temp = sin(i*M_PI/LookupEntries)*period;
      lookUp[i] = (int)(temp+0.5);           
    }
    phaseincMult = (double) period*8589.934592/microMHz;
    phaseinc = (unsigned long int) phaseincMult*freq;
    TCCR1A = theTCCR1A; 
    sei(); 
    return 1;
  }
}
void registerInit(void){
  
  TCCR1A = theTCCR1A; 
  TCCR1B = 0b00011001;
  TIMSK1 = 0b00000001;
  
  sei();             
  
  DDRB   = 0b00000110; 
  pinMode(13, OUTPUT);  
}
    
