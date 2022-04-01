//see license (Apache)
#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <cassert>

typedef std::vector<uint32_t> dstring;

struct UTF {
    int errors = 0;
/**
 int errambig: check ambiguity against hacker attacks
 some chars , for example ASCII / can be expressed as:
    00101111
    11000000 10101111
    11100000 10000000 10101111i
    etc..
 * */
    int errambig = 0;
    const int MaxCP = 0x10ffff;

    int one8len(char c)
    {
        uint8_t b0 = c;
        if ((b0 & 0x80) == 0)
            return 1;
        else if ((b0 & 0x20) == 0)
            return 2;
        else if ((b0 & 0x10) == 0)
            return 3;
        return 4;
    }

    int one8len(uint32_t d) {
        if (d <= 0x7f)
            return 1;
        else if (d <= 0x7ff)
            return 2;
        else if (d <= 0xffff)
            return 3;
        else
            return 4;
    }

    int one16len(uint32_t d) {
        if (d < 0x10000)
            return 1;
        else
            return 2;
    }

    static bool isSurrogate1(int w) {
        return w >= 0xD800 && w <= 0xDBFF;
    }
    static bool isSurrogate2(int w) {
        return w >= 0xDC00 && w <= 0xDFFF;
    }

    static bool isSurrogate(int w) {
        return isSurrogate1(w) || isSurrogate2(w);
    }

    int one16len(wchar_t wc)
    {
        uint16_t w = (uint16_t)wc;
        if (w >= 0xD800 && w <= 0xDBFF)
            return 2;
        else
            return 1;
    }

    uint32_t one8to32(const char *s, const char **end) {
        *end = s;
        uint8_t b0 = **end;
        (*end)++;
        if ((b0 & 0x80) == 0)
            return b0;
        else if ((b0 & 0x20) == 0) {
            uint8_t b1 = **end;
            (*end)++;
            if ((b1 & 0b11000000) != 128) {
                errors++;
                return 0xfffd;
            } else {
                if (!(b0 & 31)){
                    errambig++;
                    return 0xfffd;
                }
                return (b1 & 63) | ((int) (b0 & 31) << 6);
            }
        } else if ((b0 & 0x10) == 0) {
            uint8_t b1 = **end;
            (*end)++;
            uint8_t b2 = **end;
            (*end)++;
            if ((b1 & 0b11000000) != 128 || (b2 & 0b11000000) != 128) {
                errors++;
                return 0xfffd;
            } else {
                if (!(b0 & 15)&& !(b1 & 63)) {
                    errambig++;
                    return 0xfffd;
                }
                return (b2 & 63) | ((int) (b1 & 63) << 6) | ((int) (b0 & 15) << 12);
            }
        } else if ((b0 & 0x08) == 0) {
            uint8_t b1 = **end;
            (*end)++;
            uint8_t b2 = **end;
            (*end)++;
            uint8_t b3 = **end;
            (*end)++;
            if ((b1 & 0b11000000) != 128 || (b2 & 0b11000000) != 128 || (b3 & 0b11000000) != 128) {
                errors++;
                return 0xfffd;
            } else {
                if (!(b0 & 7)&& !(b1 & 63) && !(b2 & 63)) {
                    errambig++;
                    return 0xfffd;
                }
                return (b3 & 63) | ((int) (b2 & 63) << 6) | ((int) (b1 & 63) << 12) | ((int) (b0 & 7) << 18);
            }
        } else {
            errors++;
            return 0xfffd;
        }
    }

    int one32to16(int d, wchar_t *buf)
    {
        if (isSurrogate(d) || d>MaxCP) {
            d = 0xFFFD;
            errors++;
        }
        if (d < 0x10000)
        {
            buf[0] = (wchar_t)d;
            return 1;
        }
        else
        {
            buf[0] = (wchar_t)((d - 0x10000) / 0x400 + 0xD800);
            buf[1] = (wchar_t)((d - 0x10000) % 0x400 + 0xDC00);
            return 2;
        }
    }

    int getU32Len(const std::string &str) {
        int result = 0;
        int n =  0;
        while (n<str.size()) {
            result++;
            n += one8len(str[n]);
        }
        return result;
    }

    int getU32Len(const std::wstring &wstr) {
        int result = 0;
        int n =  0;
        while (n<wstr.size()) {
            result++;
            n+= one16len(wstr[n]);
        }
        return result;
    }

