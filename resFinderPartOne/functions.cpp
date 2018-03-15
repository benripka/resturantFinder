#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_ILI9341.h>
#include "functions.h"//include header file


//define variables
#define TFT_DC 9
#define TFT_CS 10
#define SD_CS 6

Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

#define DISPLAY_WIDTH  320
#define DISPLAY_HEIGHT 240
#define YEG_SIZE 2048

lcd_image_t yegImage = { "yeg-big.lcd", YEG_SIZE, YEG_SIZE };

#define JOY_VERT  A1
#define JOY_HORIZ A0
#define JOY_SEL   2

#define JOY_CENTER   512
#define JOY_DEADZONE 64

#define MAP_WIDTH 2048
#define MAP_HEIGHT 2048
#define LAT_NORTH 5361858l
#define LAT_SOUTH 5340953l
#define LON_WEST -11368652l
#define LON_EAST -11333496l


#define CURSOR_SIZE 9
#define HIGH_SPEED 500//this will be used later to varry cursor speed
// the cursor position on the display
int cursorX, cursorY;//these varialbes will hold the value of the curosor position
//with respect to the LCD screen
int32_t cursorLon;//holds the value of the longitude of the cursor
int32_t cursorLat;//holds the value of the latitude of the cursor
Sd2Card card;
// forward declaration for redrawing the cursor
void redrawCursor(uint16_t colour, int cursorX, int cursorY);

int yegMiddleX = YEG_SIZE/2 - (DISPLAY_WIDTH - 64)/2;//this is the center of the
//map of Edmonton that the program initally begins at
int yegMiddleY = YEG_SIZE/2 - DISPLAY_HEIGHT/2;
int patchPointX = yegMiddleX;
int patchPointY = yegMiddleY;


void setup() {
  init();//Initializing low level Arduino functions

  Serial.begin(9600);

	pinMode(JOY_SEL, INPUT_PULLUP);



	tft.begin();//initialize LCD screen functionality

  //follow allows program to load map of Edmonton
	Serial.print("Initializing SD card...");
	if (!SD.begin(SD_CS)) {
		Serial.println("failed! Is it inserted properly?");
		while (true) {}
	}
	Serial.println("OK!");
  //allows restaurants to be read from the SD card
  Serial.print("Initializing SPI communication for raw reads...");
   if (!card.init(SPI_HALF_SPEED, SD_CS)) {
     Serial.println("failed!");
     while (true) {}
   }
   else {
     Serial.println("OK!");
 }

	tft.setRotation(3);

  tft.fillScreen(ILI9341_BLACK);

  // draws the centre of the Edmonton map, leaving the rightmost 64 columns black

	lcd_image_draw(&yegImage, &tft, yegMiddleX, yegMiddleY,
                 0, 0, DISPLAY_WIDTH - 64, DISPLAY_HEIGHT);

  // initial cursor position is the middle of the screen
  cursorX = (DISPLAY_WIDTH -64)/2;
  cursorY = DISPLAY_HEIGHT/2;

  redrawCursor(ILI9341_RED, cursorX, cursorY);


}


void redrawMap(int cursorX, int cursorY){
  int oldX = cursorX;// since this is taken before the cursor changes, these
  //variables hold the past values of the cursor
  int oldY = cursorY;

  lcd_image_draw(&yegImage, &tft, patchPointX + oldX - CURSOR_SIZE/2, patchPointY + oldY - CURSOR_SIZE/2 ,
                 oldX- CURSOR_SIZE/2, oldY- CURSOR_SIZE/2, CURSOR_SIZE, CURSOR_SIZE);
  //this will redraw the image in the cursors old position. the 3rd and 4th
  //variables correspondes to the area of the image to draw, which is the same
  //area and size as the cursors old position. It draws the image the same w/h
  //as the curspr
}

