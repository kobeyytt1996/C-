#include <string>
#include <string.h>

#include "bytearray.h"
#include "endian.h"

namespace yuan {

/**
 * 自定义的单链表Node的函数实现
 */
ByteArray::Node::Node(size_t s)
    : ptr(new char[s])
    , size(s)
    , next(nullptr) {}

ByteArray::Node::Node()
    : ptr(nullptr)
    , size(0)
    , next(nullptr) {}

ByteArray::Node::~Node() {
    if (ptr) {
        delete [] ptr;
    }
}

/**
 * ByteArray的所有方法实现
 */
ByteArray::ByteArray(size_t base_size)
    : m_baseSize(base_size)
    , m_position(0)
    , m_size(0)
    , m_capacity(base_size)
// 网络字节序默认是大端
    , m_endian(YUAN_BIG_ENDIAN)
    , m_root(new Node(base_size))
    , m_cur(m_root) {}

ByteArray::~ByteArray() {
    Node *temp = m_root;
    while (temp) {
        m_cur = temp;
        temp = temp->next;
        delete m_cur;
    }
}

void ByteArray::writeFint8(int8_t value) {
    write(&value, sizeof(value));
}

void ByteArray::writeFuint8(uint8_t value) {
    write(&value, sizeof(value));
}

void ByteArray::writeFint16(int16_t value) {
    // ByteArray需要的字节序和本地的不一样，需要做转换
    if (m_endian != YUAN_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFuint16(uint16_t value) {
    if (m_endian != YUAN_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFint32(int32_t value) {
    if (m_endian != YUAN_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFuint32(uint32_t value) {
    if (m_endian != YUAN_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFint64(int64_t value) {
    if (m_endian != YUAN_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

void ByteArray::writeFuint64(uint64_t value) {
    if (m_endian != YUAN_BYTE_ORDER) {
        value = byteswap(value);
    }
    write(&value, sizeof(value));
}

// 按照google压缩算法，负数压缩效果不好。所以转成正数。为和正数区分，则用奇偶区分。如1转为2，-1转为1
static uint32_t EncodeZigzag32(int32_t value) {
    if (value < 0) {
        // TODO:这里对于INT32_MIN存在问题
        return ((uint32_t)(-value)) * 2 - 1;
    } else {
        return value * 2;
    }
}

// 上面编码的解码
static int32_t DecodeZigzag32(uint32_t value) {
    // 重点：下面写法非常巧妙，负号即取补码。如果value为奇数，-(value & 1)即所有位都为1
    return (value >> 1) ^ -(value & 1);
}

static uint64_t EncodeZigzag64(int64_t value) {
    if (value < 0) {
        // TODO:这里对于INT64_MIN存在问题
        return ((uint64_t)(-value)) * 2 - 1;
    } else {
        return value * 2;
    }
}

static int64_t DecodeZigzag64(uint64_t value) {
    return (value >> 1) ^ -(value & 1);
}

void ByteArray::writeInt32(int32_t value) {
    uint32_t tmp = EncodeZigzag32(value);
    writeUint32(tmp);
}

void ByteArray::writeUint32(uint32_t value) {
    // 最极端情况，压缩后增加到5个字节
    uint8_t tmp[5];
    uint8_t i = 0;
    // 使用谷歌的压缩算法。看头文件里的注释
    while (value >= 0x80) {
        tmp[i++] = (value & 0x7f) | 0x80;
        value >>= 7;
    }
    tmp[i++] = value;
    write(tmp, i);
}

void ByteArray::writeInt64(int64_t value) {
    writeUint64(EncodeZigzag64(value));
}

void ByteArray::writeUint64(uint64_t value) {
    uint8_t tmp[10];
    uint8_t i = 0;
    while (value >= 0x80) {
        tmp[i++] = (value & 0x7f) | 0x80;
        value >>= 7;
    }
    tmp[i++] = value;
    write(tmp, i);
}

void ByteArray::writeFloat(float value) {
    // TODO:理论上将浮点型应该不需要转字节序，因为实际上当作数组存储:https://zhuanlan.zhihu.com/p/45874116
    // https://blog.csdn.net/yangshuanbao/article/details/6913623
    // 但这里保险起见，转成整形，再按整形write的方式
    uint32_t tmp;
    memcpy(&tmp, &value, sizeof(value));
    writeFuint32(tmp);
}

void ByteArray::writeDouble(double value) {
    uint64_t tmp;
    memcpy(&tmp, &value, sizeof(value));
    writeFuint64(tmp);
}

void ByteArray::writeStringF16(const std::string &value) {
    writeFuint16(value.size());
    write(value.c_str(), value.size());
}

void ByteArray::writeStringF32(const std::string &value) {
    writeFuint32(value.size());
    write(value.c_str(), value.size());
}

void ByteArray::writeStringF64(const std::string &value) {
    writeFuint64(value.size());
    write(value.c_str(), value.size());
}

void ByteArray::writeStringVint(const std::string &value) {
    writeUint64(value.size());
    write(value.c_str(), value.size());
}

void ByteArray::writeStringWithoutLength(const std::string &value) {
    write(value.c_str(), value.size());
}

int8_t ByteArray::readFint8() {
    int8_t value;
    read(&value, sizeof(value));
    return value;
}

uint8_t ByteArray::readFuint8() {
    uint8_t value;
    read(&value, sizeof(value));
    return value;
}

#define READ(type) \
    type value; \
    read(&value, sizeof(value)); \
    if (YUAN_BYTE_ORDER == m_endian) { \
        value = byteswap(value); \
    } \
    return value; 

int16_t ByteArray::readFint16() {
    READ(int16_t);
}

uint16_t ByteArray::readFuint16() {
    READ(uint16_t);
}

int32_t ByteArray::readFint32() {
    READ(int32_t);
}

uint32_t ByteArray::readFuint32() {
    READ(uint32_t);
}

int64_t ByteArray::readFint64() {
    READ(int64_t);
}

uint64_t ByteArray::readFuint64() {
    READ(uint64_t);
}

#undef READ

int32_t ByteArray::readInt32() {
    return DecodeZigzag32(readUint32());
}

uint32_t ByteArray::readUint32() {
    uint32_t result = 0;
    for (int i = 0; i < 32; i += 7) {
        uint8_t tmp = readFuint8();
        if (tmp < 0x80) {
            result |= ((uint32_t)tmp) << 7;
            break;
        } else {
            result |= ((uint32_t)(tmp & 0x7f)) << 7;
        }
    }
}

int64_t ByteArray::readInt64() {
    return DecodeZigzag64(readUint64());
}

uint64_t ByteArray::readUint64() {
    uint64_t result = 0;
    for (int i = 0; i < 64; i += 7) {
        uint8_t tmp = readFuint8();
        if (tmp < 0x80) {
            result |= ((uint64_t)tmp) << 7;
            break;
        } else {
            result |= ((uint64_t)(tmp & 0x7f)) << 7;
        }
    }
}

float ByteArray::readFloat() {
    uint32_t v = readFuint32();
    float value;
    memcpy(&value, &v, sizeof(v));
    return value;
}

double ByteArray::readDouble() {
    uint64_t v = readFuint64();
    double value;
    memcpy(&value, &v, sizeof(v));
    return value;
}

std::string ByteArray::readStringF16() {
    uint16_t len = readFuint16();
    std::string buff(len, '\0');
    read(&buff[0], len);
    return buff;
}

std::string ByteArray::readStringF32() {
    uint32_t len = readFuint32();
    std::string buff(len, '\0');
    read(&buff[0], len);
    return buff;
}

std::string ByteArray::readStringF64() {
    uint64_t len = readFuint64();
    std::string buff(len, '\0');
    read(&buff[0], len);
    return buff;
}

std::string ByteArray::readStringVint() {
    uint64_t len = readFuint64();
    std::string buff(len, '\0');
    read(&buff[0], len);
    return buff;
}

void ByteArray::clear() {
    m_position = m_size = 0;
    m_capacity = m_baseSize;

    m_cur = m_root->next;
    Node *temp;
    while (m_cur) {
        temp = m_cur;
        m_cur = m_cur->next;
        delete temp;
    }
    m_cur = m_root;
    m_root->next = nullptr;
}

void ByteArray::write(const void *buf, size_t size) {
    if (size == 0) {
        return;
    }

    addCapacity(size);
    size_t node_pos = m_position % m_baseSize;
    size_t node_cap = m_cur->size - node_pos;
    size_t buf_pos = 0;

    while (size > 0) {
        if (size <= node_cap) {
            memcpy(m_cur->ptr + node_pos, buf + buf_pos, size);
            if (node_cap == size) {
                m_cur = m_cur->next;
            }
            m_position += size;
            size = 0;
        } else {
            memcpy(m_cur->ptr + node_pos, buf + buf_pos, node_cap);
            m_cur = m_cur->next;
            m_position += node_cap;
            size -= node_cap;
            buf_pos += node_cap;
            node_pos = 0;
            node_cap = m_cur->size;
        }
    }

    if (m_position > m_size) {
        m_size = m_position;
    }
}

void ByteArray::read(void *buf, size_t size) {
    if (size > getReadSize()) {
        throw std::out_of_range("not enough len");
    }

    size_t node_pos = m_position % m_cur->size;
    size_t node_cap = m_cur->size - node_pos;
    size_t buf_pos = 0;

    while (size > 0) {
        if (size <= node_cap) {
            memcpy(buf + buf_pos, m_cur->ptr + node_pos, size);
            if (size == node_cap) {
                m_cur = m_cur->next;
            }
            m_position += size;
            size = 0;
        } else {
            memcpy(buf + buf_pos, m_cur->ptr + node_pos, node_cap);
            m_cur = m_cur->next;
            m_position += node_cap;
            size -= node_cap;
            node_pos = 0;
            buf_pos += node_cap;
            node_cap = m_cur->size;
        }
    }
}

void ByteArray::setPosition(size_t val) {

}

bool ByteArray::writeToFile(const std::string &name) const {

}

void ByteArray::readFromFile(const std::string &name) {

}

void ByteArray::addCapacity(size_t value) {

}

bool ByteArray::isLittleEndian() const {
    return m_endian == YUAN_LITTLE_ENDIAN;
}

void ByteArray::setIsLittleEndian(bool is_little) {
    m_endian = is_little ? YUAN_LITTLE_ENDIAN : YUAN_BIG_ENDIAN;
}

}