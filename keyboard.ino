/* file: keyboard.ino
 *
 * Copyright 2020 Josh Boudreau
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <Keyboard.h>

#define VOL_UP 115
#define VOL_DN 114
#define BRI_UP 225
#define BRI_DN 224

// modifier key indexes
#define L_SHIFT_ROW 3
#define L_SHIFT_COL 0
#define R_SHIFT_ROW 3
#define R_SHIFT_COL 12

#define L_CTRL_ROW 4
#define L_CTRL_COL 0

#define L_ALT_ROW 4
#define L_ALT_COL 2
#define R_ALT_ROW 4
#define R_ALT_COL 10

#define WIN_ROW 4
#define WIN_COL 1

#define FN_ROW 4
#define FN_COL 11

// modifier key index tests
#define LSHIFT(x,y) (x == L_SHIFT_ROW && y == L_SHIFT_COL)
#define RSHIFT(x,y) (x == R_SHIFT_ROW && y == R_SHIFT_COL)
#define CTRL(x,y) (x == L_CTRL_ROW && y == L_CTRL_COL)
#define LALT(x,y) (x == L_ALT_ROW && y == L_ALT_COL)
#define RALT(x,y) (x == R_ALT_ROW && y == R_ALT_COL)
#define WIN(x,y) (x == WIN_ROW && y == WIN_COL)
#define FN(x,y) (x == FN_ROW && y == FN_COL)

#define N_KEYS 6 // number of keys to be sent at once (6 is max for teensyduino)

#define DEBOUNCE_LIM 50 // +/- threshold for integrator debounce

#define ROWS 5
#define COLS 16

uint8_t rowIndex[ROWS] = {2, 3, 4, 5, 6}; // pins to which rows are connected
uint8_t colIndex[COLS] = {7, 8, 9, 10, 11, 12, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23}; // and columns
uint16_t keys[ROWS][COLS] = { // big map of keycodes
  {KEY_ESC,KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9,KEY_0,KEY_MINUS,KEY_EQUAL,KEY_TILDE,KEY_BACKSPACE,KEY_DELETE},
  {KEY_TAB,KEY_Q,KEY_W,KEY_E,KEY_R,KEY_T,KEY_Y,KEY_U,KEY_I,KEY_O,KEY_P,KEY_LEFT_BRACE,KEY_RIGHT_BRACE,0,KEY_BACKSLASH,KEY_PAGE_UP},
  {KEY_CAPS_LOCK,KEY_A,KEY_S,KEY_D,KEY_F,KEY_G,KEY_H,KEY_J,KEY_K,KEY_L,KEY_SEMICOLON,KEY_QUOTE,0,KEY_ENTER,0,KEY_PAGE_DOWN},
  {MODIFIERKEY_SHIFT,0,KEY_Z,KEY_X,KEY_C,KEY_V,KEY_B,KEY_N,KEY_M,KEY_COMMA,KEY_PERIOD,KEY_SLASH,MODIFIERKEY_RIGHT_SHIFT,0,KEY_UP,KEY_END},
  {MODIFIERKEY_CTRL,MODIFIERKEY_GUI,MODIFIERKEY_ALT,0,0,0,KEY_SPACE,0,0,0,MODIFIERKEY_RIGHT_ALT,0,0,KEY_LEFT,KEY_DOWN,KEY_RIGHT}
};
uint16_t fn_keys[ROWS][COLS] = { // second layer map for when Fn is held
  {0,KEY_F1,KEY_F2,KEY_F3,KEY_F4,KEY_F5,KEY_F6,KEY_F7,KEY_F8,KEY_F9,KEY_F10,KEY_F11,KEY_F12,0,0,0},
  {0,KEY_7,KEY_8,KEY_9,0,0,0,0,0,0,0,0,0,0,0,KEY_END},
  {0,KEY_4,KEY_5,KEY_6,0,0,0,0,0,0,0,0,0,0,0,KEY_HOME},
  {0,0,KEY_1,KEY_2,KEY_3,0,0,0,0,0,0,0,0,0,VOL_UP,0},
  {0,0,0,0,0,0,KEY_0,0,0,0,0,0,0,BRI_DN,VOL_DN,BRI_UP}
};
uint8_t held[ROWS][COLS] = {0}; // locks to send and release the key only once per press

uint8_t n_key_held[N_KEYS] = {0}; // locks to help with N_KEYS-rollover

int8_t debounce[ROWS][COLS] = {0}; // debounce integrators

void send_key(uint8_t index, uint16_t key);
// allows for numeric indexing of Keyboard.set_key<n>() functions

void setup() {
  Serial.begin(9600);
  int i;
  for(i = 0; i <= COLS; i++){
    pinMode(colIndex[i], INPUT_PULLUP);
  }
  for(i = 0; i <= ROWS; i++){
    pinMode(rowIndex[i], OUTPUT);
    digitalWrite(rowIndex[i],HIGH);
  }
}


void loop() {
  int i, j;
  uint8_t n_key_index = 0;
  for(i = 0; i < ROWS; i++){
    // all but one row is held high
    digitalWrite(rowIndex[i], LOW);
    for(j = 0; j < COLS; j++){
      // if switch is closed, pulls that column low through diode
      if(digitalRead(colIndex[j]) == LOW){
        // key pressed
        // debounce through discreet integration to threshold
        // closed switch increments, open switch decrements
        if(debounce[i][j] < DEBOUNCE_LIM){
          debounce[i][j] += 1;
          continue;
        }
        debounce[i][j] = 0;
        if(held[i][j] == 0){
          // this lock ensures keypress only sent once
          held[i][j] = 1;
          if(LSHIFT(i,j)){
            Keyboard.press(MODIFIERKEY_SHIFT);
          }else if(RSHIFT(i,j)){
            Keyboard.press(MODIFIERKEY_RIGHT_SHIFT);
          }else if(CTRL(i,j)){
            Keyboard.press(MODIFIERKEY_CTRL);
          }else if(LALT(i,j)){
            Keyboard.press(MODIFIERKEY_ALT);
          }else if(RALT(i,j)){
            Keyboard.press(MODIFIERKEY_RIGHT_ALT);
          }else if(WIN(i,j)){
            Keyboard.press(MODIFIERKEY_GUI);
          }else if(!FN(i,j)){
            uint16_t keycode = keys[i][j];
            if(held[FN_ROW][FN_COL]){
              keycode = fn_keys[i][j];
            }
            // insert key code into lowest available Keyboard.set_key<n>
            for(n_key_index = 0; n_key_index < N_KEYS; n_key_index++){
              if(!n_key_held[n_key_index]) break;
            }
            n_key_held[n_key_index] = 1;
            held[i][j] = n_key_index + 1; // save index in held[][]
            send_key(n_key_index,keycode);
          }
        }
      }else{
        // same debounce but negated
        if(debounce[i][j] > -DEBOUNCE_LIM){
          debounce[i][j] -= 1;
          continue;
        }
        debounce[i][j] = 0;
        if(held[i][j]){ // only once when not held
          // since value of held[][] is needed for n_key_held[] index,
          // it is cleared later
          if(LSHIFT(i,j)){
            Keyboard.release(MODIFIERKEY_SHIFT);
          }else if(RSHIFT(i,j)){
            Keyboard.release(MODIFIERKEY_RIGHT_SHIFT);
          }else if(CTRL(i,j)){
            Keyboard.release(MODIFIERKEY_CTRL);
          }else if(LALT(i,j)){
            Keyboard.release(MODIFIERKEY_ALT);
          }else if(RALT(i,j)){
            Keyboard.release(MODIFIERKEY_RIGHT_ALT);
          }else if(WIN(i,j)){
            Keyboard.release(MODIFIERKEY_GUI);
          }else if(!FN(i,j)){
            uint8_t index = held[i][j] - 1;
            send_key(index,0); // clear key code
            n_key_held[index] = 0; // free up spot for next key press
          }
          held[i][j] = 0;
        }
      }
    }
    digitalWrite(rowIndex[i], HIGH);
  }
}

void send_key(uint8_t index, uint16_t key){
  // allows for calling of Keyboard.set_key<n>() with
  // a numeric index
  switch(index){
  case 0:
    Keyboard.set_key1(key);
    break;
  case 1:
    Keyboard.set_key2(key);
    break;
  case 2:
    Keyboard.set_key3(key);
    break;
  case 3:
    Keyboard.set_key4(key);
    break;
  case 4:
    Keyboard.set_key5(key);
    break;
  case 5:
    Keyboard.set_key6(key);
    break;
  default:
    Serial.printf("Out of bound index in press_key: index = %d",index);
    break;
  }
  Keyboard.send_now();
}
