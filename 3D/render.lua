
require "3D/registers"

local drawers=Drawers

local make_static_draw_func=function(drawer)
	local id=drawers(drawer)
	return function(o)
		API.call_list(id)
	end
end

local init_scn_
init_scn_=function(scn)
	local d,m=scn.drawer,scn.material
	if d and type(d)~="function" then
		scn.drawer=make_static_draw_func(d)
	end
	for i,v in ipairs(scn) do
		init_scn_(v)
	end
	return scn
end

local textures=Textures

local material_hooks={
	['map_Kd']=function(v)
		API.apply_texture(textures(v))
	end,
}

local apply_material_=function(mtl)
	local hook
	for k,v in pairs(mtl) do
		hook=material_hooks[k]
		if hook then hook(v) end
	end
end

local draw_scn_
draw_scn_=function(scn)
	local m,d,t=scn.matrix,scn.drawer,scn.material
	if m then API.push_and_apply_matrix(m) end
	if t then apply_material_(t) end
	if d then d(o) end
	for i,v in ipairs(scn) do
		draw_scn_(v)
	end
	if m then API.pop_matrix() end
	return scn
end

local update_scn_
update_scn_=function(scn)
	local update=scn.update
	if update then update(scn) end
	for i,v in ipairs(scn) do
		update_scn_(v)
	end
	return scn
end

init_scn,update_scn,draw_scn,apply_material=init_scn_,update_scn_,draw_scn_,apply_material_

