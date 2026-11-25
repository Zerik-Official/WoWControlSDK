#include "AddonStorage.h"
#include "core/engine/LuaEngine.h"
#include "rpc/handlers/core/LuaJson.h"
#include <Windows.h>
#include <algorithm>
#include <fstream>
#include <mutex>
#include <sstream>
#include <unordered_map>

namespace Runtime::AddonStorage
{
    namespace
    {
        constexpr ULONGLONG WRITE_INTERVAL_MS = 250;
        constexpr size_t MAX_OWNER_LENGTH = 64;
        constexpr size_t MAX_KEY_LENGTH = 128;
        constexpr const char* FOLDER = "\\WTF\\WoWControlSDK\\AddonData";

        struct Entry
        {
            SDK::Json data = SDK::Json::object();
            bool loaded = false;
            bool pending = false;
            ULONGLONG writtenAt = 0;
        };

        std::mutex s_mutex;
        std::unordered_map<std::string, Entry> s_entries;

        std::string gameDirectory()
        {
            char path[MAX_PATH] = {};
            DWORD length = GetModuleFileNameA(nullptr, path, MAX_PATH);
            std::string full(path, length);
            size_t slash = full.find_last_of("\\/");
            return slash == std::string::npos ? std::string(".") : full.substr(0, slash);
        }

        std::string filePath(const std::string& owner)
        {
            return directory() + "\\" + owner + ".json";
        }

        void ensureDirectory()
        {
            std::string path = gameDirectory();
            size_t start = 1;
            std::string folder = FOLDER;
            while (start < folder.size())
            {
                size_t next = folder.find('\\', start);
                if (next == std::string::npos)
                    next = folder.size();
                path += "\\" + folder.substr(start, next - start);
                CreateDirectoryA(path.c_str(), nullptr);
                start = next + 1;
            }
        }

        SDK::Json readFile(const std::string& owner)
        {
            std::ifstream file(filePath(owner), std::ios::binary);
            if (!file)
                return SDK::Json::object();
            std::stringstream text;
            text << file.rdbuf();
            SDK::Json data = SDK::Json::parse(text.str(), nullptr, false);
            return data.is_object() ? data : SDK::Json::object();
        }

        void writeFile(const std::string& owner, const SDK::Json& data)
        {
            std::string target = filePath(owner);
            if (data.empty())
            {
                DeleteFileA(target.c_str());
                return;
            }

            ensureDirectory();
            std::string temporary = target + ".tmp";
            {
                std::ofstream file(temporary, std::ios::binary | std::ios::trunc);
                if (!file)
                    return;
                file << data.dump(2, ' ', false, SDK::Json::error_handler_t::replace);
            }
            MoveFileExA(temporary.c_str(), target.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);
        }

        Entry& load(const std::string& owner)
        {
            Entry& entry = s_entries[owner];
            if (!entry.loaded)
            {
                entry.data = readFile(owner);
                entry.loaded = true;
            }
            return entry;
        }

        bool provide(const std::string& owner, LuaEngine::LuaValue& out)
        {
            if (!isValidOwner(owner))
                return false;
            std::lock_guard<std::mutex> lock(s_mutex);
            return Rpc::parseLuaValueJson(load(owner).data, out);
        }

        bool granted(const std::string& owner)
        {
            std::vector<std::string> addons = LuaEngine::grantedAddons();
            return std::find(addons.begin(), addons.end(), owner) != addons.end();
        }

        void writePending(bool force)
        {
            ULONGLONG now = GetTickCount64();
            std::lock_guard<std::mutex> lock(s_mutex);
            for (auto& [owner, entry] : s_entries)
            {
                if (!entry.pending || (!force && now - entry.writtenAt < WRITE_INTERVAL_MS))
                    continue;
                writeFile(owner, entry.data);
                entry.pending = false;
                entry.writtenAt = now;
            }
        }
    }

    bool isValidOwner(const std::string& owner)
    {
        if (owner.empty() || owner.size() > MAX_OWNER_LENGTH || owner.front() == '.' || owner.back() == '.'
            || owner.front() == ' ' || owner.back() == ' ' || owner.find("..") != std::string::npos)
            return false;
        for (unsigned char c : owner)
        {
            if (c < 0x20 || std::string("\\/:*?\"<>|").find(static_cast<char>(c)) != std::string::npos)
                return false;
        }
        return true;
    }

    std::string directory()
    {
        return gameDirectory() + FOLDER;
    }

    void initialize()
    {
        LuaEngine::setStorageProvider(provide);
    }

    void poll()
    {
        LuaEngine::LuaResult dirty = LuaEngine::takeDirtyStorage();
        if (dirty.ok && !dirty.values.empty() && dirty.values[0].type == LuaEngine::LuaType::Table)
        {
            for (const auto& item : dirty.values[0].items)
            {
                if (item.type != LuaEngine::LuaType::String || !isValidOwner(item.strVal))
                    continue;
                LuaEngine::LuaResult read = LuaEngine::readStorage(item.strVal);
                if (!read.ok || read.values.empty())
                    continue;
                SDK::Json data = Rpc::makeLuaValueJson(read.values[0]);
                if (!data.is_object())
                    data = SDK::Json::object();

                std::lock_guard<std::mutex> lock(s_mutex);
                Entry& entry = s_entries[item.strVal];
                entry.data = std::move(data);
                entry.loaded = true;
                entry.pending = true;
            }
        }
        writePending(false);
    }

    void flush()
    {
        writePending(true);
    }

    SDK::Json get(const std::string& owner)
    {
        std::lock_guard<std::mutex> lock(s_mutex);
        return load(owner).data;
    }

    std::string set(const std::string& owner, const std::string& key, const SDK::Json& value)
    {
        if (!isValidOwner(owner))
            return "addon must be the name of an addon";
        if (key.empty() || key.size() > MAX_KEY_LENGTH)
            return "key must be a string of 1 to 128 characters";

        LuaEngine::LuaValue luaValue;
        if (!Rpc::parseLuaValueJson(value, luaValue))
            return "value is nested too deeply";

        if (granted(owner))
        {
            LuaEngine::LuaResult result = LuaEngine::writeStorage(owner, key, luaValue);
            if (!result.ok)
                return result.error.empty() ? "the addon rejected the value" : result.error;
        }

        std::lock_guard<std::mutex> lock(s_mutex);
        Entry& entry = load(owner);
        if (value.is_null())
            entry.data.erase(key);
        else
            entry.data[key] = value;
        entry.pending = true;
        return {};
    }
}
