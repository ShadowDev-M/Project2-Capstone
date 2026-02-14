
speed = 7


--public testvar = 1.0
jumpclip = AnimationClip.new()
jumpclip:PreloadAnimation("falling")
jumpclip:PreloadAnimation("jump")
jumpclip:PreloadAnimation("land")


function Preload()

end	


function Start()

end



function Update(deltaTime) 
	
	--print(testvar)



	local newAccel = GameObject.Rigidbody.Accel

	if Game.Input.GetInputState("D") == 2 then
		
		newAccel.x = 20 / GameObject.Rigidbody.Mass
	end

	if Game.Input.GetInputState("A") == 2 then
		
		
		newAccel.x = -20 / GameObject.Rigidbody.Mass
	end


	if math.abs(GameObject.Rigidbody.Vel.x) > speed then
		print("e")
		newAccel.x = 0
	end

	if Game.Input.GetInputState("A") == 0 and Game.Input.GetInputState("D") == 0 then
		newAccel.x = 0
	end

	GameObject.Rigidbody.Accel = newAccel


	
	if Game.Input.GetInputState("SPACE") == 1 then
		jumpclip:SetAnimation("jump")
		jumpclip.Loop = false
		 
		GameObject.Animator.Clip = jumpclip
		GameObject.Animator:Play()



		newVel = GameObject.Rigidbody.Vel
		
		newVel.y = newVel.y + 10
		GameObject.Rigidbody.Vel = newVel
	end

	if math.abs(GameObject.Rigidbody.Vel.x) > 0.1 then
		local uniDirection = Vec3.new(GameObject.Rigidbody.Vel.x,0.0,0.0)
		Transform.Rotation = QMath.LookAt(-VMath.Normalize(uniDirection), Vec3.new(0,1,0))
	end

	

end


