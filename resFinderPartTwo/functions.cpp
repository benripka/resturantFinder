#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <Adafruit_ILI9341.h>
#include "functions.h"//include header file
#include <TouchScreen.h>


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

#define CURSORCOLOUR ILI9341_YELLOW

#define RADIUS 23.9

#define MAP_WIDTH 2048
#define MAP_HEIGHT 2048
#define LAT_NORTH 5361858l
#define LAT_SOUTH 5340953l
#define LON_WEST -11368652l
#define LON_EAST -11333496l

// These are the four touchscreen analog pins
#define YP A2  // must be an analog pin, use "An" notation!
#define XM A3  // must be an analog pin, use "An" notation!
#define YM 5   // can be a digital pin
#define XP 4   // can be a digital pin

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 150
#define TS_MINY 120
#define TS_MAXX 920
#define TS_MAXY 940

#define MINPRESSURE 10//max and min pressure of the touchscreen
#define MAXPRESSURE 1000

bool button1State = 0;
bool button2State = 0;
bool button3State = 0;
bool button4State = 0;
bool button5State = 0;
//corresponds to the state of each of the 5 rating buttons
//1 being the button is pressed and 0 being it isnt




// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

//following array contains allowed ratings to be read from the restaurant
uint8_t allowedRatings[11] = {11,11,11,11,11,11,11,11,11,11,11};

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
  drawCircles();
	lcd_image_draw(&yegImage, &tft, yegMiddleX, yegMiddleY,
                 0, 0, DISPLAY_WIDTH - 64, DISPLAY_HEIGHT);

  // initial cursor position is the middle of the screen
  cursorX = (DISPLAY_WIDTH -64)/2;
  cursorY = DISPLAY_HEIGHT/2;

  redrawCursor(CURSORCOLOUR, cursorX, cursorY);


}

void drawCircles(){

  //drawing all of the 5 circles
  tft.drawCircle(DISPLAY_WIDTH - 32, RADIUS, RADIUS, ILI9341_YELLOW);
  tft.drawCircle(DISPLAY_WIDTH - 32, 3*RADIUS, RADIUS, ILI9341_YELLOW);
  tft.drawCircle(DISPLAY_WIDTH - 32, 5*RADIUS, RADIUS, ILI9341_YELLOW);
  tft.drawCircle(DISPLAY_WIDTH - 32, 7*RADIUS, RADIUS, ILI9341_YELLOW);
  tft.drawCircle(DISPLAY_WIDTH - 32, 9*RADIUS, RADIUS, ILI9341_YELLOW);

  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(3);

  //prints  number between 1 and 5 between the buttons
  tft.setCursor(DISPLAY_WIDTH - 39, RADIUS - 9);
  tft.print("1");
  tft.setCursor(DISPLAY_WIDTH - 39, 3*(RADIUS) - 9);
  tft.print("2");
  tft.setCursor(DISPLAY_WIDTH - 39, 5*(RADIUS) - 9 );
  tft.print("3");
  tft.setCursor(DISPLAY_WIDTH - 39, 7*(RADIUS) - 9);
  tft.print("4");
  tft.setCursor(DISPLAY_WIDTH - 39, 9*(RADIUS) - 9);
  tft.print("5");
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

  //TODO: add rating circles before map draw.
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

			redrawCursor(CURSORCOLOUR,(DISPLAY_WIDTH -64)/2, DISPLAY_HEIGHT/2);//redraw cursor
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

				redrawCursor(CURSORCOLOUR,(DISPLAY_WIDTH -64)/2, DISPLAY_HEIGHT/2);
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

					redrawCursor(CURSORCOLOUR,(DISPLAY_WIDTH -64)/2, DISPLAY_HEIGHT/2);
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

						redrawCursor(CURSORCOLOUR,(DISPLAY_WIDTH -64)/2, DISPLAY_HEIGHT/2);
          }
          else {
            cursorY = constrain(cursorY, CURSOR_SIZE/2, 2048- DISPLAY_HEIGHT- CURSOR_SIZE);
          }
	}
    //at the end of the loop, redraw the cursor and set the current lat/270lon
		redrawCursor(CURSORCOLOUR, cursorX, cursorY);
    cursorLon = x_to_lon(cursorX+patchPointX);
    cursorLat = y_to_lat(cursorY+patchPointY);
	delay(20);


}

