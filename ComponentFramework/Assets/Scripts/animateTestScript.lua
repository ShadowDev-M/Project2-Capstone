

animclip = AnimationClip.new()
animclip:PreloadAnimation("RobotWalking")
animclip:PreloadAnimation("RobotIdle")

jumpclip = AnimationClip.new()


pressedUsers = {}


actionclip = AnimationClip.new()
actionclip.Loop = false


function Preload()
	--animclip:PreloadAnimation("dancing")
  --  print("Preload called")
end	




function Start()

	--default idle
	PlayClip("RobotIdle", GameObject.Animator.SpeedMult)

	

end


function PlayClip(name, spd)
	aClip = AnimationClip.new()
	aClip:SetAnimation(name)
	aClip.SpeedMult = spd
	GameObject.Animator.Clip = aClip
	GameObject.Animator:Play()

end


function Update(deltaTime) 
	

	
	if GameObject.Animator.Clip:GetAnimationName() == "RobotJumping" and GameObject.Animator.CurrentTime == GameObject.Animator.Length then
		--activate falling after jump animation ends
		PlayClip("RobotFalling", 0.2)
		
	end
	
	if GameObject.Animator.Clip:GetAnimationName() == "RobotFalling" then
		--In case movement or other actions increase anim speed for falling instead of their own intended animation, change it back
		if GameObject.Animator.SpeedMult > 0.2 then
			GameObject.Animator.SpeedMult = 0.2
		end
	end

	if GameObject.Animator.Clip:GetAnimationName() == "RobotLanding" and GameObject.Animator.CurrentTime == GameObject.Animator.Length then
		PlayClip("RobotIdle", GameObject.Animator.SpeedMult)
	end

	 
	if math.abs(GameObject.Rigidbody.Vel.x) > 0.7 and (GameObject.Animator.Clip:GetAnimationName() == "RobotWalking" or GameObject.Animator.Clip:GetAnimationName() == "RobotIdle" or GameObject.Animator.Clip:GetAnimationName() == "RobotLanding") then 

		if GameObject.Animator.Clip:GetAnimationName() == "RobotLanding" then
			--fast forward the landing animation when wanting to walk so we don't slide
			GameObject.Animator.SpeedMult = 2.4
		else
			--base speed of anim on movement speed (walking)
			GameObject.Animator.SpeedMult = (math.abs(GameObject.Rigidbody.Vel.x) / 5) - 0.3

			if GameObject.Animator.Clip:GetAnimationName() == "RobotIdle" then
				PlayClip("RobotWalking", GameObject.Animator.SpeedMult)
			end
		end

	else
		--Movement Stop Condition
		if GameObject.Animator.Clip:GetAnimationName() == "RobotWalking" then
			--Condition to make it a bit more of a smooth transition
			if ((GameObject.Animator.CurrentTime < 0.3 or GameObject.Animator.CurrentTime > (GameObject.Animator.Length - 0.3)) or (GameObject.Animator.CurrentTime > 0.5 and GameObject.Animator.CurrentTime < 0.6)) then
				PlayClip("RobotIdle", GameObject.Animator.SpeedMult)
			else
				--speed up mult so it gets to the smoother transition faster
				GameObject.Animator.SpeedMult = 2.0
			end
			
		end

	end

	if #(pressedUsers) == 0 then
		if GameObject.Animator.Clip:GetAnimationName() == "RobotWalking" or GameObject.Animator.Clip:GetAnimationName() == "RobotIdle" or GameObject.Animator.Clip:GetAnimationName() == "RobotLanding" then

			jumpclip:SetAnimation("RobotJumping")
			jumpclip.Loop = false
			jumpclip.SpeedMult = 1.2
			jumpclip.StartTime = 0.3

			GameObject.Animator.Clip = jumpclip

			GameObject.Animator:Play()


		end
	end

end

function OnCollisionEnter(other) 
	table.insert(pressedUsers, other)


	if (GameObject.Animator.Clip:GetAnimationName() == "RobotFalling" or GameObject.Animator.Clip:GetAnimationName() == "RobotJumping") and Physics.Raycast(GameObject.Transform.Position, Vec3.new(0,-1,0), 20).GameObject == other then
		GameObject.Rigidbody.Vel = Vec3.new(0,GameObject.Rigidbody.Vel.y,0)
		actionclip:SetAnimation("RobotLanding")
		GameObject.Animator.Clip = actionclip
		GameObject.Animator:Play()
	end
end

function OnCollisionStay(other) 

	if (GameObject.Animator.Clip:GetAnimationName() == "RobotFalling" ) and Physics.Raycast(GameObject.Transform.Position, Vec3.new(0,-1,0), 20).GameObject == other then
		
		GameObject.Rigidbody.Vel = Vec3.new(0,GameObject.Rigidbody.Vel.y,0)
		actionclip:SetAnimation("RobotLanding")
		GameObject.Animator.Clip = actionclip
		GameObject.Animator:Play()
	end


end



function OnCollisionExit(other)
    for i = #pressedUsers, 1, -1 do  -- iterate backwards so removal doesn't shift indices
        if pressedUsers[i] == other then  -- compare by reference, not name
            table.remove(pressedUsers, i)
            break  -- assuming one entry per actor
        end
    end

end
