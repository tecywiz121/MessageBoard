#include <Adafruit_CC3000.h>
#include <SPI.h>
#include "utility/debug.h"
#include "utility/socket.h"
#include <avr/sleep.h>


// include the library code:
#include <LiquidCrystal.h>

// initialize the library by associating any needed LCD interface pin
// with the arduino pin number it is connected to
const int rs = A0, en = A1, d4 = 6, d5 = 7, d6 = 8, d7 = 9;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

// These are the interrupt and control pins
#define ADAFRUIT_CC3000_IRQ   3  // MUST be an interrupt pin!
// These can be any two pins
#define ADAFRUIT_CC3000_VBAT  5
#define ADAFRUIT_CC3000_CS    10
// Use hardware SPI for the remaining pins
// On an UNO, SCK = 13, MISO = 12, and MOSI = 11
Adafruit_CC3000 cc3000 = Adafruit_CC3000(ADAFRUIT_CC3000_CS, ADAFRUIT_CC3000_IRQ, ADAFRUIT_CC3000_VBAT,
                                         SPI_CLOCK_DIVIDER); // you can change this clock speed

#define WLAN_SSID       "generic-wifi"           // cannot be longer than 32 characters!
#define WLAN_PASS       "hackmenow321"
// Security can be WLAN_SEC_UNSEC, WLAN_SEC_WEP, WLAN_SEC_WPA or WLAN_SEC_WPA2
#define WLAN_SECURITY   WLAN_SEC_WPA2

#define LISTEN_PORT           7    // What TCP port to listen on for connections.  The echo protocol uses port 7.

#define ROWS 2
#define COLS 16

Adafruit_CC3000_Server echoServer(LISTEN_PORT);

void setup(void)
{
  // set up the LCD's number of columns and rows:
  lcd.begin(ROWS, COLS);
  
  if (!cc3000.begin())
  {
    error("WiFi broken :(", "");
  }

  lcd.print("Connecting to:");
  lcd.setCursor(0, 1);
  lcd.print(WLAN_SSID);
  lcd.setCursor(0, 0);
  if (!cc3000.connectToAP(WLAN_SSID, WLAN_PASS, WLAN_SECURITY)) {
    error("Failed to", "connect!");
  }

  lcd.clear();
  lcd.print("Obtaining an IP");
  lcd.setCursor(0, 1);
  lcd.print("address...");
  while (!cc3000.checkDHCP())
  {
    delay(100); // ToDo: Insert a DHCP timeout!
  }  

  /* Display the IP address DNS, Gateway, etc. */  
  while (! displayConnectionDetails()) {
    delay(1000);
  }
  
  // Start listening for connections
  echoServer.begin();
}

uint8_t row = 0;
uint8_t col = 0;

static void newline(void)
{
  row += 1;
  col = 0;

  // Wrap around after a delay.
  if (row >= ROWS) {
   row = 0;
   delay(5000);
  }
}

void loop(void)
{
  
  // Try to get a client which is connected.
  Adafruit_CC3000_ClientRef client = echoServer.available();
  if (client) {
     // Check if there is data available to read.
     if (client.available() > 0) {
       // Read a byte and write it to all clients.
       uint8_t ch = client.read();
       switch (ch) {
        case '\n':
          newline();
          break;
        default:
         lcd.write(ch);
         col += 1;
         break;
       }

       for (uint8_t ii = col; ii < COLS; ii++) {
         lcd.print(" ");
       }
       
       if (col >= COLS) {
         newline();
       }

       lcd.setCursor(col, row);
     }
  }
}

static void error(String line0, String line1)
{
  lcd.clear();
  while (1) {
    lcd.print(line0);
    lcd.setCursor(0, 1);
    lcd.print(line1);

    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_mode();
    lcd.clear();
    delay(500);
  }
}

/**************************************************************************/
/*!
    @brief  Tries to read the IP address and other connection details
*/
/**************************************************************************/
bool displayConnectionDetails(void)
{
  uint32_t ipAddress, netmask, gateway, dhcpserv, dnsserv;
  
  if(!cc3000.getIPAddress(&ipAddress, &netmask, &gateway, &dhcpserv, &dnsserv))
  {
    error("Failed to get", "IP address");
    return false;
  }
  else
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("My IP:");
    lcd.setCursor(0, 1);
    lcd.print((uint8_t)(ipAddress >> 24));
    lcd.print('.');
    lcd.print((uint8_t)(ipAddress >> 16));
    lcd.print('.');
    lcd.print((uint8_t)(ipAddress >> 8));
    lcd.print('.');
    lcd.print((uint8_t)(ipAddress));
    lcd.setCursor(0, 0);

    return true;
  }
}
