#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "server.h"

short ShortSwap (short s)
{
	union
	{
		short	s;
		unsigned char b[2];
	} dat1, dat2;
	dat1.s = s;
	dat2.b[0] = dat1.b[1];
	dat2.b[1] = dat1.b[0];
	return dat2.s;
}

int LongSwap (int l)
{
	union
	{
		int		l;
		unsigned char b[4];
	} dat1, dat2;
	dat1.l = l;
	dat2.b[0] = dat1.b[3];
	dat2.b[1] = dat1.b[2];
	dat2.b[2] = dat1.b[1];
	dat2.b[3] = dat1.b[0];
	return dat2.l;
}

float FloatSwap (float f)
{
	union
	{
		float	f;
		unsigned char b[4];
	} dat1, dat2;
	dat1.f = f;
	dat2.b[0] = dat1.b[3];
	dat2.b[1] = dat1.b[2];
	dat2.b[2] = dat1.b[1];
	dat2.b[3] = dat1.b[0];
	return dat2.f;
}


/*
 * will print to console and console only
 */
void Print_Console(const char *format, ...)
{
	va_list argptr;
	va_start (argptr, format);
	vprintf(format, argptr);
	va_end(argptr);
}

/*
 * String Compare
 */
qboolean String_Compare(char *s1, char *s2)
{
	if (s1 == NULL || s2 == NULL)
		return false;
	if (strcmp(s1, s2) == 0)
		return true;
	return false;
}

/*
 * String Compare Length
 */
qboolean String_CompareLength(char *s1, char *s2, int *len)
{
	int l;

	if (s1 == NULL || s2 == NULL)
		return false;

	if (len == NULL)
		l = strlen(s1);
	else
		l = *len;

	if (strncmp(s1, s2, l) == 0)
		return true;
	return false;
}

/*
 * Checks of s1 only contains chars from s2
 */

qboolean String_CheckOnly(char *s1, const char *s2)
{
	int i, x;
	char *s;

	if (s1 == NULL || s2 == NULL)
		return false;

	s = s1;
	x = strlen(s2);

	while (*s)
	{
		for (i=0; i<x; i++)
		{
			if (*s != s2[i])
				return false;
		}
		s++;
	}
	return true;
}

char *File_Read(char *name, int *length)
{
	FILE *f;
	size_t rlen;
	char *buf;
	int len;

	if (name == NULL)
		return NULL;

	f = fopen(name, "r");

	if (f == NULL)
		return NULL;

	fseek(f, 0, SEEK_END);
	len = ftell(f);

	if (len < 0)
	{
		fclose(f);
		return NULL;
	}

	fseek(f, 0, SEEK_SET);

	buf = calloc(len + 1, sizeof(char));

	if (!buf)
	{
		fclose(f);
		return NULL;
	}

	rlen = fread(buf, 1, len, f);

	fclose(f);

	if (len != rlen || len <= 0)
	{
		free(buf);
		return NULL;

	}
	if (length != NULL)
		*length = len;

	return buf;
}

double Tools_DoubleTime(struct server *server)
{
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	if (!server->secbase)
	{
		server->secbase = ts.tv_sec;
		return ts.tv_nsec / 1000000000.0;
	}

	return (ts.tv_sec - server->secbase) + ts.tv_nsec / 1000000000.0;
}

void Print_ToAll(struct server *server, qboolean reliable, int level,  char *format, ...)
{
	struct buffer buffer;
	va_list argptr;

	if (!server)
		return;

	va_start(argptr, format);
	buffer.position = vsnprintf((char *)&buffer.data[0], BUFFER_SIZE, format, argptr);
	va_end(argptr);

	if (reliable)
		SendReliable_ToAll(server, "ccs", svc_print, level, buffer.data);
	else
		Send_ToAll(server, "ccs", svc_print, level, buffer.data);
}

void Print_ToClient(struct client *client, qboolean reliable, int level, char *format, ...)
{
	struct buffer buffer;
	va_list argptr;

	if (!client)
		return;

	va_start(argptr, format);
	buffer.position = vsnprintf((char *)&buffer.data[0], BUFFER_SIZE, format, argptr);
	va_end(argptr);

	if (reliable)
		Client_WriteReliable(client, "ccs", svc_print, level, buffer.data);
	else
		Client_Write(client, "ccs", svc_print, level, buffer.data);
}

void Print_ToClientOOB(struct server *server, struct client *client, qboolean flush, char *format, ...)
{
	va_list argptr;
	char buffer[BUFFER_SIZE];
	int size;
	int i;
	char *packet;
	int packet_size;

	if (!client || !server)
		return;

	if (client->oob_print.position == 0)
	{
		Packet_WriteToBuffer(&client->oob_print, "ob", A2C_PRINT);
	}

	if (!format && flush)
	{
		for (i=0; i<5; i++)
			printf("%X ", client->oob_print.data[i]);
		printf("\n");
		printf("sending 3 %i\n", client->oob_print.position);
		if (client->oob_print.position > 5)
			NET_Send(server->net, client->oob_print.data, client->oob_print.position, &client->address);
		client->oob_print.position = 0;
		return;
	}

	if (!format)
		return;

	va_start(argptr, format);
	size = vsnprintf(buffer, BUFFER_SIZE - 5 , format, argptr);
	va_end(argptr);


	if (size > BUFFER_SIZE - client->oob_print.position)
	{
		for (i=0; i<5; i++)
			printf("%X ", client->oob_print.data[i]);
		printf("\n");
		printf("sending 1 %i\n", client->oob_print.position);
		NET_Send(server->net, client->oob_print.data, client->oob_print.position, &client->address);
		client->oob_print.position = 0;
	}

	{
		memcpy(client->oob_print.data + client->oob_print.position, buffer, size);
		client->oob_print.position += size;
		client->oob_print.data[client->oob_print.position+1] = '\0';
	}

	if (flush)
	{
		for (i=0; i<5; i++)
			printf("%X ", client->oob_print.data[i]);
		printf("\n");
		printf("sending 2 %i\n", client->oob_print.position);
		printf("%s\n", client->oob_print.data +5);
		NET_Send(server->net, client->oob_print.data, client->oob_print.position, &client->address);
		client->oob_print.position = 0;
		packet = (char *)Packet_Create(&packet_size, "ics", HEADER_ID, A2C_PRINT, "\nTTTEEEEST!\n");
		printf("%i\n", NET_Send(server->net, packet, packet_size, &client->address));
		free(packet);
		return;
	}
}

