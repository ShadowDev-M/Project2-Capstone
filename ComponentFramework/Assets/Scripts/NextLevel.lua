function Start()
end

function Update(deltaTime)
end

function OnTriggerEnter(other)
	if other.Tag == "Player" then
		Scene.LoadNext()
	end
end