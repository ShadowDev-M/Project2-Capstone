function Preload()

end	


function Start()

end



function Update(deltaTime) 
	if Transform.Rotation.ijk.z < 0 then
		Game:Find("Door2").Rigidbody.Vel = Vec3.new(0,3,0)
	end
end
