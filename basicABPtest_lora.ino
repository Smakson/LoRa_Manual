/*******************************************************************************
 * Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
 *
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *H
 * This example sends a valid LoRaWAN packet with payload "Hello,
 * world!", using frequency and encryption settings matching those of
 * the Thing Network
 *
 * Change DEVADDR to a unique address!
 * See http://thethingsnetwork.org/wiki/AddressSpace
 *
 * Do not forget to define the radio type correctly in 
 * "(Documents/<username>)\Arduino\libraries\MCCI_LoRaWAN_LMIC_library\project_config\lora_project_config.h" .
 *
 *
 * Required Library: 
 *    * https://github.com/matthijskooijman/arduino-lmic 
 * 
 * Require Hardware:
 *    * LoRa Shield + Arduino Uno
 *******************************************************************************/

#include <lmic.h> //relevant LoRaWAN implementation library
#include <hal/hal.h> //the default Hardware Abstraction Layer done by the library creators for the Arduino boards 
#include <SPI.h> //library for Serial Peripheral Interfaces, e.g. communication with computer over USB for our intents and purposes


/* INSERT YOUR OWN NETWORK SECURITY KEY BELOW. MAKE SURE IN THE TTN CONSOLE THAT YOU ARE USING ABP MODE FOR THE DEVICE.
Read more at: https://www.thethingsnetwork.org/docs/lorawan/addressing.html*/
static const u1_t  NWKSKEY[16] PROGMEM  = { };


/* INSERT YOUR OWN APPLICATION SECURITY KEY BELOW. MAKE SURE IN THE TTN CONSOLE THAT YOU ARE USING ABP MODE FOR THE DEVICE. */
static const u1_t  APPSKEY[16] PROGMEM = {  };


/* INSERT YOUR OWN DEVICE ADDRESS BELOW. MAKE SURE IN THE TTN CONSOLE THAT YOU ARE USING ABP MODE FOR THE DEVICE.*/
static const u4_t DEVADDR = ;

// ALL OF THE ABOVE KEYS/ADDRESSES ARE NORMALLY AVAILABLE IN THE TTN CONSOLE IF YOU REGISTERED EVERYTHING CORRECTLY AND ARE USING ABP MODE



// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely otherwise the linker will complain).
void os_getArtEui (u1_t* buf) { }
void os_getDevEui (u1_t* buf) { }
void os_getDevKey (u1_t* buf) { }

static uint8_t mydata[] = "Hello, world!"; //the data we wish to send
static osjob_t sendjob; //the way a library works is over a common struct that manages tasks, called osjob_t

// A constant used above for scheduling transmits.
const unsigned TX_INTERVAL = 60;

