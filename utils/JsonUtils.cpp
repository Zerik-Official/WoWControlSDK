#include "JsonUtils.h"
#include <cstdio>

namespace JsonUtils {

std::string escape(const char* src)
{
    std::string out;
    out.reserve(64);
    for (const unsigned char* p = (const unsigned char*)src; *p; ++p) {
        unsigned char c = *p;
        if      (c == '"')  { out += "\\\""; }
        else if (c == '\\') { out += "\\\\"; }
        else if (c == '\n') { out += "\\n";  }
        else if (c == '\r') { out += "\\r";  }
        else if (c == '\t') { out += "\\t";  }
        else if (c < 0x20) {
            char esc[8];
            snprintf(esc, sizeof(esc), "\\u%04X", (unsigned)c);
            out += esc;
        }
        else { out += (char)c; }
    }
    return out;
}

std::string getString(const char* json, const char* key)
{
    std::string needle = "\"";
    needle += key;
    needle += "\"";

    const char* pos = strstr(json, needle.c_str());
    if (!pos) return {};

    pos += needle.size();
    while (*pos == ' ' || *pos == '\t') ++pos;
    if (*pos != ':') return {};
    ++pos;
    while (*pos == ' ' || *pos == '\t') ++pos;

    if (*pos == '"') {
        ++pos;
        std::string val;
        while (*pos && *pos != '"') {
            if (*pos == '\\' && *(pos + 1)) {
                ++pos;
                switch (*pos) {
                case '"':  val += '"';  break;
                case '\\': val += '\\'; break;
                case 'n':  val += '\n'; break;
                case 'r':  val += '\r'; break;
                case 't':  val += '\t'; break;
                default:   val += *pos; break;
                }
            } else {
                val += *pos;
            }
            ++pos;
        }
        return val;
    }

    std::string val;
    while (*pos && *pos != ',' && *pos != '}' && *pos != ' ') {
        val += *pos++;
    }
    return val;
}

}