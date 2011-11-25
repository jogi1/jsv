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
	item_armorInv = {model="progs/armor.mdl", skinnum=2},
	item_health = {model="maps/b_bh10.bsp"},
	item_armor1 = {model="progs/armor.mdl", skinnum=0},
	item_armor2 = {model="progs/armor.mdl", skinnum=1},
	item_cells = {model="maps/b_batt1.bsp"},
	item_shells= {model="maps/b_shell0.bsp"},
	item_rockets = {model="maps/b_rock0.bsp"},
	item_spikes = {model="maps/b_nail0.bsp"},
	weapon_lightning = {model="progs/g_light.mdl"},
	weapon_grenadelauncher = {model="progs/g_rock.mdl"},
	weapon_rocketlauncher = {model="progs/g_rock2.mdl"},
	weapon_supernailgun = {model="progs/g_nail2.mdl"},
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
static_entities = {}
lights = {}
entities = {}
spawns = {}
intermission = {}
player_start = {}
teleporter_destination = {}
teleporter_trigger = {}
trigger_push = {}
trigger_hurt = {}
edicts = {}
testvar = 0;

function FUNC_map_start(server_ptr, client, ...)
end

function FUNC_entity_preload(server_ptr, client, ...)
	--preload models and sounds
	local key, value;
	for key, value in pairs(preload_models) do
		server.precached_models[value] = server:precache_model(value, true)
	end

	for key, value in pairs(preload_sounds) do
		server.precached_sounds[value] = server:precache_sound(value)
	end

	for key, value in pairs(lightstyles) do
		server:add_lightstyle(value);
	end
end

function FUNC_entity_load (server_ptr, entity)
	local lent = {};
	local field = nil;
	local value = nil;
	local key = nil;

	for field, value in string.gmatch(entity, "\"([^\".]*)\" \"([^\".]*)\"") do
		if (field == 'classname') then
			lent.classname = value;
		elseif (field == 'origin') then
			lent.origin = value;
		elseif (field == 'light') then
			lent.light = value;
		elseif (field == 'mangle') then
			lent.mangle = value;
		elseif (field == 'angle') then
			lent.angle = value;
		elseif (field == 'model') then
			lent.model = value;
		elseif (field == 'targetname') then
			lent.targetname = value;
		elseif (field == 'target') then
			lent.target = value;
		elseif (field == 'speed') then
			lent.speed = value;
		elseif (field == 'style') then
			lent.style = value;
		elseif (field == 'delay') then
			lent.delay = value;
		elseif (field == 'spawnflags') then
			lent.spawnflags = value;
		elseif (field == 'message') then
			lent.message = value;
		elseif (field == 'dmg') then
			lent.dmg = value;
		else
			print("unknown: " .. field .. " - " .. value );
		end
	end

	if (lent.origin) then
		lent.origin = vector.from_string(lent.origin);
	end

	if (lent.angles) then
		lent.angles = vector.from_string(lent.angles);
	end


	if (lent.classname == 'light') then
		lights[#lights + 1] = lent;
		return;
	end

	if (lent.classname == 'info_player_deathmatch') then
		spawns[#spawns + 1] = lent;
		return
	end

	if (lent.classname == 'info_intermission') then
		intermission = lent;
		return
	end

	if (lent.classname == 'info_teleport_destination') then
		teleporter_destination[#teleporter_destination + 1] = lent;
		return
	end

	if (lent.classname == 'trigger_teleport') then
		teleporter_trigger[#teleporter_trigger + 1] = lent;
		return
	end

	if (lent.classname == 'trigger_push') then
		trigger_push[#trigger_push + 1] = lent;
		return
	end

	if (lent.classname == 'trigger_hurt') then
		trigger_hurt[#trigger_hurt + 1] = lent;
		return
	end



	if (lent.classname == 'info_player_start') then
		player_start = lent;
		return
	end

	if (lent.classname == 'worldspawn') then
		server:set_map_name(lent.message);
		return
	end

	for key, value in pairs(items_types) do
		if (lent.classname == key) then
			entities[#entities] = lent;
			entities[#entities]["model"] = value.model;
			if (value.skinnum) then
				entities[#entities]["skinnum"] = value.skinnum;
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
	for key, value in pairs(entities) do
		server.precached_models[value["model"]] = server:precache_model(value["model"], true)
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

		if (value.skinnum) then
			edict.set_skinnum(v, value.skinnum);
		end

		-- this should always be last
		edict.set_baseline(v);
	end

	-- set the models to 0 on triggers and do other trigger setup
	for key, value in pairs(teleporter_trigger) do
		e = server:get_edict_for_inline_model(value.model)
		edict.set_modelindex(e, 0)
		edict.set_baseline(e);
	end

	for key, value in pairs(trigger_push) do
		e = server:get_edict_for_inline_model(value.model)
		edict.set_modelindex(e, 0)
		edict.set_baseline(e);
	end

	for key, value in pairs(trigger_hurt) do
		e = server:get_edict_for_inline_model(value.model)
		edict.set_modelindex(e, 0)
		edict.set_baseline(e);
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

function FUNC_print_var (server_ptr, client, trigger)
	local test = _G[trigger];
	local k, v, k1, v1;
	if (test == nil) then
		server:print_to_client(client, trigger .. " is not defined\n");
		return;
	end
	
	if (type(test) == "table") then
		server:print_to_client(client, trigger .. " is a table\n");
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
		server:print_to_client(client, trigger .. "is a " .. tostring(type(test)) .. ": " .. tostring(test) .. "\n")
	end



end