// Pin mapping, used by the HAL defined for Arduino. Can also be obtained from the Dragino wiki at
// https://wiki.dragino.com/index.php?title=Lora_Shield
const lmic_pinmap lmic_pins = {
    .nss = 10,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 9,
    .dio = {2, 6, 7},
};
/* The event handler which is the backbone of the LMIC programming model. The library defines several events which are called by the 
underlying code to signify to your application that something is going on, reminiscent of Javascript. More details can be found in 
LMIC specification/documentation in "(Documents/<username>)\Arduino\libraries\MCCI_LoRaWAN_LMIC_library\docs" or the User Manual in the
repository.
*/
void onEvent (ev_t ev) {
    Serial.print(os_getTime());
    Serial.print(": ");
    switch(ev) {
        case EV_SCAN_TIMEOUT:
            Serial.println(F("EV_SCAN_TIMEOUT"));
            break;
        case EV_BEACON_FOUND:
            Serial.println(F("EV_BEACON_FOUND"));
            break;
        case EV_BEACON_MISSED:
            Serial.println(F("EV_BEACON_MISSED"));
            break;
        case EV_BEACON_TRACKED:
            Serial.println(F("EV_BEACON_TRACKED"));
            break;
        case EV_JOINING:
            Serial.println(F("EV_JOINING"));
            break;
        case EV_JOINED:
            Serial.println(F("EV_JOINED"));
            break;
        case EV_RFU1:
            Serial.println(F("EV_RFU1"));
            break;
        case EV_JOIN_FAILED:
            Serial.println(F("EV_JOIN_FAILED"));
            break;
        case EV_REJOIN_FAILED:
            Serial.println(F("EV_REJOIN_FAILED"));
            break;
        case EV_TXCOMPLETE:
            Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
            /*The receive windows defined by LoRaWAN for class A devices are done automatically and the EV_TXCOMPLETE
              is raised at the end of all of them. This means that when the event is raised, we need to check whether
              anything was received. All received data is automatically stored in the LMIC struct, for ease of access.*/
            
            if(LMIC.dataLen) {
                // checks whether there is any data
                Serial.print(F("Data Received: ")); //a simple command which literally prints the string representation of the data
                //onto the Serial port. The 'F()' enforces that the string is stored in flash memory, ensuring that it does not take
                //up valuable SRAM space (used for local and global non-string variables.
                Serial.write(LMIC.frame+LMIC.dataBeg, LMIC.dataLen); // 'write' sends pure binary data, in terms of bytes
                // the above command looks into the LMIC struct, and copies all data written from there onto the Serial port.
                Serial.println(); //simply print with a carriage return and end-of-line character at the end.
            }
            os_setTimedCallback(&sendjob, os_getTime()+sec2osticks(TX_INTERVAL), do_send);
            /* callbacks allow you to use the underlying OS provided by the library to schedule functions (jobs) for later */
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            Serial.println(F("EV_RXCOMPLETE"));
            break;
        case EV_LINK_DEAD:
            Serial.println(F("EV_LINK_DEAD"));
            break;
        case EV_LINK_ALIVE:
            Serial.println(F("EV_LINK_ALIVE"));
            break;
        case EV_TXSTART:
            //means the transmission started, an event the original Dragino code forgets to include
            Serial.println(F("EV_TXSTART));
            break;
         default:
            Serial.println(F("Unknown event"));
            break;
            
    }
}

void do_send(osjob_t* j){
    // TX - transmission, RX - reception
    // Checks if there is not a current TX/RX job running. Normally, should not be necessary for Class A devices but the library
    // seems to support Class B as well, where TX and RX jobs might collide.
    if (LMIC.opmode & OP_TXRXPEND) {
        Serial.println(F("OP_TXRXPEND, not sending"));
    // in case that there is no TX/RX job currently running
    } else {
        // Prepare upstream data transmission at the next possible time.
        LMIC_setTxData2(1, mydata, sizeof(mydata)-1, 0);
        // This command basically means send the bytes starting at "mydata" (pointers and arrays are mostly interchangeable)
        // for a length of "sizeof(mydata)-1" bytes on port 1. The 0 means that the transmission does not require a confirmation.
        Serial.println(F("Packet queued"));
    }
    // Next TX is scheduled after TX_COMPLETE event using the callback function. Since nodes can trasmit at any time in LoRaWAN
    // We can be sure that we will be able to transmit at that time that "do_send" is scheduled, essentially turning the "LMIC_setTxData2()"
    // into a send command.
}

void setup() {
    Serial.begin(9600);
    Serial.println(F("Starting"));
    
    // LMIC init, initializes the necessary structs and starts the clock, other OS things.
    os_init();
    // Reset the MAC state. Session and pending data transfers will be discarded, as well as the frame counter which means you need to
    // reset it on TTN as well.
    LMIC_reset();

    // Set static session parameters. Instead of dynamically establishing a session
    // by joining the network, precomputed session parameters are be provided.
    // On AVR, these values are stored in flash and only copied to RAM
    // once. Copy them to a temporary buffer here, LMIC_setSession will
    // copy them into a buffer of its own again.
    uint8_t appskey[sizeof(APPSKEY)];
    uint8_t nwkskey[sizeof(NWKSKEY)];
    memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
    memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));
    LMIC_setSession (0x1, DEVADDR, nwkskey, appskey);
    Serial.println(F("Session"));


    // Disable link check validation
    LMIC_setLinkCheckMode(0);

    // Set data rate and transmit power (note: txpow seems to be ignored by the library)
    LMIC_setDrTxpow(DR_SF7,14);

    // Start job
    Serial.println(F("First job"));

    do_send(&sendjob);
}

void loop() {
    os_runloop_once();
}
