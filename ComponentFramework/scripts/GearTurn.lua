

degrees = 0


function Start()

end



function Update(deltaTime) 
	degrees =  degrees + 10 * deltaTime


	newRot = QMath.AngleAxisRotation(degrees, Transform.Rotation * Vec3.new(0,0,1))


	Transform.Rotation = newRot

end


