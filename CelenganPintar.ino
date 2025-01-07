#define S2 2        /*Define S2 Pin Number of ESP32*/
#define S3 15       /*Define S3 Pin Number of ESP32*/
#define sensorOut 4 /*Define Sensor Output Pin Number of ESP32*/
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>       // Include EEPROM library
#include <WiFiManager.h>  // Include WiFiManager library

#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET -1        // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C  ///< See datasheet for Address
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define EEPROM_SIZE 512  // Define EEPROM size
#define RESET_BUTTON 5   // Define reset button pin

// Telegram BOT Token (Get from Botfather)
#define BOT_TOKEN "6527652038:AAEy5hAaSS72aX-NnJnYdQBFqp_9wtkX6QQ"

WiFiClientSecure secured_client;
UniversalTelegramBot bot(BOT_TOKEN, secured_client);

/* Variables */
int Red = 0;
int Green = 0;
int Blue = 0;
int Frequency = 0;
bool statusUang = 0;
bool msg = 0;
int Uang = 0;
int lastUang = 20;   // Menyimpan saldo terakhir untuk deteksi perubahan
String chatID = "";  // Chat ID for Telegram

void setup() {
  pinMode(S2, OUTPUT);                 /*Define S2 Pin as a OUTPUT*/
  pinMode(S3, OUTPUT);                 /*Define S3 Pin as a OUTPUT*/
  pinMode(sensorOut, INPUT);           /*Define Sensor Output Pin as a INPUT*/
  pinMode(RESET_BUTTON, INPUT_PULLUP); /*Define Reset Button with internal pullup*/
  Serial.begin(115200);                /*Set the baudrate to 115200*/

  // Initialize EEPROM
  EEPROM.begin(EEPROM_SIZE);
  Uang = EEPROM.readInt(0);         // Read the initial saldo from EEPROM
  chatID = EEPROM.readString(100);  // Read the stored chat ID from EEPROM

  if (chatID == "") {
    Serial.println("Chat ID not set, defaulting to empty.");
  } else {
    Serial.print("Stored Chat ID: ");
    Serial.println(chatID);
  }

  // Configure WiFi using WiFiManager
  WiFiManager wifiManager;
  wifiManager.autoConnect("ESP32_Celengan");

  Serial.print("WiFi connected. IP address: ");
  Serial.println(WiFi.localIP());

  secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);  // Add root certificate for api.telegram.org

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }
  display.clearDisplay();
  display.display();

  // Request Chat ID if not set
  if (chatID == "") {
    requestChatID();
  }
}

void loop() {
  // Check if reset button is pressed
  if (digitalRead(RESET_BUTTON) == LOW) {    // Button is pressed
    delay(500);                              // Debounce delay
    if (digitalRead(RESET_BUTTON) == LOW) {  // Confirm button is still pressed
      resetSaldoEEPROM();
    }
  }

  // Display current saldo only if changed
  if (Uang != lastUang) {
    displaySaldo();
    lastUang = Uang;
  }

  // Read sensor values
  Red = getRed();
  Green = getGreen();
  Blue = getBlue();

  // Print sensor values to Serial Monitor
  Serial.print("Red: ");
  Serial.print(Red);
  Serial.print(", Green: ");
  Serial.print(Green);
  Serial.print(", Blue: ");
  Serial.println(Blue);

  // Check for nominal detection
  if (detectNominal(2000, 00, 00, 00, 00, 00, 00)) {
    processNominal(2000);
  } else if (detectNominal(5000, 00, 00, 00, 00, 00, 00)) {
    processNominal(5000);
  } else if (detectNominal(10000, 00, 00, 00, 00, 00, 00)) {
    processNominal(10000);
  } else if (detectNominal(20000, 26, 30, 22, 26, 21, 25)) {
    processNominal(20000);
  } else if (detectNominal(50000, 35, 39, 30, 34, 22, 26)) {
    processNominal(50000);
  } else if (detectNominal(100000, 19, 23, 31, 35, 24, 28)) {
    processNominal(100000);
  } else if (Red > 100 && Green > 100 && Blue > 100) {  // Reset condition
    statusUang = 0;
    msg = 0;
  }

  // Check Telegram messages
  int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
  for (int i = 0; i < numNewMessages; i++) {
    handleTelegramMessage(i);
  }


  // Send saldo to Telegram if new uang is detected
  if (statusUang == 1 && msg == 0) {
    if (chatID != "") {
      bot.sendMessage(chatID, "Saldo Celengan Saat Ini :");
      bot.sendMessage(chatID, String(Uang));
    }
    msg = 1;
  }
}

