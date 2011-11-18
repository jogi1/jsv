#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "server.h"

#warning probably should split the stuff in here up into proper files

/*
 * Creates a packet
 * a = angle
 * c = char
 * C = coord
 * b = byte
 * B = buffer
 * i = int
 * S = short <- this might suck ...
 * f = float
 * d = double
 * s = string \0 terminated
 * z = string unterminated
 */
unsigned char *Packet_Create(int *len, const char *format, ...)
{
	int size;
	unsigned char *buf;
	char *f;
	va_list argptr;
	char *sval;
	char cval;
	unsigned char bval;
	double dval;
	float fval;
	int ival;

	f = (char *)format;
	size = 0;

	va_start(argptr, format);
	while (*f)
	{
		switch (*f)
		{
			case 'c':
			case 'b':
				va_arg(argptr, int);
				size += sizeof(char);
				break;
			case 'i':
				va_arg(argptr, int);
				size += sizeof(int);
				break;
			case 'f':
				va_arg(argptr, int);
				size += sizeof(float);
				break;
			case 'd':
				va_arg(argptr, int);
				size += sizeof(double);
				break;
			case 's':
				sval = va_arg(argptr, char *);
				size += strlen(sval);
				break;
			default:
				Print_Console("invalid type \'%c\' in format_string\n", *f);
				return NULL;
		}
		f++;
	}
	va_end(argptr);
	buf = malloc(size+1);
	if (buf == NULL)
	{
		Print_Console("error creating packet.\n");
		return NULL;
	}

	buf[size] = '\0';

	if (len)
		*len = size;

	f = (char *)format;
	size = 0;

	va_start(argptr, format);
	while (*f)
	{
		switch (*f)
		{
			case 'c':
				cval = (char)va_arg(argptr, int);
				memcpy(buf+size, (const void*)&cval, sizeof(char));
				size += sizeof(char);
				break;
			case 'b':
				bval = (unsigned char)va_arg(argptr, int);
				memcpy(buf+size, (const void*)&bval, sizeof(char));
				size += sizeof(char);
				break;
			case 'i':
				ival = (int)va_arg(argptr, int);
				/*
				sval = buf+size;
				sval[0] = ival & 0xFF;
				sval[1] = (ival>>8) & 0xFF;
				sval[2] = (ival>>16) & 0xFF;
				sval[3] = ival>24;
				*/
				memcpy(buf+size, (const void*)&ival, sizeof(int));
				size += sizeof(int);
				break;
			case 'f':
				fval = (float)va_arg(argptr, int);
				memcpy(buf+size, (const void*)&fval, sizeof(float));
				size += sizeof(float);
				break;
			case 'd':
				dval = va_arg(argptr, int);
				memcpy(buf+size, (const void*)&dval, sizeof(double));
				size += sizeof(double);
				break;
			case 's':
				sval = (char *)va_arg(argptr, int);
				memcpy(buf+size, (const void *)sval, strlen(sval));
				size += strlen(sval);
				break;
			default:
				Print_Console("invalid type \'%c\' in format_string\n", *f);
				return NULL;
		}
		f++;
	}
	va_end(argptr);


	return buf;
}

qboolean Packet_ReadMessageType(struct packet *packet)
{
	if (packet->offset + sizeof(char) > packet->length)
		return false;
	packet->message_type = (unsigned char)packet->data.data.data[packet->offset];
	packet->offset += sizeof(char);
	return true;
}

struct packet_message *PacketMessageList_Add(struct packet_message_list *pml, int type)
{
	struct packet_message *pm;

	pm = calloc(1, sizeof(struct packet_message));

	if (pm == NULL)
		return NULL;

	pm->type = type;

	if (pml->first == NULL)
	{
		pml->first = pm;
		pml->last = pm;
	}
	else
	{
		pml->last->next = pm;
		pml->last = pm;
	}
	return pm;
}

