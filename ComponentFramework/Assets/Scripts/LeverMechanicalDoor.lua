startRot = Quaternion.new()
endRot = Quaternion.new()


leverStartRot = Quaternion.new()
leverEndRot = Quaternion.new()
leverStartPos = Vec3.new()
leverEndPos = Vec3.new()


speed = 1.0
openRange = 0.0
leverOpenRange = 0.0

pressedUsers = {}

active = false

function Start()

	startRot = GameObject.Parent:FindFirstChild("DoorPart").Transform.Rotation	

	endRot = GameObject.Parent:FindFirstChild("DoorEndRot").Transform.Rotation	

	leverStartRot = GameObject:FindFirstChild("LeverPart").Transform.Rotation	

	leverEndRot = GameObject:FindFirstChild("LeverEnd").Transform.Rotation	

	leverStartPos = GameObject:FindFirstChild("LeverPart").Transform.Position	

	leverEndPos = GameObject:FindFirstChild("LeverEnd").Transform.Position	



end

function Update(deltaTime)

	for key, value in pairs(pressedUsers) do
		--Use hitbox as range, and then check for the keypress
		if value.Tag == "Player" then
			if Game.Input.GetInputState("E") == 1 then
				active = not active
			end
			if Game.Input.GetInputState("F") == 1 then
				active = false
			end
		end

	end

	local change = false
	local leverChange = false


	--opening
	if active then

		--Lever
		if leverOpenRange >= 1 then 
			leverOpenRange = 1
		else
			leverChange = true
			leverOpenRange = leverOpenRange + deltaTime * speed * 3
		end
		
		--Door
		if openRange >= 1 then 
			openRange = 1
		else
			change = true
			openRange = openRange + deltaTime * speed
		end
	end

	--closing
	if not active then

		--Lever
		if leverOpenRange <= 0 then
			leverOpenRange = 0
		else
			leverChange = true
			leverOpenRange = leverOpenRange - deltaTime * speed * 3
		end

		--Door
		if openRange <= 0 then
			openRange = 0
		else
			change = true
			openRange = openRange - deltaTime * speed
		end
		
	end
		
	--Update door part
	if change == true then
		GameObject.Parent:FindFirstChild("DoorPart").Transform.Rotation = QMath.Slerp(startRot, endRot, openRange)
	end

	if leverChange == true then
		GameObject:FindFirstChild("LeverPart").Transform.Rotation = QMath.Slerp(leverStartRot, leverEndRot, leverOpenRange)
		GameObject:FindFirstChild("LeverPart").Transform.Position = VMath.Lerp(leverStartPos, leverEndPos, leverOpenRange)

	end

end


function OnTriggerEnter(other)

	--Add the colliding actor to the list
	table.insert(pressedUsers, other)

end

function OnTriggerExit(other)
    for i = #pressedUsers, 1, -1 do 
        if pressedUsers[i] == other then  
            table.remove(pressedUsers, i)
            break 
        end
    end

end
