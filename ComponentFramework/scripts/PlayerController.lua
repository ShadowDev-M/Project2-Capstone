
speed = 10

function Preload()

end	


function Start()

end


function Update(deltaTime) 
	
	local newVel = GameObject.Rigidbody.Vel

	if Game.Input.GetInputState("D") == 2 then
		
		newVel.x = speed

	end
	if Game.Input.GetInputState("A") == 2 then
		
		
		newVel.x = -speed

	end
	
	
	GameObject.Rigidbody.Vel = newVel

	

	if VMath.Magnitude(GameObject.Rigidbody.Vel) > 0.1 then
		Transform.Rotation = QMath.LookAt(-VMath.Normalize(GameObject.Rigidbody.Vel), Vec3.new(0,1,0))
	end

end


