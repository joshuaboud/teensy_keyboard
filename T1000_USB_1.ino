#include <Keyboard.h>

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

#define SHIFT(x,y) (x == L_SHIFT_ROW && y == L_SHIFT_COL || x == R_SHIFT_ROW && y == R_SHIFT_COL)
#define CTRL(x,y) (x == L_CTRL_ROW && y == L_CTRL_COL)
#define ALT(x,y) (x == L_ALT_ROW && y == L_ALT_COL || x == R_ALT_ROW && y == R_ALT_COL)
#define WIN(x,y) (x == WIN_ROW && y == WIN_COL)
#define FN(x,y) (x == FN_ROW && y == FN_COL)

const byte ROWS = 5; //eight rows
const byte COLS = 16; //sixteen columns
char rowIndex[ROWS] = {2, 3, 4, 5, 6};
char colIndex[COLS] = {7, 8, 9, 10, 11, 12, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
char keys[ROWS][COLS] = {
  {KEY_ESC,KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9,KEY_0,KEY_MINUS,KEY_EQUAL,KEY_TILDE,KEY_BACKSPACE,KEY_DELETE},
  {KEY_TAB,KEY_Q,KEY_W,KEY_E,KEY_R,KEY_T,KEY_Y,KEY_U,KEY_I,KEY_O,KEY_P,KEY_LEFT_BRACE,KEY_RIGHT_BRACE,'0',KEY_BACKSLASH,KEY_PAGE_UP},
  {KEY_CAPS_LOCK,KEY_A,KEY_S,KEY_D,KEY_F,KEY_G,KEY_H,KEY_J,KEY_K,KEY_L,KEY_SEMICOLON,KEY_QUOTE,'1',KEY_ENTER,'1',KEY_PAGE_DOWN},
  {KEY_LEFT_SHIFT,'1',KEY_Z,KEY_X,KEY_C,KEY_V,KEY_B,KEY_N,KEY_M,KEY_COMMA,KEY_PERIOD,KEY_SLASH,KEY_RIGHT_SHIFT,'1',KEY_UP,KEY_END},
  {KEY_LEFT_CTRL,KEY_LEFT_GUI,KEY_LEFT_ALT,'1','2','3',KEY_SPACE,'1','2','3',KEY_RIGHT_ALT,'9','1',KEY_LEFT,KEY_DOWN,KEY_RIGHT}
};
char fn_keys[ROWS][COLS] = {
  {0,KEY_F1,KEY_F2,KEY_F3,KEY_F4,KEY_F5,KEY_F6,KEY_F7,KEY_F8,KEY_F9,KEY_F10,KEY_F11,KEY_F12,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,KEY_END},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,KEY_HOME},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
  {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}
};
unsigned char held[ROWS][COLS] = {0};

void setup() {
    int i;
    for(i = 0; i <= COLS; i++){
      pinMode(colIndex[i], OUTPUT);
      digitalWrite(i, LOW);
    }
    for(i = 0; i <= ROWS; i++){
      pinMode(rowIndex[i], INPUT_PULLDOWN);
    }
}


void loop() {
  int i, j;
  for(i = 0; i < COLS; i++){
    digitalWrite(colIndex[i], HIGH);
    for(j = 0; j < ROWS; j++){
      if(digitalRead(rowIndex[j]) == HIGH){
        // key pressed
        if(held[j][i] == 0){
          held[j][i] = 1;
          if(SHIFT(j,i)){
            Keyboard.press(MODIFIERKEY_SHIFT);
          }else if(CTRL(j,i)){
            Keyboard.press(MODIFIERKEY_CTRL);
          }else if(ALT(j,i)){
            Keyboard.press(MODIFIERKEY_ALT);
          }else if(WIN(j,i)){
            Keyboard.press(MODIFIERKEY_GUI);
          }else if(!FN(j,i)){
            char keycode = keys[j][i];
            if(held[FN_ROW][FN_COL]){
              keycode = fn_keys[j][i];
            }
            Keyboard.set_key1(keycode);
            Keyboard.send_now();
          }
        }
      }else if(held[j][i]){ // only once when not held
        if(SHIFT(j,i)){
          Keyboard.release(MODIFIERKEY_SHIFT);
        }else if(CTRL(j,i)){
          Keyboard.release(MODIFIERKEY_CTRL);
        }else if(ALT(j,i)){
          Keyboard.release(MODIFIERKEY_ALT);
        }else if(WIN(j,i)){
          Keyboard.release(MODIFIERKEY_GUI);
        }else if(!FN(j,i)){
          Keyboard.set_key1(0);
          Keyboard.send_now();
        }
        held[j][i] = 0;
      }
    }
    digitalWrite(colIndex[i], LOW);
  }
}
