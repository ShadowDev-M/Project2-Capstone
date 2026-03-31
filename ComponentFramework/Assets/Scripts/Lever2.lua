function Preload()

end	


function Start()

end



function Update(deltaTime) 
	if Transform.Rotation.ijk.z < 0 then
		Game:Find("PipeBlock").Transform.Position = Game:Find("PipeBlock").Transform.Position + Vec3.new(0, 0, 30)
	end
end
