#pragma once

namespace LuaBridge {

inline constexpr const char* SOURCE_STORAGE = R"lua(
local exposed = {}

local function expose(owner, name, handler)
    if type(name) ~= "string" or #name > 64 or not string.match(name, "^[%a_][%w_]*$")
        or type(handler) ~= "function" then
        error("Usage: SDK.Expose(name, function) with a Lua identifier of up to 64 characters", 3)
    end
    exposed[owner] = exposed[owner] or {}
    exposed[owner][name] = handler
    return true
end

function bridge.callExposed(owner, name, ...)
    local functions = exposed[owner]
    local handler = functions and functions[name]
    if type(handler) ~= "function" then
        error(tostring(owner) .. " does not expose " .. tostring(name), 0)
    end
    return handler(...)
end

local storage = {}
local storageDirty = {}
local storageListeners = {}
local storageKeyLimit = 128
local storageMaxKeys = 1000
local storageMaxDepth = 12
local storageMaxEntries = 2000

local function plainValue(value, depth, seen, counter)
    local kind = type(value)
    if kind == "string" or kind == "boolean" then
        return true
    end
    if kind == "number" then
        if value ~= value or value == math.huge or value == -math.huge then
            return false, "numbers must be finite"
        end
        return true
    end
    if kind ~= "table" then
        return false, "only strings, numbers, booleans and tables can be saved"
    end
    if depth > storageMaxDepth then
        return false, "tables are nested too deeply"
    end
    if seen[value] then
        return false, "a table cannot contain itself"
    end
    seen[value] = true
    for key, item in pairs(value) do
        counter.entries = counter.entries + 1
        if counter.entries > storageMaxEntries then
            return false, "the value has more than " .. storageMaxEntries .. " entries"
        end
        local keyKind = type(key)
        if keyKind ~= "string" and keyKind ~= "number" then
            return false, "table keys must be strings or numbers"
        end
        local ok, problem = plainValue(item, depth + 1, seen, counter)
        if not ok then
            return false, problem
        end
    end
    seen[value] = nil
    return true
end

local function copyValue(value)
    if type(value) ~= "table" then
        return value
    end
    local copy = {}
    for key, item in pairs(value) do
        copy[key] = copyValue(item)
    end
    return copy
end

local function storageOf(owner)
    local data = storage[owner]
    if not data then
        data = {}
        storage[owner] = data
    end
    return data
end

local function checkKey(key, usage)
    if type(key) ~= "string" or key == "" or #key > storageKeyLimit then
        error(usage .. " with a key of 1 to " .. storageKeyLimit .. " characters", 3)
    end
end

local function storageSave(owner, key, value)
    checkKey(key, "Usage: SDK.Save(key, value)")
    local data = storageOf(owner)
    if value == nil then
        if data[key] ~= nil then
            data[key] = nil
            storageDirty[owner] = true
        end
        return true
    end
    local ok, problem = plainValue(value, 0, {}, { entries = 0 })
    if not ok then
        error("SDK.Save: " .. problem, 3)
    end
    if data[key] == nil then
        local count = 0
        for _ in pairs(data) do
            count = count + 1
        end
        if count >= storageMaxKeys then
            error("SDK.Save: an addon can keep at most " .. storageMaxKeys .. " keys", 3)
        end
    end
    data[key] = copyValue(value)
    storageDirty[owner] = true
    return true
end

local function storageLoad(owner, key, default)
    checkKey(key, "Usage: SDK.Load(key[, default])")
    local value = storageOf(owner)[key]
    if value == nil then
        return default
    end
    return copyValue(value)
end

local function storageKeys(owner)
    local keys = {}
    for key in pairs(storageOf(owner)) do
        keys[#keys + 1] = key
    end
    table.sort(keys)
    return keys
end

local function storageListen(owner, callback)
    if type(callback) ~= "function" then
        error("Usage: SDK.OnDataChanged(function(key, value))", 3)
    end
    storageListeners[owner] = storageListeners[owner] or {}
    table.insert(storageListeners[owner], callback)
    return true
end

function bridge.storageInit(owner, data)
    if type(data) == "table" and storage[owner] == nil then
        storage[owner] = data
    end
end

function bridge.storageTakeDirty()
    local owners = {}
    for owner in pairs(storageDirty) do
        owners[#owners + 1] = owner
    end
    storageDirty = {}
    return owners
end

function bridge.storageOf(owner)
    return storage[owner] or {}
end

function bridge.storageSet(owner, key, value)
    if type(key) ~= "string" or key == "" or #key > storageKeyLimit then
        error("key must be a string of 1 to " .. storageKeyLimit .. " characters", 0)
    end
    if value ~= nil then
        local ok, problem = plainValue(value, 0, {}, { entries = 0 })
        if not ok then
            error(problem, 0)
        end
    end
    storageOf(owner)[key] = copyValue(value)
    for _, callback in ipairs(storageListeners[owner] or {}) do
        pcall(callback, key, copyValue(value))
    end
    return true
end

local function addStorageMethods(methods, owner, guard)
    function methods.Save(key, value)
        guard()
        return storageSave(owner, key, value)
    end

    function methods.Load(key, default)
        guard()
        return storageLoad(owner, key, default)
    end

    function methods.Delete(key)
        guard()
        return storageSave(owner, key, nil)
    end

    function methods.Keys()
        guard()
        return storageKeys(owner)
    end

    function methods.OnDataChanged(callback)
        guard()
        return storageListen(owner, callback)
    end
end

)lua";

}
