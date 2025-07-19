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

#ifndef __MACROS_H__
#define __MACROS_H__

#include "config.h" // IWYU pragma: keep

/* clang-format off */
#define TW_VERSION    "1.0.0"
#define TW_BUILD_DATE __DATE__ " " __TIME__
#define TW_COPYRIGHT  "Copyright 2025 Nuo Shen, Nanjing University"
#define TW_LICENSE    "Apache License 2.0"
/* clang-format on */

#if defined(__GNUC__) || defined(__clang__)
#define __unreachable() __builtin_unreachable()
#elif defined(_MSC_VER)
#define __unreachable() __assume(false)
#else /* unspported compilers */
/* clang-format off */
#define __unreachable() do { /* nop */ } while (0)
/* clang-format on */
#endif

#endif /* __MACROS_H__ */
