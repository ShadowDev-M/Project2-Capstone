
speed = 7
accelSpeed = 20
canJump = 1
jumpCooldown = 2
canInteract = 1
canInteract2 = 1

--public testvar = 1.0
jumpclip = AnimationClip.new()
jumpclip:PreloadAnimation("RobotFalling")
jumpclip:PreloadAnimation("RobotJumping")
jumpclip:PreloadAnimation("RobotLanding")


function Preload()

end	


function Start()

end



function Update(deltaTime) 
	
	if GameObject.Animator.Clip:GetAnimationName() == "RobotJumping" and canJump then
		canJump = 0


	end


	

	-- added sprint key
	if Game.Input.GetInputState("Left Shift") == 2 then
		accelSpeed = 35
		speed = 12
	else
		accelSpeed = 20
		speed = 7
	end

	if Game.Input.GetInputState("D") == 2 then
		if math.abs(GameObject.Rigidbody.Vel.x) < speed then	
			GameObject.Rigidbody:AddAccel(Vec3.new(accelSpeed, 0, 0))
		end
	end

	if Game.Input.GetInputState("A") == 2 then
		if math.abs(GameObject.Rigidbody.Vel.x) < speed then	
			GameObject.Rigidbody:AddAccel(Vec3.new(-accelSpeed, 0, 0))
		end
	end
	
	if math.abs(GameObject.Rigidbody.Vel.x) > 0.1 then
		local uniDirection = Vec3.new(GameObject.Rigidbody.Vel.x,0.0,0.0)
		Transform.Rotation = QMath.LookAt(-VMath.Normalize(uniDirection), Vec3.new(0,1,0))
	end
	

	if (GameObject.Rigidbody.Vel.y) < 5 and canJump == 0 then
		GameObject.Rigidbody:AddAccel( Vec3.new(0, -accelSpeed, 0))
	end
	

end

 -- applied bandaid fix for jump spamming and velocity stacking
function OnCollisionStay(other)


	--print(other.Name)

	if Game.Input.GetInputState("Space") == 1 and Physics.Raycast(GameObject.Transform.Position, Vec3.new(0,-1,0), 20).GameObject == other then
		canJump = 0

		jumpclip:SetAnimation("RobotJumping")
		jumpclip.Loop = false
		jumpclip.SpeedMult = 1.2
		jumpclip.StartTime = 0.3

		GameObject.Animator.Clip = jumpclip

		GameObject.Animator:Play()
		newVel = GameObject.Rigidbody.Vel
		
		newVel.y = 15
		GameObject.Rigidbody.Vel = newVel
	end

	if canJump == 0 and Physics.Raycast(GameObject.Transform.Position, Vec3.new(0,-1,0), 20).GameObject == other then
		canJump = 1
	end

end