void redrawCursor(uint16_t colour, int cursorX, int cursorY) {
  tft.fillRect(cursorX - CURSOR_SIZE/2, cursorY - CURSOR_SIZE/2,
               CURSOR_SIZE, CURSOR_SIZE, colour);
               //simply draws a red 9*9 rectangle


}




void modeZero() {

	int xVal = analogRead(JOY_HORIZ);
	int yVal = analogRead(JOY_VERT);
	//will read the value of the position of the joystick in each loop



int constrainedPatchX = constrain((patchPointX - DISPLAY_WIDTH + 64), 0, 2048-(DISPLAY_WIDTH-64));

	if((yVal >= 500 && yVal <=520) && xVal == 500){
    ;
  }
	// if the values read from the joystick are constant the redraw map function
	// does not fire. After running some tests i found the y value varried by a
	//couples points while the joystick was still hence the range in the yVal
	//section of the if statement
	else{
		redrawMap(cursorX, cursorY);//calls redrawMap function using old values of
		//the cursor
	}

	// now move the cursor
	if (yVal < JOY_CENTER - JOY_DEADZONE) {
		cursorY -= 3; // decrease the y coordinate of the cursor
	}
	else if (yVal > JOY_CENTER + JOY_DEADZONE) {
		cursorY += 3;
	}

	// remember the x-reading increases as we push left
	if (xVal > JOY_CENTER + JOY_DEADZONE) {
		cursorX -= 3;

	}
	else if (xVal < JOY_CENTER - JOY_DEADZONE) {
		cursorX += 3;

	}

	// the following if statements will increase the speed of the cursor by
	//assigning a greater value to the cursor variables if the joystick reads higher/lower
	//than the HIGH_SPEED varialble.
	if(yVal < JOY_CENTER - HIGH_SPEED ){
		cursorY -= 5;
	}
	else if(yVal > JOY_CENTER + HIGH_SPEED){
		cursorY += 5;
	}

	if(xVal < JOY_CENTER - HIGH_SPEED ){
		cursorX += 5;

	}
	else if(xVal > JOY_CENTER + HIGH_SPEED){
		cursorX -= 5;

	}

  //lines 179 to 217 are cases that occur when the cursor reaches the edge of
  //the lcd screen


	if(cursorX < CURSOR_SIZE/2){//when the cursor reaches the left side of the display
    //constrainedPatchXR will prevent the LCD from changing to a screen off the map
    int constrainedPatchXL = constrain((patchPointX - DISPLAY_WIDTH + 64), 0, 2048-(DISPLAY_WIDTH-64));
		lcd_image_draw(&yegImage, &tft, constrainedPatchXL ,patchPointY ,
			0, 0, DISPLAY_WIDTH - 64, DISPLAY_HEIGHT);//redraw the map translated to a new area
      //specific to what side the cursor went to e.g, if the cursor reaches the top
      //of the screen, translate the lcd screen down by DISPLAY_HEIGHT
			//set patchPoint to new positon on the map
      Serial.println(patchPointX);
      if(patchPointX != 0)  {
      patchPointX = constrainedPatchXL;
			cursorX = (DISPLAY_WIDTH -64)/2;//set cursor to the center of the LCD screen
			cursorY = DISPLAY_HEIGHT/2;

			redrawCursor(ILI9341_RED,(DISPLAY_WIDTH -64)/2, DISPLAY_HEIGHT/2);//redraw cursor
      //in center of screen
      }
      else {
        cursorX = constrain(cursorX, 0, 2048- CURSOR_SIZE);
      }



  }

	else if(cursorX > DISPLAY_WIDTH - 65 - CURSOR_SIZE/2 ){//cusor reaches right
    //side of the display. note for this we had issues with clipping so we increased
    // the value of the black portion of the display to 65 since it was clipping
    //by approximately a pixel
      //constrainedPatchXR will prevent the LCD from changing to a screen off the map
      int constrainedPatchXR = constrain((patchPointX + DISPLAY_WIDTH - 64), 0,2048 - (DISPLAY_WIDTH-64) );
			lcd_image_draw(&yegImage, &tft, constrainedPatchXR ,patchPointY ,
				0, 0, DISPLAY_WIDTH - 64, DISPLAY_HEIGHT);
        if(patchPointX != 2048 - (DISPLAY_WIDTH-64)) {
				patchPointX = constrainedPatchXR;
				cursorX = (DISPLAY_WIDTH -64)/2;
				cursorY = DISPLAY_HEIGHT/2;

				redrawCursor(ILI9341_RED,(DISPLAY_WIDTH -64)/2, DISPLAY_HEIGHT/2);
      }
      else {
        cursorX = constrain(cursorX, 0, (DISPLAY_WIDTH-65-CURSOR_SIZE/2));
      }
	}



	if(cursorY < CURSOR_SIZE/2 ){//cursor reaches the top of the display
        //constrainedPatchYT will prevent the LCD from changing to a screen off the map
        int constrainedPatchYT = constrain(patchPointY - DISPLAY_HEIGHT, 0, 2048- DISPLAY_HEIGHT);
				lcd_image_draw(&yegImage, &tft, patchPointX , constrainedPatchYT,
					0, 0, DISPLAY_WIDTH - 64, DISPLAY_HEIGHT);
          if(patchPointY != 0 ) {
					patchPointY = constrainedPatchYT;
					cursorX = (DISPLAY_WIDTH -64)/2;
					cursorY = DISPLAY_HEIGHT/2;

					redrawCursor(ILI9341_RED,(DISPLAY_WIDTH -64)/2, DISPLAY_HEIGHT/2);
        }
        else {
          cursorY = constrain(cursorY, CURSOR_SIZE/2, 2048- DISPLAY_HEIGHT);
        }

	}

	else if( cursorY > DISPLAY_HEIGHT - CURSOR_SIZE/2 ){//cursor reaches the
    //bottom of the display
    //constrainedPatchYB will prevent the LCD from changing to a screen off the map
          int constrainedPatchYB = constrain(patchPointY + DISPLAY_HEIGHT, 0, 2048- DISPLAY_HEIGHT);
          if(patchPointY != (2048 - DISPLAY_HEIGHT)) {
          lcd_image_draw(&yegImage, &tft, patchPointX ,patchPointY + DISPLAY_HEIGHT,
						0, 0, DISPLAY_WIDTH - 64, DISPLAY_HEIGHT);
						patchPointY = constrainedPatchYB;
						cursorX = (DISPLAY_WIDTH -64)/2;
					  cursorY = DISPLAY_HEIGHT/2;

						redrawCursor(ILI9341_RED,(DISPLAY_WIDTH -64)/2, DISPLAY_HEIGHT/2);
          }
          else {
            cursorY = constrain(cursorY, CURSOR_SIZE/2, 2048- DISPLAY_HEIGHT- CURSOR_SIZE);
          }
	}
    //at the end of the loop, redraw the cursor and set the current lat/lon
		redrawCursor(ILI9341_RED, cursorX, cursorY);
    cursorLon = x_to_lon(cursorX+patchPointX);
    cursorLat = y_to_lat(cursorY+patchPointY);
	delay(20);


}



