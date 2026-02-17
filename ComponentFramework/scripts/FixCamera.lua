
function Preload()

end	




function Start()

end

--Rigidbody is the Physics Component of the script's user actor. Set and get the properties with Rigidbody.YOURVARIABLE 

--Game Handler Script 

function Update(deltaTime) 
	local tPos = Transform.Position

	tPos.z = Game.UsedCamera.Transform.Position.z
	
	tPos.y = tPos.y + 5

	Game.UsedCamera.Transform.Position = tPos

end


