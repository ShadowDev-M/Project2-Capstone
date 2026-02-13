
speed = 10


public testvar = 1.0



function Preload()

end	


function Start()

end


function Update(deltaTime) 
	
	print(testvar)



	local newVel = GameObject.Rigidbody.Vel

	if Game.Input.GetInputState("D") == 2 then
		
		newVel.x = speed

	end
	if Game.Input.GetInputState("A") == 2 then
		
		
		newVel.x = -speed

	end
	
	
	GameObject.Rigidbody.Vel = newVel

	if math.abs(GameObject.Rigidbody.Vel.x) > 0.1 then
		local uniDirection = Vec3.new(GameObject.Rigidbody.Vel.x,0.0,0.0)
		Transform.Rotation = QMath.LookAt(-VMath.Normalize(uniDirection), Vec3.new(0,1,0))
	end

	

end


function OnCollisionEnter(other) 
	testobj = other

	print("hi")
	print(other)
	print(other.Name)


end