/* Draws the referenced image to the LCD screen.
 *
 * img           : the image to draw
 * tft           : the initialized tft struct
 * icol, irow    : the upper-left corner of the image patch to draw
 * scol, srow    : the upper-left corner of the screen to draw to
 * width, height : controls the size of the patch drawn.
 */
void lcd_image_draw(lcd_image_t *img, Adafruit_ILI9341 *tft,
		    uint16_t icol, uint16_t irow,
		    uint16_t scol, uint16_t srow,
		    uint16_t width, uint16_t height)
{
  File file;

  // Open requested file on SD card if not already open
  if ((file = SD.open(img->file_name)) == NULL) {
    Serial.print("File not found:'");
    Serial.print(img->file_name);
    Serial.println('\'');
    return;  // how do we inform the caller than things went wrong?
  }

  for (uint16_t row=0; row < height; row++) {
    uint16_t pixels[width];
    // Seek to start of pixels to read from, need 32 bit arith for big images
    uint32_t pos = ( (uint32_t) irow +  (uint32_t) row) *
      (2 *  (uint32_t) img->ncols) +  (uint32_t) icol * 2;
    file.seek(pos);

    // Read row of pixels
    if (file.read((uint8_t *) pixels, 2 * width) != 2 * width) {
      Serial.println("SD Card Read Error!");
      file.close();
      return;
    }

		tft->startWrite();
		// Setup display to receive window of pixels
		tft->setAddrWindow(scol, srow+row, width, 1);

    // Send pixels to display
    for (uint16_t col=0; col < width; col++) {
      uint16_t pixel = pixels[col];

      // pixel bytes in reverse order on card
      pixel = (pixel << 8) | (pixel >> 8);
			//tft->writePixel(scol+col, srow+row, pixel);
      tft->pushColor(pixel);
    }
		tft->endWrite();
  }
  file.close();
}

