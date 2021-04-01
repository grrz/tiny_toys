function love.load()
	love.keyboard.setKeyRepeat(true)
	love.window.setMode(800, 600, {resizable=true, vsync=true, minwidth=400, minheight=300})

	infox = 10
	infoy = 10

	love.graphics.setPointSize(1)
	-- love.graphics.setPointStyle("smooth")

	rA = 250
	rroll = 50
	rpoint = 1

	rrstep = 2.5
	rpstep = 0.05

	pi = math.pi
	pi2 = math.pi * 2
end

function love.draw()

	xc = math.floor(love.graphics.getWidth() / 2)
	yc = math.floor(love.graphics.getHeight() / 2)

	x1 = xc + rA + math.floor(rroll * (rpoint - 1))
	y1 = yc

	love.graphics.setColor(0, 255, 0)

	for t = 0, 200, 0.01 do

		t1 = rroll * (pi2 * t - rpoint * math.sin(pi2 * t))
		Rr = rA + rroll * (rpoint * math.cos(pi2 * t) - 1)

	    x = xc + math.floor(Rr * math.cos(t1 / rA))
	    y = yc + math.floor(Rr * math.sin(t1 / rA))

	    love.graphics.line(x1, y1, x, y)

	    x1 = x
	    y1 = y	    
	end

	love.graphics.setColor(127, 127, 127)
    love.graphics.print(string.format("rroll: %i\nrpoint: %f", rroll, rpoint), infox, infoy)
end

function love.keypressed (key, isrepeat) 
	if key == "left" then
		rpoint = rpoint - rpstep
	elseif key == "right" then
		rpoint = rpoint + rpstep
	elseif key == "up" then
		rroll = rroll + rrstep
	elseif key == "down" then
		rroll = rroll - rrstep
	else return end
end