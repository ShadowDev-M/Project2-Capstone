

animclip = AnimationClip.new()
animclip:PreloadAnimation("walking")
animclip:PreloadAnimation("idle2")


actionclip = AnimationClip.new()
actionclip.Loop = false


local printtest = animclip:GetAnimationName()

function Preload()
	--animclip:PreloadAnimation("dancing")
  --  print("Preload called")
end	




function Start()



	animclip:SetAnimation("idle2")
	--local printtest = animclip:GetAnimationName()
	print(printtest)

	

	GameObject.Animator.Clip = animclip

	GameObject.Animator:Play()


	print(Game:Find("Cube").Transform.GameObject.Transform.Position)

end

--Rigidbody is the Physics Component of the script's user actor. Set and get the properties with Rigidbody.YOURVARIABLE 

--Game Handler Script 

function Update(deltaTime) 
	
	
	if GameObject.Animator.Clip:GetAnimationName() == "jump" and GameObject.Animator.CurrentTime == GameObject.Animator.Length then
		animclip:SetAnimation("falling")
		GameObject.Animator.Clip = animclip
		GameObject.Animator:Play()
	end
	
	if GameObject.Animator.Clip:GetAnimationName() == "land" and GameObject.Animator.CurrentTime == GameObject.Animator.Length then
		animclip:SetAnimation("idle2")
		GameObject.Animator.Clip = animclip
		GameObject.Animator:Play()
	end


	if math.abs(GameObject.Rigidbody.Vel.x) > 0.01 then 
		GameObject.Animator.SpeedMult = (math.abs(GameObject.Rigidbody.Vel.x) / 5) + 0.3
		
		if GameObject.Animator.Clip:GetAnimationName() == "idle2" then
			print(GameObject.Animator.Clip:GetAnimationName())
		end
		if GameObject.Animator.Clip:GetAnimationName() == "idle2" then
			animclip:SetAnimation("walking")
			GameObject.Animator.Clip = animclip
			GameObject.Animator:Play()

		end
	else
		
		if GameObject.Animator.Clip:GetAnimationName() == "walking" and ((GameObject.Animator.CurrentTime < 0.3 or GameObject.Animator.CurrentTime > (GameObject.Animator.Length - 0.3)) or (GameObject.Animator.CurrentTime > 0.5 and GameObject.Animator.CurrentTime < 0.6)) then
			animclip:SetAnimation("idle2")
			GameObject.Animator.Clip = animclip
			GameObject.Animator:Play()
		end
	end

end

function OnCollisionEnter(other) 
	if GameObject.Animator.Clip:GetAnimationName() == "falling" then
		actionclip:SetAnimation("land")
		GameObject.Animator.Clip = actionclip
		GameObject.Animator:Play()
	end
end


