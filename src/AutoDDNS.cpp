#include <Arduino.h>

#if defined(ESP8266)
  #include "ESP8266WiFi.h"
  #include "ESP8266HTTPClient.h"
#elif defined(ESP32)
  #include "WiFi.h"
  #include "HTTPClient.h"
#endif

#include <EasyDDNS.h>

#include "auth.h"

WiFiServer server(80);

void start_deepsleep() {
    Serial.println("Going to sleep");
#if defined(ESP8266)
    ESP.deepSleep(update_interval_s * 1000000); 
#elif defined(ESP32)
    esp_sleep_enable_timer_wakeup(update_interval_s * 1000000);
    esp_deep_sleep_start();
#else
    Serial.println("No Deep Sleep in this cpu.");
    delay(update_interval_s * 1000);
#endif

}

void setup() {
  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println(WiFi.localIP()); // Print the IP address
  server.begin();

  /*
    List of supported DDNS providers:
    - "duckdns"
    - "noip"
    - "dyndns"
    - "dynu"
    - "enom"
    - "all-inkl"
    - "selfhost.de"
    - "dyndns.it"
    - "strato"
    - "freemyip"
    - "afraid.org"
  */
  EasyDDNS.service(ddns_provider);

  /*
    For DDNS Providers where you get a token:
      Use this: EasyDDNS.client("domain", "token");
    
    For DDNS Providers where you get username and password: ( Leave the password field empty "" if not required )
      Use this: EasyDDNS.client("domain", "username", "password");
  */
  EasyDDNS.client(ddns_host, ddns_user, ddns_password); // Enter your DDNS Domain & Token

  // Get Notified when your IP changes
  EasyDDNS.onUpdate([&](const char* oldIP, const char* newIP){
    Serial.print("EasyDDNS - IP Change Detected: ");
    Serial.println(newIP);
  });

  EasyDDNS.update(0);

  if (use_deepsleep) {
    start_deepsleep();
  }
}

void loop() {
  if (!use_deepsleep) {
    delay(update_interval_s * 1000);
  }
  
  Serial.println("Checking IP...");
  // Check for new public IP every 300 seconds == 5 minutes
  EasyDDNS.update(update_interval_s * 1000);

  if (use_deepsleep) {
    start_deepsleep();
  }
}
