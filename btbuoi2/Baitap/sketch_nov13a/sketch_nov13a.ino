#include <Wire.h>
#include <RTClib.h>
#include <LiquidCrystal_I2C.h>

// Khởi tạo thư viện
LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS1307 rtc;

// Định nghĩa các chân (đổi BUZZER_PIN -> 7)
#define BUZZER_PIN 7
#define MODE_PIN   11
#define UP_PIN     10
#define DOWN_PIN   9
#define SET_PIN    8

// Biến lưu trữ báo thức
byte alarmHour = 6;
byte alarmMinute = 0;
bool isAlarmOn = false;
bool isAlarmRinging = false;

// Biến cho máy trạng thái (state machine)
enum Mode { 
  DISPLAY_TIME, 
  SET_ALARM_HOUR, 
  SET_ALARM_MINUTE 
};
Mode currentMode = DISPLAY_TIME;
Mode lastMode = (Mode)-1; // Dùng để phát hiện thay đổi mode

// Biến cho xử lý nút nhấn (chống dội & nhấn giữ)
bool lastUpState = HIGH;
bool lastDownState = HIGH;
bool lastSetState = HIGH;

unsigned long modePressStartTime = 0;
bool modeLongPressHandled = false;
unsigned long ignoreButtonsUntil = 0; // Chống dội khi tắt báo thức

// Biến hiệu ứng nhấp nháy
bool blinkState = false;
unsigned long lastBlinkTime = 0;

// Thêm biến để lưu giây cuối cùng đã hiển thị
byte lastSecondDisplayed = 60; // buộc cập nhật lần đầu

void setup() {
  // Khởi tạo LCD
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);

  // Khởi tạo & Kiểm tra RTC
  if (!rtc.begin()) {
    lcd.print("RTC not found!");
    while (1);
  }

  // Cài đặt thời gian cho RTC từ máy tính nếu RTC chưa chạy
  if (!rtc.isrunning()) {
    lcd.clear();
    lcd.print("RTC adjust...");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    delay(2000);
  }

  // Cài đặt Pin
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); // Tắt còi lúc khởi động

  // Cài đặt các nút nhấn với điện trở kéo lên nội
  pinMode(MODE_PIN, INPUT_PULLUP);
  pinMode(UP_PIN, INPUT_PULLUP);
  pinMode(DOWN_PIN, INPUT_PULLUP);
  pinMode(SET_PIN, INPUT_PULLUP);

  lcd.clear();
}

void loop() {
  handleButtons();
  checkAlarm();
  updateDisplay();
}

void handleButtons() {
  if (millis() < ignoreButtonsUntil) {
    lastUpState = (digitalRead(UP_PIN) == LOW);
    lastDownState = (digitalRead(DOWN_PIN) == LOW);
    lastSetState = (digitalRead(SET_PIN) == LOW);
    return;
  }

  bool modeState = (digitalRead(MODE_PIN) == LOW);
  bool upState = (digitalRead(UP_PIN) == LOW);
  bool downState = (digitalRead(DOWN_PIN) == LOW);
  bool setState = (digitalRead(SET_PIN) == LOW);

  // 1. Tắt báo thức đang kêu
  if (isAlarmRinging && (modeState || upState || downState || setState)) {
    isAlarmRinging = false;
    digitalWrite(BUZZER_PIN, LOW);
    ignoreButtonsUntil = millis() + 500;
    return;
  }

  // 2. MODE (nhấn ngắn & nhấn giữ)
  if (modeState) {
    if (modePressStartTime == 0) {
      modePressStartTime = millis();
      modeLongPressHandled = false;
    } else if (!modeLongPressHandled && (millis() - modePressStartTime > 2000)) {
      if (currentMode == DISPLAY_TIME) {
        isAlarmOn = !isAlarmOn;
        lastSecondDisplayed = 60;
      }
      modeLongPressHandled = true;
    }
  } else {
    if (modePressStartTime > 0 && !modeLongPressHandled) {
      if (currentMode == DISPLAY_TIME) {
        currentMode = SET_ALARM_HOUR;
      }
    }
    modePressStartTime = 0;
  }

  // 3. UP
  if (upState && !lastUpState) {
    if (currentMode == SET_ALARM_HOUR) {
      alarmHour = (alarmHour + 1) % 24;
    } else if (currentMode == SET_ALARM_MINUTE) {
      alarmMinute = (alarmMinute + 1) % 60;
    }
  }

  // 4. DOWN
  if (downState && !lastDownState) {
    if (currentMode == SET_ALARM_HOUR) {
      alarmHour = (alarmHour == 0) ? 23 : alarmHour - 1;
    } else if (currentMode == SET_ALARM_MINUTE) {
      alarmMinute = (alarmMinute == 0) ? 59 : alarmMinute - 1;
    }
  }

  // 5. SET
  if (setState && !lastSetState) {
    if (currentMode == SET_ALARM_HOUR) {
      currentMode = SET_ALARM_MINUTE;
    } else if (currentMode == SET_ALARM_MINUTE) {
      currentMode = DISPLAY_TIME;
    }
  }

  lastUpState = upState;
  lastDownState = downState;
  lastSetState = setState;
}