void PacketMessageList_Clean(struct packet_message_list *pml)
{
	struct packet_message *pm, *npm;
	pm = pml->first;
	while(pm)
	{
		npm = pm->next;
		switch (pm->type)
		{
			case svc_print:
				free(pm->data.svc_print_data.string);
				break;
		}
		free(pm);
		pm = npm;
	}
	pml->first = NULL;
	pml->last = NULL;
}

static void PacketMessage_SetSize(struct packet_message *pm)
{
	switch (pm->type)
	{
		case svc_nop:
		case svc_bad:
			pm->size = sizeof(char);
			break;
		case svc_print:
			pm->size = sizeof(char) * 2;
			if (pm->data.svc_print_data.string)
				pm->size += strlen(pm->data.svc_print_data.string) + 1;
			break;
	}
}

static void PacketMessage_ToBuffer(unsigned char *buffer, struct packet_message *pm)
{
	int offset;

	offset = 0;
	switch (pm->type)
	{
		case svc_nop:
		case svc_bad:
			memset(buffer, pm->type, 1);
			break;

		case svc_print:
			memset(buffer, pm->type, 1);
			offset += 1;
			memset(buffer+offset, pm->data.svc_print_data.type, 1);
			offset += 1;
			memcpy(buffer+offset, pm->data.svc_print_data.string, pm->size -2);
			offset += pm->size-3;
			memset(buffer+offset, 0, 1);
			break;
	}
}

unsigned char *PacketMessageList_CreatePacket(int *packet_size, unsigned int os, unsigned int is, struct packet_message_list *pml)
{
	int size;
	int offset;
	struct packet_message *pm;
	unsigned char *packet;

	size = 2* sizeof(int);

	pm = pml->first;
	while(pm)
	{
		PacketMessage_SetSize(pm);
		size += pm->size;
		pm = pm->next;
	}

	if (packet_size)
		*packet_size = size;

	packet = malloc(size);
	if (packet == NULL)
		return NULL;

	offset = 0;
	memset(packet+offset, (int)os, sizeof(int));
	offset += sizeof(int);
	memset(packet+offset, (int)is, sizeof(int));
	offset += sizeof(int);
	pm = pml->first;
	while (pm)
	{
		PacketMessage_ToBuffer(packet + offset, pm);
		offset += pm->size;
		pm = pm->next;
	}
	return packet;
}

int Packet_ReadByte(struct packet *packet)
{
	int offset;

	if (packet->offset + 1 > packet->length)
	{
		packet->error = true;
		return -1;
	}

	offset = packet->offset;
	packet->offset += sizeof(char);
	return packet->data.data.data[offset];
}

int Packet_ReadShort(struct packet *packet)
{
	int c;

	if (packet->offset + 2 > packet->length)
	{
		packet->error = true;
		return -1;
	}

	c = (short)(packet->data.data.data[packet->offset] + 
			(packet->data.data.data[packet->offset + 1] << 8));
	packet->offset += 2;

	return c;
}

int Packet_ReadLong(struct packet *packet)
{
	int c;

	if (packet->offset + 4 > packet->length)
	{
		packet->error = true;
		return -1;
	}

	c = (short)(packet->data.data.data[packet->offset] + 
			(packet->data.data.data[packet->offset + 1] << 8) +
			(packet->data.data.data[packet->offset + 2] << 16) +
			(packet->data.data.data[packet->offset + 3] << 24));
	packet->offset += 4;

	return c;
}

float Packet_ReadFloat(struct packet *packet)
{
	union
	{
		unsigned char b[4];
		float f;
		int l;
	} dat;

	if (packet->offset + 4 > packet->length)
	{
		packet->error = true;
		return -1;
	}

	dat.b[0] = packet->data.data.data[packet->offset];
	dat.b[1] = packet->data.data.data[packet->offset +1];
	dat.b[2] = packet->data.data.data[packet->offset +2];
	dat.b[3] = packet->data.data.data[packet->offset +3];

	packet->offset += 4;

	dat.l = LittleLong(dat.l);
	return dat.f;
}

