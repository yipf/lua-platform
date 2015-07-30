
make_register_table=function(kv_map)
	local t={}
	return function(key,value)
		if not key then return t end
		if value then
			t[key]=value
		else
			value=t[key]
			if (not value) and kv_map then 
				value=kv_map(key)
				t[key]=value 
			end
		end
		return value
	end
end