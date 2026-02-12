

animclip = AnimationClip.new()
animclip:PreloadAnimation("dancing")
local printtest = animclip:GetAnimationName()

function Preload()
	--animclip:PreloadAnimation("dancing")
  --  print("Preload called")
end	




function Start()



	animclip:SetAnimation("dancing")
	--local printtest = animclip:GetAnimationName()
	print(printtest)

	

	GameObject.Animator.Clip = animclip

	GameObject.Animator:Play()


	print(Game:Find("Cube").Transform.GameObject.Transform.Position)

end

--Rigidbody is the Physics Component of the script's user actor. Set and get the properties with Rigidbody.YOURVARIABLE 

--Game Handler Script 

function Update(deltaTime) 



end


