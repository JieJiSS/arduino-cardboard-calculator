#include <Wire.h> 
#include <LiquidCrystal_I2C.h>

#define KOhm 1000
#define CALC_DBG 1
const int res_pin = A0;

LiquidCrystal_I2C lcd(0x27, 16, 1);

double fabs_(double num) {
  if(num < 0) return -num;
  return num;
}

long _lround(double f) {
  return (long)round(f);
}

const int V_in = 5;    // voltage at 5V pin of arduino
double V_out = 0;      // voltage at res_pin pin of arduino

const double R1 = 8.2 * KOhm;  // known resistance

#if CALC_DBG
double oldRes = 0.0, newRes = 0.0; // for displayDbl(...) filter
#endif

double oldResist = 0.0; // for loop() filter
long lastTap = 0;

// 'I' for inital, '+' for add & input, '-' for minus & input,
// '*' for multiple & input, '/' for divise & input, '=' for equal & input.
char currentState = 0;

double current = 0.0;
double last = 0.0;
bool hasSign = false;

const double TOTAL_RES = 85.1 * KOhm;
double BUTTON_OPPO[2] = {83.0 * KOhm, 83.2 * KOhm};
double BUTTON_DIVI[2] = {80.0 * KOhm, 81.3 * KOhm};
double BUTTON_MULT[2] = {74.8 * KOhm, 75.1 * KOhm};
double BUTTON_MINU[2] = {64.7 * KOhm, 64.9 * KOhm};
double BUTTON_ADD [2] = {37.3 * KOhm, 37.6 * KOhm};
double BUTTON_NUM0[2] = {79.1 * KOhm, 79.4 * KOhm};
double BUTTON_NUM9[2] = {70.0 * KOhm, 70.4 * KOhm};
double BUTTON_NUM8[2] = {54.3 * KOhm, 54.6 * KOhm};
double BUTTON_NUM7[2] = {17.2 * KOhm, 17.4 * KOhm};
double BUTTON_NUM6[2] = {68.7 * KOhm, 69.0 * KOhm};
double BUTTON_NUM5[2] = {50.0 * KOhm, 50.2 * KOhm};
double BUTTON_NUM4[2] = { 7.0 * KOhm,  7.4 * KOhm};
double BUTTON_NUM3[2] = {48.0 * KOhm, 48.3 * KOhm};
double BUTTON_NUM2[2] = { 2.7 * KOhm,  3.0 * KOhm};
double BUTTON_NUM1[2] = {40.2 * KOhm, 40.5 * KOhm};
double BUTTON_EQUA[2] = { 1.2 * KOhm,  1.5 * KOhm};
double BUTTON_CLR [2] = { 4.1 * KOhm,  4.4 * KOhm};

char getCharFromRes(double res) {
  if(res >= TOTAL_RES - 0.3 * KOhm) {
    if(res >= TOTAL_RES * 2) {
      Serial.println("Resistance too big");
    }
    return 'N';
  }
  else if(BUTTON_NUM1[0] <= res && res <= BUTTON_NUM1[1])
      return '1';
  else if(BUTTON_NUM2[0] <= res && res <= BUTTON_NUM2[1])
      return '2';
  else if(BUTTON_NUM3[0] <= res && res <= BUTTON_NUM3[1])
      return '3';
  else if(BUTTON_NUM4[0] <= res && res <= BUTTON_NUM4[1])
      return '4';
  else if(BUTTON_NUM5[0] <= res && res <= BUTTON_NUM5[1])
      return '5';
  else if(BUTTON_NUM6[0] <= res && res <= BUTTON_NUM6[1])
      return '6';
  else if(BUTTON_NUM7[0] <= res && res <= BUTTON_NUM7[1])
      return '7';
  else if(BUTTON_NUM8[0] <= res && res <= BUTTON_NUM8[1])
      return '8';
  else if(BUTTON_NUM9[0] <= res && res <= BUTTON_NUM9[1])
      return '9';
  else if(BUTTON_NUM0[0] <= res && res <= BUTTON_NUM0[1])
      return '0';
  else if(BUTTON_ADD [0] <= res && res <= BUTTON_ADD [1])
      return '+';
  else if(BUTTON_MINU[0] <= res && res <= BUTTON_MINU[1])
      return '-';
  else if(BUTTON_MULT[0] <= res && res <= BUTTON_MULT[1])
      return '*';
  else if(BUTTON_DIVI[0] <= res && res <= BUTTON_DIVI[1])
      return '/';
  else if(BUTTON_EQUA[0] <= res && res <= BUTTON_EQUA[1])
      return '=';
  else if(BUTTON_OPPO[0] <= res && res <= BUTTON_OPPO[1])
      return '#';
  else if(BUTTON_CLR [0] <= res && res <= BUTTON_CLR [1])
      return '!';
  else {
      Serial.print("Failed to match resistance: ");
      Serial.print(res / 1000);
      Serial.println(" kOhm");
      return 'N';
  }
}

