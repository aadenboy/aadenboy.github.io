-- this is messily written but whatever
local last = io.open("eotlast", "r")
local u = os.time()
local lt = tonumber(last:read("*n"))
last:close()
local f = "!%H:%M:%S, %d %b %Y"
local np = math.floor(math.ceil(u/2147483647*100)/100*(2^31-1))
local pdiff = np-u
local d = os.date
function plural(n, s) return n.." "..s:sub(1, n == 1 and -2 or -1) end
local ds = {}
if pdiff       > 86400 then ds[#ds+1] = plural(math.floor(pdiff      /86400), "days")    end
if pdiff%86400 > 3600  then ds[#ds+1] = plural(math.floor(pdiff%86400/3600 ), "hours") end
if pdiff%3600  > 60    then ds[#ds+1] = plural(math.floor(pdiff%3600 /60   ), "minutes")   end
if pdiff%60    > 0     then ds[#ds+1] = plural(math.floor(pdiff%60         ), "seconds") end
print(
    ("As of %s, we are %.12f%% to the end of time%s. The next percentile is in %s on %s.")
    :format(
        d(f, u),
        u/2147483647*100,
        lt and (", which has gone up %.12f%% since this was last ran on %s")
            :format(
                (u-lt)/2147483647*100,
                d(f, lt)
            ) or "",
        table.concat(ds, ", "):gsub(", ([^,]+)$", ", and %1"),
        d(f, np)
    )
)
last = io.open("eotlast", "w")
last:write(tostring(u))
last:close()