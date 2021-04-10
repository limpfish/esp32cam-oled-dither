/*
 * Required libraries:
 * Adafruit GFX
 * Adafruit SSD1306
 * 
 * 
 * Board:
 * Add to boards manager : https://dl.espressif.com/dl/package_esp32_index.json
 *
 * Board: AI Thinker ESP32 CAM
 *
 * GPIO 0 connected to GND to upload, Press reset to put in flashing mode


*/

#include "esp_camera.h"

#include <Adafruit_GFX.h>

#include <Adafruit_SSD1306.h>



// Select camera model
//  #define CAMERA_MODEL_WROVER_KIT
//#define CAMERA_MODEL_ESP_EYE
//#define CAMERA_MODEL_M5STACK_PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE
#define CAMERA_MODEL_AI_THINKER

#include "camera_pins.h"
byte x,y;
uint8_t oldpixel, newpixel;
int quanterror,i;

#define OLED_MOSI  13
#define OLED_CLK   15
#define OLED_DC    14
#define OLED_RESET 2
#define OLED_CS    12

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 

Adafruit_SSD1306 display(SCREEN_WIDTH,SCREEN_HEIGHT, OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);


// the setup function runs once when you press reset or power the board
void setup() {

  Serial.begin(115200);

 //  display.begin(SSD1306_SWITCHCAPVCC);
 display.begin(SSD1306_SWITCHCAPVCC);
 display.clearDisplay();
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_GRAYSCALE; //PIXFORMAT_JPEG;
  config.frame_size =  FRAMESIZE_QQVGA; //FRAMESIZE_QQVGA2; // 128x160
  config.fb_count = 1;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  //initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1);//flip it back
    s->set_brightness(s, 1);//up the blightness just a bit
  }

}


void loop() {

 // capture camera frame
camera_fb_t *fb = esp_camera_fb_get();

i=0;


for (y=0;y<SCREEN_HEIGHT;y++)
{
  for(x=0;x<SCREEN_WIDTH;x++)
  {
    oldpixel= fb->buf[i]; // keep original greyscale value
    newpixel = (255 * (oldpixel >> 7));  // threshold of 128 
    fb->buf[i] = newpixel;  // cam buffer now mono, either 0 or 255

    // floyd-steignburg dithering:
    quanterror = oldpixel - newpixel; // error difference between pixels

    // distribute this error to neighbouring pixels:

   //right
      if (x < SCREEN_WIDTH-1) // bounds checking...
      {

        fb->buf[(x+1) +(y*SCREEN_WIDTH)] +=  ((quanterror * 7)>>4);
      }

      //leftdown
      if ((x > 1) && (y < SCREEN_HEIGHT-1)) // bounds checking...
      {
        fb->buf[(x-1) +((y+1)*SCREEN_WIDTH)] += ((quanterror * 3)>>4); 
      }

      //down
      if (y < 63) // bounds checking...
      {
         fb->buf[(x) + ((y + 1) * SCREEN_WIDTH)] += ((quanterror * 5) >> 4);
      }

      //rightdown
      if ((x < SCREEN_WIDTH-1) && (y < SCREEN_HEIGHT-1)) // bounds checking...
      {
         fb->buf[(x + 1) + ((y + 1) * SCREEN_WIDTH)] += (quanterror >> 4);
      }

    // draw this pixel
    display.drawPixel( x,y,  fb->buf[i]%2);
    i++;

  }

}

display.display();

}
