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

    static bool insideU8code(unsigned char b) {
        return (b & 0b11000000) == 0b10000000;
    }
    /*
     * By first byte:
     * 0: inside UTF8 multibyte
     * 1: ASCII
     * 2..6 len
     * 7 bad 11111110
     * 8 bad 11111111
     * */
    static int determineU8Len(unsigned char b) {
        if ((b & 0b10000000) == 0) return 1;
        if (insideU8code(b)) return 0;
        int mask = 0b00100000;
        for (int len=2; len<8; len++) {
            if ((b & mask) == 0) return len;
            mask >>= 1;
        }
        return 8;
    }

    /* not check ambiguity in this stage */
    static bool isCorrectU8code(const char *s, const char *eos, int &len) {
        len = 1;
        if (insideU8code(*s))
            return false;
        int expectLen = determineU8Len(*s);
        if (expectLen>6)
            return false;
        for (int i=2; i<=expectLen; i++) {
            s++;
            if (s>=eos)
                return false;
            if (!insideU8code(*s))
                return false;
            len++;
        }
        return true;
    }

    uint32_t one8to32(const char *s, const char *eos, const char **end) {
        int len;
        bool isOK = isCorrectU8code(s, eos, len);
        *end = s+len;
        if (!isOK) {
            errors++;
            return 0xfffd;
        }
        if (len==1)
            return *s;
        else {
            assert(len>1 && len<=6);
            int mask0 = 127 >> len;
            int d = *s & mask0;
            int minimal = 0;
            for (int i=1; i<len; i++) {
                d = (d <<6) | s[i] & 63;
                if (minimal == 0)
                    minimal = 0x80;
                else if (minimal == 0x80)
                    minimal = 0x800;
                else
                    minimal *= 2;
            }
            if (d<minimal) {
                errambig++;
                errors++;
                return 0xfffd;
            }
            return d;
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
        const char *s;
        const char *sc = s = str.c_str();
        const char *eos = sc+str.length();
        while (s-sc<str.size()) {
            uint32_t d = one8to32(s,eos,&s);
            result ++;
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
        const char *s;
        const char *sc = s = str.c_str();
        const char *eos = sc+str.length();
        while (s-sc<str.size()) {
            uint32_t d = one8to32(s,eos,&s);
            result += one16len(d);
        }
        return result;
    }

    int getU8Len(const std::wstring &wstr) {
        const wchar_t *wsc;
        const wchar_t *ws = wsc = wstr.c_str();
        int len = 0;
        while (ws-wsc<wstr.size()) {
            uint32_t d = one16to32(ws,&ws);
            char buf[4];
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
        for (int i = 0; i < dstr.size(); i++) {
            int d = dstr[i];
            if (isSurrogate(d) || d > MaxCP) {
                d = 0xFFFD;
            }
            if (d < 0x10000)
                len16++;
            else
                len16 += 2;
        }
        return len16;
    }

    std::wstring u8to16(const std::string &str) {
        std::wstring result;
        result.resize(getU16Len(str));
        const char *sc;
        const char *s = sc = str.c_str();
        const char *eos = sc+str.length();
        int len = 0;
        while (s-sc<str.size()) {
            uint32_t d = one8to32(s, eos, &s);
            wchar_t pair[2];
            int k = one32to16(d, pair);
            result[len] = pair[0];
            if (k>1)
                result[len+1] = pair[1];
            len += k;
        }
        return result;
    }

    dstring u8to32(const std::string &str) {
        const char *sc = str.c_str();
        const char *eos = sc + str.length();
        dstring result(getU32Len(str));
        for (int i=0; i<result.size(); i++) {
            result[i] = one8to32(sc, eos, &sc);
        }
        assert(sc == str.c_str() + str.size());
        return result;
    }

    std::string u16to8(const std::wstring &wstr) {
        std::string result;
        result.resize(getU8Len(wstr));
        const wchar_t *wsc;
        const wchar_t *ws = wsc = wstr.c_str();
        int len = 0;
        while (ws-wsc<wstr.size()) {
            uint32_t d = one16to32(ws,&ws);
            char buf[4];
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

