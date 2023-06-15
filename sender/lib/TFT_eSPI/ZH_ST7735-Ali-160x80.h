// Setup for ESP32 and ST7735 80 x 160 TFT from Ali
#define USER_SETUP_ID 43
#define ST7735_160_80_ALI
// See SetupX_Template.h for all options available

#define ST7735_DRIVER


#define TFT_WIDTH  80
#define TFT_HEIGHT 160


#define ST7735_GREENTAB160x80

// For ST7735, ST7789 and ILI9341 ONLY, define the colour order IF the blue and red are swapped on your display
// Try ONE option at a time to find the correct colour order for your display

//  #define TFT_RGB_ORDER TFT_RGB  // Colour order Red-Green-Blue
//  #define TFT_RGB_ORDER TFT_BGR  // Colour order Blue-Green-Red

#ifdef ESP32
    #define TFT_MISO 40     // not in use
    #define TFT_MOSI 4
    #define TFT_SCLK 5
    #define TFT_CS    2  // Chip select control pin
    #define TFT_DC    3  // Data Command control pin
    #define TFT_RST   7  // Reset pin (could connect to RST pin)
#else
    #error "only ESP32 compatible"
#endif

#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:.
// #define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
#define LOAD_FONT8N // Font 8. Alternative to Font 8 above, slightly narrower, so 3 digits fit a 160 pixel TFT
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

#define SMOOTH_FONT


//#define SPI_FREQUENCY  20000000
#define SPI_FREQUENCY  27000000 // Actually sets it to 26.67MHz = 80/3
// #define SPI_FREQUENCY  80000000