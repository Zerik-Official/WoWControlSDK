#include "CharCache.h"
#include "client/GameClient.h"

static size_t s_detectedStride = 0;
static std::vector<CachedChar> s_cache;

static bool looksLikeCharData(const unsigned char* ptr)
{
    const uint64_t guid = *reinterpret_cast<const uint64_t*>(ptr);
    if (guid == 0) return false;

    const char* name = reinterpret_cast<const char*>(ptr + 8);
    if (name[0] == '\0') return false;

    for (int i = 0; i < 48; i++) {
        unsigned char c = (unsigned char)name[i];
        if (c == '\0') return true;
        if (c < 0x20)  return false;
    }
    return false;
}

static size_t detectStride(const LoginUI::CharVector* chars)
{
    if (chars->size < 2)
        return sizeof(LoginUI::CharVectorEntry);

    const unsigned char* base      = reinterpret_cast<const unsigned char*>(chars->buf);
    const size_t         minStride = sizeof(LoginUI::CharData);
    const size_t         maxStride = sizeof(LoginUI::CharData) + 128;

    for (size_t stride = minStride; stride <= maxStride; stride += 4) {
        if (!looksLikeCharData(base + stride)) continue;
        if (chars->size >= 3 && !looksLikeCharData(base + stride * 2)) continue;
        return stride;
    }

    return sizeof(LoginUI::CharVectorEntry);
}

static const LoginUI::CharData* getCharData(const LoginUI::CharVector* chars, int i)
{
    const unsigned char* base = reinterpret_cast<const unsigned char*>(chars->buf);
    return reinterpret_cast<const LoginUI::CharData*>(base + s_detectedStride * (size_t)i);
}

namespace CharCache {

void refresh()
{
    LoginUI::CharVector* chars = LoginUI::GetChars();
    if (!chars || chars->size <= 0) return;

    if (s_detectedStride == 0)
        s_detectedStride = detectStride(chars);

    s_cache.clear();
    s_cache.reserve(chars->size);

    for (int i = 0; i < chars->size; i++) {
        const LoginUI::CharData* d = getCharData(chars, i);
        CachedChar cc = {};
        strncpy_s(cc.name, sizeof(cc.name), d->name, _TRUNCATE);
        cc.level  = (int)(unsigned char)d->level;
        cc.class_ = (int)(unsigned char)d->class_;
        cc.race   = (int)(unsigned char)d->race;
        cc.gender = (int)(unsigned char)d->gender;
        cc.map    = d->map;
        cc.zone   = d->zone;
        s_cache.push_back(cc);
    }
}

void invalidate()
{
    s_cache.clear();
    s_detectedStride = 0;
}

const std::vector<CachedChar>& get()
{
    return s_cache;
}

}