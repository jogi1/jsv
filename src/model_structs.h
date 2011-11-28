#ifndef MODEL_STRUCTS_H
#define MODEL_STRUCTS_H

#define	LUMP_ENTITIES	0
#define	LUMP_PLANES	1
#define	LUMP_TEXTURES	2
#define	LUMP_VERTEXES	3
#define	LUMP_VISIBILITY	4
#define	LUMP_NODES	5
#define	LUMP_TEXINFO	6
#define	LUMP_FACES	7
#define	LUMP_LIGHTING	8
#define	LUMP_CLIPNODES	9
#define	LUMP_LEAFS	10
#define	LUMP_MARKSURFACES 11
#define	LUMP_EDGES	12
#define	LUMP_SURFEDGES	13
#define	LUMP_MODELS	14
#define	HEADER_LUMPS	15

#define Q1_BSPVERSION 29

#define AMBIENT_WATER 0
#define AMBIENT_SKY 1
#define AMBIENT_SLIME 2
#define AMBIENT_LAVA 3

#define NUM_AMBIENTS 4


#define	MAX_MAP_HULLS		4

#define	MAX_MAP_MODELS		256
#define	MAX_MAP_BRUSHES		4096
#define	MAX_MAP_ENTITIES	1024
#define	MAX_MAP_ENTSTRING	65536

#define	MAX_MAP_PLANES		8192
#define	MAX_MAP_NODES		32767	// because negative shorts are contents
#define	MAX_MAP_CLIPNODES	32767	//
#define	MAX_MAP_LEAFS		32767	//
#define	MAX_MAP_VERTS		65535
#define	MAX_MAP_FACES		65535
#define	MAX_MAP_MARKSURFACES	65535
#define	MAX_MAP_TEXINFO		4096
#define	MAX_MAP_EDGES		256000
#define	MAX_MAP_SURFEDGES	512000
#define	MAX_MAP_MIPTEX		0x200000
#define	MAX_MAP_LIGHTING	0x100000
#define	MAX_MAP_VISIBILITY	0x100000

#define	CONTENTS_EMPTY	-1
#define	CONTENTS_SOLID	-2
#define	CONTENTS_WATER	-3
#define	CONTENTS_SLIME	-4
#define	CONTENTS_LAVA	-5
#define	CONTENTS_SKY	-6

struct lump
{
	int fileofs, filelen;
};

struct plane
{
	vec3_t normal;
	float dist;
	unsigned char type;
	unsigned char signbits;
	unsigned char pad[2];
};

struct dplane
{
	float normal[3];
	float dist;
	int type;
};

struct node
{
	int contents;
	struct node *parent;

	struct plane *plane;
	struct node *children[2];
};

struct dnode
{
	int		planenum;
	short		children[2];	// negative numbers are -(leafs+1), not nodes
	short		mins[3];	// for sphere culling
	short		maxs[3];
	unsigned short	firstface;
	unsigned short	numfaces;	// counting both sides
};

struct leaf
{
	int contents;
	struct node *parent;

	unsigned char ambient_sound_level[NUM_AMBIENTS];
};

struct dleaf
{
		int		contents;
	int		visofs;		// -1 = no visibility info

	short		mins[3];	// for frustum culling
	short		maxs[3];

	unsigned short	firstmarksurface;
	unsigned short	nummarksurfaces;

	unsigned char ambient_level[NUM_AMBIENTS];
};

struct clipnode
{
	int planenum;
	short children[2];
};

struct dsubmodel
{
	float mins[3], maxs[3], origins[3];
	int headnode[MAX_MAP_HULLS];
	int visleafs;
	int firstface, numfaces;
};

struct hull
{
	struct clipnode *clipnodes;
	struct plane *planes;
	int firstclipnode, lastclipnode;
	vec3_t clip_mins, clip_maxs;
};

struct submodel
{
	vec3_t mins, maxs, origins;
	struct hull hulls[MAX_MAP_HULLS];
};

struct model
{
	char *name;
	unsigned int checksum, checksum2;
	int version;
	struct mplane *planes;
	int planes_count;
};

struct model_header
{
	int version;
	struct lump lumps[HEADER_LUMPS];
};

struct map
{
	char *name;

	struct model_header *header;
	unsigned char *data;
	unsigned char *mod_base;

	unsigned int checksum, checksum2;
	int version;
	char *entity_string;
	int visleafs;
	struct plane *planes;
	int planes_count;
	struct leaf *leafs;
	int leafs_count;
	struct node *nodes;
	int nodes_count;
	struct clipnode *clipnodes;
	int clipnodes_count;
	struct submodel submodels[MAX_MAP_MODELS];
	int submodels_count;
	int vis_rowbytes, vis_rowlongs;
	unsigned char *pvs;
	unsigned char *phs;
	unsigned char novis[MAX_MAP_LEAFS/8];

	vec3_t mins, max;
};

struct pvs
{
	int bytes;
	unsigned char pvs[MAX_MAP_LEAFS/8];
	vec3_t origin;
};

#endif
