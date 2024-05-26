//
// Touch-ER
// Un'interfaccia simile a Makey Makey basata su Blue Pill
//
// conti@istruzioneer.gov.it - maurizio.conti@fablabromagna.org
// 25 maggio 2024
//

// Riferimenti alle board utilizzate nell'IDE di Arduino
// http://dan.drown.org/stm32duino/package_STM32duino_index.json
// https://github.com/earlephilhower/arduino-pico/releases/download/global/package_rp2040_index.json
// https://github.com/stm32duino/BoardManagerFiles/raw/main/package_stmicroelectronics_index.json

/// Nota //////////////////////////////
// Utilizzare un STM32 dotato di compatibile con il pin no-grounding 
#define ADCTOUCH_INTERNAL_GROUNDING

// Nota //////////////////////////////
// Nel mio PC è stato necessario modificare il file 
// C:\Users\maurizio\AppData\Local\Arduino15\packages\STMicroelectronics\hardware\stm32\2.7.1\system\Middlewares\ST\STM32_USB_Device_Library\Class\HID\Inc\usbd_hid.h
// 
// ho cambiato il valore della #define HID_MOUSE_REPORT_DESC_SIZE
// dal valore 74U al valore 63U
 
// Nota //////////////////////////////
// La velocità in Mhz della scheda va settata su "Normal" 72MHz
// Su Arduino IDE 1.8.15  il default è 72MHz
// Su Arduino IDE 2       il default è 48MHz

// Libreria ADCTouchSensor
// https://github.com/arpruss/ADCTouchSensor
#include <ADCTouchSensor.h>                                  
#define QUANTI_PIEDINI  8
ADCTouchSensor* capacitiveInput[QUANTI_PIEDINI];

// Libreria USBHID (by Varius)
// https://github.com/arpruss/USBComposite_stm32f1/tree/master
#include <USBComposite.h>

USBHID HID;                              // Istanzia un plugin USBHID
HIDKeyboard Keyboard(HID);               // crea tutte le istanze dei profili
HIDMouse Mouse(HID);                     // in setup, faremo HID.begin() per far partire tutto
USBMIDI midi;
USBCompositeSerial CompositeSerial;

#define LED_BUILTIN PB12 // Il led a bordo
#define LED1 PB13        // Il mio led esterno
#define P1 PB14          // Il mio pulsante
#define THRESHOLD 20      // Il valore minimo di trigger

// Solo i PIN Analogici possono essere trasformati in ingressi touch capacitivi
unsigned pins[] = { PA0, PA1, PA2, PA3, PA4, PA5, PA6, PA7, PB0, PB1 };

// i tasti che vengono generati non sono ordinati per esigenze di cablaggio (ho attaccato i fili come mi rimaneva più comodo per saldarli)
unsigned tastiera[] = { KEY_RIGHT_ARROW, 'w', KEY_LEFT_ARROW, 'a', KEY_UP_ARROW, 's', ' ', KEY_DOWN_ARROW, 'd', '.' }; 

// le note MIDI suonate
//                              C   D   E   F   G   A   B   C   C#  D#  F#  G#  A#
const uint8_t tastieraMidi[] = {60, 62, 64, 65, 67, 69, 71, 72, 61, 63, 66, 68, 70};
bool statoMidi[QUANTI_PIEDINI];

// Determina se usiamo la seriale o il midi
bool isMidiOn = false;

void taraturaAnalogica() {
    for (int idx=0; idx<QUANTI_PIEDINI; idx++) {
        if( capacitiveInput[idx] )  // Teniamo d'occhio i memory leaks... non siamo in C#!!!
          delete capacitiveInput[idx];

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

void gestioneTastieraMouseSeriale() {

    for (int idx=0; idx<QUANTI_PIEDINI; idx++) {
      int val = capacitiveInput[idx]->read();
      if( val < 0)
          val = 0;

      CompositeSerial.print( val );

      if (val > THRESHOLD) {
        LedOn();
        delay(40);
        CompositeSerial.print( "*\t" );

/*
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
*/
        LedOff();
      }
      else
        CompositeSerial.print( "\t" );

    }

    CompositeSerial.println( "-" );
    delay(100);
}

void gestioneMidi() {
  for (int idx=0; idx<QUANTI_PIEDINI; idx++) {
    if (capacitiveInput[idx]->read() > THRESHOLD) {
      if( !statoMidi[idx] ) {
        statoMidi[idx]=true;
        LedOn();
        midi.sendNoteOn(0, tastieraMidi[idx], 127);
      }
    }
    else
    {
      if( statoMidi[idx] ) {
       statoMidi[idx]=false;
       midi.sendNoteOff(0, tastieraMidi[idx], 0);
       LedOff();
      }
    }
  }
}

void setup() 
{
    // I miei PIN
    pinMode( LED_BUILTIN, OUTPUT );
    pinMode( LED1, OUTPUT );
    pinMode( P1, INPUT_PULLUP );

    if( !digitalRead(P1) )
    {
      while( !digitalRead(P1) );
      
      // Usiamo midi
      isMidiOn = true;
      midi.begin(); 
      
      blinkLed();
    }
    else {
      // Usiamo seriale
      isMidiOn = false;

      Keyboard.begin(); 
      blinkLed();
      Mouse.begin(); 
      blinkLed();

      HID.begin(CompositeSerial);
      while (!USBComposite) ;
      blinkLed();
    }
        

    // Accendo il midi  
    //midi.begin();
    //blinkLed();
    //HID.begin(CompositeSerial);
    //while (!USBComposite) ;
    //blinkLed();

    // Accendo Tastiera e mouse
    //Keyboard.begin(); 
    //blinkLed();
    //Mouse.begin(); 
    //HID.registerComponent();


    // Si assicura che l'uscita sia off prima di configurarla
    LedOff();    

    delay( 500 );

    while( digitalRead( P1 ) ) {
      LedOn();
    }
    taraturaAnalogica();

    LedOff();    
 } 


void loop() 
{
    uint8_t pressedIndicator = 0;
    
    if( isMidiOn )
    {
      gestioneMidi();
    }
    else
    {
      gestioneTastieraMouseSeriale();
    }

    if( !digitalRead(P1) ) {
      while( !digitalRead(P1) ) ;
      taraturaAnalogica();     
      blinkLed();
    }
}
