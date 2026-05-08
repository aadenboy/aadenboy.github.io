local insdf = io.open(arg[1], "r")
s = insdf:read("*a")
io.close()
s = s:gsub("\n", " ")
local tokens = {name = "OUTER", inner = {}}
local current = tokens
local i = 1
repeat
	local g = s:sub(i)
	if g:match("^%([a-z]+") then
		local token = g:match("^%b()")
		local name, code = token:match("^%((%w+)%s*(.+)%)$")
		local object = {
			name = name,
			inner = {},
			parent = current
		}
		current.complex = true
		table.insert(current.inner, object)
		current = object
		i = i + 1 + #name
	elseif g:match("^%(%-?%d*%.?%d+") then
		local token = g:match("^%b()"):sub(2, -2)
		local object = {
			inner = {},
			parent = current
		}
		table.insert(current.inner, object)
		current = object
		i = i + 1
	elseif g:match("^%-?%d*%.?%d+") then
		table.insert(current.inner, g:match("%-?%d*%.?%d+"))
		i = i + #current.inner[#current.inner]
	elseif g:sub(1, 1) == ")" then
		current = current.parent
		i = i + 1
	else i = i + 1 end
until i > #s
local o = ""
function printer(object, tabs)
	for i,v in ipairs(object.inner) do
		if type(v) == "string" then
			o = o..(object.complex and "\n"..tabs or (i > 1 and " " or ""))..v
		else
			o = o..(object.complex and "\n"..tabs or (i > 1 and " " or "")).."("..(v.name and v.name.." " or "")
			printer(v, tabs.."  ")
			o = o..(object.complex and v.complex and "\n"..tabs or "")..")"
		end
	end
end
local d = ""
printer(tokens, "")
local f = io.open(arg[1]:gsub("%..+$", "_expanded%0"), "w")
f:write(o:sub(2))
io.close()
