
function Preload()

end	




function Start()

end

--Rigidbody is the Physics Component of the script's user actor. Set and get the properties with Rigidbody.YOURVARIABLE 

--Game Handler Script 

function Update(deltaTime) 
	local cam = Game.UsedCamera
	if cam == nil then return end

	local tPos = Transform.WorldPosition

	tPos.z = cam.Transform.WorldPosition.z
	tPos.y = tPos.y + 5

	cam.Transform.Position = tPos
end


