#include <stdlib.h>
#include <string.h>
#include "server.h"

void Model_Free(struct model *model)
{
	if (!model)
		return;
	if (model->name)
		free(model->name);
	free(model);
}

void Model_MapFree(struct map *map)
{
	if (!map)
		return;
	free(map->name);
	free(map->data);
	free(map->planes);
	free(map->leafs);
	free(map->nodes);
	free(map->clipnodes);

	if (map->submodels_count)
		free(map->submodels[0].hulls[0].clipnodes);
	free(map->submodels);
	free(map->pvs);
	free(map->phs);
	free(map->entity_string);
	free(map);
}

struct plane *Model_LoadPlanes(struct lump *lump, unsigned char *base, int *planes_count)
{
	int i, j, count, bits;
	struct plane *mp, *mpp;
	struct dplane *dp;

	if (lump->filelen % sizeof(*dp))
	{
		*planes_count = -1;
		return NULL;
	}

	dp = (struct dplane *)(base + lump->fileofs);

	count = lump->filelen % sizeof(*dp);

	mp = calloc(count, sizeof(*mp));
	if (mp == NULL)
	{
		*planes_count = -1;
		return NULL;
	}
	mpp = mp;

	*planes_count = count;

	for (i=0; i<count; i++, mpp++, dp++)
	{
		bits = 0;
		for (j=0; j<3; j++)
		{
			mpp->normal[j] = LittleFloat(dp->normal[j]);
			if (mpp->normal[j] > 0)
				bits |= 1 << j;
		}

		mpp->dist = LittleFloat(dp->dist);
		mpp->type = LittleLong(dp->type);
		mpp->signbits = bits;
	}

	return mpp;
}

struct leaf *Model_LoadLeafs(struct lump *lump, unsigned char *base, int *leafs_count)
{
	struct dleaf *in;
	struct leaf *out, *rout;
	int i, j, count, p;

	in = (struct dleaf *)(base + lump->fileofs);

	if (lump->filelen % sizeof(*in))
	{
		*leafs_count = -1;
		return NULL;
	}

	*leafs_count = count;

	count = lump->filelen / sizeof(*in);
	out = (struct leaf *) calloc(count, sizeof(*out));

	rout = out;


	for (i = 0; i < count; i++, in++, out++) {
		p = LittleLong(in->contents);
		out->contents = p;
		for (j = 0; j < 4; j++)
			out->ambient_sound_level[j] = in->ambient_level[j];
	}

	return rout;
}

static void Model_SetNodeParent (struct node *node, struct node *parent)
{
	node->parent = parent;
	if (node->contents < 0)
		return;
	Model_SetNodeParent(node->children[0], node);
	Model_SetNodeParent(node->children[1], node);
}

struct node *Model_LoadNodes(struct lump *lump, unsigned char *base, int *nodes_count, struct plane *planes, struct leaf *leafs)
{
	int i, j, count, p;
	struct node *out, *rout;
	struct dnode *in;

	in = (struct dnode *)(base + lump->fileofs);

	if (lump->filelen % sizeof(*in))
	{
		*nodes_count = 0;
		return NULL;
	}

	count = lump->filelen / sizeof(*in);
	out = (struct node *) calloc(count * sizeof(*out), sizeof(char));

	if (out == NULL)
	{
		*nodes_count = 0;
		return NULL;
	}

	rout = out;

	for (i=0; i<count; i++, in++, out++)
	{
		p = LittleLong(in->planenum);
		out->plane = planes + p;

		for (j=0; j<2; j++)
		{
			p = LittleShort(in->children[j]);
			out->children[j] = (p >= 0) ? (rout + p) : ((struct node *)(leafs + (-1 - p)));
		}
	}
	Model_SetNodeParent(rout, NULL);
	*nodes_count = count;
	return rout;
}

struct clipnode *Model_LoadClipnodes(struct lump *lump, unsigned char *base, int *nodes_count)
{
	struct clipnode *in, *out, *rout;
	int i, count;

	if (lump->filelen % sizeof(*in))
	{
		*nodes_count = 0;
		return NULL;
	}

	in = (struct clipnode *)(base + lump->fileofs);

	count = lump->filelen / sizeof(*in);
	out = (struct clipnode *) calloc(count, sizeof(*in));
	rout = out;

	for (i=0; i<count; i++, out++, in++)
	{
		out->planenum = LittleLong(in->planenum);
		out->children[0] = LittleShort(in->children[0]);
		out->children[1] = LittleShort(in->children[1]);
	}

	return rout;
}

unsigned char *LEAF_PVS(struct map *map, struct leaf *leaf)
{
	if (leaf == map->leafs)
		return map->novis;

	return map->phs + (leaf - 1 - map->leafs) * map->vis_rowbytes;
}

