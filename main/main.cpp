/* HTTP GET Example using plain POSIX sockets

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/

// This example uses cJSON to parse the obtained JSON from the OpenWeatherMap.
// This template is modified by uncle-yong: https://github.com/uncle-yong

#include <string.h>

#include <string>
#include <sstream>
#include <iomanip>
#include <iostream>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event_loop.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"

#include "cJSON.h"

#include "i2c_lcd.h"

/* The examples use simple WiFi configuration that you can set via
   'make menuconfig'.

   If you'd rather not, just change the below entries to strings with
   the config you want - ie #define EXAMPLE_WIFI_SSID "mywifissid"
*/

// Please supply your own WIFI SSID and Password!
#define WIFI_SSID "mywifi"
#define WIFI_PASS "mywifi123"

// Openweathermap sample: http://samples.openweathermap.org/data/2.5/weather?id=524901&appid=b1b15e88fa797225412429c1c50c122a1
// source: http://zeflo.com/2014/esp8266-weather-display/

/* FreeRTOS event group to signal when we are connected & ready to make a request */
static EventGroupHandle_t wifi_event_group;

/* The event group allows multiple bits for each event,
   but we only care about one event - are we connected
   to the AP with an IP? */
const int CONNECTED_BIT = BIT0;

/* Constants that aren't configurable in menuconfig */
#define WEB_SERVER "api.openweathermap.org"                                           // <- place your openweathermap URL there.
#define WEB_PORT 80
#define WEB_URL "/data/2.5/weather?id=524901&appid=b1b15e88fa797225412429c1c50c122a1" // <- place your openweathermap URL there.

static const char *TAG = "example";

static const char *REQUEST = "GET " WEB_URL " HTTP/1.0\r\n"
    "Host: " WEB_SERVER "\r\n"
    "User-Agent: esp-idf/1.0 esp32\r\n"
    "\r\n";

static char inputHttpBuffer[2048];    

static esp_err_t event_handler(void *ctx, system_event_t *event)
{
    static int err_wifi_connect;
    ESP_LOGI(TAG, "Event ID: %d\n", event->event_id);
    ESP_LOGI(TAG, "Wifi Status: %d\n", err_wifi_connect);
    
    switch(event->event_id) {
    case SYSTEM_EVENT_STA_START:
        err_wifi_connect = esp_wifi_connect();
        break;
    case SYSTEM_EVENT_STA_GOT_IP:
        xEventGroupSetBits(wifi_event_group, CONNECTED_BIT);
        break;
    case SYSTEM_EVENT_STA_DISCONNECTED:
        /* This is a workaround as ESP32 WiFi libs don't currently
           auto-reassociate. */
        err_wifi_connect = esp_wifi_connect();
        xEventGroupClearBits(wifi_event_group, CONNECTED_BIT);
        break;
    default:
        break;
    }
    return ESP_OK;
}

static void initialise_wifi(void)
{
    tcpip_adapter_init();
    wifi_event_group = xEventGroupCreate();
    ESP_ERROR_CHECK( esp_event_loop_init(event_handler, NULL) );
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK( esp_wifi_init(&cfg) );
    ESP_ERROR_CHECK( esp_wifi_set_storage(WIFI_STORAGE_RAM) );
    
    // Only for C! Doesn't compile for C++!!
    //wifi_config_t wifi_config = {
    //    .sta = {
    //        .ssid =     WIFI_SSID,
    //        .password = WIFI_PASS,
    //    },
    //};

    static wifi_config_t wifi_config = { };
    strcpy((char*)wifi_config.sta.ssid, (const char*)WIFI_SSID);
    strcpy((char*)wifi_config.sta.password, (const char*)WIFI_PASS);
    
    //ESP_LOGI(TAG, "Setting WiFi configuration SSID %s...", wifi_config.sta.ssid);
    //ESP_LOGI(TAG, "password %s...", wifi_config.sta.password);
    ESP_ERROR_CHECK( esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK( esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config) );
    ESP_ERROR_CHECK( esp_wifi_start() );
}

