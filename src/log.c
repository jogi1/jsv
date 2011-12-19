#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "server.h"

void Log_Cleanup(struct log *log)
{
	struct log_entry *e, *ne;

	e = log->first;

	while (e)
	{
		ne = e->next;
		free(e->string);
		free(e);
		e = ne;
	}

	if (log->main)
		fclose(log->main);

	if (log->lua)
		fclose(log->lua);

	if (log->debug)
		fclose(log->debug);

	free(log->log_dir);
	free(log);

}

static void *Log_Thread(void *data)
{
	struct log *log;
	char date[1024];
	char buffer[8192];
	struct log_entry *e, *ne;
	struct tm *tm;
	FILE *f;
	int i;

	log = (struct log *) data;

	if (!log)
		return NULL;

	//open the files
	snprintf(buffer, sizeof(buffer), "%smain", log->log_dir);
	log->main = fopen(buffer, "wa");
	if (log->main == NULL)
	{
		Print_Console("could not open logfile: %s\n", buffer);
	}

	snprintf(buffer, sizeof(buffer), "%slua", log->log_dir);
	log->lua = fopen(buffer, "wa");
	if (log->lua == NULL)
	{
		Print_Console("could not open logfile: %s\n", buffer);
	}

	snprintf(buffer, sizeof(buffer), "%sdebug", log->log_dir);
	log->debug = fopen(buffer, "wa");
	if (log->debug == NULL)
	{
		Print_Console("could not open logfile: %s\n", buffer);
	}

	while (1)
	{

		if (log->run == false)
			if (log->first == log->last)
				break;

		e = log->first;
		while (e)
		{
			ne = e->next;

			if (!e->done)
			{
				tm = localtime(&e->time);
				strftime(date, sizeof(date), "%T", tm);
				if (e->type == log_main)
					f = log->main;
				else if (e->type == log_lua)
					f = log->lua;
				else if (e->type == log_debug)
					f = log->debug;
				else
					f = NULL;

				if (f)
				{
					i = strlen(e->string);
					i--;
					if (i)
						if (e->string[i] == '\n')
							e->string[i] = '\0';
					fprintf(f, "%s: %s\n", date, e->string);
					fflush(f);
				}
				e->done = true;
			}

			if (e != log->last)
			{
				if (e->done)
				{
					if (e == log->first)
						log->first = ne;
					free(e->string);
					free(e);
				}
			}
			e = ne;
		}
		usleep(10000);
	}

	Log_Cleanup(log);
	return NULL;
}

struct log *Log_Init(struct server *server)
{
	struct log *log;
	struct log_entry *le;
	char buffer[1024];
	char date[1024];
	struct tm *tm;
	int status;

	log = calloc(1, sizeof(struct log));
	if (!log)
		return NULL;

	le = calloc(1, sizeof(*le));
	
	if (!le)
	{
		free(log);
		return NULL;
	}

	le->done = true;
	log->first = le;
	log->last = le;

	tm = localtime(&server->time_start);
	strftime(date, sizeof(date), "%Y-%m-%d-%H%M%S", tm);
	snprintf(buffer, sizeof(buffer), "log/%s__%i/", date, server->pid);
	status = mkdir(buffer, S_IRWXU| S_IRWXG);
	if (status == -1)
	{
		status = mkdir("log", S_IRWXU| S_IRWXG);
		if (status == -1)
		{
			Print_Console("could not create log dir \"%s\"\n", buffer);
			free(log);
			free(le);
			return NULL;
		}
		status = mkdir(buffer, S_IRWXU| S_IRWXG);
		if (status == -1)
		{
			Print_Console("could not create log dir \"%s\"\n", buffer);
			free(log);
			free(le);
			return NULL;
		}

	}
	log->log_dir = strdup(buffer);
	Print_Console("logging to %s\n", log->log_dir);

	log->run = true;
	pthread_create(&server->log_thread, NULL, (void *) &Log_Thread, (void *)log);

	return log;
}

void Log_Print(struct log *log, enum log_type type, char *format, ...)
{
	struct log_entry *entry;
	va_list argptr;
	char buffer[8192];

	if (log)
	{
		entry = calloc(1, sizeof(*entry));
		if (!entry)
			return;

		va_start(argptr, format);
		vsnprintf(buffer, sizeof(buffer), format, argptr);
		va_end(argptr);

		entry->type = type;
		entry->string = strdup(buffer);
		time(&entry->time);
		log->last->next = entry;
		log->last = entry;
	}
	else
	{
		va_start(argptr, format);
		vsnprintf(buffer, sizeof(buffer), format, argptr);
		va_end(argptr);
		switch (type)
		{
			case log_main:
				printf("main : %s\n", buffer);
				break;
			case log_debug:
				printf("debug: %s\n", buffer);
				break;
			case log_lua:
				printf("lua  : %s\n", buffer);
				break;
		}
	}
}
