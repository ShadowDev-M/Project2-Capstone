

animclip = AnimationClip.new()
animclip:PreloadAnimation("dancing")

function Preload()
	--animclip:PreloadAnimation("dancing")
  --  print("Preload called")
end	



function Start()



	animclip:SetAnimation("dancing")
	local printtest = animclip:GetAnimationName()
	print(printtest)

	GameObject.Animator.Clip = animclip

	GameObject.Animator:Play()

	testobj = Game:Find("Cube")

	print(testobj.Transform.Position)

end

--Rigidbody is the Physics Component of the script's user actor. Set and get the properties with Rigidbody.YOURVARIABLE 

function Update(deltaTime) 
	--print(VMath.Distance(Vec3.new(1,1,0), Vec3.new()))

	--"W" key input is held (2)
	if Input.GetInputState("W") == 2 then
		--Set the actor's Rigidbody velocity to a new Vec3 that preserves the old x value but changes the y.
		--Rigidbody.Vel = Vec3.new(Rigidbody.Vel.x, 10, 0)
		oldPos = Game.UsedCamera:GetTransform().Position
		print(oldPos)
		Game.UsedCamera:GetTransform().Position = Vec3.new(oldPos.x, oldPos.y + 5 * deltaTime, oldPos.z)
	end

	--"S" key input is held (2)
	if Input.GetInputState("S") == 2 then
		--Set the actor's Rigidbody velocity to a new Vec3 that preserves the old x value but changes the y.
		--Rigidbody.Vel = Vec3.new(Rigidbody.Vel.x, -10, 0)
		oldPos = Game.UsedCamera:GetTransform().Position
		Game.UsedCamera:GetTransform().Position = Vec3.new(oldPos.x, oldPos.y - 5 * deltaTime, oldPos.z)

	end

	--"A" key input is held (2)
	if Input.GetInputState("A") == 2 then
		--Set the actor's Rigidbody velocity to a new Vec3 that preserves the old y value but changes the x.

		--Rigidbody.Vel = Vec3.new(-10, Rigidbody.Vel.y, 0)
		oldPos = Game.UsedCamera:GetTransform().Position
		Game.UsedCamera:GetTransform().Position = Vec3.new(oldPos.x - 5 * deltaTime, oldPos.y, oldPos.z)

	end

	--"D" key input is held (2)
	if Input.GetInputState("D") == 2 then
		--Set the actor's Rigidbody velocity to a new Vec3 that preserves the old y value but changes the x.
		--Rigidbody.Vel = Vec3.new(10, Rigidbody.Vel.y, 0)

		oldPos = Game.UsedCamera:GetTransform().Position
		Game.UsedCamera:GetTransform().Position = Vec3.new(oldPos.x + 5 * deltaTime, oldPos.y, oldPos.z)

	end

end


