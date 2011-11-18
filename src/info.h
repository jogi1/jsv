#ifndef INFO_H
#define INFO_H

#define IF_ALL			(1<<0)	// copy everything
#define IF_CLIENT		(1<<1)	// copy only information relevant to other clients
#define IF_SERVER		(1<<2)	// copy only information relevant to the server


void Info_Clean(struct info *info);
struct info *Info_FromString(char *string);
qboolean Info_SetValue(struct info *info, char *key, char *value);
qboolean Info_CopyToBuffer(struct info *info, struct buffer *buffer, int flags);
char *Info_GetValue(struct info *info, char *key);

#endif
