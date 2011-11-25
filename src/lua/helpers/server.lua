
-- not sure about this
function server.precache_model (self, model, add)
	return self.__precache_model(self.__pointer, model, add)
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
