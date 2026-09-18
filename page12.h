#ifndef page12_h
#define page12_h
#include "GlobalModule.h"
#include "GlobalParameter.h"

void showPage12();
void updateSelectedText(int index);
void toggleCoin(); // สลับเหรียญและอัปเดตการแสดงผล
void toggleCurrency();
void toggleStatus(int index) ;
void saveInputTextToAlert(int index, String newText);
#endif
