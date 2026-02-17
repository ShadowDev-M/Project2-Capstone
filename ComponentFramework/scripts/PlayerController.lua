
speed = 7
accelSpeed = 20
canJump = 1
jumpCooldown = 2
canInteract = 1
canInteract2 = 1

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
	
	-- bandaid fix for jump spamming
	if canJump == 0 then
		jumpCooldown = jumpCooldown - deltaTime
		if jumpCooldown <= 0 then
			canJump = 1
			jumpCooldown = 2
		end
	end


	local newAccel = GameObject.Rigidbody.Accel

	-- added sprint key
	if Game.Input.GetInputState("Left Shift") == 2 then
		accelSpeed = 35
		speed = 12
	else
		accelSpeed = 20
		speed = 7
	end

	if Game.Input.GetInputState("D") == 2 then
		
		newAccel.x = accelSpeed / GameObject.Rigidbody.Mass
	end

	if Game.Input.GetInputState("A") == 2 then
		
		
		newAccel.x = -accelSpeed / GameObject.Rigidbody.Mass
	end
	

	if math.abs(GameObject.Rigidbody.Vel.x) > speed then
		newAccel.x = 0
	end

	if Game.Input.GetInputState("A") == 0 and Game.Input.GetInputState("D") == 0 then
		newAccel.x = 0
	end

	GameObject.Rigidbody.Accel = newAccel

	if math.abs(GameObject.Rigidbody.Vel.x) > 0.1 then
		local uniDirection = Vec3.new(GameObject.Rigidbody.Vel.x,0.0,0.0)
		Transform.Rotation = QMath.LookAt(-VMath.Normalize(uniDirection), Vec3.new(0,1,0))
	end

	

end

 -- applied bandaid fix for jump spamming and velocity stacking
function OnCollisionStay(other)
	if Game.Input.GetInputState("SPACE") == 1 and canJump == 1 then
		canJump = 0

		jumpclip:SetAnimation("jump")
		jumpclip.Loop = false
		 
		GameObject.Animator.Clip = jumpclip
		GameObject.Animator:Play()

		newVel = GameObject.Rigidbody.Vel
		
		newVel.y = 10
		GameObject.Rigidbody.Vel = newVel
	end

end

function OnTriggerStay(other)
	if Game:Find("Lever") == other then
		if Game.Input.GetInputState("E") == 1 and canInteract == 1 then
			leverRot = QMath.AngleAxisRotation(-45, Vec3.new(0, 0, 1))
			Game:Find("Lever").Transform.Rotation = leverRot
			canInteract = 0
		end
	end
	if Game:Find("Lever2") == other then
		if Game.Input.GetInputState("E") == 1 and canInteract2 == 1 then
			leverRot = QMath.AngleAxisRotation(-45, Vec3.new(0, 0, 1))
			Game:Find("Lever2").Transform.Rotation = leverRot
			canInteract2 = 0
		end
	end
end