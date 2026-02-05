

local animclip 
local animclip2

function Start()
	animclip = AnimationClip.new()
	animclip2 = AnimationClip.new()

	animclip:SetAnimation("dancing")
	local printtest = animclip:GetAnimationName()
	print(printtest)

	Animator.Clip = animclip



	Animator:Play()
end

function Update(deltaTime)

end