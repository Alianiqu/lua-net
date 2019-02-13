--[[
  #!/usr/bin/lua

  local lua = require('net')

  lua.setip('192.168.3.1', '255.255.255.0', 'eth0')
  lua.setroute('0.0.0.0', '0.0.0.0', '192.168.3.2', 'eth0', 50)

]]

local net = require("net.core")

local M = {
  setip = net.setip,
  setroute = net.setroute
}

return M
