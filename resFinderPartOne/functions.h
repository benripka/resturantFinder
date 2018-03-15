
#ifndef FUNCTIONS_H
#define FUNCTIONS_H


//structures to be used in formula
typedef struct {
  char *file_name;
  uint16_t ncols;
  uint16_t nrows;
} lcd_image_t;


struct restaurant {
  int32_t lat;
  int32_t lon;
  uint8_t rating; // from 0 to 10
  char name[55];
};

struct RestDist {
  uint16_t index; // index of restaurant from 0 to NUM_RESTAURANTS-1
  uint16_t dist;  // Manhatten distance to cursor position
};



void setup();


//MODE 0 FUNCTIONS
void redrawMap(int cursorX, int cursorY);

void modeOne();
void modeZero();
void redrawCursor(uint16_t colour, int cursorX, int cursorY, int pCursorX, int pCursorY);

void lcd_image_draw(lcd_image_t *img, Adafruit_ILI9341 *tft,
		    uint16_t icol, uint16_t irow,
		    uint16_t scol, uint16_t srow,
		    uint16_t width, uint16_t height);




//MODE 1 FUNCTIONS


void testStructAlignment();

uint16_t manhatten(uint16_t cursorx,uint16_t cursory, uint16_t restx, uint16_t resty);

void getTop30();

void displayTop30();

void getRestaurantFast(restaurant* restPtr, int length);

void swap_rest(RestDist *ptr_rest1, RestDist *ptr_rest2);

void ssort(RestDist *rest_dist, int len);

void reinitMap(int32_t lat, int32_t lon);

int32_t x_to_lon(int16_t x);

int32_t y_to_lat(int16_t y);

int16_t lon_to_x(int32_t lon);

int16_t lat_to_y(int32_t lat);

#endif
