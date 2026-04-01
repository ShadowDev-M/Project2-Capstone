

animclip = AnimationClip.new()
animclip:PreloadAnimation("RobotWalking")
animclip:PreloadAnimation("RobotIdle")


actionclip = AnimationClip.new()
actionclip.Loop = false


local printtest = animclip:GetAnimationName()

function Preload()
	--animclip:PreloadAnimation("dancing")
  --  print("Preload called")
end	




function Start()



	animclip:SetAnimation("RobotIdle")
	--local printtest = animclip:GetAnimationName()
	print(printtest)

	

	GameObject.Animator.Clip = animclip

	GameObject.Animator:Play()


	print(Game:Find("Cube").Transform.GameObject.Transform.Position)

end

--Rigidbody is the Physics Component of the script's user actor. Set and get the properties with Rigidbody.YOURVARIABLE 

--Game Handler Script 

function Update(deltaTime) 
	
	
	if GameObject.Animator.Clip:GetAnimationName() == "RobotJumping" and GameObject.Animator.CurrentTime == GameObject.Animator.Length then
		animclip:SetAnimation("RobotFalling")
		animclip.SpeedMult = 0.2
		GameObject.Animator.Clip = animclip
		GameObject.Animator:Play()
	end
	
	if GameObject.Animator.Clip:GetAnimationName() == "RobotFalling" then
		if GameObject.Animator.SpeedMult > 0.2 then
			GameObject.Animator.SpeedMult = 0.2
		end
	end

	if GameObject.Animator.Clip:GetAnimationName() == "RobotLanding" and GameObject.Animator.CurrentTime == GameObject.Animator.Length then
		animclip:SetAnimation("RobotIdle")
		GameObject.Animator.Clip = animclip
		GameObject.Animator:Play()
	end


	if math.abs(GameObject.Rigidbody.Vel.x) > 0.01 and (GameObject.Animator.Clip:GetAnimationName() == "RobotWalking" or GameObject.Animator.Clip:GetAnimationName() == "RobotIdle") then 
		GameObject.Animator.SpeedMult = (math.abs(GameObject.Rigidbody.Vel.x) / 5) + 0.3
		
		if GameObject.Animator.Clip:GetAnimationName() == "RobotIdle" then
			print(GameObject.Animator.Clip:GetAnimationName())
		end
		if GameObject.Animator.Clip:GetAnimationName() == "RobotIdle" then
			animclip:SetAnimation("RobotWalking")
			GameObject.Animator.Clip = animclip
			GameObject.Animator:Play()

		end
	else
		
		if GameObject.Animator.Clip:GetAnimationName() == "RobotWalking" and ((GameObject.Animator.CurrentTime < 0.3 or GameObject.Animator.CurrentTime > (GameObject.Animator.Length - 0.3)) or (GameObject.Animator.CurrentTime > 0.5 and GameObject.Animator.CurrentTime < 0.6)) then
			animclip:SetAnimation("RobotIdle")
			GameObject.Animator.Clip = animclip
			GameObject.Animator:Play()
		end
	end

end

function OnCollisionEnter(other) 
	if GameObject.Animator.Clip:GetAnimationName() == "RobotFalling" then
		actionclip:SetAnimation("RobotLanding")
		GameObject.Animator.Clip = actionclip
		GameObject.Animator:Play()
	end
end


