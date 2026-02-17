
pressedUsers = {}




function Preload()

end	


function Start()

end



function Update(deltaTime) 
	
	if #(pressedUsers) > 0 then
		--print("open")
		Game:Find("Door3").Rigidbody.Vel = Vec3.new(0,3,0)
	end

	for i = 1, # (pressedUsers) do 
		--print("exists")
		--print(pressedUsers[i].Name)
	end

end


function OnTriggerEnter(other)

	--Add the colliding actor to the list
	table.insert(pressedUsers, other)

end

function OnTriggerExit(other)
	for i = 1, # (pressedUsers) do 

		if pressedUsers[i].Name == other.Name then
			print("removed")
			table.remove(pressedUsers, i)
		end
	end
end