static void PVS_Add(struct map *map, struct pvs *pvs, struct node *node)
{
	int i;
	float d;
	struct plane *plane;
	unsigned char *lpvs;

	while(1)
	{
		if (node->contents < 0)
		{
			if (node->contents != CONTENTS_SOLID)
			{
				lpvs = LEAF_PVS(map, (struct leaf *)node);
				for (i=0; i<pvs->bytes; i++)
					pvs->pvs [i] |= lpvs[i];
			}
			return;
		}

		plane = node->plane;
		d = Vector_DotProduct(pvs->origin, plane->normal) - plane->dist;
		if (d>8)
			node = node->children[0];
		else if (d<-8)
			node = node->children[1];
		else
		{
			PVS_Add(map, pvs, node->children[0]);
			node = node->children[1];
		}
	}
}

qboolean Model_PVSFromOrigin(struct map *map, struct pvs *pvs, vec3_t *origin)
{
	if (!map || !pvs || !origin)
		return false;

	memset(pvs, 0, sizeof(*pvs));
	pvs->bytes = (map->visleafs + 31) >> 3;
	memcpy(&pvs->origin, origin, sizeof(*origin));
	PVS_Add(map, pvs, map->nodes);
	return true;
}

static qboolean Model_MapCreateHulls(struct map *map)
{
	struct node *in, *child;
	struct clipnode *out;
	int i, j, count;

	in = map->nodes;
	count = map->nodes_count;

	out = (struct clipnode *) calloc(count, sizeof(*out));
	if (out == NULL)
		return false;

	for (i=0; i<map->submodels_count; i++)
	{
		map->submodels[i].hulls[0].clipnodes = out;
		map->submodels[i].hulls[0].lastclipnode = count - 1;
	}

	for (i=0; i<count; i++, out++, in++)
	{
		out->planenum = in->plane - map->planes;
		for (j=0; j<2; j++)
		{
			child = in->children[j];
			out->children[j] = (child->contents < 0) ? (child->contents) : (child - map->nodes);
		}
	}
	return true;
}

/*
** DecompressVis
*/
static unsigned char *DecompressVis (struct map *map, unsigned char *in)
{
	static unsigned char decompressed[MAX_MAP_LEAFS/8];
	int c, row;
	unsigned char *out;

	row = (map->visleafs + 7) >> 3;
	out = decompressed;

	if (!in)
	{ // no vis info, so make all visible
		while (row)
		{
			*out++ = 0xff;
			row--;
		}
		return decompressed;
	}

	do
	{
		if (*in)
		{
			*out++ = *in++;
			continue;
		}
	
		c = in[1];
		in += 2;
		while (c)
		{
			*out++ = 0;
			c--;
		}
	} while (out - decompressed < row);
	
	return decompressed;
}

static qboolean Model_MapCreatePVS(struct map *map)
{
	unsigned char *visdata, *scan;
	struct dleaf *in;
	int p, i;

	map->vis_rowlongs = (map->visleafs + 31) >> 5;
	map->vis_rowbytes = map->vis_rowlongs * 4;
	map->pvs = (unsigned char *) calloc(map->vis_rowbytes * map->visleafs, sizeof(unsigned char));
	if (map->pvs == NULL)
		return false;

	if (map->header->lumps[LUMP_VISIBILITY].filelen == 0)
		return false;

	visdata = map->mod_base + map->header->lumps[LUMP_VISIBILITY].fileofs;

	in = (struct dleaf *)(map->mod_base + map->header->lumps[LUMP_LEAFS].fileofs);
	in++;
	scan = map->pvs;

	for (i=0; i<map->visleafs; i++, in++, scan += map->vis_rowbytes)
	{
		p = LittleLong(in->visofs);
		memcpy(scan, (p == -1) ? map->novis : DecompressVis (map, visdata + p), map->vis_rowbytes);
	}

	return true;
}

static qboolean Model_MapCreatePHS(struct map *map)
{
	int i, j, k, l, index1, bitbyte;
	unsigned *dest, *src;
	unsigned char *scan;

	map->phs = (unsigned char *) calloc(map->vis_rowbytes * map->visleafs, sizeof(char));
	if (map->phs == 0)
		return false;
	scan = map->phs;
	dest = (unsigned *)map->phs;

	for (i=0; i<map->visleafs; i++, dest += map->vis_rowlongs, scan += map->vis_rowbytes)
	{
//		memcpy(dest, scan, map->vis_rowbytes);
		memmove(dest, scan, map->vis_rowbytes);

		for (j=0; j<map->vis_rowbytes; j++)
		{
			bitbyte = scan[j];
			if (!bitbyte)
				continue;
			for (k=0; k<8; k++)
			{
				if (!(bitbyte & (1<<k)))
					continue;
				index1 = (j<<3) + k;
				if (index1 >= map->visleafs)
					continue;
				src = (unsigned *)map->pvs + index1 * map->vis_rowlongs;
				for (l=0; l<map->vis_rowlongs; l++)
					dest[l] |= src[l];
			}
		}
	}
	return true;
}

