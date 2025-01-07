# Celengan Pintar dengan ESP32

Proyek ini bertujuan untuk membuat celengan pintar menggunakan ESP32 yang dilengkapi dengan sensor warna, layar OLED, dan integrasi Telegram. Celengan ini dapat mendeteksi nominal uang berdasarkan warna, menyimpan saldo ke EEPROM, dan mengirimkan notifikasi ke Telegram.

## Fitur Utama
1. **Deteksi Nominal Uang:** Menggunakan sensor warna untuk mendeteksi nominal uang.
2. **Tampilan Saldo:** Menampilkan saldo saat ini di layar OLED.
3. **Notifikasi Telegram:** Mengirimkan saldo terbaru ke bot Telegram.
4. **Penyimpanan Saldo:** Menyimpan saldo secara permanen di EEPROM.
5. **WiFi Auto Connect:** Menggunakan WiFiManager untuk koneksi otomatis ke WiFi.
6. **Reset Saldo:** Menyediakan tombol reset untuk mengatur ulang saldo.

## Komponen yang Dibutuhkan
| No | Komponen                  | Jumlah |
|----|---------------------------|--------|
| 1  | ESP32                     | 1      |
| 2  | Sensor Warna (TCS3200)    | 1      |
| 3  | Layar OLED 128x64 (I2C)   | 1      |
| 4  | Push Button               | 1      |
| 5  | Kapasitor 10uF 25V        | 1      |
| 6  | Resistor 10kΩ             | 1      |
| 7  | Kabel Jumper              | Beberapa |
| 8  | Breadboard                | 1      |

## Rangkaian
| Komponen         | Pin ESP32       | Pin Komponen     | Keterangan                  |
|------------------|-----------------|------------------|-----------------------------|
| Sensor TCS3200   | 2               | S2               | Pin kontrol sensor          |
| Sensor TCS3200   | 15              | S3               | Pin kontrol sensor          |
| Sensor TCS3200   | 4               | OUT              | Output frekuensi warna      |
| Sensor TCS3200   | 3.3V            | VCC              | Daya sensor                 |
| Sensor TCS3200   | GND             | GND              | Ground sensor               |
| Sensor TCS3200   | 3.3V            | S0               | Mode pengaturan skala       |
| Sensor TCS3200   | 3.3V            | S1               | Mode pengaturan skala       |
| Sensor TCS3200   | 3.3V            | LED              | Daya LED sensor             |
| OLED Display     | 21              | SDA              | Data I2C                    |
| OLED Display     | 22              | SCL              | Clock I2C                   |
| OLED Display     | 3.3V            | VCC              | Daya OLED                   |
| OLED Display     | GND             | GND              | Ground OLED                 |
| Push Button      | 5               | Terminal 1       | Input reset saldo           |
| Push Button      | GND             | Terminal 2       | Ground                      |
| Kapasitor 10uF   | EN              | + Terminal       | Stabilisasi daya            |
| Kapasitor 10uF   | GND             | - Terminal       | Ground                      |

## Cara Kerja
1. **Inisialisasi:**
   - ESP32 terhubung ke jaringan WiFi menggunakan WiFiManager.
   - Jika belum ada Chat ID untuk bot Telegram, pengguna diminta mengirim pesan pertama ke bot.

2. **Deteksi Nominal:**
   - Sensor warna mendeteksi nilai RGB dari uang yang dimasukkan.
   - Berdasarkan nilai RGB, nominal uang ditambahkan ke saldo.

3. **Tampilkan Saldo:**
   - Saldo terbaru ditampilkan di layar OLED.

4. **Notifikasi Telegram:**
   - Saldo terbaru dikirimkan ke bot Telegram secara otomatis.

5. **Reset Saldo:**
   - Tekan tombol reset untuk mengatur ulang saldo ke 0.

## Cara Kalibrasi Uang
1. **Langkah Awal:**
   - Pastikan sensor warna (TCS3200) terhubung dengan benar sesuai dengan tabel rangkaian.
   - Upload program ke ESP32 dan nyalakan perangkat.

2. **Proses Kalibrasi:**
   - Masukkan satu jenis uang ke depan sensor warna.
   - Catat nilai RGB yang tercetak pada Serial Monitor.
   - Ulangi proses ini untuk semua jenis nominal uang yang ingin dikenali.

3. **Update Kode:**
   - Buka bagian kode fungsi `detectNominal()`.
   - Tambahkan atau sesuaikan nilai rentang RGB untuk setiap nominal uang sesuai hasil kalibrasi.
   - Contoh:
     ```cpp
     else if (detectNominal(5000, 45, 55, 35, 45, 25, 35)) {
         processNominal(5000);
     }
     ```

4. **Uji Coba:**
   - Setelah memperbarui kode, upload kembali ke ESP32.
   - Masukkan uang dan periksa apakah nominal dikenali dengan benar.

## Cara Menggunakan
1. **Susun Rangkaian:**
   - Hubungkan semua komponen sesuai dengan tabel rangkaian di atas menggunakan breadboard dan kabel jumper.

2. **Upload Kode:**
   - Buka Arduino IDE.
   - Pastikan semua library yang dibutuhkan telah diinstal:
     - WiFiManager
     - UniversalTelegramBot
     - Adafruit_SSD1306
     - EEPROM
   - Upload kode program ke ESP32.

3. **Inisialisasi Bot Telegram:**
   - Buka Telegram dan cari BotFather.
   - Buat bot baru dan salin Token Bot.
   - Masukkan Token Bot ke dalam kode di bagian `#define BOT_TOKEN`.

4. **Pengoperasian:**
   - Nyalakan ESP32.
   - Hubungkan ESP32 ke WiFi melalui WiFiManager.
   - Masukkan uang ke sensor untuk mendeteksi nominal.
   - Lihat saldo pada layar OLED dan notifikasi Telegram.

## Kode Program
```cpp
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
int lastUang = -1;   // Menyimpan saldo terakhir untuk deteksi perubahan
String chatID = "";  // Chat ID for Telegram

// (Kode lainnya mengikuti program utama Anda)
```

## Lisensi
Proyek ini dilisensikan di bawah [MIT License](LICENSE).

