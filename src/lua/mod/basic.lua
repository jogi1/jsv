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

require 'vector'
require 'trains'
require 'items'

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

	if (lent.spawnflags) then
		lent.spawnflags = tonumber(lent.spawnflags);
	end

	item_setup(lent);
	server.entities[#server.entities + 1] = lent;

--	print(entity);
--	print("-------------------------------------------\n");
end

-- used to preload models read in by the map
function FUNC_entity_load_finished (server_ptr)
	local key, value;
	local v, e;

	for key, value in pairs(server.entities) do
		v = edict.get_unused(server_ptr);
		if (not server.edicts[v]) then
			server.edicts[v] = {};
		end
		server.edicts[v]["e_type"] = "static";
		server.edicts[v]["entity"] = value;
		server.edicts[v]["entities_index"] = key;
		server.edicts[v]["model_s"] = server:precache_model(value.model, true);
		server.edicts[v]["model_index"] = server.edicts[v]["model_s"].index;
		edict.set_modelindex(v, server.edicts[v]["model_index"]);

		if (value["origin"]) then
			edict.set_origin(v, value.origin);
		end
		if (value["angles"]) then
			edict.set_angles(v, value.angles);
		end

		if (value.skinnum) then
			edict.set_skinnum(v, value.skinnum);
		end

		if (value.size) then

			edict.set_mm(v, value.size.mins, value.size.maxs);

			local stop = vector.from_values(value.origin.x, value.origin.y, value.origin.z - 256);
			local trace = server:trace_edict(v, value.origin, stop, 0, nil);
			if (trace) then
				edict.set_origin(v, trace.endpos);
			end
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
	trains.setup();
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
	return server.spawns[math.random(1, #server.spawns)].origin;
end

function FUNC_print_info()
	server:print("entites info etc:");
	server:print("lights: " .. #server.lights);
	server:print("spawns: " .. #server.spawns);
	server:print("entities: " .. #server.entities);
	server:print("telporters: " .. #server.teleporter_trigger);
	server:print("teleporter_destination: " .. #server.teleporter_destination);
	server:print("doors: " .. #server.doors);
	server:print("trains: " .. #server.trains);
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
			server:print("origin: " .. v.origin.x .. " - " ..  v.origin.y .. " - " .. v.origin.z);
			if (v.angles) then
				edict.set_angles(v.edict, v.angles.x, v.angles.y, v.angles.z);
			end
			edict.set_modelindex(v.edict, server.spawn_model);
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

function put_client_on_server (server_ptr, client, edict_in)
	local mins = { x = -16, y = -16, z = -24};
	local maxs = { x = 16, y = 16, z = 32};
	edict.set_mm(edict_in, mins, maxs);
	server:print("test!");
end