float Packet_ReadAngle16(struct packet *packet)
{
	return Packet_ReadShort(packet) * (360.0/65536);
}

void Packet_ReadDeltaUsercmd(struct packet *packet, struct usercmd *ucmd)
{
	int bits;

	bits = Packet_ReadByte(packet);
	
	if (bits & CM_ANGLE1)
		ucmd->angles[0] = Packet_ReadAngle16(packet);
	if (bits & CM_ANGLE2)
		ucmd->angles[1] = Packet_ReadAngle16(packet);
	if (bits & CM_ANGLE3)
		ucmd->angles[2] = Packet_ReadAngle16(packet);

	if (bits & CM_FORWARD)
		ucmd->forwardmove = Packet_ReadShort(packet);
	if (bits & CM_SIDE)
		ucmd->sidemove = Packet_ReadShort(packet);
	if (bits & CM_UP)
		ucmd->upmove = Packet_ReadShort(packet);

	ucmd->msec = Packet_ReadByte(packet);
}

void Packet_WriteDeltaUsercmd(struct buffer *packet, struct usercmd *from, struct usercmd *cmd)
{
	int bits;

	bits = 0;
	if (cmd->angles[0] != from->angles[0])
		bits |= CM_ANGLE1;
	if (cmd->angles[1] != from->angles[1])
		bits |= CM_ANGLE2;
	if (cmd->angles[2] != from->angles[2])
		bits |= CM_ANGLE3;
	if (cmd->forwardmove != from->forwardmove)
		bits |= CM_FORWARD;
	if (cmd->sidemove != from->sidemove)
		bits |= CM_SIDE;
	if (cmd->upmove != from->upmove)
		bits |= CM_UP;
	if (cmd->buttons != from->buttons)
		bits |= CM_BUTTONS;
	if (cmd->impulse != from->impulse)
		bits |= CM_IMPULSE;

	Packet_WriteToBuffer(packet, "c", bits);

	if (bits & CM_ANGLE1)
		Packet_WriteToBuffer(packet, "A", cmd->angles[0]);
	if (bits & CM_ANGLE2)
		Packet_WriteToBuffer(packet, "A", cmd->angles[1]);
	if (bits & CM_ANGLE3)
		Packet_WriteToBuffer(packet, "A", cmd->angles[2]);
	if (bits & CM_FORWARD)
		Packet_WriteToBuffer(packet, "S", cmd->forwardmove);
	if (bits & CM_SIDE)
		Packet_WriteToBuffer(packet, "S", cmd->sidemove);
	if (bits & CM_UP)
		Packet_WriteToBuffer(packet, "S", cmd->upmove);
	if (bits & CM_BUTTONS)
		Packet_WriteToBuffer(packet, "c", cmd->buttons);
	if (bits & CM_IMPULSE)
		Packet_WriteToBuffer(packet, "c", cmd->impulse);

	Packet_WriteToBuffer(packet, "c", cmd->msec);

}

char *Packet_ReadString(struct packet *packet)
{
	int offset;
	offset = packet->offset;
	packet->offset += strlen((char *)packet->data.data.data + packet->offset) + 1;

	return (char *)packet->data.data.data + offset;
}

/*
 * will get the size
 */
int Packet_GetSizeV(const char *format, va_list *argptr_in)
{
	int size;
	char *f;
	va_list argptr;
	char *sval;
	struct buffer *buf;

	f = (char *)format;
	size = 0;

	va_copy(argptr, *argptr_in);
	while (*f)
	{
		switch (*f)
		{
			case 'A':
			case 'c':
			case 'b':
				va_arg(argptr, int);
				size += sizeof(char);
				break;
			case 'B':
				buf = (struct buffer *)va_arg(argptr, int);
				printf("%p\n",buf);
				size += buf->position;
				break;
			case 'i':
				va_arg(argptr, int);
				size += sizeof(int);
				break;
			case 'C':
			case 'S':
				va_arg(argptr, short);
				size += sizeof(short);
				break;
			case 'f':
				va_arg(argptr, int);
				size += sizeof(float);
				break;
			case 'd':
				va_arg(argptr, int);
				size += sizeof(double);
				break;
			case 's':
				sval = va_arg(argptr, char *);
				size += strlen(sval) + 1;
				break;
			case 'z':
				sval = va_arg(argptr, char *);
				size += strlen(sval);
				break;
			default:
				Print_Console("invalid type \'%c\' in format_string\n", *f);
				va_end(argptr);
				return -1;
		}
		f++;
	}
	va_end(argptr);
	return size;
}

