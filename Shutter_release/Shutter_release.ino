
// Segment display
#include <TM1637.h>
#define CLK1 4
#define DIO1 5
#define CLK2 7
#define DIO2 8
TM1637 tm(CLK1, DIO1);
TM1637 tm2(CLK2, DIO2);


String dir = "";
unsigned long last_run = 0;
unsigned long last_run2 = 0;
unsigned long last_press = 0;
unsigned long last_mode = 0;
unsigned long last_secmin = 0;
unsigned long last_num_inter = 0;
unsigned long last_reset = 0;
boolean secmin = 0; //0:sec, 1:min
boolean num_inter = 1; // 0: num shot, 1: intervalt

#define ENCODER_DT  9
#define ENCODER2_CLK 3
#define ENCODER2_DT 10
#define ENCODER_CLK 2
#define ENCODER2_SW 12

#define BUTTON 11

#define SEL_MODE A0
#define RESET A1
#define SEL_TIME A2
#define TRIGGER A3

int sec = 0;
int minute = 0;
int sec_inter = 0;
int mode = 1;
int numShot = 1;

void setup() {
  Serial.begin(9600);
  pinMode(ENCODER_CLK, INPUT);
  pinMode(ENCODER_DT, INPUT);
  pinMode(ENCODER2_CLK, INPUT);
  pinMode(ENCODER2_DT, INPUT);
  pinMode(ENCODER2_SW, INPUT_PULLUP);

  attachInterrupt(digitalPinToInterrupt(ENCODER_CLK), setTime, FALLING);
  attachInterrupt(digitalPinToInterrupt(ENCODER2_CLK), setIntervalt, FALLING);

  pinMode(BUTTON, INPUT);
  pinMode(SEL_MODE, INPUT);
  pinMode(SEL_TIME, INPUT);
  pinMode(RESET, INPUT);
  pinMode(TRIGGER, OUTPUT);

  tm.init();
  tm.set(BRIGHT_DARKEST);
  tm2.init();
  tm2.set(BRIGHT_DARKEST);

  tm2.displayStr((char *)"  by");
  tm.displayStr((char *)" DAk");
  delay(2000);

  updateDisplay1();
  updateDisplay2();
}

void setTime() {
  if (millis() - last_run > 10) {
    if (digitalRead(ENCODER_DT) == 1) {
      if (secmin == 0) {
        sec++;
        dir = "CW";
        if (sec > 59) {
          sec = 0;
          minute ++;
        }
      }
      if (secmin == 1) {
        minute ++;
        if (minute > 99) {
          minute = 0;
        }
      }
    }

    if (digitalRead(ENCODER_DT) == 0) {
      if (secmin == 0) {
        sec--;
        dir = "CCW";
        if (sec < 0 && minute > 0) {
          sec = 59;
          minute --;
        }
        if (sec < 0 && minute == 0) {
          sec = 59;
        }
      }
      if (secmin == 1) {
        minute --;
        if (minute < 0) {
          minute = 99;
        }
      }
    }
    last_run = millis();
  }
  updateDisplay1();
}

void setIntervalt() {
  if (millis() - last_run2 > 10) {
    if (digitalRead(ENCODER2_DT) == 1) {
      if (num_inter == 1) {
        sec_inter++;
        if (sec_inter > 99) {
          sec_inter = 0;
        }
      }
      if (num_inter == 0) {
        numShot ++;
        if (numShot > 999) {
          numShot = 0;
        }
      }
    }
    if (digitalRead(ENCODER2_DT) == 0) {
      if (num_inter == 1) {
        sec_inter--;
        if (sec_inter < 0) {
          sec_inter = 99;
        }
      }
      if (num_inter == 0) {
        numShot --;
        if (numShot < 0) {
          numShot = 999;
        }
      }
    }

    last_run2 = millis();
  }
  updateDisplay2();
}

void updateDisplay1() {
  //Digit update
  tm.display(0, (minute / 10) % 10);
  tm.display(1, minute % 10);
  tm.display(2, (sec / 10) % 10);
  tm.display(3, sec % 10);
}

