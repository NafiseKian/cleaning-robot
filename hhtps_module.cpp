#include <iostream>
#include <curl/curl.h>

// Callback function to handle the response
size_t write_callback(char* ptr, size_t size, size_t nmemb, std::string* data) {
    data->append(ptr, size * nmemb);
    return size * nmemb;
}

int main() {
    CURL* curl;
    CURLcode res;
    std::string response;

    curl_global_init(CURL_GLOBAL_ALL);
    curl = curl_easy_init();

    if (curl) {
        // Set the URL you want to send the request to
        curl_easy_setopt(curl, CURLOPT_URL, "https://example.com");

        // Set the callback function to handle the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

        // Perform the request
        res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            std::cout << "Response received: " << response << std::endl;
        }

        // Clean up
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    return 0;
}
