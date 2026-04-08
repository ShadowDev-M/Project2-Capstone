function Start()
end

function Update(deltaTime)
end

function OnTriggerEnter(other)
	if other.Tag == "Player" then
		Game:Destroy(other)
		Scene.Load(Scene.GetActiveSceneName())
	end
end
