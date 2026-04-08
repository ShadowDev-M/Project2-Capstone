
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

	tPos.z = -33
	tPos.y = tPos.y + 15



	local distPos1 = cam.Transform.Position
	local distPos2 = tPos 



	if VMath.Distance(distPos1, distPos2) > 0.01 then 
		if math.abs(cam.Transform.Position.x - tPos.x) <= 0.01 then
			tPos.x = cam.Transform.Position.x
		end
		if math.abs(cam.Transform.Position.y - tPos.y) <= 0.01 then
			tPos.y = cam.Transform.Position.y
		end
		if math.abs(cam.Transform.Position.z - tPos.z) <= 0.01 then
			tPos.z = cam.Transform.Position.z
		end


		cam.Transform.Position = tPos

	end
end


