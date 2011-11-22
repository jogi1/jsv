
-- not sure about this
function server.precache_model (self, model)
	return self.__precache_model(self.__pointer, model)
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
