#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <math.h>

// ===== Panel config =====
#define PANEL_RES_X 64
#define PANEL_RES_Y 32
#define PANEL_CHAIN 1

// ===== RGB pins =====
#define R1_PIN 25
#define B1_PIN 26
#define G1_PIN 27

#define R2_PIN 14
#define B2_PIN 33
#define G2_PIN 32

// ===== Row select pins =====
#define A_PIN 23
#define B_PIN 19
#define C_PIN 5
#define D_PIN 17
#define E_PIN 21

// ===== Control pins =====
#define LAT_PIN 4
#define OE_PIN 15
#define CLK_PIN 16

// Setup history size
#define HISTORY_SIZE 64

// ===== Create matrix config =====
HUB75_I2S_CFG mxconfig(
  PANEL_RES_X, PANEL_RES_Y, PANEL_CHAIN,
  { R1_PIN, B1_PIN, G1_PIN,
    R2_PIN, B2_PIN, G2_PIN,
    A_PIN, B_PIN, C_PIN, D_PIN, E_PIN,
    LAT_PIN, OE_PIN, CLK_PIN }
);


MatrixPanel_I2S_DMA* display = nullptr;

// ===== History simulation =====

float priceHistory[HISTORY_SIZE] = {0};
int historyIndex = 0;
float minPrice = 0;
float maxPrice = 0;
bool updated = false;

// Replace with your network credentials
const char* ssid     = "abcd";
const char* password = "1234";

void setupWiFi() {
    Serial.print("Connecting to WiFi: ");
    Serial.println(ssid);

    WiFi.begin(ssid, password);

    int retryCount = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(200);
        Serial.print(".");
        retryCount++;

        // Optional: fail after 20 retries (~10 seconds)
        if (retryCount > 50) {
            Serial.println("\nFailed to connect to WiFi");
            return;
        }
    }

    Serial.println("\nWiFi connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  setupWiFi();

  //Config to prevent ghosting and flickering
  mxconfig.clkphase = false;
  mxconfig.latch_blanking = 4;
  mxconfig.i2sspeed = HUB75_I2S_CFG::HZ_8M;

  display = new MatrixPanel_I2S_DMA(mxconfig);

  display->begin();
  display->setBrightness8(90);
  display->clearScreen();

  float initial_price = getPrice();

  // Initialize random price history
  for (int i = 0; i < HISTORY_SIZE; i++) {
    priceHistory[i] = initial_price; // simulate BTC $20k-$40k
  }
  updateMinMax();
}

void loop() {
  addPrice(getPrice());
  updateMinMax();

  drawGraph();

  delay(10000); // 10-second updates
}

void updateMinMax() {
  minPrice = priceHistory[0];
  maxPrice = priceHistory[0];
  for (int i = 0; i < HISTORY_SIZE; i++) {
    minPrice = min(minPrice, priceHistory[i]);
    maxPrice = max(maxPrice, priceHistory[i]);
  }
}

void addPrice(float price){
  for(int i=0; i<HISTORY_SIZE-1; i++){
    priceHistory[i] = priceHistory[i+1];
  }
  priceHistory[HISTORY_SIZE-1] = price;
  if(historyIndex < HISTORY_SIZE){
    historyIndex++;
  }
  if(!updated && historyIndex == HISTORY_SIZE){
    updated = true;
  }
}

float getPrice(){
    const char* url = "https://min-api.cryptocompare.com/data/generateAvg?fsym=BTC&tsym=USD&e=coinbase";

    HTTPClient https;
    https.begin(url);

    int httpCode = https.GET();
    if (httpCode != HTTP_CODE_OK) {
        Serial.printf("HTTP GET failed: %d\n", httpCode);
        https.end();
        return -1;
    }

    String payload = https.getString();
    https.end();

    // Parse JSON
    StaticJsonDocument<256> doc;
    auto error = deserializeJson(doc, payload);
    if (error) {
        Serial.println("JSON parse failed");
        return -1;
    }

    // Extract price
    float price = doc["RAW"]["PRICE"] | -1.0;
    return price;
}

void drawGraph() {
  display->clearScreen();

  // Draw graph lines
  for (int x = 0; x < HISTORY_SIZE; x++) {
    int height = int( max( (priceHistory[x]-minPrice)/((maxPrice+1)-minPrice)*16, float(1.0) ) );
    if(x == HISTORY_SIZE-1){
      Serial.println(height);
    }

    if(updated || x>=(HISTORY_SIZE-historyIndex)){
      display->drawLine(x, PANEL_RES_Y-1, x, PANEL_RES_Y-height, display->color565(0,0,255));
    }
    else{
      display->drawLine(x, PANEL_RES_Y-1, x, PANEL_RES_Y-height, display->color565(0,255,0));
    }

  }

  // Draw latest "price" text
  char buf[15];
  snprintf(buf, sizeof(buf), "$%.0f", priceHistory[HISTORY_SIZE-1]);

  
  display->setCursor(0, 0);
  display->setTextSize(1);
  display->setTextColor(display->color565(255,255,255), display->color565(0,0,0));
  display->print("BTC:");
  display->setCursor(0, 9);
  display->print(buf);
}