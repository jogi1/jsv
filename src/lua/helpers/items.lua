
function item_setup_armor (item)
	item.model = "progs/armor.mdl";
	item.size = {mins={x=-16, y=-16, z=0}, maxs={x=16, y=16, z=56}};
	if (item.classname == "item_armor1") then
		item.skinnum = 0;
	elseif (item.classname == "item_armor2") then
		item.skinnum = 1;
	elseif (item.classname == "item_armorInv") then
		item.skinnum = 2;
	end
	return item;
end

function item_setup_health (item)
	if (item.spawnflags == nil) then
		item.model = "maps/b_bh25.bsp";
		item.value = 25;
	else
		print (item.spawnflags);
		if (item.spawnflags == 2) then
			item.model = "maps/b_bh100.bsp";
			item.value = 100;
		elseif (item.spawnflags == 1) then
			item.model = "maps/b_bh10.bsp";
			item.value = 10;
		end
	end
	item.size = {mins = { x=0, y=0, z=0}, maxs = { x=32, y=32, z=56}};
	return item;
end

function item_setup_artifact_super_damage (item)
	item.model = "progs/quaddama.mdl";
	item.size = {mins={x=-16, y=-16, z=-24}, maxs={x=16, y=16, z=32}};
	return item;
end

function item_setup_artifact_invisibility (item)
	item.model = "progs/invisibl.mdl";
	item.size = {mins={x=-16, y=-16, z=-24}, maxs={x=16, y=16, z=32}};
	return item;
end

function item_setup_artifact_invulnerability(item)
	item.model = "progs/invulner.mdl";
	item.size = {mins={x=-16, y=-16, z=-24}, maxs={x=16, y=16, z=32}};
	return item;
end

function item_setup_shells(item)
	if (item.spawnflags) then
		item.model = "maps/b_shell1.bsp";
	else
		item.model = "maps/b_shell0.bsp";
	end
	item.size = {mins={x=0, y=0, z=0}, maxs={x=32, y=32, z=56}};
	return item;
end

function item_setup_spikes(item)
	if (item.spawnflags) then
		item.model = "maps/b_nail1.bsp";
	else
		item.model = "maps/b_nail0.bsp";
	end
	item.size = {mins={x=0, y=0, z=0}, maxs={x=32, y=32, z=56}};
	return item;
end

function item_setup_rockets(item)
	if (item.spawnflags) then
		item.model = "maps/b_rock1.bsp";
	else
		item.model = "maps/b_rock0.bsp";
	end
	item.size = {mins={x=0, y=0, z=0}, maxs={x=32, y=32, z=56}};
	return item;
end

function item_setup_cells(item)
	if (item.spawnflags) then
		item.model = "maps/b_batt1.bsp";
	else
		item.model = "maps/b_batt0.bsp";
	end
	item.size = {mins={x=0, y=0, z=0}, maxs={x=32, y=32, z=56}};
	return item;
end

function item_setup_weapon_lightning (item)
	item.model = "progs/g_light.mdl";
	item.size = {mins={x=-16, y=-16, z=0}, maxs={x=16, y=16, z=56}};
	return item;
end

function item_setup_weapon_rocketlauncher(item)
	item.model = "progs/g_rock2.mdl";
	item.size = {mins={x=-16, y=-16, z=0}, maxs={x=16, y=16, z=56}};
	return item;
end

function item_setup_weapon_grenadelauncher(item)
	item.model = "progs/g_rock.mdl";
	item.size = {mins={x=-16, y=-16, z=0}, maxs={x=16, y=16, z=56}};
	return item;
end

function item_setup_weapon_supernailgun(item)
	item.model = "progs/g_nail2.mdl";
	item.size = {mins={x=-16, y=-16, z=0}, maxs={x=16, y=16, z=56}};
	return item;
end

function item_setup_weapon_nailgun(item)
	item.model = "progs/g_nail.mdl";
	item.size = {mins={x=-16, y=-16, z=0}, maxs={x=16, y=16, z=56}};
	return item;
end

function item_setup_weapon_supershotgun(item)
	item.model = "progs/g_shot.mdl";
	item.size = {mins={x=-16, y=-16, z=0}, maxs={x=16, y=16, z=56}};
	return item;
end

function item_setup (item)
	if (item.classname == "item_armor1" or item.classname == "item_armor2" or item.classname == "item_armorInv") then
		item = item_setup_armor (item);
	elseif (item.classname == "item_health") then
		item = item_setup_health (item);
	elseif (item.classname == "item_artifact_super_damage") then
		item = item_setup_artifact_super_damage (item);
	elseif (item.classname == "item_artifact_invisibility") then
		item = item_setup_artifact_invisibility (item);
	elseif (item.classname == "item_artifact_invulnerability") then
		item = item_setup_artifact_invulnerability (item);
	elseif (item.classname == "item_shells") then
		item = item_setup_shells (item);
	elseif (item.classname == "item_spikes") then
		item = item_setup_spikes(item);
	elseif (item.classname == "item_rockets") then
		item = item_setup_rockets(item);
	elseif (item.classname == "item_cells") then
		item = item_setup_cells(item);
	elseif (item.classname == "weapon_lightning") then
		item = item_setup_weapon_lightning(item);
	elseif (item.classname == "weapon_rocketlauncher") then
		item = item_setup_weapon_rocketlauncher(item);
	elseif (item.classname == "weapon_grenadelauncher") then
		item = item_setup_weapon_grenadelauncher(item);
	elseif (item.classname == "weapon_supernailgun") then
		item = item_setup_weapon_supernailgun(item);
	elseif (item.classname == "weapon_nailgun") then
		item = item_setup_weapon_nailgun(item);
	elseif (item.classname == "weapon_supershotgun") then
		item = item_setup_weapon_supershotgun(item);
	else
		return nil;
	end

	return item;
end
