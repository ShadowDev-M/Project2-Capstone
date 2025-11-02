
function Start()
	print("Hello from LuaJIT!") 
end

function Update(deltaTime)
	local v1 = Vec3.new()    
	v1 = Vec3.new(1,0,0) + Vec3.new(2,0,0)    	
	print(v1)                
 
	local v2 = Quaternion.new()       	
	print(v2)               

end