void updateDisplay2() {
  if (num_inter == 1) {
    //Digit update
    tm2.clearDisplay();
    tm2.display(0, mode);
    tm2.display(2, (sec_inter / 10) % 10);
    tm2.display(3, sec_inter % 10);
  }
  if (num_inter == 0) {
    tm2.clearDisplay();
    tm2.display(0, mode);
    tm2.display(1, (numShot / 100) % 10);
    tm2.display(2, (numShot / 10) % 10);
    tm2.display(3, numShot % 10);
  }
}

void bulb() {
  //Delay function
  int timer = sec_inter;
  while (timer > 0) {

    tm2.display(2, (timer / 10) % 10);
    tm2.display(3, timer % 10);
    delay(1000);
    timer --;
  }
  tm2.display(2, (timer / 10) % 10);
  tm2.display(3, timer % 10);

  //Start bulb mode
  digitalWrite(TRIGGER, HIGH);
  int temp_sec = sec;
  int temp_min = minute;

  //Countdowm
  while (minute > 0 || sec > 0) {
    updateDisplay1();
    delay(1000);
    if (sec == 0) {
      if (minute > 0) {
        minute--;   // Reduce minute
        sec = 59; // Reset sec
      }
    } else {
      sec--; // Reduce sec
    }
  }

  //End bulb mode
  sec = temp_sec;
  minute = temp_min;
  digitalWrite(TRIGGER, LOW);
  updateDisplay1();
  updateDisplay2();
}

void selMode() {
  if (millis() - last_mode > 150) {
    if (digitalRead(SEL_MODE) == 1) {
      if (mode == 3) {
        numShot = 0;
      }
      mode ++;
      if (mode > 4) {
        mode = 1;
      }
      updateDisplay2();
      if (mode == 1) {
        tm.displayStr((char *)"bulb");
        delay(1000);
        updateDisplay1();
      }
      if (mode == 2) {
        tm.displayStr((char *)"tine");
        delay(1000);
        updateDisplay1();
      }
      if (mode == 3) {
        tm.displayStr((char*)"intr");
        delay(1000);
        updateDisplay1();
      }
      if (mode == 4) {
        tm.displayStr((char *)"null");
        delay(1000);
        updateDisplay1();
      }
    }
    last_mode = millis();
  }
}

void selSecMin() {
  if (millis() - last_secmin > 150) {
    if (digitalRead(SEL_TIME) == 1) {
      secmin = !secmin;
      if (secmin == 0) {
        tm.displayStr((char *)"  --");
        delay(500);
        updateDisplay1();
      }
      if (secmin == 1) {
        tm.displayStr((char *)"--  ");
        delay(500);
        updateDisplay1();
      }
    }
    last_secmin = millis();
  }
}

void selNumshot() {
  if (millis() - last_num_inter > 150) {
    if (digitalRead(ENCODER2_SW) == 0) {
      num_inter = !num_inter;
      if (num_inter == 0) {
        tm2.displayStr((char *)"Nunn");
        delay(1000);
        updateDisplay2();
      }
      if (num_inter == 1) {
        tm2.displayStr((char *)"Intr");
        delay(1000);
        updateDisplay2();
      }

    }

    last_num_inter = millis();
  }
}

void shutter() {
  //mode 1: bulb mode
  //mode 2: timer
  //mode 3: intervalt
  if (millis() - last_press > 50) {
    if (digitalRead(BUTTON) == HIGH) {
      if (mode == 1) {
        bulb();
      }
      if (mode == 2) {
        timer();
      }
      if (mode == 3) {
        intervalt();
      }
    }
    last_press = millis();
  }
}