static void http_get_task(void *pvParameters)
{
    // Only for C! Doesn't compile for C++!!
    //const struct addrinfo hints = {
    //    .ai_family = AF_INET,
    //    .ai_socktype = SOCK_STREAM,
    //};

    struct addrinfo hints{};
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo *res;
    struct in_addr *addr;
    //int s, r;
    //char recv_buf[64];

    int s;

    // Initialize I2C LCD module:
    I2CLCD i2clcd1;
    i2clcd1.backlightOn();

    // Deep sleep for an amount of seconds:    
    const int deep_sleep_sec = 1200;

    while (1)
    {
        i2clcd1.writeText("Connecting...",0, 0);   

        // Start WiFi module:
        initialise_wifi();

        while (1)
        {
            /* Wait for the callback to set the CONNECTED_BIT in the
            event group. */
            xEventGroupWaitBits(wifi_event_group, CONNECTED_BIT,
                                false, true, portMAX_DELAY);
            ESP_LOGI(TAG, "Connected to AP");

            int err = getaddrinfo(WEB_SERVER, "80", &hints, &res);

            if (err != 0 || res == NULL)
            {
                ESP_LOGE(TAG, "DNS lookup failed err=%d res=%p", err, res);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                continue;
            }

            /* Code to print the resolved IP.

           Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
            addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
            ESP_LOGI(TAG, "DNS lookup succeeded. IP=%s", inet_ntoa(*addr));

            s = socket(res->ai_family, res->ai_socktype, 0);
            if (s < 0)
            {
                ESP_LOGE(TAG, "... Failed to allocate socket.");
                freeaddrinfo(res);
                vTaskDelay(1000 / portTICK_PERIOD_MS);
                continue;
            }
            ESP_LOGI(TAG, "... allocated socket");

            if (connect(s, res->ai_addr, res->ai_addrlen) != 0)
            {
                ESP_LOGE(TAG, "... socket connect failed errno=%d", errno);
                close(s);
                freeaddrinfo(res);
                vTaskDelay(4000 / portTICK_PERIOD_MS);
                continue;
            }

            ESP_LOGI(TAG, "... connected");
            freeaddrinfo(res);

            if (write(s, REQUEST, strlen(REQUEST)) < 0)
            {
                ESP_LOGE(TAG, "... socket send failed");
                close(s);
                vTaskDelay(4000 / portTICK_PERIOD_MS);
                continue;
            }
            ESP_LOGI(TAG, "... socket send success");

            struct timeval receiving_timeout;
            receiving_timeout.tv_sec = 5;
            receiving_timeout.tv_usec = 0;
            if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, &receiving_timeout,
                           sizeof(receiving_timeout)) < 0)
            {
                ESP_LOGE(TAG, "... failed to set socket receiving timeout");
                close(s);
                vTaskDelay(4000 / portTICK_PERIOD_MS);
                continue;
            }
            ESP_LOGI(TAG, "... set socket receiving timeout success");

            break;
        }

        /* Read HTTP response */

        // Collect response and paste it character by character through UART:
        //do
        //{
        //    bzero(recv_buf, sizeof(recv_buf));
        //    r = read(s, recv_buf, sizeof(recv_buf) - 1);
        //    for (int i = 0; i < r; i++)
        //    {
        //        putchar(recv_buf[i]);
        //    }
        //} while (r > 0);

        // Dump the stuff into the buffer for further reading...
        bzero(inputHttpBuffer, sizeof(inputHttpBuffer));
        read(s, inputHttpBuffer, sizeof(inputHttpBuffer) - 1);
        //printf("%s", inputHttpBuffer);

        // Retrieve the Json information so that the Json parser can successfully read it.
        // We want only the Json, the other retrieved information can be discarded by pointing the location
        // to the Json area and copy it back into the string.
        std::string inputHttpStr(inputHttpBuffer);
        unsigned int locationToStartParsingJson = inputHttpStr.find("\r\n\r\n");
        inputHttpStr = inputHttpStr.substr(locationToStartParsingJson);

        double temperature = 0.00f;
        char temperature_str[8];

        char *weather_status_str;

        // Convert this back into C string because the Json parser do not support strings.
        strcpy(inputHttpBuffer, inputHttpStr.c_str());

        cJSON *root = cJSON_Parse(inputHttpBuffer);

        // Getting the "main" one - for the temperature, humidity and etc.
        cJSON *weather_main = cJSON_GetObjectItem(root, "main");
        cJSON *weather_temp = cJSON_GetObjectItem(weather_main, "temp");

        // The "weather" inside the output is in the form of an array!
        cJSON *weather_item = cJSON_GetObjectItem(root, "weather");
        cJSON *weather_subitem = cJSON_GetArrayItem(weather_item, 0);
        cJSON *weather_subitem_main = cJSON_GetObjectItem(weather_subitem, "description");

        // Place them inside the variables.
        temperature = weather_temp->valuedouble / 10.00f;
        weather_status_str = weather_subitem_main->valuestring;

        // Self-explanatory.
        printf("Weather now is: %s\n", weather_status_str);
        printf("Temperature: %f\n", temperature);

        sprintf(temperature_str, "%2.2f%cC", temperature, 223);
        i2clcd1.clearDisplay();
        i2clcd1.writeText(weather_status_str, 0, 0);
        i2clcd1.writeText(temperature_str, 1, 0);

        cJSON_Delete(root);

        close(s);

        printf("\nRetrieving weather data done...\n");

        //vTaskDelay(1000 / portTICK_PERIOD_MS);

        // Disconnect WiFi here:
        esp_wifi_disconnect();

        // Deep sleep for a number of seconds:
        esp_deep_sleep(1000000LL * deep_sleep_sec);

    }
}

extern "C" void app_main()
{
    ESP_ERROR_CHECK(nvs_flash_init());
    //initialise_wifi();
    xTaskCreate(&http_get_task,
                "http_get_task", 4096, NULL, 5, NULL);
}
