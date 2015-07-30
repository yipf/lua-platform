require "utils/register_table"
require "utils/helpers"
local make_msg=combine_string

------------------------------------------------------------------------------------------------------------------------------------------
-- Drawers
------------------------------------------------------------------------------------------------------------------------------------------

local drawer_hooks=make_register_table()

local direct_draw=function(d)
	local key=d[1]
	local hook=drawer_hooks(key)
	assert(hook,combine_string("Invalid drawer type:",key))
	pcall(hook,unpack(d,2))
end

local obj2callid=function(o)
	id=API.begin_gen_calllist()
	direct_draw(o)
	API.end_gen_calllist()
	return id
end

Drawers=make_register_table(obj2callid)
Drawer_hooks=drawer_hooks
Direct_draw=direct_draw
------------------------------------------------------------------------------------------------------------------------------------------
-- Textures
------------------------------------------------------------------------------------------------------------------------------------------

local texture_hooks=make_register_table()

local obj2texid=function(o)
	local key=o[1]
	local f=texture_hooks(key)
	assert(f,combine_string("Invalid texture type:",key))
	return f(unpack(o,2))
end

Textures=make_register_table(obj2texid)
Texture_hooks=texture_hooks

------------------------------------------------------------------------------------------------------------------------------------------
-- Shaders
------------------------------------------------------------------------------------------------------------------------------------------

local shader_hooks=make_register_table()

local shader2shaderid=function(o)
	local key=o[1]
	local f=shader_hooks(key)
	assert(f,combine_string("Invalid shader type:",o[1]))
	return f(unpack(o,2))
end

Shaders=make_register_table(shader2shaderid)
Shader_hooks=shader_hooks

