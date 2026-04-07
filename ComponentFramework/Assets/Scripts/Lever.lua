PREFAB_NAME = "Cube"
 
leverStartRot = Quaternion.new()
leverEndRot   = Quaternion.new()
leverStartPos = Vec3.new()
leverEndPos   = Vec3.new()
 
leverOpenRange = 0.0
speed = 1.0
 
active = false
 
prefabInstance = nil
 
pressedUsers = {}
 
 
function Start()
	leverStartRot = GameObject:FindFirstChild("LeverPart").Transform.Rotation
	leverEndRot   = GameObject:FindFirstChild("LeverEnd").Transform.Rotation
	leverStartPos = GameObject:FindFirstChild("LeverPart").Transform.Position
	leverEndPos   = GameObject:FindFirstChild("LeverEnd").Transform.Position
end
 
 
function Update(deltaTime)
 
	local playerInRange = false
	for _, value in pairs(pressedUsers) do
		if value.Tag == "Player" then
			playerInRange = true
			break
		end
	end
 
	if playerInRange then
		if Game.Input.GetInputState("E") == 1 then
			active = not active
 
			if prefabInstance ~= nil then
				Game:Destroy(prefabInstance)
				prefabInstance = nil
			end
			prefabInstance = Game:Instantiate(PREFAB_NAME, Vec3.new(-14.49, 45.373, -23.548))
		end
	end
 
	if active then
		leverOpenRange = leverOpenRange + deltaTime * speed * 3
		if leverOpenRange >= 1 then leverOpenRange = 1 end
	else
		leverOpenRange = leverOpenRange - deltaTime * speed * 3
		if leverOpenRange <= 0 then leverOpenRange = 0 end
	end
 
	GameObject:FindFirstChild("LeverPart").Transform.Rotation = QMath.Slerp(leverStartRot, leverEndRot, leverOpenRange)
	GameObject:FindFirstChild("LeverPart").Transform.Position = VMath.Lerp(leverStartPos, leverEndPos, leverOpenRange)
 
end
 
 
function OnTriggerEnter(other)
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