#include <Adafruit_SSD1306.h>

//#include <SPI.h>
#include <Wire.h>
//#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DELAY 1000 / 50

#define FW SCREEN_WIDTH / 2
//#define FH SCREEN_HEIGHT / 2
#define FH SCREEN_HEIGHT / 8 * 5
#define FX (SCREEN_WIDTH - FW) / 2
#define FY (SCREEN_HEIGHT - FH) / 2
#define FW_BYTES FW / 8
#define FSIZE FW_BYTES * FH

void setup() {
  randomSeed(analogRead(0));
  
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C
  // init done

  display.drawPixel(0, 0, WHITE);
  display.drawPixel(SCREEN_WIDTH - 1, 0, WHITE);
  display.drawPixel(0, SCREEN_HEIGHT - 1, WHITE);
  display.drawPixel(SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, WHITE);
  
  display.drawPixel(FX, FY, WHITE);
  display.drawPixel(FX + FW - 1, FY, WHITE);
  display.drawPixel(FX, FY + FH - 1, WHITE);
  display.drawPixel(FX + FW - 1, FY + FH - 1, WHITE);
  delay(1000);

  do_CGoL();
}


void loop() { }


void do_CGoL(void) {  
  uint8_t field[2][FSIZE];
  
  // init
  uint8_t fid = 0;          // current field id, can be 0 or 1
  uint8_t nfid;             // new field id
  int16_t xyc, xya, xyb, xx;  // pre-evaluated above, current and bolow y-coordinates with xbyte-aligned component, xx for special cases
  uint8_t _e;               // cell environment
  uint8_t xmask = 1 << 7;   // mask for fetching cell state
  uint8_t row8;             // resulting 8-bit row

  switch (random(3)) {
    case 1:
      // planers
      field[fid][FW_BYTES + 1] = 2 << 3; field[fid][FW_BYTES * 2 + 1] = 1 << 3; field[fid][FW_BYTES * 3 + 1] = 7 << 3;
      field[fid][FW_BYTES * 10 + 1] = 2; field[fid][FW_BYTES * 11 + 1] = 4; field[fid][FW_BYTES * 12 + 1] = 7;
      field[fid][FW_BYTES * 2 + 4] = 7 << 2; field[fid][FW_BYTES * 3 + 4] = 1 << 2; field[fid][FW_BYTES * 4 + 4] = 2 << 2;
      field[fid][FW_BYTES * 10 + 8] = 7; field[fid][FW_BYTES * 11 + 8] = 4; field[fid][FW_BYTES * 12 + 8] = 2;
      break;
    
    case 2:
      // flyers
      field[fid][FW_BYTES] = 2; field[fid][FW_BYTES * 2] = 1; field[fid][FW_BYTES * 3] = 1 + 64; field[fid][FW_BYTES * 4] = 63;
      field[fid][FW_BYTES * 7 + 10] = 8; field[fid][FW_BYTES * 8 + 10] = 16; field[fid][FW_BYTES * 9 + 10] = 1 + 16; field[fid][FW_BYTES * 10 + 10] = 30;
      field[fid][FW_BYTES * 20 + 12] = 2; field[fid][FW_BYTES * 19 + 12] = 1; field[fid][FW_BYTES * 18 + 12] = 1 + 32; field[fid][FW_BYTES * 17 + 12] = 31;
      field[fid][FW_BYTES * 27 + 10] = 8 << 2; field[fid][FW_BYTES * 26 + 10] = 16 << 2; field[fid][FW_BYTES * 25 + 10] = (1 + 16) << 2; field[fid][FW_BYTES * 24 + 10] = 30 << 2;
      break;
    
    default:
      for (uint16_t i = 0; i < FSIZE - 1; i++) {
        field[fid][i] = random(256) & random(256);
      }
  }

  //display.display_buffer(field[fid]);
  display.clearDisplay();
  display.drawBitmap(FX, FY, field[fid], FW, FH, WHITE);
  display.display();
  delay(DELAY);

  while (true) {
    
    nfid = 1 - fid;
    
    for (uint8_t y = 0; y < FH; y++) {
      xya = (y > 0 ? y - 1 : FH - 1) * FW_BYTES;
      xyc = y * FW_BYTES;
      xyb = (y < FH - 1 ? y + 1 : 0) * FW_BYTES;
      
      for (uint8_t x = 0; x < FW_BYTES; x++) {
        xmask = 1 << 7;
        
        row8 = 0;
        for (uint8_t xb = 6; xb > 0; xb--) {
          xmask = xmask >> 1;
          _e =
              ((field[fid][xya] & xmask << 1) >> xb + 1) + ((field[fid][xya] & xmask) >> xb) + ((field[fid][xya] & xmask >> 1) >> xb - 1)
            + ((field[fid][xyc] & xmask << 1) >> xb + 1)                                     + ((field[fid][xyc] & xmask >> 1) >> xb - 1)
            + ((field[fid][xyb] & xmask << 1) >> xb + 1) + ((field[fid][xyb] & xmask) >> xb) + ((field[fid][xyb] & xmask >> 1) >> xb - 1);

          if (_e == 2) row8 += field[fid][xyc] & xmask; else if (_e == 3) row8 += xmask;
        }

        // special cases
        // leftmost bit
        xx = x != 0 ? -1 : FW_BYTES - 1;
        _e =
            (field[fid][xya + xx] & 1) + ((field[fid][xya] & 128) >> 7) + ((field[fid][xya] & 64) >> 6)
          + (field[fid][xyc + xx] & 1)                                  + ((field[fid][xyc] & 64) >> 6)
          + (field[fid][xyb + xx] & 1) + ((field[fid][xyb] & 128) >> 7) + ((field[fid][xyb] & 64) >> 6);
        if (_e == 2) row8 += field[fid][xyc] & 128; else if (_e == 3) row8 += 128;

        // rightmost bit
        xx = x != FW_BYTES - 1 ? 1 : -FW_BYTES + 1;
        _e =
            ((field[fid][xya] & 2) >> 1) + (field[fid][xya] & 1) + ((field[fid][xya + xx] & 128) >> 7)
          + ((field[fid][xyc] & 2) >> 1)                         + ((field[fid][xyc + xx] & 128) >> 7)
          + ((field[fid][xyb] & 2) >> 1) + (field[fid][xyb] & 1) + ((field[fid][xyb + xx] & 128) >> 7);
        if (_e == 2) row8 += field[fid][xyc] & 1; else if (_e == 3) row8 += 1;
        
        field[nfid][xyc] = row8;
        
        xya++;
        xyb++;
        xyc++;
      }
    }
    
    fid = nfid;
    
    //display.display_buffer(field[fid]);
    display.clearDisplay();
    display.drawBitmap(FX, FY, field[fid], FW, FH, WHITE);
//    display.drawPixel(0, 0, WHITE);
//    display.drawPixel(SCREEN_WIDTH - 1, 0, WHITE);
//    display.drawPixel(0, SCREEN_HEIGHT - 1, WHITE);
//    display.drawPixel(SCREEN_WIDTH - 1, SCREEN_HEIGHT - 1, WHITE);
    
    display.drawPixel(FX, FY, WHITE);
    display.drawPixel(FX + FW - 1, FY, WHITE);
    display.drawPixel(FX, FY + FH - 1, WHITE);
    display.drawPixel(FX + FW - 1, FY + FH - 1, WHITE);
    display.display();
    delay(DELAY);
  }
}
