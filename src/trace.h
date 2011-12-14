struct trace *Trace_HullTrace(struct trace *trace, struct hull *hull, vec3_t start, vec3_t stop);
struct trace *Trace_Trace(struct server *server, struct trace *trace_in, vec3_t mins, vec3_t maxs, vec3_t start, vec3_t stop, int type, struct edict *pass_edict);