void waitOnTouch(){

  TSPoint p = ts.getPoint();//reads the touchscreen repeatedly and stores the
  //pressure value, and x/y location if it is pressed


  if(p.z != 0 && p.y < 200){//if a touch has been read over the rating selectors
    tft.setTextSize(3);
    if(150 <= p.x && p.x <= 250){//1 star rating button
      if(button1State == 0){//if button pressed while originally off
        button1State = 1;//change state
        tft.fillCircle(DISPLAY_WIDTH - 32, RADIUS, RADIUS-1, ILI9341_WHITE);
        tft.setTextColor(ILI9341_BLACK);
        tft.setCursor(DISPLAY_WIDTH - 39, RADIUS - 9);
        tft.print("1");
        //redraws the circle and number, with the text being black and circle
        //white

        allowedRatings[0] = {0};
        allowedRatings[1] = {1};//will allow restaurants with the ratings to be
        //read from the SD card
        allowedRatings[2] = {2};

      }
      else if(button1State == 1){//turning off that button
        button1State = 0;
        tft.fillCircle(DISPLAY_WIDTH - 32, RADIUS, RADIUS-1, ILI9341_BLACK);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(DISPLAY_WIDTH - 39, RADIUS - 9);
        tft.print("1");
        allowedRatings[0] = {11};//11 corresponds to a unused state AKA
        //no restaurants of this rating will be read from the card
        allowedRatings[1] = {11};
        allowedRatings[2] = {11};
      }
      delay(100);//slows down so that button isnt pressed once and then immediatly again

    }
    else if(300 <= p.x && p.x <= 400){
      if(button2State == 0){//2 star rating button
        button2State = 1;
        tft.fillCircle(DISPLAY_WIDTH - 32, 3*RADIUS, RADIUS-1, ILI9341_WHITE);
        tft.setTextColor(ILI9341_BLACK);
        tft.setCursor(DISPLAY_WIDTH - 39, 3*RADIUS - 9);
        tft.print("2");
        allowedRatings[3] = {3};
        allowedRatings[4] = {4};
      }
      else if(button2State == 1){
        button2State = 0;
        tft.fillCircle(DISPLAY_WIDTH - 32, 3*RADIUS, RADIUS-1, ILI9341_BLACK);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(DISPLAY_WIDTH - 39, 3*RADIUS - 9);
        tft.print("2");
        allowedRatings[3] = {11};
        allowedRatings[4] = {11};
      }

      delay(100);
    }
    else if(450 <= p.x && p.x <= 550){
      if(button3State == 0){//3 star rating button
        button3State = 1;
        tft.fillCircle(DISPLAY_WIDTH - 32, 5*RADIUS, RADIUS-1, ILI9341_WHITE);
        tft.setTextColor(ILI9341_BLACK);
        tft.setCursor(DISPLAY_WIDTH - 39, 5*RADIUS - 9);
        tft.print("3");
        allowedRatings[5] = {5};
        allowedRatings[6] = {6};
      }
      else if(button3State == 1){
        button3State = 0;
        tft.fillCircle(DISPLAY_WIDTH - 32, 5*RADIUS, RADIUS-1, ILI9341_BLACK);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(DISPLAY_WIDTH - 39, 5*RADIUS - 9);
        tft.print("3");
        allowedRatings[5] = {11};
        allowedRatings[6] = {11};
      }

      delay(100);
    }
    else if(600 <= p.x && p.x <= 700){
      if(button4State == 0){//4 star rating button
        button4State = 1;
        tft.fillCircle(DISPLAY_WIDTH - 32, 7*RADIUS, RADIUS-1, ILI9341_WHITE);
        tft.setTextColor(ILI9341_BLACK);
        tft.setCursor(DISPLAY_WIDTH - 39, 7*RADIUS - 9);
        tft.print("4");
        allowedRatings[7] = {7};
        allowedRatings[8] = {8};
      }
      else if(button4State == 1){
        button4State = 0;
        tft.fillCircle(DISPLAY_WIDTH - 32, 7*RADIUS, RADIUS-1, ILI9341_BLACK);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(DISPLAY_WIDTH - 39, 7*RADIUS - 9);
        tft.print("4");
        allowedRatings[7] = {11};
        allowedRatings[8] = {11};
      }

      delay(100);
    }
    else if(750 <= p.x && p.x <= 850){
      if(button5State == 0){//5 star rating button
        button5State = 1;
        tft.fillCircle(DISPLAY_WIDTH - 32, 9*RADIUS, RADIUS-1, ILI9341_WHITE);
        tft.setTextColor(ILI9341_BLACK);
        tft.setCursor(DISPLAY_WIDTH - 39, 9*RADIUS - 9);
        tft.print("5");
        allowedRatings[9] = {9};
        allowedRatings[10] ={10};
      }
      else if(button5State == 1){
        button5State = 0;
        tft.fillCircle(DISPLAY_WIDTH - 32, 9*RADIUS, RADIUS-1, ILI9341_BLACK);
        tft.setTextColor(ILI9341_WHITE);
        tft.setCursor(DISPLAY_WIDTH - 39, 9*RADIUS - 9);
        tft.print("5");
        allowedRatings[9] = {11};
        allowedRatings[10] = {11};
      }

      delay(100);
    }
  }
}

