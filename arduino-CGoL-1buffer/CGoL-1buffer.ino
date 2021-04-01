//#include <SPI.h>
#include <Wire.h>
//#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define DELAY 1000 / 2

#define FW SCREEN_WIDTH / 2
#define FH SCREEN_HEIGHT
#define FX (SCREEN_WIDTH - FW) / 2
#define FY (SCREEN_HEIGHT - FH) / 2
#define FW_BYTES FW / 8
#define FSIZE FW_BYTES * FH

void setup() {
  randomSeed(analogRead(0));
  
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C
  // init done
  pinMode(LED_BUILTIN, OUTPUT);
  
  do_CGoL();

  //do_test();
}


void loop() { }


void do_CGoL(void) {
    
  // init
  uint8_t field[FSIZE];
  uint8_t f_firstrow[FW_BYTES], f_prev[2][FW_BYTES];

  uint8_t fid, nfid;
  int16_t xyc, xya, xyb, xx;  // pre-evaluated above, current and bolow y-coordinates with xbyte-aligned component, xx for special cases
  uint8_t _e;               // cell environment
  uint8_t xmask = 1 << 7;   // mask for fetching cell state
  uint8_t row8;             // resulting 8-bit row

  for (uint16_t i = 0; i < FSIZE; i++)
    field[i] = 0;
  
  switch (random(3)) {
    case 1:
      // planers
      field[FW_BYTES + 1] = 2 << 3; field[FW_BYTES * 2 + 1] = 1 << 3; field[FW_BYTES * 3 + 1] = 7 << 3;
      field[FW_BYTES * 10 + 1] = 2; field[FW_BYTES * 11 + 1] = 4; field[FW_BYTES * 12 + 1] = 7;
      field[FW_BYTES * 2 + 4] = 7 << 2; field[FW_BYTES * 3 + 4] = 1 << 2; field[FW_BYTES * 4 + 4] = 2 << 2;
      field[FW_BYTES * 10 + 8] = 7; field[FW_BYTES * 11 + 8] = 4; field[FW_BYTES * 12 + 8] = 2;
      break;
    
    case 2:
      // flyers
      field[FW_BYTES] = 2; field[FW_BYTES * 2] = 1; field[FW_BYTES * 3] = 1 + 64; field[FW_BYTES * 4] = 63;
      field[FW_BYTES * 7 + 10] = 8; field[FW_BYTES * 8 + 10] = 16; field[FW_BYTES * 9 + 10] = 1 + 16; field[FW_BYTES * 10 + 10] = 30;
      field[FW_BYTES * 20 + 12] = 2; field[FW_BYTES * 19 + 12] = 1; field[FW_BYTES * 18 + 12] = 1 + 32; field[FW_BYTES * 17 + 12] = 31;
      field[FW_BYTES * 27 + 10] = 8 << 2; field[FW_BYTES * 26 + 10] = 16 << 2; field[FW_BYTES * 25 + 10] = (1 + 16) << 2; field[FW_BYTES * 24 + 10] = 30 << 2;
      break;
    
    default:
      for (uint16_t i = 0; i < FSIZE - 1; i++) {
        field[i] = random(256) & random(256);
      }
  }

  //display.display_buffer(field);
  display.clearDisplay();
  display.drawBitmap(FX, FY, field, FW, FH, WHITE);
  display.display();
  digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(DELAY);
  digitalWrite(LED_BUILTIN, LOW);   // turn the LED on (HIGH is the voltage level)

  while (true) {
    
    fid = 0;
    
    // backup first row for last row calculation
    // %%% try memcpy() here
    for (uint8_t i = 0; i < FW_BYTES; i++)
      f_firstrow[i] = field[i];

    // backup last row for the fist row to eval
    for (uint8_t i = 0; i < FW_BYTES; i++)
      f_prev[fid][i] = field[FSIZE - FW_BYTES + i];

    for (uint8_t y = 0; y < FH - 1; y++) {
      xyc = y * FW_BYTES;
      xyb = (y + 1) * FW_BYTES;
      
      nfid = 1 - fid;
      
      for (uint8_t x = 0; x < FW_BYTES; x++) {
        // backup current row for next row calculation
        f_prev[nfid][x] = field[xyc];
        
        xmask = 1 << 7;
        
        row8 = 0;
        for (uint8_t xb = 6; xb > 0; xb--) {
          xmask = xmask >> 1;
          _e =
              ((f_prev[fid][x] & xmask << 1) >> xb + 1) + ((f_prev[fid][x] & xmask) >> xb) + ((f_prev[fid][x] & xmask >> 1) >> xb - 1)
            + ((field[xyc] & xmask << 1) >> xb + 1)                                        + ((field[xyc] & xmask >> 1) >> xb - 1)
            + ((field[xyb] & xmask << 1) >> xb + 1)     + ((field[xyb] & xmask) >> xb)     + ((field[xyb] & xmask >> 1) >> xb - 1);
          
          if (_e == 2) row8 += field[xyc] & xmask; else if (_e == 3) row8 += xmask;
        }
        
        // special cases
        // leftmost bit
        xx = x != 0 ? -1 : FW_BYTES - 1;
        _e =
            (f_prev[fid][x + xx] & 1)                                  + ((f_prev[fid][x] & 128) >> 7) + ((f_prev[fid][x] & 64) >> 6)
          + (xx == -1 ? f_prev[nfid][x - 1] & 1 : field[xyc + xx] & 1)                                 + ((field[xyc] & 64) >> 6)
          + (field[xyb + xx] & 1)                                      + ((field[xyb] & 128) >> 7)     + ((field[xyb] & 64) >> 6);
        if (_e == 2) row8 += field[xyc] & 128; else if (_e == 3) row8 += 128;
        
        // rightmost bit
        xx = x != FW_BYTES - 1 ? 1 : -FW_BYTES + 1;
        _e =
            ((f_prev[fid][x] & 2) >> 1) + (f_prev[fid][x] & 1) + ((f_prev[fid][x + xx] & 128) >> 7)
          + ((field[xyc] & 2) >> 1)                            + (xx == 1 ? (field[xyc + 1] & 128) >> 7 : (f_prev[nfid][0] & 128) >> 7)
          + ((field[xyb] & 2) >> 1)     + (field[xyb] & 1)     + ((field[xyb + xx] & 128) >> 7);
        if (_e == 2) row8 += field[xyc] & 1; else if (_e == 3) row8 += 1;
        
        field[xyc] = row8;
        
        xyb++;
        xyc++;
      } // for x

      fid = nfid;
    } // for y

    //
    // ultra special case for the last row
    xyc = (FH - 1) * FW_BYTES;
    nfid = 1 - fid;
      
    for (uint8_t x = 0; x < FW_BYTES; x++) {
      // backup current row for next row calculation
      f_prev[nfid][x] = field[xyc];
      
      xmask = 1 << 7;
      
      row8 = 0;
      for (uint8_t xb = 6; xb > 0; xb--) {
        xmask = xmask >> 1;
        _e =
            ((f_prev[fid][x] & xmask << 1) >> xb + 1) + ((f_prev[fid][x] & xmask) >> xb) + ((f_prev[fid][x] & xmask >> 1) >> xb - 1)
          + ((field[xyc] & xmask << 1) >> xb + 1)                                        + ((field[xyc] & xmask >> 1) >> xb - 1)
          + ((f_firstrow[x] & xmask << 1) >> xb + 1)     + ((f_firstrow[x] & xmask) >> xb)     + ((f_firstrow[x] & xmask >> 1) >> xb - 1);
          
        if (_e == 2) row8 += field[xyc] & xmask; else if (_e == 3) row8 += xmask;
      }
        
      // special cases
      // leftmost bit
      xx = x != 0 ? -1 : FW_BYTES - 1;
      _e =
          (f_prev[fid][x + xx] & 1)                                  + ((f_prev[fid][x] & 128) >> 7) + ((f_prev[fid][x] & 64) >> 6)
        + (xx == -1 ? f_prev[nfid][x - 1] & 1 : field[xyc + xx] & 1)                                 + ((field[xyc] & 64) >> 6)
        + (f_firstrow[x + xx] & 1)                                      + ((f_firstrow[x] & 128) >> 7)     + ((f_firstrow[x] & 64) >> 6);
      if (_e == 2) row8 += field[xyc] & 128; else if (_e == 3) row8 += 128;
        
      // rightmost bit
      xx = x != FW_BYTES - 1 ? 1 : -FW_BYTES + 1;
      _e =
          ((f_prev[fid][x] & 2) >> 1) + (f_prev[fid][x] & 1) + ((f_prev[fid][x + xx] & 128) >> 7)
        + ((field[xyc] & 2) >> 1)                            + (xx == 1 ? (field[xyc + 1] & 128) >> 7 : (f_prev[nfid][0] & 128) >> 7)
        + ((f_firstrow[x] & 2) >> 1)     + (f_firstrow[x] & 1)     + ((f_firstrow[x + xx] & 128) >> 7);
      if (_e == 2) row8 += field[xyc] & 1; else if (_e == 3) row8 += 1;
        
      field[xyc] = row8;
        
      xyb++;
      xyc++;
    } // for x
    
    //display.display_buffer(field);
    display.clearDisplay();
    display.drawBitmap(FX, FY, field, FW, FH, WHITE);
    display.display();
    delay(DELAY);
  } // while true
}

//void do_test(void) {
//  for (uint16_t i = 0; i < FSIZE; i++)
//    field[i] = 0;
//
//  uint8_t a = 3, dir = 0;
//  
//  while (true) {
//
//    if (dir == 1) {
//      // right
//      a >>= 1;
//      if (a == 3) dir = 0;
//    } else {
//      // left
//      a <<= 1;
//      if (a == 128 + 64) dir = 1;
//    }
//
//    for (uint16_t i = 0; i < FSIZE; i++)
//      field[i] = a;
//    //field[0] = a;
//    field[1] = 15;
//    //field[FW_BYTES] = a;
//    //display.display_buffer(field);
//
//    delay(500);
//  }
//
//}
