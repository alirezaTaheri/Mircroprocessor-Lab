/*
 * IR_Sernsor.c
 *
 * Created: 27/10/2019 08:32:34 �.�
 * Author: Alireza
 */

#include <io.h>
#include <alcd.h> 
#include <delay.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <mega16.h>
 
// Voltage Reference: AREF pin
#define ADC_VREF_TYPE ((0<<REFS1) | (0<<REFS0) | (0<<ADLAR))
char *menuItems[] = {"Show ADC Value", "Calibration", "Show ADC Voltage"};
char *adcMenuItems[] = {"ADC 1", "ADC 2", "ADC 3"};
char *voltageMenuItems[] = {"ADC 1", "ADC 2", "ADC 3"};
char *calibrationMenuItems[] = {"ADC 1", "ADC 2", "ADC 3"};
int floorValues[3] = {300, 300, 300};
int adcLastValue[3] = {0, 0, 0};
int selectedItem = -1;
int showingItem = 0;
int adcSelectedItem = -1;
int adcShowingItem = 0;
int calibrationSelectedItem = -1;
int calibrationShowingItem = 0;
int nextKeyPressedCount = 0;
bool nextKeyPressed = false;
int confirmKeyPressedCount = 0;
bool confirmKeyPressed = false;

void init();
void showADCValue(int);
void showStringOnRow(int, char *string);
void handleLED(int channel);
bool isNextKeyDebounced();
bool isConfirmKeyDebounced();

// Timer 0 overflow interrupt service routine
interrupt [TIM0_OVF] void timer0_ovf_isr(void)
{
// Reinitialize Timer 0 value
TCNT0=0x64;
// Place your code here
if (nextKeyPressed) nextKeyPressedCount++;
if (confirmKeyPressed) confirmKeyPressedCount++;
if (isNextKeyDebounced()){
   switch (selectedItem){
    case -1 :         
    showingItem++;
    if (showingItem > 2) showingItem = 0;
    showStringOnRow(1, menuItems[showingItem]);
    break;
    case 0:
    adcShowingItem++;
    if (adcShowingItem > 2) adcShowingItem = 0;
    showStringOnRow(1, adcMenuItems[adcShowingItem]);    
    
    break;
    case 1:
    calibrationShowingItem++;
    if (calibrationShowingItem > 2) calibrationShowingItem = 0;
    showStringOnRow(1, calibrationMenuItems[calibrationShowingItem]);  
    
    break;
    case 2:
    break;
    default:break;
    } 
} if (isConfirmKeyDebounced()){
   switch (selectedItem){
    case -1 :
    selectedItem = showingItem; 
    showStringOnRow(0, menuItems[selectedItem]);
    showStringOnRow(1, adcMenuItems[adcShowingItem]);
    break;
    case 0:
    if (adcSelectedItem != -1){
        adcSelectedItem = -1;
        adcShowingItem = 0;
        selectedItem = -1;
        showingItem = 0;
        showStringOnRow(0, "Please Select");
        showStringOnRow(1, menuItems[showingItem]); 
    }
    else{    
    adcSelectedItem = adcShowingItem;
    showStringOnRow(0,"Value:");
    }
    break;
    case 1:
    if (calibrationSelectedItem != -1){
        floorValues[calibrationSelectedItem] = adcLastValue[calibrationSelectedItem];
        calibrationSelectedItem = -1;
        calibrationShowingItem = 0;
        selectedItem = -1;
        showingItem = 0;
        showStringOnRow(0, "Please Select");
        showStringOnRow(1, menuItems[showingItem]);
        }else{
    calibrationSelectedItem = calibrationShowingItem;
    showStringOnRow(0,"Confirm To Save");
              }
    break;
    case 2:
    break;
    default:break;
    } 
}
}

// External Interrupt 0 service routine
interrupt [EXT_INT0] void ext_int0_isr(void)
{   if(PIND.2 == 1){
    nextKeyPressed = true;
    
    }else{
    nextKeyPressed = false;
    nextKeyPressedCount = 0;
    }
}


    char title[16];
    char adcselectedString[1];
// External Interrupt 1 service routine
interrupt [EXT_INT1] void ext_int1_isr(void)
{   if (PIND.3 == 1){
   confirmKeyPressed = true;
    
    }else{
    confirmKeyPressed = false;
    confirmKeyPressedCount = 0;
    }
}
// Read the AD conversion result
unsigned int read_adc(unsigned char adc_input)
{
ADMUX=adc_input | ADC_VREF_TYPE;
// Delay needed for the stabilization of the ADC input voltage
delay_us(10);
// Start the AD conversion
ADCSRA|=(1<<ADSC);
// Wait for the AD conversion to complete
while ((ADCSRA & (1<<ADIF))==0);
ADCSRA|=(1<<ADIF);
return ADCW;
}

