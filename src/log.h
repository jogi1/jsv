#ifndef LOG_H
#define LOG_H

#define DEBUG_INFO Log_GetDebugInfo(__FILE__, __LINE__, __FUNCTION__)


struct log *Log_Init(struct server *server);
void Log_Print(struct log *log, enum log_type type, char *format, ...);
char *Log_GetDebugInfo(char *file, int line, char *function);
#endif
