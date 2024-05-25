
//Nota 1
//Utilizzare un STM32 dotato di compatibile con il pin no-grounding 
#define ADCTOUCH_INTERNAL_GROUNDING

/*
Nota 2
Nel mio PC è stato necessario eseguire questa modifica

Nel file 
C:\Users\maurizio\AppData\Local\Arduino15\packages\STMicroelectronics\hardware\stm32\2.7.1\system\Middlewares\ST\STM32_USB_Device_Library\Class\HID\Inc\usbd_hid.h

ho cambiato il valore della define
#define HID_MOUSE_REPORT_DESC_SIZE 63U

da 74U
a 63U

La velocità in Mhz della scheda va settata su "Normal" 72MHz
Su Arduino IDE 1.8.15  il default è 72MHz
Su Arduino IDE 2       il default è 48MHz

*/

// Dagli esempi della libreria USBComposite di STM32Duino
#include <ADCTouchSensor.h>                                  
#include <USBComposite.h> 
                                                                                                                                                                                   
USBHID HID;
HIDKeyboard Keyboard(HID);
HIDMouse Mouse(HID);
USBCompositeSerial CompositeSerial;

#define LED_BUILTIN PB12 // Il led a bordo
#define LED1 PB13        // Il mio led esterno
#define P1 PB14          // Il mio pulsante

#define QUANTI_PIEDINI  10
unsigned pins[QUANTI_PIEDINI] = {PA0,PA1,PA2,PA3,PA4,PA5,PA6,PA7,PB0,PB1};

unsigned tastiera[QUANTI_PIEDINI] = { KEY_RIGHT_ARROW, 'w', KEY_LEFT_ARROW, 'a', KEY_UP_ARROW, 's', ' ', KEY_DOWN_ARROW, 'd', '.' }; 
//unsigned tastiera[QUANTI_PIEDINI] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'l'};
 
ADCTouchSensor* capacitiveInput[QUANTI_PIEDINI];

void taraturaAnalogica() {
    for (int idx=0; idx<QUANTI_PIEDINI; idx++) {
        capacitiveInput[idx] = new ADCTouchSensor(pins[idx]);
        capacitiveInput[idx]->begin();
    }
}

void LedOn() {
  digitalWrite(LED_BUILTIN, 0);     
  digitalWrite(LED1, 1);     
}

void LedOff() {
  digitalWrite(LED_BUILTIN, 1);     
  digitalWrite(LED1, 0);     
}


void blinkLed()
{
  LedOn();
  delay( 200 );
  LedOff();
  delay( 200 );
}


void setup() 
{
    // Si assicura che l'uscita sia off prima di configurarla
    LedOff();    
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(LED1, OUTPUT);

	  // Accende la porta USB, e aspetta un esito OK
    HID.begin(CompositeSerial);
    while (!USBComposite)
      ;
      
    Keyboard.begin(); 
    blinkLed();
    Mouse.begin(); 
    blinkLed();

    delay( 500 );

    pinMode(P1, INPUT_PULLUP);
    while( digitalRead( P1 ) ) {
      LedOn();
    }
    taraturaAnalogica();

    LedOff();    
 } 


void loop() 
{
    uint8_t pressedIndicator = 0;
    
    for (int idx=0; idx<QUANTI_PIEDINI; idx++) {
      CompositeSerial.print( capacitiveInput[idx]->read() );
      CompositeSerial.print( "\t" );

      if (capacitiveInput[idx]->read() > 50) {
        LedOn();
        if ( idx==9 ) {
          // Il decimo è il click del mouse
          Mouse.press(MOUSE_LEFT);
          delay(40);
          Mouse.release(MOUSE_ALL);
          delay(20);
        }
        else{
         Keyboard.press(tastiera[idx]);
         delay(40);
         Keyboard.release(tastiera[idx]);
         delay(20);
        }
        LedOff();
      }
    }

    CompositeSerial.println( "-" );
    delay(100);

    if( !digitalRead(P1) ){
      while( !digitalRead(P1) )
      ;

      taraturaAnalogica();     
      LedOn();
      delay(500);
      LedOff();
    }
}