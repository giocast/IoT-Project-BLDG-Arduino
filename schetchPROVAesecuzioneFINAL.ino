#include <ArduinoBLE.h>
#include <Arduino_LSM9DS1.h>

//----------------------------------------------------------------------------------------------------------------------
// BLE UUIDs
//----------------------------------------------------------------------------------------------------------------------

#define BLE_UUID_BLDG_SERVICE               "9A48ECBA-2E92-082F-C079-9E75AAE428B1"

#define BLE_UUID_COUNTER                    "1A3AC130-31EE-758A-BC50-54A61958EF81"
#define BLE_UUID_EXPOSITION              "FE4E19FF-B132-0099-5E94-3FFB2CF07940"

//----------------------------------------------------------------------------------------------------------------------
// BLE
//----------------------------------------------------------------------------------------------------------------------

BLEService testService( BLE_UUID_BLDG_SERVICE );
BLEUnsignedLongCharacteristic counterCharacteristic( BLE_UUID_COUNTER, BLERead | BLENotify );
BLEUnsignedLongCharacteristic expositionCharacteristic( BLE_UUID_EXPOSITION, BLERead | BLENotify );


const int BLE_LED_PIN = LED_BUILTIN;
const int RSSI_LED_PIN = LED_PWR;


void setup()
{
  Serial.begin( 9600 );
  //  while ( !Serial );

  pinMode( BLE_LED_PIN, OUTPUT );
  pinMode( RSSI_LED_PIN, OUTPUT );


  if ( !IMU.begin() )
  {
    Serial.println( "Failed to initialize IMU!" );
    while ( 1 );
  }
  
  if( setupBleMode() )
  {
    digitalWrite( BLE_LED_PIN, HIGH );
    
 
  }
} // setup


void loop()
{
  static unsigned long counter = 0;
  static unsigned long exposition = 0;
  static long previousMillis = 0;
  static long prova = 0; //0 stampo prima serie e 1 stampo seconda serie e 2 stampo terza serie
  static long countDiCambioLoop = 0; //quando raggiunge 6 switcha a prova = 1 e così via
  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();

  if ( central )
  {
    Serial.print( "Connected to central: " );
    Serial.println( central.address() );

    
    while ( central.connected() )
    {
      
      long interval = 2000; 
      unsigned long currentMillis = millis();
      if( currentMillis - previousMillis > interval )
      {
        previousMillis = currentMillis;

        Serial.print( "Central RSSI: " );
        Serial.println( central.rssi() );
        delay(500); //aggiunto per regolare timing con wipy
        if( central.rssi() != 0 )
        {
          digitalWrite( RSSI_LED_PIN, LOW );
          //prelevo dati da sensore (ad un certo pin dell scheda Nano) e setto counter identificativo della misurazione e li scrivo
          //char Str4[] = "arduino";
          counter++; //inizializzato a 0, incrementato
          Serial.println(counter);
          counterCharacteristic.writeValue( counter );

          if(countDiCambioLoop == 6) {
            prova = 1;
          } else if(countDiCambioLoop == 9) {
            prova = 2;  
          } else if(countDiCambioLoop == 19) {
            prova = 0;
          } else if(countDiCambioLoop == 53) {
              prova = 1;
          }
          
          if( prova == 0 ) {
              //STAMPO PRIMA SERIE
              exposition = 1;
          
              Serial.println(exposition);
              expositionCharacteristic.writeValue( exposition );

              prova = 0;
              countDiCambioLoop+=1;
            } else if( prova == 1){
              //STAMPO SECONDA SERIE
              exposition = 0;
          
              Serial.println(exposition);
              expositionCharacteristic.writeValue( exposition );

              prova = 1;
              countDiCambioLoop+=1;
            } else {
            //STAMPO TERZA SERIE
              exposition = 1;
          
              Serial.println(exposition);
              expositionCharacteristic.writeValue( exposition );
              prova = 0;
              countDiCambioLoop+=1;
              }
          
          //wipy vede carateristiche e valute se il counter è relativo a nuova misurazione (altrimenti scarta) e prende valore e generat LUI la data
        }
        else
        {
          digitalWrite( RSSI_LED_PIN, HIGH );
        }
      } // intervall
    } // while connected

    Serial.print( F( "Disconnected from central: " ) );
    Serial.println( central.address() );
  } // if central
} // loop



bool setupBleMode()
{
  if ( !BLE.begin() )
  {
    return false;
  }

  // set advertised local name and service UUID:
  BLE.setDeviceName( "BLDG Arduino Nano 33 BLE" );
  BLE.setLocalName( "BLDG Arduino Nano 33 BLE" );
  BLE.setAdvertisedService( testService );

  // BLE add characteristics
  testService.addCharacteristic( counterCharacteristic );
  testService.addCharacteristic( expositionCharacteristic );

  // add service
  BLE.addService( testService );

  // set the initial value for the characeristic:
  counterCharacteristic.writeValue( 0 );
  expositionCharacteristic.writeValue( 0 );

  // start advertising
  BLE.advertise();

  return true;
}
