-- Rotação contínua simples
local rotSpeed = { x = 0.0, y = 90.0, z = 0.0 }

function OnStart()
    print("Rotate script iniciado em: " .. self.name)
end

function OnUpdate(dt)
    self.rotation.x = self.rotation.x + rotSpeed.x * dt
    self.rotation.y = self.rotation.y + rotSpeed.y * dt
    self.rotation.z = self.rotation.z + rotSpeed.z * dt
end
