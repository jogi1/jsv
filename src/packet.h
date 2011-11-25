#ifndef PACKETS_H
#define PACKETS_H
qboolean Packet_ReadMessageType(struct packet *packet);
unsigned char *Packet_Create(int *len, const char *format, ...);
int Packet_ReadByte(struct packet *packet);
int Packet_ReadShort(struct packet *packet);
int Packet_ReadLong(struct packet *packet);
float Packet_ReadFloat(struct packet *packet);
float Packet_ReadAngle16(struct packet *packet);
void Packet_ReadDeltaUsercmd(struct packet *packet, struct usercmd *ucmd);
char *Packet_ReadString(struct packet *packet);
struct packet_message *PacketMessageList_Add(struct packet_message_list *pml, int type);
unsigned char *PacketMessageList_CreatePacket(int *packet_size, unsigned int os, unsigned int is, struct packet_message_list *pml);
qboolean Packet_WriteToBuffer(struct buffer *buffer, const char *format, ...);
qboolean Buffer_Copy(struct buffer *to, struct buffer *from, qboolean failonsize);
qboolean Client_WriteReliable(struct client *client, const char *format, ...);
qboolean Client_Write(struct client *client, const char *format, ...);
qboolean Client_WriteBuffer(struct client *client, struct buffer *buffer, qboolean failonsize);
qboolean Client_RemoveBackbuffer(struct client *client);
void Send_ToAll (struct server *server, const char *format, ...);
void SendReliable_ToAll (struct server *server, const char *format, ...);
#endif