//MODE 2 FUNCTIONS
#define REST_START_BLOCK 4000000
#define NUM_RESTAURANTS 1066

//following functions convert pixel value on Edmonton map to true lat/lon
//and vice versa
int32_t x_to_lon(int16_t x) {
    return map(x, 0, MAP_WIDTH, LON_WEST, LON_EAST);
}

int32_t y_to_lat(int16_t y) {
    return map(y, 0, MAP_HEIGHT, LAT_NORTH, LAT_SOUTH);
}

int16_t lon_to_x(int32_t lon) {
    return map(lon, LON_WEST, LON_EAST, 0, MAP_WIDTH);
}

int16_t lat_to_y(int32_t lat) {
    return map(lat, LAT_NORTH, LAT_SOUTH, 0, MAP_HEIGHT);
}


void modeOne(){

  tft.fillScreen(0);
  tft.setCursor(0, 0); // where the characters will be displayed
  tft.setTextWrap(false);

  testStructAlignment();
  getTop30();
  displayTop30();
}

void testStructAlignment() {
  restaurant rest;

  if ((int) &rest != (int) &rest.lat) {
    Serial.println("latitude not at start");
  }
  else if (((int) &rest) + 4 != (int) &rest.lon) {
    Serial.println("longitude not 4 bytes from the start");
  }
  else if (((int) &rest) + 8 != (int) &rest.rating) {
    Serial.println("rating not 8 bytes from the start");
  }
  else if (((int) &rest) + 9 != (int) &rest.name) {
    Serial.println("name not 9 bytes from the start");
  }
  else {
    Serial.println("restaurant struct alignment is ok");
  }
}

uint32_t manhatten(int32_t cursorLon,int32_t cursorLat, int32_t restx, int32_t resty) {
  int32_t mdx = cursorLon - restx;
  int32_t mdy = cursorLat - resty;
  if(mdx < 0) {
    mdx = -mdx;
  }
  if(mdy < 0) {
    mdy = -mdy;
  }
  int32_t md = mdy + mdx;
  return md;
}

// read the restaurant at position "restIndex" from the card
// and store at the location pointed to by restPtr
uint32_t lastBlockNum = -1;
restaurant restBlockCache[8]; // Caches the last 8 restaurants in the global scope
void getRestaurantFast(int restIndex, restaurant* restPtr) {
  // calculate the block containing this restaurant

  uint32_t blockNum = REST_START_BLOCK + restIndex/8;
  // Keeps track of our last block number

  // fetch the block of restaurants containing the restaurant
  // with index "restIndex" if it's in a new block
  if (lastBlockNum != blockNum) {
    while (!card.readBlock(blockNum, (uint8_t*) restBlockCache)) {
      Serial.println("Read block failed, trying again.");
    }
    lastBlockNum = blockNum;
  }

  // Our cache will always be updated at this point
  // set our pointer to be the index into the block
  *restPtr = restBlockCache[restIndex % 8];
}
void swap_rest(RestDist *ptr_rest1, RestDist *ptr_rest2) {
  RestDist tmp = *ptr_rest1;
  *ptr_rest1 = *ptr_rest2;
  *ptr_rest2 = tmp;
}

