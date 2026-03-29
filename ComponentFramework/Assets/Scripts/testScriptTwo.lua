
function Start()
	print("Hello from Script2!") 
end

function Update(deltaTime)
	Transform.Position = Transform.Position + new Vec3(0, 0, 1) 
end