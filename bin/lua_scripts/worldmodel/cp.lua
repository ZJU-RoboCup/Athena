module(..., package.seeall)

function specified(p)
	return function ()
		return p
	end
end

function full()
	return 1000
end

-- 纯闭包函数，这个函数只是用在开射门的条件中
-- role1 为接球车
function toPlayer(role1)
	return function(role2)
		local dist = player.toPlayerDist(role1, role2)
		if IS_SIMULATION then
			return dist*dist/980.0
		else
			return dist
		end
	end
end

function toTarget(p)
	return function()
		local tmpP
		if type(p) == "function" then
			tmpP = p()
		elseif type(p) == "userdata" then
			tmpP = p
		end

		local dist = ball.toPointDist(tmpP)
		--[[if IS_SIMULATION then
			return dist*dist/980.0
		else
			dist = dist * 0.6 - 38
			-- dist = dist * 0.4857 - 16.19
			if dist <= 60 then
				dist = 60
			end
			return dist
		end--]]
		return dist * 0.8
	end
end