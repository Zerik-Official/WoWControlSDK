#pragma once

namespace LuaBridge {

inline constexpr const char* SOURCE = R"lua(
local bridge = {}
local queueLimit = 256
local refLimit = 256

bridge.queue = {}
bridge.refs = {}
bridge.refCount = 0
bridge.nextRef = 0
bridge.hooks = {}

local function push(kind, name, addon, ...)
    local queue = bridge.queue
    if #queue >= queueLimit then
        bridge.dropped = (bridge.dropped or 0) + 1
        return
    end
    queue[#queue + 1] = { kind = kind, name = name, addon = addon, n = select("#", ...), ... }
end

local function resolve(path)
    local value = _G
    local walked
    for segment in string.gmatch(path, "[^%.]+") do
        if type(value) ~= "table" then
            error((walked or "_G") .. " is not a table", 0)
        end
        value = value[tonumber(segment) or segment]
        walked = walked and (walked .. "." .. segment) or segment
    end
    return value
end

local function parent(path)
    local ownerPath, key = string.match(path, "^(.+)%.([^%.]+)$")
    if not ownerPath then
        return _G, path
    end
    local owner = resolve(ownerPath)
    if type(owner) ~= "table" then
        error(ownerPath .. " is not a table", 0)
    end
    return owner, tonumber(key) or key
end

local function method(path)
    return string.match(path, "^(.+):([%a_][%w_]*)$")
end

local function owned(path)
    local objectPath, name = method(path)
    if not objectPath then
        return parent(path)
    end
    local object = resolve(objectPath)
    if object == nil then
        error(objectPath .. " is nil", 0)
    end
    return object, name
end

local function readOnly(methods)
    return setmetatable({}, {
        __index = methods,
        __newindex = function()
            error("SDK is read-only", 2)
        end,
        __metatable = false,
    })
end

function bridge.errorHandler(message)
    local stack
    if debugstack then
        stack = debugstack(2)
    end
    return { message = tostring(message), stack = stack }
end

function bridge.get(path)
    return resolve(path)
end

function bridge.set(path, value)
    local owner, key = parent(path)
    owner[key] = value
end

function bridge.call(path, ...)
    local objectPath, name = method(path)
    if objectPath then
        local object = resolve(objectPath)
        if object == nil then
            error(objectPath .. " is nil", 0)
        end
        local fn = object[name]
        if type(fn) ~= "function" then
            error(path .. " is not a function", 0)
        end
        return fn(object, ...)
    end
    local fn = resolve(path)
    if type(fn) ~= "function" then
        error(path .. " is not a function", 0)
    end
    return fn(...)
end

function bridge.callRef(id, name, ...)
    local object = bridge.refs[id]
    if object == nil then
        error("ref " .. tostring(id) .. " does not exist", 0)
    end
    if name then
        local fn = object[name]
        if type(fn) ~= "function" then
            error("ref " .. tostring(id) .. " has no method " .. name, 0)
        end
        return fn(object, ...)
    end
    if type(object) ~= "function" then
        error("ref " .. tostring(id) .. " is not a function", 0)
    end
    return object(...)
end

function bridge.ref(path)
    local value = resolve(path)
    if value == nil then
        error(path .. " is nil", 0)
    end
    if bridge.refCount >= refLimit then
        error("too many refs, release unused ones with lua.releaseRef", 0)
    end
    bridge.nextRef = bridge.nextRef + 1
    bridge.refs[bridge.nextRef] = value
    bridge.refCount = bridge.refCount + 1
    return bridge.nextRef, type(value)
end

function bridge.unref(id)
    if bridge.refs[id] == nil then
        return false
    end
    bridge.refs[id] = nil
    bridge.refCount = bridge.refCount - 1
    return true
end

function bridge.hook(path)
    if bridge.hooks[path] == nil then
        if type(hooksecurefunc) ~= "function" then
            error("hooksecurefunc is not available", 0)
        end
        local owner, key = owned(path)
        if type(owner) ~= "table" then
            error(path .. " has no owner table", 0)
        end
        if type(owner[key]) ~= "function" then
            error(path .. " is not a function", 0)
        end
        hooksecurefunc(owner, key, function(...)
            if bridge.hooks[path] then
                push("hook", path, nil, ...)
            end
        end)
    end
    bridge.hooks[path] = true
    return true
end

function bridge.unhook(path)
    if not bridge.hooks[path] then
        return false
    end
    bridge.hooks[path] = false
    return true
end

function bridge.hookList()
    local list = {}
    for path, active in pairs(bridge.hooks) do
        if active then
            list[#list + 1] = path
        end
    end
    table.sort(list)
    return list
end

function bridge.grant(namespace, addon)
    if type(namespace) ~= "table" or type(addon) ~= "string" then
        return false
    end
    local flag = GetAddOnMetadata and GetAddOnMetadata(addon, "X-WoWControlSDK")
    if not flag or flag == "" or flag == "0" then
        return false
    end

    local methods = { addon = addon }

    function methods.Emit(name, ...)
        if type(name) ~= "string" or name == "" or #name > 64 then
            error("Usage: SDK.Emit(name, ...) with a name of 1 to 64 characters", 2)
        end
        push("emit", name, addon, ...)
    end

    rawset(namespace, "SDK", readOnly(methods))
    return true
end

local errorWindow = 10
local errorMemory = 200
local errorSeen = {}
local errorCount = 0
local wrapping = false

local function reportError(message)
    local text = tostring(message)
    local now = GetTime and GetTime() or 0
    local last = errorSeen[text]
    if last and now - last < errorWindow then
        return
    end
    if not last then
        errorCount = errorCount + 1
        if errorCount > errorMemory then
            errorSeen = {}
            errorCount = 1
        end
    end
    errorSeen[text] = now
    local addon = string.match(text, "AddOns[\\/]([^\\/]+)[\\/]")
    local stack = debugstack and debugstack(3) or nil
    push("error", text, addon, stack)
end

local function wrapErrorHandler(handler)
    if type(handler) ~= "function" or handler == bridge.errorWrapper then
        return
    end
    local wrapper = function(message, ...)
        pcall(reportError, message)
        return handler(message, ...)
    end
    bridge.errorWrapper = wrapper
    wrapping = true
    seterrorhandler(wrapper)
    wrapping = false
end

if type(seterrorhandler) == "function" and type(geterrorhandler) == "function" then
    wrapErrorHandler(geterrorhandler())
    if type(hooksecurefunc) == "function" then
        hooksecurefunc("seterrorhandler", function(handler)
            if not wrapping then
                wrapErrorHandler(handler)
            end
        end)
    end
end

return bridge
)lua";

}
