function edict.set_origin(e, x, y, z)
	if (vec == nil) then
		return;
	end

	if (type(x) == "table") then
		edict.__set_origin(e, x.x, x.y, x.z);
		return;
	end
	edict.__set_origin(e, x == nil and 0 or x, y == nil and 0 or y, z == nil and 0 or z);
end

function edict.set_angle(e, x, y, z)
	if (vec == nil) then
		return;
	end
	if (x) then
		if (type(x) == table) then
			edict.__set_angle(e, x.x, x.y, x.z);
			return;
		end
	end

	edict.__set_angle(e, x == nil and 0 or x, y == nil and 0 or y, z == nil and 0 or z);
end

function edict.set_mm(e, mins, maxs)
	edict.__set_mm(e, mins.x, mins.y, mins.z, maxs.x, maxs.y, maxs.z);
end
