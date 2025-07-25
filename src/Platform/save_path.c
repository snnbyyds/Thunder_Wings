/*
 * Copyright 2025 Nuo Shen, Nanjing University
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "Platform/save_path.h"
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#ifdef _WIN32
#include <windows.h>
#define PATH_SEP '\\'
#else
#include <unistd.h>
#define PATH_SEP '/'
#endif

static const char *APP_NAME = "Thunder_Wings";
static const char *SAVE_FILE = "game_data.json";

static int mkdir_p(const char *path) {
    char tmp[SAVE_PATH_MAX];
    size_t len = strlen(path);
    if (len == 0 || len >= sizeof(tmp))
        return -1;
    memcpy(tmp, path, len + 1);
    for (size_t i = 1; i < len; i++) {
        if (tmp[i] == '/' || tmp[i] == '\\') {
            tmp[i] = '\0';
            if (mkdir(tmp, 0755) && errno != EEXIST)
                return -1;
            tmp[i] = PATH_SEP;
        }
    }
    return mkdir(tmp, 0755) && errno != EEXIST ? -1 : 0;
}

int get_save_directory(char *out_dir) {
    const char *base = NULL;
#ifdef _WIN32
    /* On Windows, prefer %APPDATA% */
    base = getenv("APPDATA");
#else
    /* On Linux/macOS, prefer XDG_CONFIG_HOME */
    base = getenv("XDG_CONFIG_HOME");
    if (!base || !*base) {
        const char *home = getenv("HOME");
        if (!home || !*home)
            return -1;
        /* fallback to ~/.config */
        static char fallback[SAVE_PATH_MAX];
        if (snprintf(fallback, sizeof(fallback), "%s%c.config", home,
                     PATH_SEP) >= (int)sizeof(fallback))
            return -1;
        base = fallback;
    }
#endif

    if (snprintf(out_dir, SAVE_PATH_MAX, "%s%c%s", base, PATH_SEP, APP_NAME) >=
        SAVE_PATH_MAX) {
        return -1;
    }

    return mkdir_p(out_dir);
}

int get_save_file_path(char *out_path) {
    static char dir[SAVE_PATH_MAX];
    if (get_save_directory(dir) != 0)
        return -1;
    if (snprintf(out_path, SAVE_PATH_MAX, "%s%c%s", dir, PATH_SEP, SAVE_FILE) >=
        SAVE_PATH_MAX)
        return -1;
    return 0;
}
