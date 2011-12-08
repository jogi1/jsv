trains = {};

function trains.setup()
	local key, value, location, v, k, e, n;
	for key, value in pairs(server.trains) do
		train = value;
		if (not train.speed) then
			train.speed = 100;
		end
		train.position = 1;
		train.path = {};
		train.traveled = 0;
		v = server:get_edict_for_inline_model(train.model);
		train.edict = v;
		train.wait = 0;
		if (not server.edicts[v]) then
			server.edicts[v] = {};
		end
		server.edicts[v].func = trains.handle;
		server.edicts[v].train = train;
		-- get first location
		location = get_path_corner(train.target);
		if (location == nil) then
			return
		end
		train.model = server:precache_model(train.model, false);

		train.path[#train.path + 1] = {};
		if (location.wait) then
			train.path[#train.path].wait = location.wait;
		end

		train.path[#train.path].origin = vector.subtract(location.origin, train.model.mins)
		size = vector.subtract(train.model.maxs, train.model.mins);

		train.path[#train.path].target = location.target;
		train.path[#train.path].targetname = location.targetname;

		local run = 1;

		while (run == 1) do
			location = get_path_corner(train.path[#train.path].target);
			if (location == nil) then
				return
			end
			train.path[#train.path + 1] = {};
			train.path[#train.path].origin = vector.subtract(location.origin, train.model.mins)
			train.path[#train.path].target = location.target;
			train.path[#train.path].targetname = location.targetname;
			if (location.target == train.path[1].targetname) then
				run = 0;
			end

			train.path[#train.path].wait = 0;
			if (location.wait) then
				train.path[#train.path].wait = location.wait;
			end
		end

		for k, v in pairs(train.path) do
			if (k == #train.path) then
				n = train.path[1];
			else
				n = train.path[k+1];
			end
			train.path[k].direction_vector = vector.subtract(n.origin, v.origin)
			train.path[k].distance = vector.length(train.path[k].direction_vector);
		end

		edict.set_origin(train.path[1].origin);
	end
end

function trains.handle(self, e)
	local k, v;
	local t = self.train;

	t.traveled = t.traveled + server.frametime * t.speed;
	if (t.traveled > t.path[t.position].distance) then
		t.position = t.position + 1;
		if (t.position > #t.path) then
			t.position = 1;
		end
		t.traveled = 0;
		t.wait = t.path[t.position].wait;
		if (t.wait == nil) then
			t.wait = 0;
		end
	end

	if (t.wait > 0) then
		t.wait = t.wait - server.frametime;
		edict.set_origin(e, t.path[t.position].origin);
	end

	v = vector.scale(t.path[t.position].direction_vector, t.traveled/t.path[t.position].distance);
	v = vector.add(v, t.path[t.position].origin);
	edict.set_origin(e, v);

end
