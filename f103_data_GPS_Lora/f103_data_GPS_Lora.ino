#include <SPI.h>
#include <RadioLib.h>
#include <TinyGPSPlus.h>
#include <Wire.h>
#include <SSD1306Ascii.h>
#include <SSD1306AsciiWire.h>

// ========================================
// RFM95 Pins
// ========================================
#define RFM95_CS    PA4
#define RFM95_DIO0  PA0
#define RFM95_RST   PB0
#define RFM95_DIO1  PA1

// ========================================
// UART
// ========================================
HardwareSerial Serial2(PA3, PA2); // Serial2 -> GPS (RX=PA3, TX=PA2)

TinyGPSPlus gps;
SSD1306AsciiWire oled;

// ========================================
// LoRa Radio & Keys
// ========================================
SX1276 radio = new Module(RFM95_CS, RFM95_DIO0, RFM95_RST, RFM95_DIO1);

uint64_t joinEUI = 0x0000000000000000;
uint64_t devEUI  = 0x45E8FA4865D1696D;
uint8_t appKey[] = { 0x15, 0x63, 0x29, 0xC1, 0x88, 0xFF, 0x0D, 0x11, 0xD0, 0xFF, 0xFF, 0xEB, 0xF3, 0x25, 0xCA, 0x13 };
uint8_t nwkKey[] = { 0x15, 0x63, 0x29, 0xC1, 0x88, 0xFF, 0x0D, 0x11, 0xD0, 0xFF, 0xFF, 0xEB, 0xF3, 0x25, 0xCA, 0x13 };

LoRaWANNode node(&radio, &AS923_2, 2);

// ========================================
// Biến toàn cục
// ========================================
uint32_t packetCount = 1; // Bộ đếm số gói tin đã gửi
int sendInterval = 10;
float rssi = -999;
float snr  = -999;    // Thời gian chờ giữa 2 lần gửi (giây)

// ========================================
// Hàm Read GPS
// ========================================
bool readGPS(float &lat, float &lon, unsigned long timeout_ms) {
  unsigned long start = millis();
  while (millis() - start < timeout_ms) {
    while (Serial2.available()) {
      gps.encode(Serial2.read());
    }
    if (gps.location.isValid() && gps.location.age() < 2000) {
      lat = gps.location.lat();
      lon = gps.location.lng();
      return true;
    }
  }
  return false;
}

void setup() {
  Serial.begin(115200);
  delay(2000);

  Serial.println("\n=== GPS + LoRaWAN + Blue Pill ===");

  // 1. Khởi tạo OLED & GPS UART
  Serial2.begin(9600);
  Wire.begin();
  oled.begin(&Adafruit128x64, 0x3C);
  oled.setFont(System5x7);

  // GIAO DIỆN 1: START LORA NODE
  oled.clear();
  oled.println("== Start Lora Node ==");
  oled.println();
  oled.print("GPS:    ");
  oled.println("OK"); // HardwareSerial mặc định khởi tạo thành công

  SPI.begin();
  
  oled.print("RFM95:  ");
  Serial.print("Khoi tao RFM95... ");
  int state = radio.begin(921.4);
  
  if (state != RADIOLIB_ERR_NONE) {
    oled.println("Failed");
    Serial.print("THAT BAI! Ma loi: ");
    Serial.println(state);
    while (true); // Treo máy nếu RFM95 lỗi
  } else {
    oled.println("OK");
    Serial.println("OK");
  }
  
  delay(2500); // Chờ 2.5s để người dùng đọc kịp màn hình khởi tạo

  // 2. Khởi tạo OTAA
  node.beginOTAA(joinEUI, devEUI, nwkKey, appKey);

  // GIAO DIỆN 2: JOINING
  oled.clear();
  oled.println("== Joining ... ==");
  oled.println();
  oled.println("Freq: 921.4 MHz");
  oled.println("SF: 7");

  Serial.println("Dang join mang...");
  int attempt = 1;

  while (true) {
    Serial.print("Thu lan "); Serial.print(attempt++); Serial.print("... ");
    int joinState = node.activateOTAA();
    Serial.println(joinState);

    if (joinState == RADIOLIB_LORAWAN_NEW_SESSION) {
      Serial.println("JOIN THANH CONG!");
      
      // GIAO DIỆN 3: JOIN ACCEPTED
      oled.clear();
      oled.println("== Join Accepted ==");
      oled.println();
      oled.println("OTAA: OK");
      oled.print("Freq: "); oled.println("921.4 MHz");
      oled.print("Plan: "); oled.println("AS923-2");
      delay(3000); // Chờ 3s để xem thông số sóng
      break;
    }

    // Đọc GPS trong lúc chờ 10s để không rớt tín hiệu
    unsigned long waitStart = millis();
    while (millis() - waitStart < 10000) {
      while (Serial2.available()) {
        gps.encode(Serial2.read());
      }
    }
  }
}