#warning revert this back
qboolean Model_LoadSubmodels(struct map *map)
{
	struct submodel *out, *rout;
	struct dsubmodel *in;
	int i, j, count;

	if (map->header->lumps[LUMP_MODELS].filelen % sizeof(*in))
		return false;

	in = (struct dsubmodel *)(map->mod_base + map->header->lumps[LUMP_MODELS].fileofs);

	map->submodels_count = map->header->lumps[LUMP_MODELS].filelen / sizeof(*in);

	if (map->submodels_count< 1)
		return false;

	if (map->submodels_count > MAX_MAP_MODELS)
		return false;

	out = map->submodels;
	rout = out;

	map->visleafs = LittleLong(in[0].visleafs);

	for (i=0; i<map->submodels_count ; i++, in++, out++)
	{
		for (j=0; j<3; j++)
		{
			out->mins[j] = LittleFloat(in->mins[j]) - 1;
			out->maxs[j] = LittleFloat(in->maxs[j]) + 1;
			out->origins[j] = LittleFloat(in->origins[j]);
		}

		for (j=0; j<MAX_MAP_HULLS; j++)
		{
			out->hulls[j].planes = map->planes;
			out->hulls[j].clipnodes = map->clipnodes;
			out->hulls[j].firstclipnode = LittleLong(in->headnode[j]);
			out->hulls[j].lastclipnode = map->clipnodes_count - 1;
		}

		Vector_Clear(out->hulls[0].clip_mins);
		Vector_Clear(out->hulls[0].clip_maxs);

		Vector_Set(out->hulls[1].clip_mins, -16, -16, -24);
		Vector_Set(out->hulls[1].clip_maxs, 16, 16, 32);

		Vector_Set(out->hulls[2].clip_mins, -32, -32, -24);
		Vector_Set(out->hulls[2].clip_maxs, 32, 32, 64);
	}
	return true;
}

void Model_MapCleanup(struct map *map)
{
	free(map->name);
	free(map->data);
	free(map->entity_string);
	free(map->planes);
	free(map->leafs);
	free(map->nodes);
	free(map->submodels);
	free(map->pvs);
	free(map->phs);
	free(map);
}

struct map *Model_MapLoad(struct server *server, char *filename)
{
	int size;
	struct map *map;
	char buffer[1024];
	char *file;
	int i;

	map = calloc(1, sizeof(*map));
	if (!map)
	{
		return NULL;
	}

	if (server->data_dir)
	{
		snprintf(buffer, sizeof(buffer), "%s/%s", server->data_dir, filename);
		file = buffer;
	}
	else
	{
		file = filename;
	}

	file = File_Read(file, &size);
	if (file == NULL)
	{
		return NULL;
	}

	map->data = (unsigned char *)file;

	map->header = (struct model_header *)file;

	map->mod_base = (unsigned char *)map->header;

	map->version = LittleLong(map->header->version);
	printf("%s - version: %i\n", filename, map->version);

	for (i=0;i<sizeof(struct model_header)/4;i++)
		((int *)map->header)[i] = LittleLong(((int *)map->header)[i]);

	for (i=0; i<HEADER_LUMPS; i++)
	{
		if (i == LUMP_ENTITIES)
			continue;

		map->checksum ^= LittleLong(MD4_BlockChecksum((void *)map->mod_base + map->header->lumps[i].fileofs, map->header->lumps[i].filelen));

		if (i == LUMP_VISIBILITY || i == LUMP_LEAFS || i == LUMP_NODES)
			continue;

		map->checksum2 ^= LittleLong(MD4_BlockChecksum((void *)map->mod_base + map->header->lumps[i].fileofs, map->header->lumps[i].filelen));
	}

	memset(map->novis, 0xff, sizeof(map->novis));

	map->planes = Model_LoadPlanes(&map->header->lumps[LUMP_PLANES], map->mod_base, &map->planes_count);
	if (map->planes)
	{
		map->entity_string = strdup((char *)map->mod_base + map->header->lumps[LUMP_ENTITIES].fileofs);
		if (map->entity_string)
		{
			map->leafs = Model_LoadLeafs(&map->header->lumps[LUMP_LEAFS], map->mod_base, &map->leafs_count);
			if (map->leafs)
			{
				map->nodes = Model_LoadNodes(&map->header->lumps[LUMP_NODES], map->mod_base, &map->nodes_count, map->planes, map->leafs);
				if (map->nodes)
				{
					map->clipnodes = Model_LoadClipnodes(&map->header->lumps[LUMP_CLIPNODES], map->mod_base, &map->clipnodes_count);
					if (map->clipnodes)
					{
						if (Model_LoadSubmodels(map))
						{
							if (Model_MapCreateHulls(map))
							{
								if (Model_MapCreatePVS(map))
								{
									if (Model_MapCreatePHS(map))
									{
										free(map->data);
										map->data = NULL;
										return map;
									}
									else
										printf("Model_MapCreatePHS failed.\n");
								}
								else
									printf("Model_MapCreatePVS failed.\n");
							}
						}
						else
							printf("Model_LoadSubmodels failed. %i\n", map->submodels_count);
					}
					else
						printf("Model_LoadClipnodes failed.\n");
				}
			}
		}
	}
	Model_MapCleanup(map);
	return NULL;
}
