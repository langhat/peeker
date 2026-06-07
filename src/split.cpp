#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <cctype>
#include <cstdint>
#include <cstring>

// 字符类别枚举
enum class CharType {
    SPACE,      // 空格、制表符等空白
    PUNCT_TERM, // 句号 . 或 。
    LETTER,     // 英文字母
    DIGIT,      // 数字
    CJK,        // 汉字（CJK统一表意文字）
    OTHER       // 其他字符
};

// 判断是否是中文字符（主范围）
bool isChineseChar(uint32_t cp) {
    return (cp >= 0x4E00 && cp <= 0x9FFF) ||
           (cp >= 0x3400 && cp <= 0x4DBF) ||
           (cp >= 0x20000 && cp <= 0x2A6DF);
}

// 获取字符类别
CharType getCharType(uint32_t cp) {
    if (cp == ' ' || cp == '\t' || cp == '\n' || cp == '\r')
        return CharType::SPACE;
    // 句号：. 或 中文句号 0x3002 或 0xFF0E 等，我们直接比较 UTF-8 字节序列会更简单
    if (cp == '.')
        return CharType::PUNCT_TERM;
    // 由于无法直接写 '。'，我们在 decodeUTF8 里返回特殊标记，这里用数值判断
    if (cp == 0x3002 || cp == 0xFF0E)  // 中文句号 '。' 和 全角句号
        return CharType::PUNCT_TERM;
    
    if ((cp >= 'a' && cp <= 'z') || (cp >= 'A' && cp <= 'Z'))
        return CharType::LETTER;
    if (cp >= '0' && cp <= '9')
        return CharType::DIGIT;
    if (isChineseChar(cp))
        return CharType::CJK;
    return CharType::OTHER;
}

// 从 UTF-8 字符串解码下一个字符，返回 (码点, 下一个位置)
std::pair<uint32_t, size_t> decodeUTF8(const std::string &s, size_t pos) {
    if (pos >= s.size()) return {0, pos};
    unsigned char c = s[pos];
    uint32_t cp = 0;
    size_t len = 0;
    if ((c & 0x80) == 0) {           // ASCII
        cp = c;
        len = 1;
    } else if ((c & 0xE0) == 0xC0) { // 2字节
        cp = c & 0x1F;
        len = 2;
    } else if ((c & 0xF0) == 0xE0) { // 3字节
        cp = c & 0x0F;
        len = 3;
    } else if ((c & 0xF8) == 0xF0) { // 4字节
        cp = c & 0x07;
        len = 4;
    } else {
        return {0, pos + 1}; // 无效
    }
    for (size_t i = 1; i < len && pos + i < s.size(); ++i) {
        if ((s[pos + i] & 0xC0) != 0x80) return {0, pos + i};
        cp = (cp << 6) | (s[pos + i] & 0x3F);
    }
    return {cp, pos + len};
}

// 分词主函数
std::pair<std::vector<std::string>, size_t> tokenize(const std::string &text, size_t start = 0) {
    std::vector<std::string> tokens;
    size_t pos = start;
    std::string current_token;
    CharType last_type = CharType::OTHER;

    while (pos < text.size()) {
        auto res = decodeUTF8(text, pos);
        uint32_t cp = res.first;
        size_t next_pos = res.second;
        if (cp == 0) { // 无效字符，跳过
            pos = next_pos;
            continue;
        }

        CharType cur_type = getCharType(cp);

        // 遇到句号（包括 '.' 和 '。'）
        if (cur_type == CharType::PUNCT_TERM) {
            if (!current_token.empty()) {
                tokens.push_back(current_token);
                current_token.clear();
            }
            // 将句点字符本身作为 token 加入
            tokens.push_back(std::string(text.begin() + pos, text.begin() + next_pos));
            return {tokens, next_pos};
        }

        // 遇到空格：保存当前 token，跳过空格
        if (cur_type == CharType::SPACE) {
            if (!current_token.empty()) {
                tokens.push_back(current_token);
                current_token.clear();
            }
            pos = next_pos;
            last_type = cur_type;
            continue;
        }

        // 字符类型切换：保存之前的 token
        if (!current_token.empty() && cur_type != last_type) {
            tokens.push_back(current_token);
            current_token.clear();
        }

        // 将当前字符追加到当前 token
        current_token.append(text.begin() + pos, text.begin() + next_pos);
        last_type = cur_type;
        pos = next_pos;
    }

    if (!current_token.empty()) {
        tokens.push_back(current_token);
    }
    return {tokens, pos};
}

std::vector<std::string> split(const std::string &text) {
    auto [tokens, next_pos] = tokenize(text, 0);
    if (!tokens.empty() && (tokens.back() == "." || tokens.back() == "。")) {
        tokens.pop_back();
    }
    return tokens;
}

std::vector<std::vector<std::string>> split2(const std::string &text) {
    std::vector<std::vector<std::string>> result;
    size_t pos = 0;
    while (pos < text.size()) {
        auto [tokens, next_pos] = tokenize(text, pos);
        if (tokens.empty()) {
            break; // 没有更多内容
        }
        // tokens 的最后一个元素必然是句号（根据 tokenize 的设计），移除它
        if (!tokens.empty() && (tokens.back() == "." || tokens.back() == "。")) {
            tokens.pop_back();
        }
        // 只有当 tokens 非空时才加入结果
        if (!tokens.empty()) {
            result.push_back(tokens);
        }
        pos = next_pos;
        // 跳过可能存在的空格或换行
        while (pos < text.size() && (text[pos] == ' ' || text[pos] == '\t' || text[pos] == '\n' || text[pos] == '\r')) {
            ++pos;
        }
    }
    return result;
}