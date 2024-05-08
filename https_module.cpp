#include <iostream>
#include <string>
#include <curl/curl.h>

// Helper function to set up the CURL handle for JSON POST requests
static void setup_curl_for_json(CURL *curl, const std::string& data, struct curl_slist *headers) {
    // Set the URL that the request is sent to
    curl_easy_setopt(curl, CURLOPT_URL, "http://35.225.141.60:3000/update");

    // Set the content-type header
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    // Set the POST data
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data.c_str());

    // Enable that we want to use the POST method
    curl_easy_setopt(curl, CURLOPT_POST, 1L);

    // Follow redirection if necessary
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
}

int main() {
    CURL *curl;
    CURLcode res;
    struct curl_slist *headers = NULL;

    // JSON data to send
    std::string jsonData = "{\"location\": \"32.123,-117.123\", \"batteryLevel\": 75, \"trashLevel\": 30, \"wifiLocation\": \"office\"}";

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if(curl) {
        setup_curl_for_json(curl, jsonData, headers);

        // Perform the request, res will get the return code
        res = curl_easy_perform(curl);
        
        // Check for errors
        if(res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        }

        // Cleanup
        curl_easy_cleanup(curl);
        curl_slist_free_all(headers);
    }

    curl_global_cleanup();
    return 0;
}
