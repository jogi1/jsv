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
