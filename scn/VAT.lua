require "3D/utils"

local make_stick_matrix=function(from,to,coord)
	coord=coord or API.create_mat4x4()
	local v=sub(to,from)
	local length=math.sqrt(dot(v,v))
	local Z,X,Y=v,{1,0,0}
	if eq(Z,X) then X={0,0,Z[1]>0 and -1 or 1} end
	Y=cross(Z,X)
	X=cross(Y,Z)
	Y=normalize(Y)
	X=normalize(X)
	coord[0]=X[1]	coord[1]=X[2]	coord[2]=X[3]	coord[3]=0
	coord[4]=Y[1]	coord[5]=Y[2]	coord[6]=Y[3]	coord[7]=0
	coord[8]=v[1]	coord[9]=v[2]	coord[10]=v[3]	coord[11]=0
	coord[12]=from[1]	coord[13]=from[2]	coord[14]=from[3]	coord[15]=1
	return coord
end

local make_vat=function(motion,E)
	local id=motion.id or 1
	local V=motion[id]
	local push=table.insert
	-- define nodes
	local nodes={material={map_Kd={"color",{255,255,0}}}}
	local cell={"sphere",2}
	local x,y,z
	for i,v in ipairs(V) do
		x,y,z=unpack(v)
		push(nodes,{drawer=cell,matrix=API.make_translate(API.create_mat4x4(),x,y,z),id=i})
	end
	-- define edges
	local edges={material={map_Kd={"color",{0,0,255}}}}
	local tx,ty,tz,from,to
	local meta_stick={"meta-stick",0.4}
	for k,es in pairs(E) do
		from=V[k]
		x,y,z=unpack(from)
		for i,e in ipairs(es) do
			print(k,e)
			to=V[e]
			tx,ty,tz=unpack(to)
			push(edges,{drawer=meta_stick,matrix=make_stick_matrix(from,to,API.create_mat4x4()),from=k,to=e})
		end
	end
	-- define update functions
	local update=function(o)
		local id=motion.id
		if id then			id=id==#motion and 1 or id+1		else			id=1		end
		motion.id=id
		local V=motion[id]
		local x,y,z
		for i,n in ipairs(nodes) do
			x,y,z=unpack(V[n.id])
			n.matrix=API.make_translate(n.matrix,x,y,z)
		end
		local tx,ty,tz
		for i,e in ipairs(edges) do
			e.matrix=make_stick_matrix(V[e.from],V[e.to],e.matrix)
		end
	end
	return {nodes,edges,update=update,motion=motion}
end

local G={
	relations=	{
		[1]={2,3},
--~ 		[2]={3},
	},
	{  {0,10,0},	{-5,10,5}, 	{5,10,0}},

}



local combine_vector=function(A,B,b)
	local a=1-b
	local C={}
	for i,v in ipairs(A) do
		C[i]=a*v+b*B[i]
	end
	return C
end

local combine_nodes=function(from,to,a)
	local b=1-a
	assert(#from<=#to,"Elements of start vector and end vector should be equal")
	local vec,vv,vto={}
	for i,v in ipairs(from) do
		vec[i]=combine_vector(v,to[i],a)
	end
	return vec
end

local gen_motion=function(from,to,n)
	local sample=samples(0,1,n)
	local motion={}
	for i,v in ipairs(sample) do
		motion[i]=combine_nodes(from,to,v)
	end
	return motion
end

local S={{0,10,0},{0,10,10},{10,10,0}}
local E={{0,10,0},{0,20,10},{10,5,0}}

local motion=gen_motion(S,E,100)
local E={
		[1]={2,3},
		[2]={3},
}

local scn={
 config={"Config The Opengl Windows",0,1,0,"65 105 225",1,20,40,20,1,1,1},
  
  light_shader={"built-in","spot-light&shadow"},
  
--~   dofile("./data/base_scn.lua"),
  
  drawer={"scn-file","./data/base_scn.lua"},

	make_vat(motion,E),

--~ {drawer={"obj","/host/Files/lua-platform/data/horse.obj"}},
}

------------------------------------------------------------------------------------------
-- UI
------------------------------------------------------------------------------------------

local dialog,split,frame,vbox,tabs=iup.dialog,iup.split,iup.frame,iup.vbox,iup.tabs

require 'UI/gl_canvas'

local API=API
local camera=API.create_camera()
camera=API.make_camera(camera,0,0,0,100)
camera=API.set_camera_projection(camera,1,1000,math.rad(60),1)
camera=API.update_camera(camera)

local glw,gl_cfg=make_gl_canvas(scn,camera,800,800)
local gl_panel=frame{title="GL",glw}


local Update=iup.Update
local timer_toggle,timer=make_timer("timer",30,function()
    update_scn(scn)
	Update(glw)
end)

-- main dialog

local about_str=[[
YIPF Copyright

2008-2013
]]




local tabs=tabs{ expand="yes",
 vbox{tabtitle="Operation",expand='yes',gl_cfg,timer_toggle,make_space()},
 vbox{tabtitle="Option",expand='yes',cfg_btn,make_space()},
 vbox{tabtitle="About",expand='yes',make_space(about_str)},
}

local op_panel=frame{title="Operations",size="200x400",tabs,expand='yes'}

local dlg=dialog{title="my",iup.split{gl_panel,op_panel;orientation='verticle'}}

dlg:show()
 
if (not iup.MainLoopLevel or iup.MainLoopLevel()==0) then
  iup.MainLoop()
end