void timer() {
  //Delay function
  int timer = sec_inter;
  while (timer > 0) {

    tm2.display(2, (timer / 10) % 10);
    tm2.display(3, timer % 10);
    delay(1000);
    timer --;
  }
  tm2.display(2, (timer / 10) % 10);
  tm2.display(3, timer % 10);

  int numShotTemp = 0;
  if (sec == 0) {
    numShotTemp = 1;
  }
  if (sec != 0) {
    numShotTemp = sec;
  }

  while (numShotTemp > 0) {
    digitalWrite(TRIGGER, HIGH);
    delay (700);
    digitalWrite(TRIGGER, LOW);
    numShotTemp --;
    tm.display(1, (numShotTemp / 100) % 10);
    tm.display(2, (numShotTemp / 10) % 10);
    tm.display(3, numShotTemp % 10);
    delay(1500);
  }
  updateDisplay1();
  updateDisplay2();
}

void intervalt() {
  //Delay function
  int timer = 5;
  while (timer > 0) {
    tm2.display(2, (timer / 10) % 10);
    tm2.display(3, timer % 10);
    delay(1000);
    timer --;

  }
  tm2.display(2, (timer / 10) % 10);
  tm2.display(3, timer % 10);


  //Case 1
  if (sec == 0 && minute == 0) {
    int numShotTemp = numShot;
    while (numShotTemp > 0) {

      //Trigger
      digitalWrite(TRIGGER, HIGH);
      delay (700);
      digitalWrite(TRIGGER, LOW);

      numShotTemp--;
      tm2.display(1, (numShotTemp / 100) % 10);
      tm2.display(2, (numShotTemp / 10) % 10);
      tm2.display(3, numShotTemp % 10);
      delay(1000);
      tm2.clearDisplay();


      //Intervalt
      int timer = sec_inter;
      while (timer > 0) {

        tm2.display(2, (timer / 10) % 10);
        tm2.display(3, timer % 10);
        delay(1000);
        timer --;
      }
      tm2.display(2, (timer / 10) % 10);
      tm2.display(3, timer % 10);
    }
  }


  //Case 2
  if (sec != 0 || minute != 0) {
    int numShotTemp = numShot;
    while (numShotTemp > 0) {

      //Start bulb mode
      digitalWrite(TRIGGER, HIGH);
      int temp_sec = sec;
      int temp_min = minute;

      //Countdowm
      while (minute > 0 || sec > 0) {
        updateDisplay1();
        delay(1000);
        if (sec == 0) {
          if (minute > 0) {
            minute--;   // Reduce minute
            sec = 59; // Reset sec
          }
        } else {
          sec--; // Reduce sec
        }
      }

      //End bulb mode
      sec = temp_sec;
      minute = temp_min;
      digitalWrite(TRIGGER, LOW);
      updateDisplay1();
      updateDisplay2();

      numShotTemp --;
      tm2.display(1, (numShotTemp / 100) % 10);
      tm2.display(2, (numShotTemp / 10) % 10);
      tm2.display(3, numShotTemp % 10);
      delay(1000);
      tm2.clearDisplay();

      if (numShotTemp == 0) {
        updateDisplay1();
        updateDisplay2();
        return;
      }

      //Intervalt
      int timer = sec_inter;
      while (timer > 0) {
        tm2.display(0, mode);
        tm2.display(2, (timer / 10) % 10);
        tm2.display(3, timer % 10);
        delay(1000);
        timer --;
      }
      tm2.display(0, mode );
      tm2.display(2, (timer / 10) % 10);
      tm2.display(3, timer % 10);

    }
  }
  updateDisplay1();
  updateDisplay2();

}

void checkModeInter() {
  if (numShot > 1 && mode != 3) {
    mode = 3;
    tm.displayStr((char*)"intr");
    delay(1000);
    updateDisplay1();
  }
}

void reset() {
  if (millis() - last_reset > 150) {
    if (digitalRead(RESET) == 1) {
      tm.displayStr((char *)" rst");
      sec = 0;
      minute = 0;
      mode = 1;
      numShot = 0;
      sec_inter = 0;
      delay(1000);
      updateDisplay1();
      updateDisplay2();
    }
    last_reset = millis();
  }
}

void loop() {
  checkModeInter();
  selNumshot();
  selSecMin();
  selMode();
  shutter();
  reset();
}
