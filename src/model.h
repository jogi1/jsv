#ifndef MODEL_H
#define MODEL_H

struct map *Model_MapLoad(struct server *server, char *filename);
void Model_Free(struct model *model);
void Model_MapFree(struct map *map);

#endif