void main(void)
{
  init();
  showStringOnRow(0, "Please Select");
  showStringOnRow(1, menuItems[showingItem]);
// Global enable interrupts
#asm("sei")
while (1)
    {
    switch(selectedItem){
    case 0 :
    if (adcSelectedItem != -1){
        showADCValue(adcSelectedItem);
        }
    break;
    case 1 :   
    if (calibrationSelectedItem != -1){
        showADCValue(calibrationSelectedItem);
        }
    break;
    case 2 :
    break;
    default:break;
    }  
    }
    }
    void showADCValue(int channel){
    char string[5];
    lcd_gotoxy(0, 1);
    lcd_puts("                ");
    adcLastValue[channel] = read_adc(channel);
    itoa(adcLastValue[channel], string);
    showStringOnRow(1, string);
    handleLED(channel);
    delay_ms(10);
    }
    
    void showStringOnRow(int row, char *string){
        lcd_gotoxy(0, row);
        lcd_puts("                ");
        lcd_gotoxy(0, row);
        lcd_puts(string);    
    }
    void handleLED(int channel){
       switch(channel){
        case 0:
        if (adcLastValue[channel] > floorValues[channel])
            PORTC.0 = 1; else PORTC.0 = 0;
        break;
        case 1:
        if (adcLastValue[channel] > floorValues[channel])
            PORTC.1 = 1; else PORTC.1 = 0;
        break;
        case 2:
        if (adcLastValue[channel] > floorValues[channel])
            PORTC.2 = 1; else PORTC.2 = 0;
        break;
       }
    }
    
    bool isNextKeyDebounced(){
        return nextKeyPressed && nextKeyPressedCount == 20;
    }
    
    bool isConfirmKeyDebounced(){
        return confirmKeyPressed && confirmKeyPressedCount == 20;
    } 
    
    void init(){
    DDRC = 0xff;
    
DDRD=(1<<DDD7) | (0<<DDD6) | (0<<DDD5) | (0<<DDD4) | (0<<DDD3) | (0<<DDD2) | (0<<DDD1) | (0<<DDD0);
// State: Bit7=T Bit6=T Bit5=T Bit4=T Bit3=T Bit2=T Bit1=T Bit0=T 
PORTD=(1<<PORTD7) | (0<<PORTD6) | (0<<PORTD5) | (0<<PORTD4) | (0<<PORTD3) | (0<<PORTD2) | (0<<PORTD1) | (0<<PORTD0);
    PORTD.0 = 1;
// ADC initialization
// ADC Clock frequency: 125/000 kHz
// ADC Voltage Reference: AREF pin
// ADC Auto Trigger Source: ADC Stopped
ADMUX=ADC_VREF_TYPE;
ADCSRA=(1<<ADEN) | (0<<ADSC) | (0<<ADATE) | (0<<ADIF) | (0<<ADIE) | (1<<ADPS2) | (1<<ADPS1) | (0<<ADPS0);
SFIOR=(0<<ADTS2) | (0<<ADTS1) | (0<<ADTS0);
//ADCSRA = 0x86;
//ADMUX = ADC_VREF_TYPE & 0xff;

// External Interrupt(s) initialization
// INT0: On
// INT0 Mode: Any change
// INT1: On
// INT1 Mode: Any change
// INT2: Off
GICR|=(1<<INT1) | (1<<INT0) | (0<<INT2);
MCUCR=(0<<ISC11) | (1<<ISC10) | (0<<ISC01) | (1<<ISC00);
MCUCSR=(0<<ISC2);
GIFR=(1<<INTF1) | (1<<INTF0) | (0<<INTF2);
    // Timer/Counter 0 initialization
// Clock source: System Clock
// Clock value: 31/250 kHz
// Mode: Normal top=0xFF
// OC0 output: Disconnected
// Timer Period: 4/992 ms
TCCR0=(0<<WGM00) | (0<<COM01) | (0<<COM00) | (0<<WGM01) | (1<<CS02) | (0<<CS01) | (0<<CS00);
TCNT0=0x64;
OCR0=0x00;

// Timer(s)/Counter(s) Interrupt(s) initialization
TIMSK=(0<<OCIE2) | (0<<TOIE2) | (0<<TICIE1) | (0<<OCIE1A) | (0<<OCIE1B) | (0<<TOIE1) | (0<<OCIE0) | (1<<TOIE0);

lcd_init(16);
    }