qboolean Packet_BufferCheckSize(struct buffer *buffer, int write_size)
{
	if (write_size >= BUFFER_SIZE - buffer->position)
		return false;
	return true;
}

/*
 * this function expects that there is enough room in the buffer
 */
qboolean Packet_WriteToBufferV(struct buffer *buffer, const char *format, va_list *argptr_in)
{
	int size;
	char *f;
	char *sval;
	char cval;
	unsigned char bval;
	double dval;
	float fval;
	int ival;
	va_list argptr;
	short short_val;
	struct buffer *buf;

	f = (char *)format;
	size = 0;

	va_copy(argptr, *argptr_in);
	while (*f)
	{
		switch (*f)
		{
			case 'A':
				short_val = (short)va_arg(argptr, int);
				cval = Q_rint(short_val);
				memcpy(buffer->data + buffer->position, (const void*)&cval, sizeof(char));
				buffer->position += sizeof(char);
				break;
			case 'c':
				cval = (char)va_arg(argptr, int);
				memcpy(buffer->data + buffer->position, (const void*)&cval, sizeof(char));
				buffer->position += sizeof(char);
				break;
			case 'b':
				bval = (unsigned char)va_arg(argptr, int);
				memcpy(buffer->data + buffer->position, (const void*)&bval, sizeof(char));
				buffer->position += sizeof(char);
				break;
			case 'B':
				buf = (struct buffer *)va_arg(argptr, int);
				memcpy(buffer->data + buffer->position, (const void *)buf->data, buf->position);
				buffer->position += buf->position;
				break;
			case 'i':
				ival = (int)va_arg(argptr, int);
				memcpy(buffer->data + buffer->position, (const void*)&ival, sizeof(int));
				buffer->position += 4;
				break;
			case 'S':
				short_val = (short)va_arg(argptr, short);
				memcpy(buffer->data + buffer->position, (const void*)&short_val, sizeof(short));
				buffer->position += sizeof(short);
				break;
			case 'C':
				short_val = (int)va_arg(argptr, short) * 8;
				memcpy(buffer->data + buffer->position, (const void*)&short_val, sizeof(short));
				buffer->position += sizeof(short);
				break;

			case 'f':
				fval = (float)va_arg(argptr, int);
				memcpy(buffer->data + buffer->position, (const void*)&fval, sizeof(float));
				buffer->position += sizeof(float);
				break;
			case 'd':
				dval = va_arg(argptr, int);
				memcpy(buffer->data + buffer->position, (const void*)&dval, sizeof(double));
				buffer->position += sizeof(double);
				break;
			case 's':
				sval = (char *)va_arg(argptr, int);
				memcpy(buffer->data + buffer->position, (const void *)sval, strlen(sval));
				buffer->position += strlen(sval);
				buffer->data[buffer->position] = '\0';
				buffer->position++;
				break;
			case 'z':
				sval = (char *)va_arg(argptr, int);
				memcpy(buffer->data + buffer->position, (const void *)sval, strlen(sval));
				buffer->position += strlen(sval);
				break;
			default:
				Print_Console("invalid type \'%c\' in format_string\n", *f);
				va_end(argptr);
				return false;
		}
		f++;
	}
	va_end(argptr);
	return true;
}

