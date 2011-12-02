--[[
items_types = {
	{classname = "item_armorInv", model = "progs/invulner.mdl"},
	{classname = "item_health", model = "maps/b_bh10.bsp"},
	{classname = "item_armor1", model = "progs/armor.mdl"},
	{classname = "item_armor2", model = "progs/armor.mdl"},
	{classname = "item_cells", model = "maps/b_batt1.bsp"},
	{classname = "item_rockets", model = "maps/b_rock0.bsp"},
	{classname = "item_spikes", model = "maps/b_nail0.bsp"},
	{classname = "weapon_lightning", model = "progs/g_light.mdl"},
	{classname = "weapon_grenadelauncher", model = "progs/g_rock.mdl"},
	{classname = "weapon_rocketlauncher", model = "progs/g_rock2.mdl"},
	{classname = "weapon_supernailgun", model = "progs/g_nail2.mdl"}};
	--]]
-- item types and corresponding models
-- so that models will be automatically be preloaded for entities

require "vector"

-- well, this really sucks :P
items_types = {
	item_health = {spawnflags = { "maps/b_bh10.bsp", "maps/b_bh25.bsp", "maps/b_bh100.bsp" }},
	item_armor1 = {model="progs/armor.mdl", skinnum=0},
	item_armor2 = {model="progs/armor.mdl", skinnum=1},
	item_armorInv = {model="progs/armor.mdl", skinnum=2},
	item_cells = {spawnflags = {"maps/b_batt0.bsp", "maps/b_batt1.bsp"}},
	item_shells= {spawnflags = {"maps/b_shell0.bsp", "maps/b_shell1.bsp"}},
	item_rockets = {spawnflags = {"maps/b_rock0.bsp", "maps/b_rock1.bsp"}},
	item_spikes = {spawnflags = {"maps/b_nail0.bsp", "maps/b_nail1.bsp"}},
	item_artifact_super_damage = {model="progs/quaddama.mdl"},
	item_artifact_invisibility = {model="progs/invisibl.mdl"},
	item_artifact_invulnerability = {model="progs/invulner.mdl"},
	weapon_lightning = {model="progs/g_light.mdl"},
	weapon_grenadelauncher = {model="progs/g_rock.mdl"},
	weapon_rocketlauncher = {model="progs/g_rock2.mdl"},
	weapon_supernailgun = {model="progs/g_nail2.mdl"},
	weapon_nailgun = {model="progs/g_nail.mdl"},
	weapon_supershotgun = {model="progs/g_shot.mdl"}
};

lightstyles = {
	"m",
	"mmnmmommommnonmmonqnmmo",
	"abcdefghijklmnopqrstuvwxyzyxwvutsrqponmlkjihgfedcba",
	"mmmmmaaaaammmmmaaaaaabcdefgabcdefg",
	"mamamamamama",
	"jklmnopqrstuvwxyzyxwvutsrqponmlkj",
	"nmonqnmomnmomomno",
	"mmmaaaabcdefgmmmmaaaammmaamm",
	"mmmaaammmaaammmabcdefaaaammmmabcdefmmmaaaa",
	"aaaaaaaazzzzzzzz",
	"mmamammmmammamamaaamammma",
	"abcdefghijklmnopqrrqponmlkjihgfedcba"
};

preload_models = {"progs/player.mdl"}
preload_sounds = {"player/plyrjmp8.wav", "player/h2ojump.wav"}
server.precached_models = {}
server.precached_sounds = {}
server.path_corner = {}
server.func_wall = {}
server.func_plat = {}
server.func_train = {}
server.info_null = {}
server.static_entities = {}
server.lights = {}
server.entities = {}
server.spawns = {}
server.trains = {}
server.intermission = {}
server.player_start = {}
server.doors = {}
server.buttons = {}
server.teleporter_destination = {}
server.teleporter_trigger = {}
server.trigger_push = {}
server.trigger_hurt = {}
server.trigger_multiple = {}
server.trigger_changelevel = {}
server.edicts = {}
server.spawn_model = "progs/player.mdl";
server.show_spawns = false;
server.frametime = 0;
server.realtime = 0;

function FUNC_map_start(server_ptr, client, ...)
end

function FUNC_entity_preload(server_ptr, client, ...)
	--preload models and sounds
	local key, value, k, v;
	for key, value in pairs(preload_models) do
		server:precache_model(value, true)
	end

	for key, value in pairs(preload_sounds) do
		server.precached_sounds[value] = server:precache_sound(value)
	end

	for key, value in pairs(lightstyles) do
		server:add_lightstyle(value);
	end
