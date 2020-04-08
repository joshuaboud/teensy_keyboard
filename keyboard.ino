#include <Keyboard.h>

#define VOL_UP 115
#define VOL_DN 114
#define BRI_UP 225
#define BRI_DN 224

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

#define LSHIFT(x,y) (x == L_SHIFT_ROW && y == L_SHIFT_COL)
#define RSHIFT(x,y) (x == R_SHIFT_ROW && y == R_SHIFT_COL)
#define CTRL(x,y) (x == L_CTRL_ROW && y == L_CTRL_COL)
#define LALT(x,y) (x == L_ALT_ROW && y == L_ALT_COL)
#define RALT(x,y) (x == R_ALT_ROW && y == R_ALT_COL)
#define WIN(x,y) (x == WIN_ROW && y == WIN_COL)
#define FN(x,y) (x == FN_ROW && y == FN_COL)

#define N_KEYS 6

#define DEBOUNCE_LIM 50

#define ROWS 5
#define COLS 16

uint8_t rowIndex[ROWS] = {2, 3, 4, 5, 6};
uint8_t colIndex[COLS] = {7, 8, 9, 10, 11, 12, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23};
uint16_t keys[ROWS][COLS] = {
  {KEY_ESC,KEY_1,KEY_2,KEY_3,KEY_4,KEY_5,KEY_6,KEY_7,KEY_8,KEY_9,KEY_0,KEY_MINUS,KEY_EQUAL,KEY_TILDE,KEY_BACKSPACE,KEY_DELETE},
  {KEY_TAB,KEY_Q,KEY_W,KEY_E,KEY_R,KEY_T,KEY_Y,KEY_U,KEY_I,KEY_O,KEY_P,KEY_LEFT_BRACE,KEY_RIGHT_BRACE,'0',KEY_BACKSLASH,KEY_PAGE_UP},
  {KEY_CAPS_LOCK,KEY_A,KEY_S,KEY_D,KEY_F,KEY_G,KEY_H,KEY_J,KEY_K,KEY_L,KEY_SEMICOLON,KEY_QUOTE,'1',KEY_ENTER,'1',KEY_PAGE_DOWN},
  {MODIFIERKEY_SHIFT,'1',KEY_Z,KEY_X,KEY_C,KEY_V,KEY_B,KEY_N,KEY_M,KEY_COMMA,KEY_PERIOD,KEY_SLASH,MODIFIERKEY_RIGHT_SHIFT,'1',KEY_UP,KEY_END},
  {MODIFIERKEY_CTRL,MODIFIERKEY_GUI,MODIFIERKEY_ALT,'1','2','3',KEY_SPACE,'1','2','3',MODIFIERKEY_RIGHT_ALT,'9','1',KEY_LEFT,KEY_DOWN,KEY_RIGHT}
};
uint16_t fn_keys[ROWS][COLS] = {
  {0,KEY_F1,KEY_F2,KEY_F3,KEY_F4,KEY_F5,KEY_F6,KEY_F7,KEY_F8,KEY_F9,KEY_F10,KEY_F11,KEY_F12,0,0,0},
  {0,KEY_7,KEY_8,KEY_9,0,0,0,0,0,0,0,0,0,0,0,KEY_END},
  {0,KEY_4,KEY_5,KEY_6,0,0,0,0,0,0,0,0,0,0,0,KEY_HOME},
  {0,0,KEY_1,KEY_2,KEY_3,0,0,0,0,0,0,0,0,0,VOL_UP,0},
  {0,0,0,0,0,0,KEY_0,0,0,0,0,0,0,BRI_DN,VOL_DN,BRI_UP}
};
uint8_t held[ROWS][COLS] = {0};

uint8_t n_key_held[N_KEYS] = {0};

int8_t debounce[ROWS][COLS] = {0};

void send_key(uint8_t index, uint16_t key);

void setup() {
  Serial.begin(9600);
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
  uint8_t n_key_index = 0;
  for(i = 0; i < COLS; i++){
    digitalWrite(colIndex[i], HIGH);
    for(j = 0; j < ROWS; j++){
      if(digitalRead(rowIndex[j]) == HIGH){
        // key pressed
        if(debounce[j][i] < DEBOUNCE_LIM){
          debounce[j][i] += 1;
          continue;
        }
        debounce[j][i] = 0;
        if(held[j][i] == 0){
          Serial.printf("Pressing (%d,%d)\n",j,i);
          held[j][i] = 1;
          if(LSHIFT(j,i)){
            Keyboard.press(MODIFIERKEY_SHIFT);
          }else if(RSHIFT(j,i)){
            Keyboard.press(MODIFIERKEY_RIGHT_SHIFT);
          }else if(CTRL(j,i)){
            Keyboard.press(MODIFIERKEY_CTRL);
          }else if(LALT(j,i)){
            Keyboard.press(MODIFIERKEY_ALT);
          }else if(RALT(j,i)){
            Keyboard.press(MODIFIERKEY_RIGHT_ALT);
          }else if(WIN(j,i)){
            Keyboard.press(MODIFIERKEY_GUI);
          }else if(!FN(j,i)){
            uint16_t keycode = keys[j][i];
            if(held[FN_ROW][FN_COL]){
              keycode = fn_keys[j][i];
              Serial.printf("fn modifier code: %d (%x)\n",keycode,keycode);
            }
            for(n_key_index = 0; n_key_index < N_KEYS; n_key_index++){
              if(!n_key_held[n_key_index]) break;
            }
            n_key_held[n_key_index] = 1;
            held[j][i] = n_key_index + 1;
            send_key(n_key_index,keycode);
          }
        }
      }else{
        if(debounce[j][i] > -DEBOUNCE_LIM){
          debounce[j][i] -= 1;
          continue;
        }
        debounce[j][i] = 0;
        if(held[j][i]){ // only once when not held
          if(LSHIFT(j,i)){
            Keyboard.release(MODIFIERKEY_SHIFT);
          }else if(RSHIFT(j,i)){
            Keyboard.release(MODIFIERKEY_RIGHT_SHIFT);
          }else if(CTRL(j,i)){
            Keyboard.release(MODIFIERKEY_CTRL);
          }else if(LALT(j,i)){
            Keyboard.release(MODIFIERKEY_ALT);
          }else if(RALT(j,i)){
            Keyboard.release(MODIFIERKEY_RIGHT_ALT);
          }else if(WIN(j,i)){
            Keyboard.release(MODIFIERKEY_GUI);
          }else if(!FN(j,i)){
            uint8_t index = held[j][i] - 1;
            send_key(index,0);
            n_key_held[index] = 0;
          }
          held[j][i] = 0;
        }
      }
    }
    digitalWrite(colIndex[i], LOW);
  }
}

void send_key(uint8_t index, uint16_t key){
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
