/* Copyright (C) 2014 Krzysztof Stachowiak */

#ifndef DIAGNOSTICS_H
#define DIAGNOSTICS_H

#include <stdio.h>
#include <assert.h>

#define DLVL_TRACE 0
#define DLVL_DEBUG 1
#define DLVL_WARNING 2
#define DLVL_ERROR 3

#define DIAG_LEVEL DLVL_DEBUG

#if DIAG_LEVEL <= DLVL_TRACE
#   define DIAG_TRACE(FORMAT, ...) \
		printf("[TRACE] %s:%d " FORMAT "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#   define DIAG_TRACE_FUNC \
		printf("[TRACE] %s:%d In function %s\n", __FILE__, __LINE__, __FUNCTION__);
#else
#   define DIAG_TRACE(...)
#   define DIAG_TRACE_FUNC
#endif

#if DIAG_LEVEL <= DLVL_DEBUG
#   define DIAG_DEBUG(FORMAT, ...) \
		printf("[DEBUG] %s:%d " FORMAT "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#   define DIAG_DEBUG
#endif

#if DIAG_LEVEL <= DLVL_WARNING
#   define DIAG_WARNING(FORMAT, ...) \
		printf("[WARNING] %s:%d " FORMAT "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#   define DIAG_WARNING
#endif

#if DIAG_LEVEL <= DLVL_ERROR
#   define DIAG_ERROR(FORMAT, ...) \
		printf("[ERROR] %s:%d " FORMAT "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
#   define DIAG_ERROR
#endif

#endif
