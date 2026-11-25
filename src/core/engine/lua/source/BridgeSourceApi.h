#pragma once

namespace LuaBridge {

inline constexpr const char* SOURCE_API = R"lua(
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

function bridge.grant(namespace, addon, data)
    if type(namespace) ~= "table" or type(addon) ~= "string" or string.sub(addon, 1, 1) == "@" then
        return false
    end
    local flag = GetAddOnMetadata and GetAddOnMetadata(addon, "X-WoWControlSDK")
    if not flag or flag == "" or flag == "0" then
        return false
    end
    bridge.storageInit(addon, data)

    local methods = { addon = addon }

    function methods.Emit(name, ...)
        if type(name) ~= "string" or name == "" or #name > 64 then
            error("Usage: SDK.Emit(name, ...) with a name of 1 to 64 characters", 2)
        end
        push("emit", name, addon, ...)
    end

    function methods.Print(text, r, g, b)
        return chatPrint(text, r, g, b)
    end

    function methods.RegisterCommand(names, handler)
        return registerCommand(addon, nil, names, handler)
    end

    function methods.UnregisterCommand(name)
        return unregisterCommand(addon, name)
    end

    function methods.Expose(name, handler)
        return expose(addon, name, handler)
    end

    addStorageMethods(methods, addon, function() end)

    rawset(namespace, "SDK", readOnly(methods))
    return true
end

)lua";

}
