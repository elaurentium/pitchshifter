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

#include "logger.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define LOG_ERROR 1
#define LOG_WARNING 2
#define LOG_INFO 4

static Logger *instance = NULL;

Logger *logger_create(const char *path, bool use_stdout, bool timestamps, bool colors) {
    if (instance) return instance;

    instance = malloc(sizeof(Logger));
    if (!instance) return NULL;

    instance->bitmask = LOG_ERROR | LOG_WARNING | LOG_INFO;
    instance->running = true;
    instance->use_stdout = use_stdout;
    instance->log_timestamps = timestamps;
    instance->log_colors = colors;

    pthread_mutex_init(&instance->mutex, NULL);
    pthread_cond_init(&instance->cond, NULL);

    if (path && strlen(path) > 0) {
        strncpy(instance->log_file_path, path, sizeof(instance->log_file_path));
        instance->file = fopen(path, "w");
    } else {
        instance->file = NULL;
    }

    return instance;
}

Logger* logger_get_instance(void) {
    return instance;
}

void logger_set_bitmask(unsigned mask) {
    if (instance) instance->bitmask = mask;
}

unsigned logger_get_bitmask(void) {
    return instance ? instance->bitmask : 0;
}

bool logger_should_log(Logger *logger, unsigned level) {
    return (logger && (level & logger->bitmask));
}

static void timestamp(char *buffer, size_t len) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(buffer, len, "[%Y-%m-%d %H:%M:%S]", t);
}

void logger_log(Logger *logger, unsigned level, const char *class_name, const char *func_name, const char *msg) {
    if (!logger_should_log(logger, level))
        return;

    pthread_mutex_lock(&logger->mutex);

    char timebuf[64] = "";
    if (logger->log_timestamps) timestamp(timebuf, sizeof(timebuf));

    FILE* out = logger->use_stdout ? stdout : logger->file;
    if (out) {
        fprintf(out, "%s [%s::%s] %s\n",
                timebuf, class_name, func_name, msg);
        fflush(out);
    }

    pthread_mutex_unlock(&logger->mutex);
}

void logger_flush(Logger *logger) {
    if (!logger) return;
    pthread_mutex_lock(&logger->mutex);
    if (logger->file) fflush(logger->file);
    pthread_mutex_unlock(&logger->mutex);
}

void logger_destroy(Logger *logger) {
    if (!logger) return;
    pthread_mutex_destroy(&logger->mutex);
    pthread_cond_destroy(&logger->cond);
    if (logger->file) fclose(logger->file);
    free(logger);
    instance = NULL;
}

