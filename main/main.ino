#include "config.h"
#include "MagSpoof.h"

#define PIN_A       3      // L293D pin 2
#define PIN_B       4      // L293D pin 7
#define PIN_ENABLE  13     // L293D pin 1; Using Arduino pin 13 to switch on embedded LED during transmission
#define CLOCK_US    500    // Delay for bit transmission, in micro seconds

// MagSpoof magSpoof(PIN_A, PIN_B, PIN_ENABLE, CLOCK_US, MagSpoof::BPC7, MagSpoof::Odd);

// const char* testTracks[] =
//         {   
//             "%B123456781234567^LASTNAME/FIRST^YYMMSSSDDDDDDDDDDDDDDDDDDDDDDDDD?",
//             ";4242004800500=0123456789?",
//         };

// note: BPC5 = 5 data bits + 1 parity bit, even parity = MagSpoof::Even
MagSpoof magSpoof(PIN_A, PIN_B, PIN_ENABLE, CLOCK_US, MagSpoof::BPC5, MagSpoof::Odd);

// Only send Track 2; payment terminals will read account‐number/expiry/service from it.
const char* testTracks[] = {
    MY_CARD_TRACK2
};

void setup()
{
    magSpoof.setup();
    Serial.begin(9600);
}


void loop()
{
    // Transmit track every 2 seconds
    delay(2000);
    magSpoof.playTrack(testTracks[0]);
}
