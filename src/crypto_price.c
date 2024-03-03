#include "lib/crypto_price.h"
#include "lib/error_handler.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <jansson.h> // Include the json-c library for JSON parsing

#define API_URL "https://api.binance.com/api/v1/klines"
#define INTERVAL "1d" // Daily interval
#define OUTPUT_FILE "target/price.txt" // Output file path

struct MemoryStruct {
    char *memory;
    size_t size;
};

// Write the prices to the output file
void write_prices_to_file(FILE *fp, const char *symbol, const char *date, const char *price) {
    fprintf(fp, "%s -> Price: %s\n", date, price);
}

// Write callback to save data to memory
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if(ptr == NULL) {
        handle_error("not enough memory (realloc returned NULL)", NULL);
        // Pass NULL since there's no content window in crypto_price.c
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

// Function to retrieve crypto prices for the last month and write them to file
void get_crypto_prices_last_month(const char *symbol) {
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;
    FILE *fp;

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl = curl_easy_init();
    if(curl) {
        char url[256];
        snprintf(url, sizeof(url), "%s?symbol=%s&interval=%s", API_URL, symbol, INTERVAL);

        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        fp = fopen(OUTPUT_FILE, "w"); // Open output file for writing
        if (!fp) {
            handle_error("Failed to open output file", NULL);
        }

        res = curl_easy_perform(curl);
        if(res != CURLE_OK)
            handle_error("curl_easy_perform() failed", NULL);
            // Pass NULL since there's no content window in crypto_price.c

        // Parse the JSON response
        json_t *root;
        json_error_t error;
        root = json_loads(chunk.memory, 0, &error);
        if (!root) {
            handle_error("JSON parsing error", NULL);
        }

        // Process the parsed JSON data
        if (json_is_array(root)) {
            size_t i;
            json_t *data;
            json_array_foreach(root, i, data) {
                // Extract the timestamp and price from each array element
                json_t *timestamp = json_array_get(data, 0);
                json_t *price = json_array_get(data, 4);
                if (json_is_integer(timestamp) && json_is_string(price)) {
                    // Convert timestamp to date (You may need to adjust this depending on the timestamp format)
                    time_t timestamp_sec = json_integer_value(timestamp);
                    struct tm *tm_info = localtime(&timestamp_sec);
                    char date_str[20];
                    strftime(date_str, sizeof(date_str), "%d/%m/%Y", tm_info);
                    // Write date and price to file
                    write_prices_to_file(fp, symbol, date_str, json_string_value(price));
                }
            }
        } else {
            handle_error("JSON data is not an array", NULL);
        }

        // Cleanup
        json_decref(root);
        fclose(fp);
        free(chunk.memory);
        curl_easy_cleanup(curl);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <symbol>\n", argv[0]);
        return 1;
    }

    const char *symbol = argv[1];
    get_crypto_prices_last_month(symbol);

    return 0;
}