void processNominal(int nominal) {
  Uang += nominal;
  updateSaldoEEPROM();
  showReceivedAmount(nominal);
  statusUang = 1;
}

bool detectNominal(int nominal, int rMin, int rMax, int gMin, int gMax, int bMin, int bMax) {
  if (rMin == 0 && rMax == 0 && gMin == 0 && gMax == 0 && bMin == 0 && bMax == 0) {
    return false;  // Ignore unused ranges
  }
  return (Red > rMin && Red < rMax && Green > gMin && Green < gMax && Blue > bMin && Blue < bMax && statusUang == 0);
}

void resetSaldoEEPROM() {
  Uang = 0;                  // Reset nilai saldo ke 0
  EEPROM.writeInt(0, Uang);  // Tulis ulang nilai 0 ke EEPROM
  EEPROM.commit();           // Pastikan perubahan tersimpan
  Serial.println("Saldo berhasil direset ke 0!");
}

void updateSaldoEEPROM() {
  EEPROM.writeInt(0, Uang);  // Save saldo to EEPROM
  EEPROM.commit();
  Serial.println("Saldo tersimpan ke EEPROM!");
}

void requestChatID() {
  Serial.println("Meminta Chat ID dari Telegram...");
  String message = "Kirimkan pesan apa saja ke bot ini untuk menyimpan Chat ID Anda.";
  bot.sendMessage("", message);

  while (chatID == "") {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    for (int i = 0; i < numNewMessages; i++) {
      chatID = bot.messages[i].chat_id;
      EEPROM.writeString(100, chatID);
      EEPROM.commit();
      Serial.print("Chat ID berhasil disimpan: ");
      Serial.println(chatID);
    }
    delay(1000);
  }
}


void handleTelegramMessage(int index) {
  String text = bot.messages[index].text;
  String senderChatID = bot.messages[index].chat_id;

  // Update chatID if it's a new sender
  if (senderChatID != chatID) {
    chatID = senderChatID;
    EEPROM.writeString(100, chatID);  // Save new Chat ID to EEPROM
    EEPROM.commit();
    Serial.print("Chat ID diperbarui: ");
    Serial.println(chatID);
  }

  if (text.equalsIgnoreCase("/reset")) {
    resetSaldoEEPROM();
    bot.sendMessage(chatID, "Saldo berhasil direset ke 0.");
  } else {
    bot.sendMessage(chatID, "Perintah tidak dikenali. Gunakan /reset untuk mereset saldo.");
  }
}



void showReceivedAmount(int nominal) {
  display.clearDisplay();
  String nominalText = "Rp " + String(nominal);
  displayCenteredText(nominalText, 10, 2);
  displayCenteredText("Diterima", 30, 2);
  display.display();
  delay(3000);
}

void displaySaldo() {
  display.clearDisplay();
  displayCenteredText("Yuk Nabung!", 0, 1);
  displayCenteredText("Saldo:", 20, 1);
  String uangText = "Rp " + String(Uang);
  displayCenteredText(uangText, 40, 2);
  display.display();
}

void displayCenteredText(String text, int y, int textSize) {
  display.setTextSize(textSize);
  display.setTextColor(SSD1306_WHITE);
  int textWidth = text.length() * 6 * textSize;
  int x = (SCREEN_WIDTH - textWidth) / 2;
  display.setCursor(x, y);
  display.println(text);
  display.display();
}

int getRed() {
  digitalWrite(S2, LOW);
  digitalWrite(S3, LOW);
  Frequency = pulseIn(sensorOut, LOW);
  return Frequency;
}

int getGreen() {
  digitalWrite(S2, HIGH);
  digitalWrite(S3, HIGH);
  Frequency = pulseIn(sensorOut, LOW);
  return Frequency;
}

int getBlue() {
  digitalWrite(S2, LOW);
  digitalWrite(S3, HIGH);
  Frequency = pulseIn(sensorOut, LOW);
  return Frequency;
}
