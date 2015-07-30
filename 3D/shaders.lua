local FMT="3D/shaders/%s.shader"

local f=function(name)
	local str=string.format(FMT,name)
	local vert,frag=dofile(str)
	return API.build_shader(vert,frag)
end

Shader_hooks("built-in",f)