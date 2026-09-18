#ifndef CC_LITTLEFS_H
#define CC_LITTLEFS_H

#include <Arduino.h>
#include <ArduinoJson.h>
#include <FS.h>

extern bool load_config();
extern bool save_config();
extern bool save_default_config();

extern bool load_wifi();
extern bool save_wifi();
extern bool save_default_wifi();

extern bool load_profile();
extern bool save_profile();
extern bool save_default_profile();

extern bool load_usage();
extern bool save_usage();
extern bool save_default_usage();

extern bool load_alert();
extern bool save_alert();
extern bool save_default_alert();

extern bool load_coin();
extern bool save_coin();
extern bool save_default_coin();

extern bool load_ads();
extern bool save_ads();
extern bool save_default_ads();

extern void load_all();
extern void save_all();
extern void save_all_defaults();
extern void check_and_create_all();

#endif
