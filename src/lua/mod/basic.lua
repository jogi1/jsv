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

items_types = {
	item_armorInv ="progs/invulner.mdl",
	item_health = "maps/b_bh10.bsp",
	item_armor1 = "progs/armor.mdl",
	item_armor2 = "progs/armor.mdl",
	item_cells = "maps/b_batt1.bsp",
	item_rockets = "maps/b_rock0.bsp",
	item_spikes = "maps/b_nail0.bsp",
	weapon_lightning = "progs/g_light.mdl",
	weapon_grenadelauncher = "progs/g_rock.mdl",
	weapon_rocketlauncher = "progs/g_rock2.mdl",
	weapon_supernailgun = "progs/g_nail2.mdl"
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
static_entities = {}
lights = {}
entities = {}
spawns = {}
intermission = {}
player_start = {}
teleporter_destination = {}
teleporter_trigger = {}
trigger_push = {}
edicts = {}

function FUNC_map_start(server_ptr, client, ...)
end

function FUNC_entity_preload(server_ptr, client, ...)
	--preload models and sounds
	for key, value in pairs(preload_models) do
		server.precached_models[value] = server:precache_model(value)
		print(server.precached_models[value] .. " " .. value);
	end

	for key, value in pairs(preload_sounds) do
		server.precached_sounds[value] = server:precache_sound(value)
	end

	for key, value in pairs(lightstyles) do
		server:add_lightstyle(value);
	end
end

function FUNC_entity_load (server_ptr, entity)
	origin = nil;
	angle = nil;
	for field, value in string.gmatch(entity, "\"([^\".]*)\" \"([^\".]*)\"") do
		if (field == 'classname') then
			classname = value;
		elseif (field == 'origin') then
			origin = value;
		elseif (field == 'light') then
			light = value;
		elseif (field == 'mangle') then
			mangle = value;
		elseif (field == 'angle') then
			angle = value;
		elseif (field == 'model') then
			model = value;
		elseif (field == 'targetname') then
			targetname = value;
		elseif (field == 'target') then
			target = value;
		elseif (field == 'speed') then
			speed = value;
		elseif (field == 'style') then
			style = value;
		elseif (field == 'delay') then
			delay = value;
		elseif (field == 'spawnflags') then
			spawnflags = value;
		elseif (field == 'message') then
			message = value;
		else
			print(field .. " - " .. value );
		end
	end

	if (origin) then
		origin = vector.from_string(origin);
	end

	if (angles) then
		angles = vector.from_string(angles);
	end


	if (classname == 'light') then
		lights[#lights + 1] = {}
		lights[#lights]["origin"] = origin;
		lights[#lights]["light"] = light;
		return;
	end

	if (classname == 'info_player_deathmatch') then
		spawns[#spawns + 1] = {}
		print (origin.x .. " " .. origin.y .. " " .. origin.z);
		spawns[#spawns]["origin"] = origin;
		spawns[#spawns]["angle"] = angle;
		return
	end

	if (classname == 'info_intermission') then
		intermission["mangle"] = mangle;
		intermission["origin"] = origin;
		return
	end

	if (classname == 'info_teleport_destination') then
		teleporter_destination[#teleporter_destination + 1] = {};
		teleporter_destination[#teleporter_destination]["angle"] = angle;
		teleporter_destination[#teleporter_destination]["name"] = targetname;
		teleporter_destination[#teleporter_destination]["origin"] = targetname;
		return
	end

	if (classname == 'trigger_teleport') then
		teleporter_trigger[#teleporter_trigger + 1] = {};
		teleporter_trigger[#teleporter_trigger]["target"] = target;
		teleporter_trigger[#teleporter_trigger]["model"] = model;
		return
	end

	if (classname == 'trigger_push') then
		trigger_push[#trigger_push + 1] = {};
		trigger_push[#trigger_push]["angle"] = angle;
		trigger_push[#trigger_push]["speed"] = speed;
		trigger_push[#trigger_push]["style"] = style;
		trigger_push[#trigger_push]["delay"] = delay;
		trigger_push[#trigger_push]["model"] = model;
		return
	end

	if (classname == 'info_player_start') then
		player_start["origin"] = origin;
		return
	end

	if (classname == 'worldspawn') then
		server:set_map_name(message);
		return
	end

	for key, value in pairs(items_types) do
		if (classname == key) then
			entities[#entities + 1] = {}
			entities[#entities]["type"] = classname;
			entities[#entities]["origin"] = origin;
			entities[#entities]["model"] = value;
			if (angle) then
				entities[#entities]["angle"] = angle;
			end
			return
		end
	end

	print(entity);
	print("-------------------------------------------\n");
end

-- used to preload models read in by the map
function FUNC_entity_load_finished (server_ptr)
	for key, value in pairs(entities) do
		server.precached_models[value["model"]] = server:precache_model(value["model"])
		v = edict.get_unused(server_ptr);
		edicts[v] = {};
		edicts[v]["e_type"] = "static";
		edicts[v]["entity"] = value;
		edicts[v]["entities_index"] = key;
		edicts[v]["model_index"] = server.precached_models[value["model"]];
--		print (value["type"] .. " - " .. edicts[v]["model_index"] .. " - " .. value["model"]);
		edict.set_modelindex(v, edicts[v]["model_index"]);
		if (value["origin"]) then
			edict.set_origin(v, value.origin.x, value.origin.y, value.origin.z);
		end
		if (value["angels"]) then
			edict.set_angels(v, value.angels.x, value.angels.y, value.angels.z);
		end
		edict.set_baseline(v);
	end
end

function FUNC_get_spawn()
	return spawns[1].origin;
end

function FUNC_print_info()
	print("entites info etc:");
	print("lights: " .. #lights);
	print("spawns: " .. #spawns);
	print("entities: " .. #entities);
	print("telporters: " .. #teleporter_trigger);
	print("teleporter_destination: " .. #teleporter_destination);
end


