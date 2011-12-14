#ifndef TOOLS_H
#define TOOLS_H
#define PRINT_VEC(x) printf("%f %f %f\n", x[0], x[1], x[2]);
short	ShortSwap (short s);
int		LongSwap (int l);
float	FloatSwap (float f);
void Print_Console(const char *format, ...);
qboolean String_Compare(char *s1, char *s2);
qboolean String_CompareLength(char *s1, char *s2, int *len);
qboolean String_CheckOnly(char *s1, const char *s2);
char *File_Read(char *name, int *length);
double Tools_DoubleTime(struct server *server);
void Print_ToAll(struct server *server, qboolean reliable, int level,  char *format, ...);
void Print_ToClient(struct client *client, qboolean reliable, int level, char *format, ...);
void Print_ToClientOOB(struct server *server, struct client *client, qboolean flush, char *format, ...);
#endif