    int getU16Len(const std::string &str) {
        int result = 0;
        const char *s = str.c_str();
        const char *end = s;
        while (end-s<str.size()) {
            uint32_t d = one8to32(s,&end);
            result += one16len(d);
        }
        return result;
    }

    int getU8Len(const std::wstring &wstr) {
        const wchar_t *ws = wstr.c_str();
        const wchar_t *wend = ws;
        int len = 0;
        while (wend-ws<wstr.size()) {
            uint32_t d = one16to32(ws,&wend);
            char buf[10];
            len += one32to8(d, buf);
        }
        return len;
    }

    int getU8Len(const dstring &dstr) {
        int len8 = 0;
        for (int i=0; i<dstr.size(); i++)
            len8+= one8len(dstr[i]);
        return len8;
    }

    int getU16Len(const dstring &dstr) {
        int len16 = 0;
        for (int i=0; i<dstr.size(); i++)
            if (dstr[i]<0x10000)
                len16++;
            else
                len16+=2;
        return len16;
    }

    std::wstring u8to16(const std::string &str) {
        std::wstring result;
        result.resize(getU16Len(str));
        return result;
    }

    dstring u8to32(const std::string &str) {
        const char *cs = str.c_str();
        dstring result(getU32Len(str));
        for (int i=0; i<result.size(); i++) {
            result[i] = one8to32(cs, &cs);
        }
        assert(cs==str.c_str()+str.size());
        return result;
    }

    std::string u16to8(const std::wstring &wstr) {
        std::string result;
        result.resize(getU8Len(wstr));
        const wchar_t *ws = wstr.c_str();
        const wchar_t *wend = ws;
        int len = 0;
        while (wend-ws<wstr.size()) {
            uint32_t d = one16to32(ws,&wend);
            char buf[10];
            int k = one32to8(d, buf);
            for (int i=0; i<k; i++) {
                result[len] = buf[i];
                len++;
            }
        }
        return result;
    }

    int one16to32(const wchar_t *text, const wchar_t **end)
    {
        *end = text;
        uint16_t w1 = (uint16_t)**end;
        (*end)++;
        if (w1 >= 0xD800 && w1 <= 0xDBFF)
        {
            uint16_t w2 = (uint16_t)**end;
            (*end)++;
            return 0x400*((int)w1 - 0xD800) + ((int)w2 - 0xDC00) + 0x10000;
        }
        else
            return w1;
    }


    dstring u16to32(const std::wstring &wstr) {
        const wchar_t *cws = wstr.c_str();
        dstring result(getU32Len(wstr));
        for (int i=0; i<result.size(); i++) {
            result[i] = one16to32(cws, &cws);
        }
        return result;
    }

    int one32to8(int d, char *buf)
    {
        if (isSurrogate(d) || d>MaxCP) {
            d = 0xFFFD;
            errors++;
        }
        if (d <= 0x7f)
        {
            buf[0] = (char)d;
            return 1;
        }
        else if (d <= 0x7ff)
        {
            buf[0] = 0xc0 | (d >> 6);
            buf[1] = 0x80 | (d & 0x3f);
            return 2;
        }
        else if (d <= 0xffff)
        {
            buf[0] = 0xe0 | (d >> 12);
            buf[1] = 0x80 | ((d >> 6) & 0x3f);
            buf[2] = 0x80 | (d & 0x3f);
            return 3;
        }
        else
        {
            buf[0] = 0xf0 | (d >> 18);
            buf[1] = 0x80 | ((d >> 12) & 0x3f);
            buf[2] = 0x80 | ((d >> 6) & 0x3f);
            buf[3] = 0x80 | (d & 0x3f);
            return 4;
        }
    }

    std::string u32to8(const dstring &dstr) {
        std::string result;
        result.resize(getU8Len(dstr));
        int len = 0;
        for (int i=0; i<dstr.size(); i++) {
            char buf[10];
            int k = one32to8(dstr[i], buf);
            for (int i=0; i<k; i++) {
                result[len] = buf[i];
                len++;
            }
        }
        return result;
    }

    std::wstring u32to16(const dstring &dstr) {
        std::wstring result;
        result.resize(getU16Len(dstr));
        int len = 0;
        for (int i=0; i<dstr.size(); i++) {
            wchar_t pair[2];
            int k = one32to16(dstr[i], pair);
            result[len] = pair[0];
            if (k>1)
                result[len+1] = pair[1];
            len += k;
        }
        return result;
    }
};

