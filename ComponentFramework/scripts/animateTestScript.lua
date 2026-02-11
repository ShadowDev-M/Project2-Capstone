

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

	--Lets make a speed for our camera to move
	local speed = 5
	

	--"W" key input is held (2)
	if Game.Input.GetInputState("W") == 2 then
		--lets store our current UsedCamera's Position
		local originalPos = Game.UsedCamera.Transform.Position


		--lets set the UsedCamera's Position to the originalPos but shifted up in the y axis by the speed * deltaTime (To move in real time) 
		Game.UsedCamera.Transform.Position = Vec3.new(originalPos.x, originalPos.y + speed * deltaTime, originalPos.z)
	end

	--"S" key input is held (2)
	if Game.Input.GetInputState("S") == 2 then
		--lets store our current UsedCamera's Position
		local originalPos = Game.UsedCamera.Transform.Position

		--lets set the UsedCamera's Position to the originalPos but shifted down in the y axis by the speed * deltaTime (To move in real time) 
		Game.UsedCamera.Transform.Position = Vec3.new(originalPos.x, originalPos.y - speed * deltaTime, originalPos.z)

	end

	--"A" key input is held (2)
	if Game.Input.GetInputState("A") == 2 then
		--lets store our current UsedCamera's Position
		local originalPos = Game.UsedCamera.Transform.Position

		--lets set the UsedCamera's Position to the originalPos but shifted left in the x axis by the speed * deltaTime (To move in real time) 
		Game.UsedCamera.Transform.Position = Vec3.new(originalPos.x - speed * deltaTime, originalPos.y, originalPos.z)

	end

	--"D" key input is held (2)
	if Game.Input.GetInputState("D") == 2 then
		--lets store our current UsedCamera's Position
		local originalPos = Game.UsedCamera.Transform.Position

		--lets set the UsedCamera's Position to the originalPos but shifted right in the x axis by the speed * deltaTime (To move in real time) 
		Game.UsedCamera.Transform.Position = Vec3.new(originalPos.x + speed * deltaTime, originalPos.y, originalPos.z)

	end

end