/* Draws the referenced image to the LCD screen.
 *
 * img           : the image to draw
 * tft           : the initialized tft struct
 * icol, irow    : the upper-left corner of the image patch to draw
 * scol, srow    : the upper-left corner of the screen to draw to
 * width, height : controls the size of the patch drawn.int index = -1;
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

//MODE 1 FUNCTIONS
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

  uint8_t modeOneMin = 0;//this is the minmum distance that restaurants can be
  //loaded from the SD card, since the list originally displays the closest restaurants
  //to the cursor this value will be zero. Explained more in the getTop30 function.

  if(!((button1State == 0) && (button2State == 0) && (button3State == 0) &&
  (button4State == 0) && (button5State == 0))){//if atleast one of the buttons is
    //pressed launches the restaurant list function
    tft.fillScreen(0);
    tft.setCursor(0, 0); // where the characters will be displayed
    tft.setTextWrap(false);
    tft.setTextSize(1);
    testStructAlignment();
    getTop30(modeOneMin);//passes the minmum value to the getTop30 function(0)
    displayTop30(0);
  }

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

RestDist top30[30];



void getTop30(int dist) {
  restaurant rest3;//creates restaurant object called rest3
  RestDist restdist[1066];//holds the distances of the restaurants from the cursor
  uint8_t rating;//will hold the rating of the restaurants
  int numRests = -1;//index value
  int minDistance = dist;//all restaurants must be greater than this value to be
  //read from the sd card
  int distance;

  for (int i = 0; i < 1066; ++i) {
    /*reads the restaurants one at a time from the SD card and computes their distance
    + stores them in the top30 array if and only if the computed manhatten distance
    is greater then the minmum distance which is used when scrolling the restaurants in the
    list. Restaurants must also match the allowed ratings specified by the rating
    selection. */

    getRestaurantFast(i, &rest3);
    rating = rest3.rating;
    distance = manhatten(cursorLon, cursorLat, rest3.lon, rest3.lat);//computes
    //manhatten distance

    for(int j = 0; j < 11; j++){//j < 11 due to 10 possible ratings plus 11 for
      // if the rating is not selected as an option
      if(rating == allowedRatings[j] && distance >= minDistance){
        numRests++;//increase index

        restdist[numRests].dist = distance;
        restdist[numRests].index = i;
        //restdist will hold all restaurants that forfill the criteria specified
        //by the ratings and the minmum distance

      }
    }

  }

  qsort(restdist, 0, numRests);//sorts the restaurants using qsort algorithm
  //from smallest distance from the cursor to greatest


  for (int j = 0; j < 30; j++) {
    top30[j].dist = restdist[j].dist;
    top30[j].index = restdist[j].index;
  }//adds them to the top30 index

}

void swap_rest(RestDist *ptr_rest1, RestDist *ptr_rest2) {
  RestDist tmp = *ptr_rest1;
  *ptr_rest1 = *ptr_rest2;
  *ptr_rest2 = tmp;
}

int pivot(RestDist *rest_dist, int low, int high)
{
    RestDist pivot = rest_dist[high];    // pivot
    int i = (low - 1);  // Index of smaller element

    for (int j = low; j <= high- 1; j++)
    {
        // If current element is smaller than or
        // equal to pivot
        if (rest_dist[j].dist <= pivot.dist)
        {
            i++;    // increment index of smaller element
            swap_rest(&rest_dist[i], &rest_dist[j]);
        }
    }
    swap_rest(&rest_dist[i + 1], &rest_dist[high]);
    return (i + 1);
}

void qsort(RestDist *rest_dist, int low, int high)
{
  if (low < high)
  {
    /* pi is partitioning index, arr[p] is now
   at right place */
    int pi = pivot(rest_dist, low, high);

    // Separately sort elements before
    // partition and after partition
    qsort(rest_dist, low, pi - 1);
    qsort(rest_dist, pi + 1, high);
  }
}

