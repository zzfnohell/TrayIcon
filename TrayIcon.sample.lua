local core = require("core")

core:seticonon([[.\icons\cow-on.ico]])
core:seticonoff([[.\icons\cow-off.ico]])
core:setapppath([[C:\Windows\System32\cmd.exe]])
core:setappargs([[ /K]])
core:setappworkdir(".")
core:setapphide(true)

local name = "PATH"
local val = os.getenv(name)
core:setenv(name,val)
core:output(val)