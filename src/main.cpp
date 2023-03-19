#include <Arduino.h>
#include <WiFi.h>


const char *ssid = "ESP32";
const char *password = "12345678";

WiFiServer server(80);

String header;

int humedad;

const int bomba = 14;
const int sensor = 12;
void resetear(void *arg)
{
  esp_restart();
}
void setup()
{
  humedad = 0;
  pinMode(bomba, OUTPUT);
  digitalWrite(bomba, LOW);
  pinMode(sensor, INPUT_PULLDOWN);
  humedad = analogRead(sensor);
  
  if (humedad >= 2100)
  {
    digitalWrite(bomba, HIGH);
    delay(500);
    digitalWrite(bomba, LOW);
  }

  esp_timer_create_args_t restart_timer_args = {
      .callback = &resetear,
      .name = "restart_timer"};
  esp_timer_handle_t restart_timer;
  esp_timer_create(&restart_timer_args, &restart_timer);
  esp_timer_start_once(restart_timer, 120000000);

  Serial.begin(9600);

  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.begin();
  
}

void loop()
{
  WiFiClient client = server.available();

  if (client)
  {
    Serial.println("New Client.");
    String currentLine = "";
    while (client.connected())
    {
      if (client.available())
      {
        char c = client.read();
        Serial.write(c);
        header += c;
        if (c == '\n')
        {
          if (currentLine.length() == 0)
          {

            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta http-equiv=\"refresh\" name=\"viewport\" content=\"30, width=device-width, initial-scale=1\">");
            client.println("</style></head>");

            client.println("<body><h1>ESP32 Humedad</h1>");
            client.print("<h2>");
            client.print(humedad);
            client.println("</h2>");
            client.println("</body></html>");

            client.println();

            break;
          }
          else
          {
            currentLine = "";
          }
        }
        else if (c != '\r')
        {
          currentLine += c;
        }
      }
    }

    header = "";

    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}
