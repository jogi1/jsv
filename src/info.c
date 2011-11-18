#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "server.h"

void Info_Clean(struct info *info)
{
	int i;

	if (!info)
		return;

	for (i=0;i<CLIENT_INFO_MAX;i++)
	{
		if (info->info[i].key == NULL)
		{
			free(info);
			return;
		}

		free(info->info[i].key);
		free(info->info[i].value);
	}
	free(info);
}

char *Info_GetValue(struct info *info, char *key)
{
	int i;
	
	if (!info || !key)
		return NULL;

	for (i=0;i<CLIENT_INFO_MAX;i++)
	{
		if (info->info[i].key == NULL)
			return NULL;
		if (String_Compare(info->info[i].key, key))
			return info->info[i].value;
	}
	return NULL;
}

qboolean Info_SetValue(struct info *info, char *key, char *value)
{
	int i;
	char *v;

	if (!info || !key || !value)
		return false;

	v = strdup(value);
	if (!v)
		return false;

	for(i=0; i<CLIENT_INFO_MAX && info->info[i].key; i++)
		if (String_Compare(info->info[i].key, key))
		{
			if (info->info[i].value)
				free(info->info[i].value);
			info->info[i].value = v;
			return true;
		}

	info->info[i].key = strdup(key);
	if (!info->info[i].key)
	{
		free(v);
		return false;
	}

	info->info[i].value = v;
	return true;
}

#define IFM_STRCMP 0
#define IFM_STRNCMP 1
#define IFM_STRSTR 2

struct info_relevance 
{
	char *key;
	int flags;
	int check_method;
};

struct info_relevance info_relevance[] =
{
	{"name", IF_CLIENT | IF_SERVER, IFM_STRCMP},
	{"spectator", IF_SERVER | IF_CLIENT, IFM_STRCMP},
	{"rate", IF_SERVER, IFM_STRCMP},
	{"topcolor", IF_CLIENT | IF_SERVER, IFM_STRCMP},
	{"bottomcolor", IF_CLIENT | IF_SERVER, IFM_STRCMP},
	{"noaim", IF_SERVER, IFM_STRCMP},
	{"*", IF_SERVER | IF_CLIENT, IFM_STRNCMP},
};

qboolean Info_CopyToBuffer(struct info *info, struct buffer *buffer, int flags)
{
	int i, j;
	struct key_val *kv;

	if (!info || !buffer)
		return false;

	if (info->size >= BUFFER_SIZE)
		return false;

	for (i=0;i<CLIENT_INFO_MAX;i++)
	{
		kv = &info->info[i];
		if (kv->key == NULL)
			break;
		if (kv->value == NULL)
			return false;
		if (flags & kv->flags || flags & IF_ALL)
		{
			Packet_WriteToBuffer(buffer, "czcz", '\\', kv->key, '\\', kv->value);
		}
	}

	buffer->data[buffer->position] = '\0';

	return true;
}

int Info_GetFlags(char *key)
{
	int i;

	for (i=0; i<(sizeof(info_relevance)/sizeof(*info_relevance)); i++)
	{
		switch (info_relevance[i].check_method)
		{
			case IFM_STRCMP:
				if (String_Compare(info_relevance[i].key, key))
					return info_relevance[i].flags;
				break;
			case IFM_STRNCMP:
				if (String_CompareLength(info_relevance[i].key, key, NULL))
					return info_relevance[i].flags;
				break;
			case IFM_STRSTR:
				break;
		}
	}

	return IF_ALL;
}

struct info *Info_FromString(char *string)
{
	struct info *info;
	char *s, *start, *stop;
	int count;
	int i;

	if (!string)
		return NULL;

	info = calloc(1, sizeof(*info));

	if (!info)
		return NULL;


	info->size = 2; // leading and trailing '\'

	//find first
	s = strchr(string, '\\');
	s++;
	count = 0;
	while (s)
	{
		for (i=0; i<2; i++)
		{
			start = s;
			s = strchr(s, '\\');
			stop = s;
			//end of string on key entry -> exit
			if (s == NULL && i == 0)
			{
				Info_Clean(info);
				return NULL;
			}

			if (i == 0)
			{
				info->info[count].key = strndup(start, stop - start);
				info->size += stop - start + 1;
				if (info->info[count].key == NULL)
				{
					Info_Clean(info);
					return NULL;
				}
				info->info[count].flags = Info_GetFlags(info->info[count].key);
			}
			else
			{
				info->info[count].value = strndup(start, stop - start);
				info->size += stop - start + 1;
				if (info->info[count].value == NULL)
				{
					Info_Clean(info);
					return NULL;
				}
			}
			if (s == NULL && i == 1)
				break;
			s++;
			if (s == NULL && i == 1)
				break;
		}
		count++;
	}
	return info;
}


