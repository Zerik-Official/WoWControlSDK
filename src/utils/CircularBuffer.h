#pragma once

#include <vector>
#include <algorithm>

namespace Utils
{

template<typename T>
class CircularBuffer
{
public:
    explicit CircularBuffer(int maxSize = 256)
        : m_maxSize(maxSize > 0 ? maxSize : 256)
    {
        m_buffer.reserve(m_maxSize);
    }

    void push(T item)
    {
        m_buffer.push_back(std::move(item));
        trim();
    }

    int size() const { return static_cast<int>(m_buffer.size()); }

    int maxSize() const { return m_maxSize; }

    void setMaxSize(int max)
    {
        m_maxSize = (max > 0) ? max : 256;
        m_buffer.reserve(m_maxSize);
        trim();
    }

    void clear()
    {
        m_buffer.clear();
        std::vector<T>().swap(m_buffer);
        m_buffer.reserve(m_maxSize);
    }

    std::vector<T> getRange(int offset, int count) const
    {
        std::vector<T> result;
        if (m_buffer.empty()) return result;

        int total = static_cast<int>(m_buffer.size());
        if (offset < 0) offset = 0;
        if (offset >= total) return result;
        if (count <= 0) count = total - offset;
        count = (std::min)(count, total - offset);

        result.reserve(count);
        for (int i = 0; i < count; i++)
            result.push_back(m_buffer[offset + i]);

        return result;
    }

private:
    void trim()
    {
        while (static_cast<int>(m_buffer.size()) > m_maxSize)
            m_buffer.erase(m_buffer.begin());
    }

    std::vector<T> m_buffer;
    int m_maxSize;
};

}
