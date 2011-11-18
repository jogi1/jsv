#ifndef LOG_H
#define LOG_H


struct log *Log_Init(struct server *server);
void Log_Print(struct log *log, enum log_type type, char *format, ...);
#endif
