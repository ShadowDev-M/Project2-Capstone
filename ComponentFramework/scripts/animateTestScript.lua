

animclip = AnimationClip.new()

function Preload()
	animclip:PreloadAnimation("dancing")
    print("Preload called")
end	

function Start()

	animclip:SetAnimation("dancing")
	local printtest = animclip:GetAnimationName()
	print(printtest)

	Animator.Clip = animclip

	Animator:Play()
end

function Update(deltaTime) 

	--print(deltaTime)
	--print(Transform.Position)

	local testvalint = Input.GetInputState("W")
	if Input.GetInputState("W") == 2 then
		Rigidbody.Vel = Vec3.new(Rigidbody.Vel.x, 10, 0)
		--Transform.Position = Transform.Position + Vec3.new(0, 5 * deltaTime, 0) 
	end
	if Input.GetInputState("S") == 2 then
		Rigidbody.Vel = Vec3.new(Rigidbody.Vel.x, -10, 0)

		--Transform.Position = Transform.Position + Vec3.new(0, -5 * deltaTime, 0) 
	end
	if Input.GetInputState("A") == 2 then
		Rigidbody.Vel = Vec3.new(-10, Rigidbody.Vel.y, 0)

		--Transform.Position = Transform.Position + Vec3.new(-5 * deltaTime, 0, 0) 
	end
	if Input.GetInputState("D") == 2 then
		Rigidbody.Vel = Vec3.new(10, Rigidbody.Vel.y, 0)

		--Transform.Position = Transform.Position + Vec3.new(5 * deltaTime, 0, 0) 
	end
end