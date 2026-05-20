-- Exemplo de script Lua para FractalEngine
-- Disponível: self.position, self.rotation, self.scale, self.name, self.active
-- Funções: OnStart(), OnUpdate(dt), OnDestroy()

local time = 0.0
local speed = 2.0
local amplitude = 1.5

function OnStart()
    print("Entidade iniciada: " .. self.name)
    time = 0.0
end

function OnUpdate(dt)
    time = time + dt

    -- Oscila no eixo Y
    self.position.y = math.sin(time * speed) * amplitude

    -- Gira lentamente no eixo Y
    self.rotation.y = self.rotation.y + 45.0 * dt
end

function OnDestroy()
    print("Entidade destruída: " .. self.name)
end