void checkAlarm() {
  if (isAlarmRinging) {
    digitalWrite(BUZZER_PIN, HIGH);
    return;
  }

  if (isAlarmOn && currentMode == DISPLAY_TIME) {
    DateTime now = rtc.now();
    if (now.hour() == alarmHour && now.minute() == alarmMinute && now.second() == 0) {
      isAlarmRinging = true;
    }
  }
}

void updateDisplay() {
  if (millis() - lastBlinkTime > 500) {
    blinkState = !blinkState;
    lastBlinkTime = millis();
  }

  if (currentMode != lastMode) {
    lcd.clear();
    lastMode = currentMode;
    lastSecondDisplayed = 60;
  }

  DateTime now = rtc.now();

  switch (currentMode) {
    case DISPLAY_TIME:
      if (now.second() != lastSecondDisplayed) {
        lastSecondDisplayed = now.second();

        lcd.setCursor(0, 0);
        lcd.print("Time: ");
        if (now.hour() < 10) lcd.print('0');
        lcd.print(now.hour());
        lcd.print(':');
        if (now.minute() < 10) lcd.print('0');
        lcd.print(now.minute());
        lcd.print(':');
        if (now.second() < 10) lcd.print('0');
        lcd.print(now.second());
        lcd.print(" ");

        lcd.setCursor(0, 1);
        lcd.print("Alarm: ");
        if (alarmHour < 10) lcd.print('0');
        lcd.print(alarmHour);
        lcd.print(':');
        if (alarmMinute < 10) lcd.print('0');
        lcd.print(alarmMinute);
        lcd.print(isAlarmOn ? " ON " : " OFF");
      }
      break;

    case SET_ALARM_HOUR:
      lcd.setCursor(0, 0);
      lcd.print("Set Alarm Hour");
      lcd.setCursor(0, 1);

      if (blinkState) lcd.print("  ");
      else {
        if (alarmHour < 10) lcd.print('0');
        lcd.print(alarmHour);
      }

      lcd.print(":");
      if (alarmMinute < 10) lcd.print('0');
      lcd.print(alarmMinute);
      lcd.print("        ");
      break;

    case SET_ALARM_MINUTE:
      lcd.setCursor(0, 0);
      lcd.print("Set Alarm Min");
      lcd.setCursor(0, 1);

      if (alarmHour < 10) lcd.print('0');
      lcd.print(alarmHour);
      lcd.print(":");

      if (blinkState) lcd.print("  ");
      else {
        if (alarmMinute < 10) lcd.print('0');
        lcd.print(alarmMinute);
      }

      lcd.print("        ");
      break;
  }
}