// Selection sort
// rest_dist is an array of RestDist, from rest_dist[0] to rest_dist[len-1]
void ssort(RestDist *rest_dist, int len) {
  for (int i = len-1; i >= 1; --i) {
    // Find the index of furthest restaurant
    int max_idx = 0;
    for (int j = 1; j <= i; ++j) {
      if(rest_dist[j].dist > rest_dist[max_idx].dist) {
        max_idx = j;
      }
    }
    // Swap it with the last element of the sub-array
    swap_rest(&rest_dist[i], &rest_dist[max_idx]);
  }
}

RestDist top30[30];

void getTop30() {
  restaurant rest3;
  RestDist restdist[1066];
  for (int i = 0; i < 1066; ++i) {
    getRestaurantFast(i, &rest3);

    restdist[i].dist = manhatten(cursorLon, cursorLat, rest3.lon, rest3.lat);
    restdist[i].index = i;
    if(rest3.lat > 5361858) {
      Serial.println(rest3.name);
    }
    }
    ssort(restdist, 1066);
    for (int j = 0; j < 30; j++) {
      top30[j].dist = restdist[j].dist;
      top30[j].index = restdist[j].index;
      Serial.println(top30[j].dist);
    }
}

void displayTop30() {
  tft.fillScreen(0);
  tft.setCursor(0, 0); // where the characters will be displayed
  tft.setTextWrap(false);
  restaurant r;
  int selectedRest = 0; // which restaurant is selected?
  for (int i = 0; i < 30; i++) {

    getRestaurantFast(top30[i].index, &r);
    if (i != selectedRest) { // not highlighted
      tft.setTextColor(0xFFFF, 0x0000); // white characters on black background
    } else { // highlighted
      tft.setTextColor(0x0000, 0xFFFF); // black characters on white background
    }
    tft.print(r.name);
    tft.print("\n");
  }
  tft.print("\n");

  bool onClick;//holds value of button click
  int joySelectPos;//measures vertical value of joysick

  int cursorPos = 0;// positon of cursor when printing to screen
  int pixelWidth = 8;//size of font used when displaying letters
  int32_t lat;
  int32_t lon;


  while(true){
    onClick = digitalRead(2);
    joySelectPos = analogRead(JOY_VERT);

    if(joySelectPos >  JOY_CENTER + JOY_DEADZONE){
      selectedRest++;//move selection up by one(down the screen)
      ///special case if selection reaches the top of the screen, will loop to
      //the bottom
      if(selectedRest > 29){
        selectedRest = 0;
        getRestaurantFast(top30[selectedRest].index, &r);
        tft.setTextColor(0x0000, 0xFFFF); // black characters on white background
        tft.setCursor(0,0);
        tft.print(r.name);
        getRestaurantFast(top30[29].index, &r);
        tft.setTextColor(0xFFFF, 0x0000);
        tft.setCursor(0 , DISPLAY_HEIGHT-8);
        tft.print(r.name);
        cursorPos = 0;
        continue;//takes to top of the loop again.
      }
      cursorPos = cursorPos + pixelWidth;// set cursor to one selection down
      getRestaurantFast(top30[selectedRest].index, &r);//load restaurant at
      //that index from the sd card
      tft.setTextColor(0x0000, 0xFFFF);// set colour to black on white background
      tft.setCursor(0,cursorPos);//set cursor to new position
      tft.print(r.name);//print name of restaurant at that index
      getRestaurantFast(top30[selectedRest-1].index, &r);
      tft.setTextColor(0xFFFF, 0x0000);
      tft.setCursor(0 , cursorPos -  pixelWidth );
      tft.print(r.name);
    }
    else if(joySelectPos < JOY_CENTER - JOY_DEADZONE){
      selectedRest--;// move down a selection(up the screen)
      ///special case if selection reaches the bottom of the screen, will loop to
      //the top
      if(selectedRest < 0){
        selectedRest = 29;
        getRestaurantFast(top30[selectedRest].index, &r);
        tft.setTextColor(0x0000, 0xFFFF); // black characters on white background
        tft.setCursor(0,DISPLAY_HEIGHT-8);
        tft.print(r.name);
        getRestaurantFast(top30[0].index, &r);
        tft.setTextColor(0xFFFF, 0x0000);
        tft.setCursor(0,0);
        tft.print(r.name);
        cursorPos = DISPLAY_HEIGHT-8;
        continue;
      }
      cursorPos = cursorPos - pixelWidth;
      getRestaurantFast(top30[selectedRest].index, &r);
      tft.setTextColor(0x0000, 0xFFFF);
      tft.setCursor(0,cursorPos);
      tft.print(r.name);
      getRestaurantFast(top30[selectedRest+1].index, &r);
      tft.setTextColor(0xFFFF, 0x0000);
      tft.setCursor(0 , cursorPos +  pixelWidth );
      tft.print(r.name);

    }

    else if( onClick == false){
      getRestaurantFast(top30[selectedRest].index, &r);
      lon = r.lon;

      lat = r.lat;
      reinitMap(lat, lon);// if a click is read exit the loop and load the
      //map

      break;
    }
  }
}

