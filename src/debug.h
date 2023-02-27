#ifndef DEBUG_H
#define DEBUG_H

#define NODEBUG_L 0
#define ERROR_L 1
#define WARNING_L 2
#define INFO_L 3
#define DEBUG_L 4

#ifndef LOG_LEVEL
	#define LOG_LEVEL INFO_L
#endif

#ifndef DEBUG_START
	#define DEBUG_START		"\e[1;34m"
#endif
#ifndef WARNING_START
	#define WARNING_START	"\e[1;33m"
#endif
#ifndef ERROR_START
	#define ERROR_START		"\e[1;31m"
#endif
#ifndef INFO_START
	#define INFO_START		"\e[1;32m"
#endif

#define LOG_END "\e[00m\n"


#define D(str, ...) \
	fprintf(stdout, DEBUG_START "%s:%d %s: " str LOG_END, __FILE__, __LINE__, __func__,  ##__VA_ARGS__)

#if (LOG_LEVEL >= DEBUG_L)
	#define DEBUG(str, ...) \
	fprintf(stdout, DEBUG_START str LOG_END, ##__VA_ARGS__)
#endif

#if (LOG_LEVEL >= INFO_L)
	#define INFO(str, ...) \
	fprintf(stdout, INFO_START str LOG_END, ##__VA_ARGS__)
#endif

#if (LOG_LEVEL >= WARNING_L)
	#define WARNING(str, ...) \
	fprintf(stderr, WARNING_START str LOG_END, ##__VA_ARGS__)
#endif

#if (LOG_LEVEL >= ERROR_L)
	#define ERROR(str, ...) \
	fprintf(stderr, ERROR_START str LOG_END, ##__VA_ARGS__)
#endif


#ifndef DEBUG
	#define DEBUG(...)
#endif
#ifndef WARNING
	#define WARNING(...)
#endif
#ifndef ERROR
	#define ERROR(...)
#endif
#ifndef INFO
	#define INFO(...)
#endif

#endif // DEBUG_H
