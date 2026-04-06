function Start()
end

function Update(deltaTime)
end

function OnTriggerEnter(other)
	if other.Tag == "Player" then
		--print(Scene:GetActiveSceneId())
		--print(Scene:GetActiveSceneName())
		Game:Destroy(other)
		Scene:Load(Scene:GetActiveSceneName())
	end
end