void displayDbl(double num) {
  lcd.clear();
  lcd.setCursor(0, 0);
  if(num < 0) {
    num = -num;
    lcd.print('-');
  }
  int chunk = (num - (int)num) * 1000;
  while(chunk != 0 && chunk % 10 == 0) chunk /= 10;
  lcd.print((long)num);
  lcd.print('.');
  lcd.print(chunk);
#if CALC_DBG
  newRes = getResistance();
  if(fabs_(newRes - oldRes) > 0.5 * KOhm) {
    oldRes = newRes;
  }
  lcd.print(" (");
  lcd.print(int(_lround(oldRes / 1000)));
  lcd.print(" kOhm)");
#endif
}

void resetAll() {
  current = 0.0;
  last = 0.0;
  hasSign = false;
  Serial.println("Reset status");
  showResult();
  currentState = 'I';
}
void showResult() {
  double result = 0.0;
  Serial.print("result: ");
  if(!hasSign) {
    result = current;
  } else {
    result = last;
    current = last;
  }
  Serial.println(result, 4);
  displayDbl(result);
  currentState = '=';
  return;
}
void updateState() {
  switch(currentState) {
    case 'I':
      break;
    case '=':
      last = 0.0;
      break;
    case '+':
      last = last + current;
      break;
    case '-':
      last = last - current;
      break;
    case '*':
      last = last * current;
      break;
    case '/':
      last = last / current;
      break;
  }
  currentState = 'I';
}
void handleChar(char ch) {
  // handle numbers
  if(ch >= '0' && ch <= '9') {
    if(currentState == '=') {
      resetAll();
    }
    current *= 10;
    current += (double)(ch - '0');
    displayDbl(current);
    return;
  }

  // handle symbols
  switch (ch) {
    case '+':
      updateState();
      currentState = ch;
      last = current;
      current = 0.0;
      break;
    case '-':
      updateState();
      currentState = ch;
      last = current;
      current = 0.0;
      break;
    case '*':
      updateState();
      currentState = ch;
      last = current;
      current = 0.0;
      break;
    case '/':
      updateState();
      currentState = ch;
      last = current;
      current = 0.0;
      break;
    case '=':
      updateState();
      showResult();
      hasSign = true;
      return;  // to prevent disableDbl(...) invoked again
    case '?':
      Serial.print("[DBG] currentState:");
      Serial.println(currentState);
      Serial.print("[DBG] current:");
      Serial.println(current);
      Serial.print("[DBG] last:");
      Serial.println(last);
      Serial.println("----------------");
      return;
    case '!':
      resetAll();
      return;
    case '#':
      updateState();
      currentState = '*';
      last = current;
      current = -1.0;
      updateState();
      hasSign = true;
      showResult();
      return;
    default:
      Serial.print("Failed to recognize ");
      Serial.println(ch);
      return;
  }
  hasSign = true;
  displayDbl(current);
}

double getResistance() {
  int a2d_data = analogRead(res_pin);
  V_out = (a2d_data / 1024.0) * V_in;
  if(V_out < 0.03) {
    V_out = 0.03;  // to prevent devision by zero
  }
  double R2 = R1 * ((V_in - V_out) / V_out);
  return R2;
}

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  delay(300);
  showResult();
}
void loop() {
  if(Serial.available() > 0) {
    char ch = Serial.read();
    if(ch == 10) return;
    lastTap = millis();
    handleChar(ch); 
  }
  const double currResist = getResistance();
  if(fabs_(currResist - oldResist) < 0.4 * KOhm) {
    if(millis() - lastTap < 600) {
      return;
    }
  }
  oldResist = currResist;

  displayDbl(current);

  lastTap = millis();
  char ch = getCharFromRes(currResist);
  if(ch == 'N') {
    return;
  }
  handleChar(ch);
  delay(80);
}