end

function entity_load (server_ptr, entity)
	local lent = {};
	local field = nil;
	local value = nil;
	local key = nil;

	for field, value in string.gmatch(entity, "\"([^\".]*)\" \"([^\".]*)\"") do
		if (field == 'classname') then
			lent.classname = value;
		elseif (field == 'origin') then
			lent.origin = vector.from_string(value);
		elseif (field == 'angles') then
			lent.original_angles = value;
			lent.angles = vector.from_string(value);
		elseif (field == 'angle') then
			lent.angles = vector.from_values(0, tonumber(value), 0);
		elseif (field == 'dmg') then
			lent.damage = value;
		else
			lent[field] = value;
		end
	end

	if (lent.classname == 'light') then
		server.lights[#server.lights + 1] = lent;
		return;
	end

	if (lent.classname == 'info_player_deathmatch') then
		server.spawns[#server.spawns + 1] = lent;
		return
	end

	if (lent.classname == 'info_intermission') then
		server.intermission = lent;
		return
	end

	if (lent.classname == 'info_teleport_destination') then
		server.teleporter_destination[#server.teleporter_destination + 1] = lent;
		return
	end

	if (lent.classname == 'trigger_teleport') then
		server.teleporter_trigger[#server.teleporter_trigger + 1] = lent;
		return
	end

	if (lent.classname == 'trigger_multiple') then
		server.trigger_multiple[#server.trigger_multiple + 1] = lent;
		return
	end

	if (lent.classname == 'trigger_push') then
		server.trigger_push[#server.trigger_push + 1] = lent;
		return
	end

	if (lent.classname == 'trigger_hurt') then
		server.trigger_hurt[#server.trigger_hurt + 1] = lent;
		return
	end

	if (lent.classname == 'target_multiple') then
		server.target_multiple[#server.trigger_multiple + 1] = lent;
		return
	end

	if (lent.classname == 'trigger_hurt') then
		server.target_multiple[#server.trigger_multiple + 1] = lent;
		return
	end

	if (lent.classname == 'trigger_changelevel') then
		server.trigger_changelevel[#server.trigger_changelevel + 1] = lent;
		return
	end

	if (lent.classname == 'func_door') then
		server.doors[#server.doors + 1] = lent;
		return
	end

	if (lent.classname == 'func_train') then
		server.trains[#server.trains + 1] = lent;
		return
	end

	if (lent.classname == 'info_null') then
		server.info_null[#server.info_null + 1] = lent;
		return
	end

	if (lent.classname == 'path_corner') then
		server.path_corner[#server.path_corner + 1] = lent;
		return
	end

	if (lent.classname == 'func_wall') then
		server.func_wall[#server.func_wall + 1] = lent;
		return
	end

	if (lent.classname == 'func_plat') then
		server.func_plat[#server.func_plat + 1] = lent;
		return
	end

	if (lent.classname == 'info_player_start') then
		server.player_start = lent;
		return
	end

	if (lent.classname == 'worldspawn') then
		server.map_name = lent.message;
		server:set_map_name(lent.message);
		return
	end

	-- ignore monesters
	if (string.sub(lent.classname, 1, 8) == "monster_") then
		return
	end

	-- ignore ambient
	if (string.sub(lent.classname, 1, 8) == "ambient_") then
		return
	end

	for key, value in pairs(items_types) do
		if (lent.classname == key) then
			server.entities[#server.entities +1 ] = lent;
			if (value.spawnflags) then
				local index = 1;
				if (lent.spawnflags) then
					index = index + lent.spawnflags;
				end
				server.entities[#server.entities]["model"] = value.spawnflags[index];
			else
				server.entities[#server.entities]["model"] = value.model;
				if (value.skinnum) then
					server.entities[#server.entities]["skinnum"] = value.skinnum;
				end
			end
			return
		end
	end

	print(entity);
	print("-------------------------------------------\n");
end

-- used to preload models read in by the map
function FUNC_entity_load_finished (server_ptr)
	local key, value;
	local v, e;
	for key, value in pairs(server.entities) do
		server:precache_model(value["model"], true)
		v = edict.get_unused(server_ptr);
		if (not server.edicts[v]) then
			server.edicts[v] = {};
		end
		server.edicts[v]["e_type"] = "static";
		server.edicts[v]["entity"] = value;
		server.edicts[v]["entities_index"] = key;
		server.edicts[v]["model_index"] = server.precached_models[value["model"]].index;
		--print ("test: " .. value["classname"] .. " - " .. server.edicts[v]["model_index"] .. " - " .. value["model"]);
		edict.set_modelindex(v, server.edicts[v]["model_index"]);
		if (value["origin"]) then
			--print (value.origin.x .. " " .. value.origin.y .. " " .. value.origin.z);
			edict.set_origin(v, value.origin);
		end
		if (value["angles"]) then
			edict.set_angles(v, value.angles);
		end

		if (value.skinnum) then
			edict.set_skinnum(v, value.skinnum);
		end

		-- this should always be last
		edict.set_baseline(v);
	end

	--[[
	-- set the models to 0 on triggers and do other trigger setup
	for key, value in pairs(server.teleporter_trigger) do
		e = server:get_edict_for_inline_model(value.model)
		edict.set_modelindex(e, 0)
		edict.set_baseline(e);
	end

	for key, value in pairs(server.trigger_push) do
		e = server:get_edict_for_inline_model(value.model)
		edict.set_modelindex(e, 0)
		edict.set_baseline(e);
	end

	for key, value in pairs(server.trigger_hurt) do
		e = server:get_edict_for_inline_model(value.model)
		edict.set_modelindex(e, 0)
		edict.set_baseline(e);
	end

	for key, value in pairs(server.doors) do
	end

	--]]

	server.spawn_model = server:precache_model(server.spawn_model, true);
	trains_setup();
end

function train_do(self, e)
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

function trains_setup ()
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
		server.edicts[v].func = train_do;
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

function get_path_corner(name)
	local key, value, k, v;

	for key, value in pairs(server.path_corner) do
		if (value.targetname == name) then
			rval = value;
			return rval;
		end
	end
	return nil;
end

function get_spawn()
	return server.spawns[1].origin;
end

function FUNC_print_info()
	print("entites info etc:");
	print("lights: " .. #server.lights);
	print("spawns: " .. #server.spawns);
	print("entities: " .. #server.entities);
	print("telporters: " .. #server.teleporter_trigger);
	print("teleporter_destination: " .. #server.teleporter_destination);
	print("doors: " .. #server.doors);
	print("trains: " .. #server.trains);
end

function FUNC_show_times (server_ptr, client)
	server:print_to_client(client, "realtime : " .. server.realtime .. "\n");
	server:print_to_client(client, "frametime: " .. server.frametime .. "\n");
end

function FUNC_print_var (server_ptr, client, first, ...)
	local test = _G[first];
	local k, v, k1, v1;

	if (test == nil) then
		server:print_to_client(client, first .. " is not defined\n");
		return;
	end

	for k, v in ipairs(arg) do
		if (v:sub(1,2) == "^i") then
			v = tonumber(v:sub(3));
		end
		test = test[v];
		if (test == nil) then
			server:print_to_client(client, tostring(v) .. " is not defined\n");
			return;
		end
	end

	if (type(test) == "table") then
		server:print_to_client(client, tostring(test) .. " is a table\n");
		local s = string.format("%10s - %10s\n", "key", "value");
		server:print_to_client(client, s);
		for k, v in pairs(test) do
			local s = string.format("%10s - %10s\n", tostring(k), tostring(v));
			server:print_to_client(client, s);
			if (type(v) == "table") then
				for k1, v1 in pairs(v) do
					local s = string.format("  %10s - %10s\n", tostring(k1), tostring(v1));
					server:print_to_client(client, s);
				end
			end
		end
	else
		server:print_to_client(client, first .. "is a " .. tostring(type(test)) .. ": " .. tostring(test) .. "\n")
	end
end

function FUNC_show_spawns ()
	local k, v;
	if (server.show_spawns == false) then
		for k, v in pairs(server.spawns) do
			v.edict = edict.get_unused(server.__pointer);
			edict.set_origin(v.edict, v.origin.x, v.origin.y, v.origin.z);
			--print (v.origin.x .. " " .. v.origin.y .. " " .. v.origin.z .. " " .. server.spawn_model);
			if (v.angles) then
				edict.set_angles(v.edict, v.angles.x, v.angles.y, v.angles.z);
			end
			edict.set_modelindex(v.edict, server.spawn_model);
			--edict.set_baseline(v);
		end
		server.show_spawns = true;
	else
		for k, v in pairs(server.spawns) do
			edict.remove(v.edict);
		end
		server.show_spawns = false;
	end
end

function handle_entity (server_ptr, edict_in)
	if (server.edicts[edict_in]) then
		if (server.edicts[edict_in].func) then
			server.edicts[edict_in]:func(edict_in);
		end
	end
end

