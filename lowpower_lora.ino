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
 * the (early prototype version of) The Things Network.
 *
 * Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in g1,
 *  0.1% in g2).
 *
 * Change DEVADDR to a unique address!
 * See http://thethingsnetwork.org/wiki/AddressSpace
 *
 * Do not forget to define the radio type correctly in config.h.
 *
 *
 * Required Library: 
 *    * https://github.com/matthijskooijman/arduino-lmic 
 * 
 * Require Hardware:
 *    * LoRa Shield + Arduino
 *    * LoRa GPS Shield + Arduino 
 *    * LoRa Mini etc. 
 *******************************************************************************/
//THIS CODE IS UNTESTED AND MAY NOT RUN
#include <lmic.h>
#include <hal/hal.h>
#include <SPI.h>
#include "LowPower.h"

/* INSERT YOUR OWN NETWORK SECURITY KEY BELOW. MAKE SURE IN THE TTN CONSOLE THAT YOU ARE USING ABP MODE FOR THE DEVICE.
Read more at: https://www.thethingsnetwork.org/docs/lorawan/addressing.html */
static const u1_t  NWKSKEY[16] PROGMEM  = { };


/* INSERT YOUR OWN APPLICATION SECURITY KEY BELOW. MAKE SURE IN THE TTN CONSOLE THAT YOU ARE USING ABP MODE FOR THE DEVICE. */
static const u1_t  APPSKEY[16] PROGMEM = {  };


/* INSERT YOUR OWN DEVICE ADDRESS BELOW. MAKE SURE IN THE TTN CONSOLE THAT YOU ARE USING ABP MODE FOR THE DEVICE.*/
static const u4_t DEVADDR = ;

// ALL OF THE ABOVE KEYS/ADDRESSES ARE NORMALLY AVAILABLE IN THE TTN CONSOLE IF YOU REGISTERED EVERYTHING CORRECTLY AND ARE USING ABP MODE

#define SLEEP_INTERVAL 10// in intervals of 8s
#define MAX_SCANS 5 // sets the amount of scans that are done before the data is trasnmitted

static bool sleeping = false;// we will need a global variable, since the sleep loop should be run outside of a job
static u1_t count = 0;
static u1_t mydata[MAX_SCANS]; //initializes the array that will keep the data measurements
static osjob_t sendjob, scanjob; //there are two separate jobs so that the code is clearer



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

            sleeping = true; // we want it to sleep after the transmission
            
            break;
        case EV_LOST_TSYNC:
            Serial.println(F("EV_LOST_TSYNC"));
            break;
        case EV_RESET:
            Serial.println(F("EV_RESET"));
            break;
        case EV_RXCOMPLETE:
            // data received in ping slot
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
            Serial.println(F("EV_TXSTART"));
            break;
         default:
            Serial.println(F("Unknown event"));
            break;
    }
}

void do_scan(osjob_t* j){
    if (count < MAX_SCANS) {
        
        Serial.print(F("Scan no.: ")); //prints used for debugging
        Serial.println(i);
        count++;
        mydata[i] = random(255); //generates a random value since I am without a sensor 
        //os_clearCallback(&sendjob); // the two clears are here simply for debugging purposes, to make sure I am not
        //os_clearCallback(&scanjob); // scheduling jobs multiple times for some reason - this is because running the code
        //with interrupt timing made it very buggy.
        
   
        // os_clearCallback(&sendjob);
        // os_clearCallback(&scanjob);
            
        sleeping = true; // we want it to go to sleep after a scan as well.
    } else {
          count = 0;
          os_setCallback(&sendjob, do_send); //when we reach the max amount of scans, i.e. end of array, we need to send the data off
          // and we start the transmission instanty
      
      }
    
    
}

void do_send(osjob_t* j){
    // Check if there is not a current TX/RX job running
    // Prepare upstream data transmission at the next possible time.
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
    LMIC.dn2Dr = DR_SF9; //with TTN the second RX window should use SF9
    
    // Start job
    Serial.println(F("Job"));

    os_setCallback(&scanjob, do_scan); //here we start with the scan since that is the main job. We schedule it, instead of running it so that we remain inside the framework of LMIC.
}

void loop() {
  /* This part of the code basically takes control when the OS does it's part of the job - the way the Arduino program works, it will return here when `os_runloop_once()` and all the jobs inbetween are executed and check the `sleep` variable. 
   * If we do not want it to sleep yet, it will continue looking for and scheduling jobs, but if we do, it will go into the sleep part of the loop and the processor will power down. After that we schedule the scan job, so that the scanning can continue
   * amd restart the OS so that it is actually scheduled.
   */
  if (sleeping == false) {
      os_runloop_once();
  } 
  else {
      sleeping = false;
      for (int j = 0; j++; j < SLEEP_INTERVAL) { 
            LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF); //this will make it sleep SLEEP_INTERVAL * 8s, it was the simples possible implementation and good enough for what I wished to illustrate.
        }

      os_setCallback(&scanjob, do_scan); 
      os_runloop_once();
               
  }
}
