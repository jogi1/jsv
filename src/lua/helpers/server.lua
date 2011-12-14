
-- not sure about this
function server.precache_model (self, model, add)
	rval = self.__precache_model(self.__pointer, model, add)
	if (rval == nil) then
		print (model);
		print "rval is nil";
		return nil;
	end
	rval.name = model;
	self.precached_models[model] = rval;
	return rval;
end

function server.precache_sound (self, sound)
	return self.__precache_sound(self.__pointer, sound)
end

function server.set_map_name (self, map)
	self.__set_map_name(self.__pointer, map)
end

function server.add_lightstyle (self, lightstyle)
	self.__add_lightstyle (self.__pointer, lightstyle)
end

function server.get_edict_for_inline_model (self, model)
	return self.__get_edict_for_inline_model(self.__pointer, model)
end

function server.print_to_client(self, client, str)
	self.__print_to_client(self.__pointer, client, str)
end

function server.trace_edict(self, e, start, stop, t_type, passedict)
	return self.__trace_edict(self.__pointer, e, start.x, start.y, start.z, stop.x, stop.y, stop.z, t_type, passedict);
end

