degrees = 0
public speed = 10

function Start()

end



function Update(deltaTime) 
	degrees =  degrees + speed * deltaTime

	if degrees >= 180 then
		degrees = -180
		degrees =  degrees + speed * deltaTime
	elseif degrees <= -180 then
		degrees = 180
		degrees =  degrees + speed * deltaTime
	end


	newRot = QMath.AngleAxisRotation(degrees, Transform.Rotation * Vec3.new(0,0,1))


	Transform.Rotation = newRot

end


