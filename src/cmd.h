#ifndef CMD_H
#define CMD_H
void CMD_Say(struct server *server, struct client *client, struct tokenized_string *ts);
void CMD_New(struct server *server, struct client *client, struct tokenized_string *ts);
void CMD_Soundlist(struct server *server, struct client *client, struct tokenized_string *ts);
void CMD_Modellist(struct server *server, struct client *client, struct tokenized_string *ts);
void CMD_Prespawn(struct server *server, struct client *client, struct tokenized_string *ts);
void CMD_Spawn(struct server *server, struct client *client, struct tokenized_string *ts);
void CMD_Begin(struct server *server, struct client *client, struct tokenized_string *ts);
void CMD_Drop(struct server *server, struct client *client, struct tokenized_string *ts);
void CMD_Setinfo(struct server *server, struct client *client, struct tokenized_string *ts);
void CMD_Download(struct server *server, struct client *client, struct tokenized_string *ts);
#endif