void loop() {
  float lat = 0;
  float lon = 0;
  uint8_t num_sat = 255; 

  Serial.print("Dang doc GPS... ");
  bool gpsOK = readGPS(lat, lon, 10000); // Dành tối đa 10s để lấy tọa độ

  // Nạp số lượng vệ tinh
  if (gpsOK) {
    unsigned long t = millis();
    while (millis() - t < 1500) {
      while (Serial2.available()) {
        gps.encode(Serial2.read());
      }
    }
    if (gps.satellites.isValid()) num_sat = gps.satellites.value();
  }

  // Đóng gói Payload 9 bytes
  int32_t lat_int = (int32_t)(lat * 10000000);
  int32_t lon_int = (int32_t)(lon * 10000000);
  uint8_t payload[9];
  payload[0] = (lat_int >> 24) & 0xFF;
  payload[1] = (lat_int >> 16) & 0xFF;
  payload[2] = (lat_int >>  8) & 0xFF;
  payload[3] = (lat_int      ) & 0xFF;
  payload[4] = (lon_int >> 24) & 0xFF;
  payload[5] = (lon_int >> 16) & 0xFF;
  payload[6] = (lon_int >>  8) & 0xFF;
  payload[7] = (lon_int      ) & 0xFF;
  payload[8] = num_sat;

  // Gửi dữ liệu LoRaWAN
  Serial.print("Dang gui goi LoRa... ");
  uint8_t downlink[256];
  LoRaWANEvent_t eventUp;
  LoRaWANEvent_t eventDown;
  size_t downlinkLen = 0;
  int state = node.sendReceive(payload, sizeof(payload), 1, downlink, &downlinkLen, false, &eventUp, &eventDown);

  if (state == RADIOLIB_ERR_NONE || state == RADIOLIB_LORAWAN_DOWNLINK) {
    Serial.println("GUI THANH CONG!");
  } else {
    Serial.print("LOI GUI: "); Serial.println(state);
  }
  // Lưu RSSI/SNR ngay sau sendReceive
  if (state == RADIOLIB_LORAWAN_DOWNLINK) {
  rssi = radio.getRSSI();
  snr  = radio.getSNR();
  }
  uint8_t sf = 12 - eventUp.datarate;
  // =========================================================
  // GIAO DIỆN 4 & 5: HIỂN THỊ GÓI TIN VÀ ĐẾM NGƯỢC THỜI GIAN
  // =========================================================
  for (int i = sendInterval; i > 0; i--) {
    oled.clear(); // Xóa sạch màn hình ở mỗi giây để vẽ lại khung hình mới

    // Tiêu đề gói tin
    oled.print("== Packet "); oled.print(packetCount); oled.println(" ==");
    
    // Khối thông tin GPS
    if (gpsOK) {
      oled.print("Lat = "); oled.println(lat, 5);
      oled.print("Lon = "); oled.println(lon, 5);
      oled.print("Sat = "); oled.println(num_sat);
    } else {
      oled.println("GPS NOT FIX");
      oled.println("Lat/Lon = 0");
      oled.println("Sat = N/A");
    }
    // Print RSSI va SNR 
    if (rssi == -999) {
      oled.println("RSSI: N/A");
      oled.println("SNR:  N/A");
    } else {
      oled.print("RSSI:"); oled.println((int)rssi);
      oled.print("SNR: "); oled.println((int)snr);
    }

      oled.print("SF:"); oled.print(sf);
      oled.print(" Fr:"); oled.print(eventUp.freq, 1); oled.println("MHz");
      oled.print("Next: "); oled.print(i); oled.println("s");

    // Thay vì dùng delay(1000) làm treo hệ thống, ta dùng vòng lặp 1 giây để đọc nền GPS
    unsigned long tick = millis();
    while (millis() - tick < 1000) {
      while (Serial2.available()) {
        gps.encode(Serial2.read());
      }
    }
  }
  // Chuẩn bị sang chu kỳ mới
  packetCount++;
}