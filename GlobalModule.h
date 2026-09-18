
#ifndef GlobalModule_h
#define GlobalModule_h
#include <TFT_eSPI.h>
#include <XPT2046_Bitbang.h>

#include <FS.h>
extern TFT_eSPI tft; // Declare tft as an external variable

struct ImageInfo {
  const char* imageName;
  const char* nameSurname;
  const char* position;
  const char* companyName;
};

// Example array of images and their associated text
extern ImageInfo images[];

//touch screen
extern XPT2046_Bitbang ts;


#endif