void reinitMap(int32_t lat, int32_t lon){
  /*1.(after button pressed) find lat/lon of rest
  2.find patch point with respect to the lat/lon & convert back to pixels
  3. translate the patch point to lat-DISPLAY_HEIGHT/2, lon - (DISPLAY_WIDTH-64)/2
  4. redraw the map at that point
  5. redraw cursor in middle
  6. reassign the values of patchPointX,Y
  */
  patchPointX = constrain((lon_to_x(lon) - (DISPLAY_WIDTH-64)/2), 0, 2048 - DISPLAY_WIDTH + 64);
  patchPointY = constrain((lat_to_y(lat) - DISPLAY_HEIGHT/2), 0, 2048 - DISPLAY_HEIGHT);
  Serial.println(patchPointY);
  Serial.println(patchPointY);
  Serial.println(patchPointX);
  //case 1: left x
  if(patchPointX == 0 && lon_to_x(lon) >= 0){
    tft.fillScreen(ILI9341_BLACK);
    lcd_image_draw(&yegImage, &tft, patchPointX, patchPointY,0,0,DISPLAY_WIDTH-64,
      DISPLAY_HEIGHT);

      cursorX = lon_to_x(lon) - CURSOR_SIZE/2;
      cursorY = DISPLAY_HEIGHT/2 - CURSOR_SIZE/2;

      redrawCursor(ILI9341_RED, cursorX, cursorY);
    }
    //case 1: right x
    else if(patchPointX == 2048-(DISPLAY_WIDTH-64) && lon_to_x(lon) <= 2048){
      tft.fillScreen(ILI9341_BLACK);
      lcd_image_draw(&yegImage, &tft, patchPointX, patchPointY,0,0,DISPLAY_WIDTH-64,
        DISPLAY_HEIGHT);

        cursorX = (DISPLAY_WIDTH - 64) - (2048 - lon_to_x(lon));
        cursorY = DISPLAY_HEIGHT/2 - CURSOR_SIZE/2;

        redrawCursor(ILI9341_RED, cursorX, cursorY);
      }
      //case 1: up y
      else if(patchPointY == 0 && lat_to_y(lat) >= 0){
        tft.fillScreen(ILI9341_BLACK);
        lcd_image_draw(&yegImage, &tft, patchPointX, patchPointY,0,0,DISPLAY_WIDTH-64,
          DISPLAY_HEIGHT);

          cursorX = (DISPLAY_WIDTH - 64)/2 -CURSOR_SIZE/2;
          cursorY = lat_to_y(lat)- CURSOR_SIZE;

          redrawCursor(ILI9341_RED, cursorX, cursorY);
        }
        //case 1 : down y
        else if(patchPointY == (2048 - DISPLAY_HEIGHT) && lat_to_y(lat) <= 2048){
          tft.fillScreen(ILI9341_BLACK);
          lcd_image_draw(&yegImage, &tft, patchPointX, patchPointY,0,0,DISPLAY_WIDTH-64,
            DISPLAY_HEIGHT);

            cursorX = (DISPLAY_WIDTH - 64)/2 -CURSOR_SIZE/2;
            cursorY = DISPLAY_HEIGHT - (2048 - lat_to_y(lat));

            redrawCursor(ILI9341_RED, cursorX, cursorY);
          }
          //case 2: out of bounds restaurant left
          else if(patchPointX == 0 && lon_to_x(lon) < 0){
            tft.fillScreen(ILI9341_BLACK);
            lcd_image_draw(&yegImage, &tft, patchPointX, patchPointY,0,0,DISPLAY_WIDTH-64,
              DISPLAY_HEIGHT);

              cursorX = 0;
              cursorY = DISPLAY_HEIGHT/2 - CURSOR_SIZE/2;

              redrawCursor(ILI9341_RED, cursorX, cursorY);
            }

            //case 2: out of bounds x rights
            else if((patchPointX == 2048 - (DISPLAY_WIDTH - 64)) && lon_to_x(lon) > 2048){
              tft.fillScreen(ILI9341_BLACK);
              lcd_image_draw(&yegImage, &tft, patchPointX, patchPointY,0,0,DISPLAY_WIDTH-64,
                DISPLAY_HEIGHT);

                cursorX = DISPLAY_WIDTH - 64 - CURSOR_SIZE;
                cursorY = DISPLAY_HEIGHT/2 - CURSOR_SIZE/2;

                redrawCursor(ILI9341_RED, cursorX, cursorY);
              }

              //case 2: out of bounds y up
              else if(patchPointY == 0 && lat_to_y(lat) < 0) {
                tft.fillScreen(ILI9341_BLACK);
                lcd_image_draw(&yegImage, &tft, patchPointX, patchPointY,0,0,DISPLAY_WIDTH-64,
                  DISPLAY_HEIGHT);
                  cursorX = (DISPLAY_WIDTH - 64)/2 -CURSOR_SIZE/2;
                  cursorY =0;

                  redrawCursor(ILI9341_RED, cursorX, cursorY);
                }

                //case 2: out of bounds y down
                else if(patchPointY == 2048 - DISPLAY_HEIGHT && lat_to_y(lat) > 2048){
                  tft.fillScreen(ILI9341_BLACK);
                  lcd_image_draw(&yegImage, &tft, patchPointX, patchPointY,0,0,DISPLAY_WIDTH-64,
                    DISPLAY_HEIGHT);

                    cursorX = (DISPLAY_WIDTH - 64)/2 -CURSOR_SIZE/2;;
                    cursorY = DISPLAY_HEIGHT - CURSOR_SIZE;

                    redrawCursor(ILI9341_RED, cursorX, cursorY);
                  }

                  //case general
                  else{
                    tft.fillScreen(ILI9341_BLACK);
                    lcd_image_draw(&yegImage, &tft, patchPointX, patchPointY,0,0,DISPLAY_WIDTH-64,
                      DISPLAY_HEIGHT);

                      cursorX = (DISPLAY_WIDTH - 64)/2 -CURSOR_SIZE/2;;
                      cursorY = DISPLAY_HEIGHT/2 - CURSOR_SIZE/2;;


                      redrawCursor(ILI9341_RED, cursorX, cursorY);
                    }
                  }