/*each section of 30 restaurants in the mode one function are divided into sets;
the smallest distance in this set will be stored in the array distArr. set increases
when cursor scrolls to next section of restaurants. when scrolling back up
to a previous set the distArr specifies the minimum distance those restaurants
must have in order to keep them in order, hence the pass of the minDistance varialbe
to the getTop30 function. */

int distArr[36] = {0};
int set = 0;



void displayTop30(int currentLocal) {
  tft.fillScreen(0);
  tft.setCursor(0, 0); // where the characters will be displayed
  tft.setTextWrap(false);
  restaurant r;
  int selectedRest = currentLocal; // which restaurant is selected?
  for (int i = 0; i < 30; i++) {


    getRestaurantFast(top30[i].index, &r);


    if (i != selectedRest) { // not highlighted
      tft.setTextColor(0xFFFF, 0x0000); // white characters on black background
    } else { // highlighted
      tft.setTextColor(0x0000, 0xFFFF); // black characters on white background
    }
    tft.print(r.name);
    //tft.print(r.rating);

    tft.print("\n");
  }
  tft.print("\n");

  bool onClick;//holds value of button click
  int joySelectPos;//measures vertical value of joysick

  int cursorPos = 0;// positon of cursor when printing to screen
  int pixelWidth = 8;//size of font used when displaying letters

  int32_t lat;//will hold the latitude amnd longitude of the restaurant when
  int32_t lon;//one is selected


  while(true){
    onClick = digitalRead(2);//cosntantly reads the y value of the joystick and
    //if the joystick button has been pressed.
    joySelectPos = analogRead(JOY_VERT);
    delay(50);//slows down the shift between restaurants, easier for user to
    //navigate
    set = constrain(set,0,36);// does not allow the set to be higher than 36
    //since 1066/30=~ 36; keeps segmentation fault from occuring since max array
    //index is 36


    if(joySelectPos >  JOY_CENTER + JOY_DEADZONE){
      selectedRest++;//move selection up by one(down the screen)
      ///special case if selection reaches the top of the screen, will loop to
      //the bottom
      if(selectedRest > 29){//bottom of screen is reached
        distArr[set] = top30[0].dist;//adds minimum restaurant distance to array
        getTop30(top30[29].dist);//calls getTop30 making sure than all restaurants
        //read into the top30 struct have a distance greater than the max distance
        // in this set
        set++;//increases set
        displayTop30(0);//recurson used   and passes the value of the starting
        //selection. since we are going from the bottom of a list to the top
        // of a new one a proper 'scrolling' feature would keep the selection
        //at the top of the array
        break;//if the displayTop30 function has been called multiple times,
        //when the function exits it and the program needs to return to the map,
        //all displayTop30 functions must also quit hence the break statement

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
      if(selectedRest < 0){// top
        set--;//decrease set
        getTop30(distArr[set]);//call getTop30 using the minimum values of the
        //previous sets minimum distance
        displayTop30(29);//starting location now the bottom of the previous set
        break;
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
      button1State = 0;//set all rating buttons to zero for simplicity
      button2State = 0;
      button3State = 0;
      button4State = 0;
      button5State = 0;
      reinitMap(lat, lon);// if a click is read exit the loop and load the
      //map

      break;//ends while loop and therefor the function
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
    drawCircles();
    lcd_image_draw(&yegImage, &tft, patchPointX, patchPointY,0,0,DISPLAY_WIDTH-64,
      DISPLAY_HEIGHT);

      cursorX = lon_to_x(lon) - CURSOR_SIZE/2;
      cursorY = DISPLAY_HEIGHT/2 - CURSOR_SIZE/2;

      redrawCursor(CURSORCOLOUR, cursorX, cursorY);
    }
    //case 1: right x
    else if(patchPointX == 2048-(DISPLAY_WIDTH-64) && lon_to_x(lon) <= 2048){
      tft.fillScreen(ILI9341_BLACK);
      drawCircles();
      lcd_image_draw(&yegImage, &tft, patchPointX, patchPointY,0,0,DISPLAY_WIDTH-64,
        DISPLAY_HEIGHT);

        cursorX = (DISPLAY_WIDTH - 64) - (2048 - lon_to_x(lon));
        cursorY = DISPLAY_HEIGHT/2 - CURSOR_SIZE/2;

        redrawCursor(CURSORCOLOUR, cursorX, cursorY);
      }
      //case 1: up y
      else if(patchPointY == 0 && lat_to_y(lat) >= 0){
        tft.fillScreen(ILI9341_BLACK);
        drawCircles();
        lcd_image_draw(&yegImage, &tft, patchPointX, patchPointY,0,0,DISPLAY_WIDTH-64,
          DISPLAY_HEIGHT);

          cursorX = (DISPLAY_WIDTH - 64)/2 -CURSOR_SIZE/2;
          cursorY = lat_to_y(lat)- CURSOR_SIZE;

          redrawCursor(CURSORCOLOUR, cursorX, cursorY);
        }
        //case 1 : down y
        else if(patchPointY == (2048 - DISPLAY_HEIGHT) && lat_to_y(lat) <= 2048){
          tft.fillScreen(ILI9341_BLACK);
          drawCircles();
          lcd_image_draw(&yegImage, &tft, patchPointX, patchPointY,0,0,DISPLAY_WIDTH-64,
            DISPLAY_HEIGHT);

            cursorX = (DISPLAY_WIDTH - 64)/2 -CURSOR_SIZE/2;
            cursorY = DISPLAY_HEIGHT - (2048 - lat_to_y(lat));

            redrawCursor(CURSORCOLOUR, cursorX, cursorY);
          }
          //case 2: out of bounds restaurant left
          else if(patchPointX == 0 && lon_to_x(lon) < 0){
            tft.fillScreen(ILI9341_BLACK);
            drawCircles();
            lcd_image_draw(&yegImage, &tft, patchPointX, patchPointY,0,0,DISPLAY_WIDTH-64,
              DISPLAY_HEIGHT);

              cursorX = 0;
              cursorY = DISPLAY_HEIGHT/2 - CURSOR_SIZE/2;

              redrawCursor(CURSORCOLOUR, cursorX, cursorY);
            }

            //case 2: out of bounds x rights
            else if((patchPointX == 2048 - (DISPLAY_WIDTH - 64)) && lon_to_x(lon) > 2048){
              tft.fillScreen(ILI9341_BLACK);
              drawCircles();
              lcd_image_draw(&yegImage, &tft, patchPointX, patchPointY,0,0,DISPLAY_WIDTH-64,
                DISPLAY_HEIGHT);

                cursorX = DISPLAY_WIDTH - 64 - CURSOR_SIZE;
                cursorY = DISPLAY_HEIGHT/2 - CURSOR_SIZE/2;

                redrawCursor(CURSORCOLOUR, cursorX, cursorY);
              }

              //case 2: out of bounds y up
              else if(patchPointY == 0 && lat_to_y(lat) < 0) {
                tft.fillScreen(ILI9341_BLACK);
                drawCircles();
                lcd_image_draw(&yegImage, &tft, patchPointX, patchPointY,0,0,DISPLAY_WIDTH-64,
                  DISPLAY_HEIGHT);
                  cursorX = (DISPLAY_WIDTH - 64)/2 -CURSOR_SIZE/2;
                  cursorY =0;

                  redrawCursor(CURSORCOLOUR, cursorX, cursorY);
                }

                //case 2: out of bounds y down
                else if(patchPointY == 2048 - DISPLAY_HEIGHT && lat_to_y(lat) > 2048){
                  tft.fillScreen(ILI9341_BLACK);
                  drawCircles();
                  lcd_image_draw(&yegImage, &tft, patchPointX, patchPointY,0,0,DISPLAY_WIDTH-64,
                    DISPLAY_HEIGHT);

                    cursorX = (DISPLAY_WIDTH - 64)/2 -CURSOR_SIZE/2;;
                    cursorY = DISPLAY_HEIGHT - CURSOR_SIZE;

                    redrawCursor(CURSORCOLOUR, cursorX, cursorY);
                  }

                  //case general
                  else{
                    tft.fillScreen(ILI9341_BLACK);
                    drawCircles();
                    lcd_image_draw(&yegImage, &tft, patchPointX, patchPointY,0,0,DISPLAY_WIDTH-64,
                      DISPLAY_HEIGHT);

                      cursorX = (DISPLAY_WIDTH - 64)/2 -CURSOR_SIZE/2;;
                      cursorY = DISPLAY_HEIGHT/2 - CURSOR_SIZE/2;;


                      redrawCursor(CURSORCOLOUR, cursorX, cursorY);
                    }
                  }
