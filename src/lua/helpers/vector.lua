vector = {}

function vector.from_string(string)
	temp = {}
	vec = {}
	i = 1;
	for val in string.gmatch(string, "%S+") do
		temp[i] = val;
		i = i + 1;
	end
	vec.x = temp[1];
	vec.y = temp[2];
	vec.z = temp[3];
	return vec;
end

function vector.from_values(x, y, z)
	vec = {};
	vec.x = x;
	vec.y = y;
	vec.z = z;
	return vec;
end

function vector.subtract(self, x, y, z)
	vec = {};
	if (type(x) == "table") then
		vec.x = self.x - x.x;
		vec.y = self.y - x.y;
		vec.z = self.z - x.z;
	else
		vec.x = self.x - x == nil and 0 or x;
		vec.y = self.y - y == nil and 0 or y;
		vec.z = self.z - z == nil and 0 or z;
	end
	return vec;
end

function vector.add(self, x, y, z)
	vec = {};
	if (type(x) == "table") then
		vec.x = self.x + x.x;
		vec.y = self.y + x.y;
		vec.z = self.z + x.z;
	else
		vec.x = self.x + x == nil and 0 or x;
		vec.y = self.y + y == nil and 0 or y;
		vec.z = self.z + z == nil and 0 or z;
	end
	return vec;
end

function vector.scale(self, scale)
	vec = {};
	vec.x = self.x * scale;
	vec.y = self.y * scale;
	vec.z = self.z * scale;
	return vec;
end

function vector.length(self)
	if (self == nil) then
		return 0;
	end
	return math.sqrt(math.pow(self.x,2) + math.pow(self.y,2) + math.pow(self.z,2));
end


