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

#ifndef __SAVE_PATH_H__
#define __SAVE_PATH_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/**
 * Maximum path length.
 */
#define SAVE_PATH_MAX 512

/**
 * Fill `out_dir` with the directory of save files.
 *   - Windows:  %APPDATA%\<APP_NAME>\
 *   - Linux:    $XDG_CONFIG_HOME/<APP_NAME>/ or ~/.config/<APP_NAME>/
 * Returns 0 on success, -1 on error.
 */
int get_save_directory(char *out_dir);

/**
 * Fill `out_path` with the full path to game_data.json under the save dir.
 * Returns 0 on success, -1 on error.
 */
int get_save_file_path(char *out_path);

#ifdef __cplusplus
}
#endif

#endif /* __SAVE_PATH_H__ */
