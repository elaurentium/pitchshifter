/*
    MIT License

    Copyright (c) 2025 Evandro

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

*/

#ifndef LOGGER_H
#define LOGGER_H

#include <stdio.h>
#include <stdbool.h>
#include <pthread.h>

#ifdef __cplusplus
extern "C" {
#endif

// possible log bits
typedef enum {
    None = 0x00,
    Error = 0x01,
    Warning = 0x02,
    Info = 0x04,
    Debug = 0x08,
    Constructors = 0x10,
    Locks = 0x20
} LogLevels;

typedef struct Logger {
    unsigned bitmask;
    bool running;
    bool use_stdout;
    bool log_timestamps;
    bool log_colors; // Support colors
    char log_file_path[256];

    pthread_mutex_t mutex;
    pthread_cond_t cond;
    FILE *file;
} Logger;

Logger *logger_create(const char *path, bool use_stdout, bool timestamps, bool colors);

Logger *logger_get_instance(void);
void logger_set_bitmask(unsigned mask);
unsigned logger_get_bitmask(void);
bool logger_should_log(Logger *logger, unsigned level);
void logger_log(Logger *logger, unsigned level,
                const char *class_name,
                const char *func_name,
                const char *msg);
void logger_flush(Logger *logger);
void logger_destroy(Logger *logger);

#ifdef __cplusplus
}
#endif

#endif // LOGGER_H