qboolean Packet_WriteToBuffer(struct buffer *buffer, const char *format, ...)
{
	qboolean r;
	va_list argptr;
	int write_size;

	r = false;
	va_start(argptr, format);
	write_size = Packet_GetSizeV(format, &argptr);
	va_end(argptr);
	if (Packet_BufferCheckSize(buffer, write_size))
	{
		va_start(argptr, format);
		r = Packet_WriteToBufferV(buffer, format, &argptr);
		va_end(argptr);
	}
	va_end(argptr);

	return r;
}

qboolean Client_SwitchReliable(struct client *client)
{
	if (client->backbuffer_count >= MAX_BACKBUFFERS)
		return false;
	client->backbuffer_count++;
	client->reliable_buffer = &client->backbuffer[client->backbuffer_count];
	return true;
}

qboolean Client_RemoveBackbuffer(struct client *client)
{
	if (client->backbuffer_count == 0)
		return true;

	client->reliable_buffer->position = 0;
	client->backbuffer_count--;
	if (client->backbuffer_count == 0)
	{
		client->reliable_buffer = &client->message;
	}
	else
	{
		client->reliable_buffer = &client->backbuffer[client->backbuffer_count];
	}
	return true;
}

qboolean Client_CheckReliable(struct client *client, int write_size)
{
	if (client->reliable_buffer == NULL)
		client->reliable_buffer = &client->message;
	if (Packet_BufferCheckSize(client->reliable_buffer, write_size) == false)
		return Client_SwitchReliable(client);
	return true;
}

qboolean Client_WriteReliableV(struct client *client, const char *format, va_list *argptr_in)
{
	qboolean r;
	int write_size;

	write_size = Packet_GetSizeV(format, argptr_in);

	if (write_size == -1)
		return false;

	if ((r = Client_CheckReliable(client, write_size)))
	{
		r = Packet_WriteToBufferV(client->reliable_buffer, format, argptr_in);
	}
	return r;

}

qboolean Client_WriteReliable(struct client *client, const char *format, ...)
{
	qboolean r;
	va_list argptr;

	va_start(argptr, format);
	r = Client_WriteReliableV(client, format, &argptr);
	va_end(argptr);

	return r;
}

qboolean Client_Write(struct client *client, const char *format, ...)
{
	qboolean r;
	va_list argptr;
	va_start(argptr, format);
	r = Packet_WriteToBufferV(&client->message, format, &argptr);
	va_end(argptr);
	return r;
}

qboolean Client_WriteV(struct client *client, const char *format, va_list *argptr)
{
	return Packet_WriteToBufferV(&client->message, format, argptr);
}

qboolean Client_WriteBuffer(struct client *client, struct buffer *buffer, qboolean failonsize)
{
	return Buffer_Copy(&client->message, buffer, failonsize);
}

qboolean Buffer_Copy(struct buffer *to, struct buffer *from, qboolean failonsize)
{
	int size;
	if (from->position == 0)
		return true;

	if (failonsize)
		if (from->position > BUFFER_SIZE - to->position)
			return false;


	size = BUFFER_SIZE - to->position > from->position ? from->position : BUFFER_SIZE - to->position;
	memcpy(to->data +to->position , from->data, size);
	to->position += size;
	return true;
}

void SendReliable_ToAll (struct server *server , const char *format, ...)
{
	int i;
	va_list argptr;

	for (i=0;i<MAX_CLIENTS;i++)
	{
		if (server->clients[i].inuse == false)
			continue;

		va_start(argptr, format);
		Client_WriteReliableV(&server->clients[i], format, &argptr);
		va_end(argptr);
	}
}

void Send_ToAll (struct server *server , const char *format, ...)
{
	int i;
	va_list argptr;

	for (i=0;i<MAX_CLIENTS;i++)
	{
		if (server->clients[i].inuse == false)
			continue;

		va_start(argptr, format);
		Client_WriteV(&server->clients[i], format, &argptr);
		va_end(argptr);
	}
}
