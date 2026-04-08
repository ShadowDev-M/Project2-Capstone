startRot = Quaternion.new()
endRot = Quaternion.new()


speed = 1.0
openRange = 0.0
pressedUsers = {}

lightInstance = nil

function Start()
	--print("test")

	startRot = GameObject.Parent:FindFirstChild("DoorPart").Transform.Rotation	

	endRot = GameObject.Parent:FindFirstChild("DoorEndRot").Transform.Rotation	


	
end

function Update(deltaTime)
	local change = false
	
	local isActive = #pressedUsers > 0
	
		if isActive and lightInstance == nil then
			lightInstance = Game:Instantiate("LightPackage")
		end

		if not isActive and lightInstance ~= nil then
			Game:Destroy(lightInstance)
			lightInstance = nil
		end

	if isActive then
		if openRange >= 1 then
			openRange = 1
		else
			change = true
			openRange = openRange + deltaTime * speed
		end
	end
 
	if not isActive then
		if openRange <= 0 then
			openRange = 0
		else
			change = true
			openRange = openRange - deltaTime * speed
		end
	end
 
	if change == true then
		GameObject.Parent:FindFirstChild("DoorPart").Transform.Rotation = QMath.Slerp(startRot, endRot, openRange)
	end
end


function OnTriggerEnter(other)

	--Add the colliding actor to the list
	table.insert(pressedUsers, other)

end

function OnTriggerExit(other)
    for i = #pressedUsers, 1, -1 do  -- iterate backwards so removal doesn't shift indices
        if pressedUsers[i] == other then  -- compare by reference, not name
            table.remove(pressedUsers, i)
            break  -- assuming one entry per actor
        end
    end

end
