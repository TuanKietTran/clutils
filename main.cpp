// clutils.cpp - Modern C++ CLI Swiss Army Knife (2025 edition)
// Compile with CLion + vcpkg OpenSSL → fully local, no global libs

#include <charconv>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <format>
#include <span>

#ifdef _WIN32
#include <windows.h>
#include <objbase.h>
#pragma comment(lib, "ole32.lib")
#else
#include <uuid/uuid.h>
#include <openssl/evp.h>
#include <openssl/hmac.h>
#endif

class Clipboard {
public:
    static void copy(const std::string& text) {
#ifdef _WIN32
        if (OpenClipboard(nullptr)) {
            EmptyClipboard();
            HGLOBAL hg = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
            if (hg) {
                void* mem = GlobalLock(hg);
                memcpy(mem, text.c_str(), text.size() + 1);
                GlobalUnlock(hg);
                SetClipboardData(CF_TEXT, hg);
            }
            CloseClipboard();
        }
#else
        // macOS first → always pbcopy
        // Linux: proper Wayland/X11 detection
        const char* cmd;
#ifdef __APPLE__
        cmd = "pbcopy";
#else
        const char* wayland = std::getenv("WAYLAND_DISPLAY");
        const char* x11     = std::getenv("DISPLAY");

        if (wayland && wayland[0] != '\0') {
            cmd = "wl-copy";
        } else if (x11 && x11[0] != '\0') {
            cmd = "xclip -selection clipboard";
        } else {
            cmd = "pbcopy";  // safe fallback (works on macOS and minimal Linux)
        }
#endif

        if (FILE* pipe = popen(cmd, "w")) {
            fwrite(text.c_str(), 1, text.size(), pipe);
            pclose(pipe);
        }
#endif
    }
};

class UUIDGen {
public:
    static std::string generate() {
#ifdef _WIN32
        GUID guid;
        CoCreateGuid(&guid);
        return std::format("{:08X}-{:04X}-{:04X}-{:02X}{:02X}-{:02X}{:02X}{:02X}{:02X}{:02X}{:02X}",
            guid.Data1, guid.Data2, guid.Data3,
            guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
            guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);
#else
        uuid_t uuid;
        uuid_generate_random(uuid);
        char buf[37];
        uuid_unparse_lower(uuid, buf);
        return std::string(buf);
#endif
    }
};

class Encoder {
public:
    static std::string hex_encode(std::string_view input) {
        std::string out;
        out.reserve(input.size() * 2);
        for (unsigned char c : input) {
            out += std::format("{:02x}", c);
        }
        return out;
    }

    static std::string hex_decode(std::string_view input) {
        std::string result;
        result.reserve(input.size() / 2);
        for (size_t i = 0; i < input.size(); i += 2) {
            unsigned int byte;
            std::from_chars(input.data() + i, input.data() + i + 2, byte, 16);
            result.push_back(static_cast<char>(byte));
        }
        return result;
    }

    static std::string base64_encode(std::string_view input, bool url_safe = false) {
        const std::string chars = url_safe
            ? "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"
            : "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

        std::string ret;
        int val = 0, bits = -6;
        for (unsigned char c : input) {
            val = (val << 8) + c;
            bits += 8;
            while (bits >= 0) {
                ret.push_back(chars[(val >> bits) & 0x3F]);
                bits -= 6;
            }
        }
        if (bits > -6)
            ret.push_back(chars[((val << 8) >> (bits + 8)) & 0x3F]);
        while (ret.size() % 4)
            ret.push_back('=');
        if (url_safe)
            ret.erase(std::remove_if(ret.begin(), ret.end(),
                [](char c) { return c == '+' || c == '/' || c == '='; }), ret.end());
        std::replace(ret.begin(), ret.end(), '+', '-');
        std::replace(ret.begin(), ret.end(), '/', '_');
        return ret;
    }

    static std::string base64_decode(std::string_view input, bool url_safe = false) {
        std::string data = std::string(input);
        if (url_safe) {
            std::replace(data.begin(), data.end(), '-', '+');
            std::replace(data.begin(), data.end(), '_', '/');
            while (data.size() % 4) data += '=';
        }

        const std::string chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
        std::string ret;
        int val = 0, bits = -8;
        for (char c : data) {
            if (c == '=') break;
            size_t pos = chars.find(c);
            if (pos == std::string::npos) continue;
            val = (val << 6) + static_cast<int>(pos);
            bits += 6;
            if (bits >= 0) {
                ret.push_back(static_cast<char>((val >> bits) & 0xFF));
                bits -= 8;
            }
        }
        return ret;
    }

    static std::string url_encode(std::string_view value) {
        std::ostringstream escaped;
        escaped << std::hex << std::uppercase << std::setfill('0');
        for (unsigned char c : value) {
            if (isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~') {
                escaped << c;
            } else {
                escaped << '%' << std::setw(2) << static_cast<int>(c);
            }
        }
        return escaped.str();
    }

    static std::string rot13(std::string_view input) {
        std::string s(input);
        for (char& c : s) {
            if (c >= 'A' && c <= 'Z') c = ((c - 'A' + 13) % 26) + 'A';
            else if (c >= 'a' && c <= 'z') c = ((c - 'a' + 13) % 26) + 'a';
        }
        return s;
    }
};

class HMACGenerator {
public:
    static std::string hmac_sha256(std::string_view key, std::string_view data) {
#ifndef _WIN32
        unsigned char digest[32];
        unsigned int len = sizeof(digest);

        if (!HMAC(EVP_sha256(),
                  key.data(), static_cast<int>(key.size()),
                  reinterpret_cast<const unsigned char*>(data.data()), data.size(),
                  digest, &len)) {
            return "[HMAC failed]";
        }

        std::string result;
        result.reserve(64);
        for (int i = 0; i < 32; ++i) {
            result += std::format("{:02x}", digest[i]);
        }
        return result;
#else
        return "[HMAC not available on Windows build]";
#endif
    }
};

void print_help() {
    std::cout << R"(
clutils - Modern CLI Utility Toolkit

Usage:
  clutils uuid                    Generate UUID (copied)
  clutils hex <text>              Hex encode
  clutils unhex <hex>             Hex decode
  clutils b64 <text>              Base64 encode
  clutils unb64 <b64>             Base64 decode
  clutils b64url <text>           Base64URL encode
  clutils unb64url <b64url>       Base64URL decode
  clutils url <text>              URL encode
  clutils rot13 <text>            ROT13
  clutils hmac <key> <data>       HMAC-SHA256 (hex output)
  clutils --help                  This help

All results are automatically copied to clipboard!
)";
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_help();
        return 0;
    }

    std::string cmd = argv[1];
    auto copy_and_print = [](const std::string& s) {
        std::cout << s << "\nCopied to clipboard!\n";
        Clipboard::copy(s);
    };

    if (cmd == "uuid" || cmd == "guid") {
        copy_and_print(UUIDGen::generate());
    }
    else if (cmd == "hex" && argc >= 3) {
        std::string input(argv[2]);
        for (int i = 3; i < argc; ++i) input += " " + std::string(argv[i]);
        copy_and_print(Encoder::hex_encode(input));
    }
    else if (cmd == "unhex" && argc >= 3) {
        std::string input(argv[2]);
        for (int i = 3; i < argc; ++i) input += std::string(argv[i]);
        copy_and_print(Encoder::hex_decode(input));
    }
    else if (cmd == "b64" && argc >= 3) {
        std::string input(argv[2]);
        for (int i = 3; i < argc; ++i) input += " " + std::string(argv[i]);
        copy_and_print(Encoder::base64_encode(input));
    }
    else if (cmd == "unb64" && argc >= 3) {
        copy_and_print(Encoder::base64_decode(argv[2]));
    }
    else if (cmd == "b64url" && argc >= 3) {
        std::string input(argv[2]);
        for (int i = 3; i < argc; ++i) input += " " + std::string(argv[i]);
        copy_and_print(Encoder::base64_encode(input, true));
    }
    else if (cmd == "unb64url" && argc >= 3) {
        copy_and_print(Encoder::base64_decode(argv[2], true));
    }
    else if (cmd == "url" && argc >= 3) {
        std::string input(argv[2]);
        for (int i = 3; i < argc; ++i) input += " " + std::string(argv[i]);
        copy_and_print(Encoder::url_encode(input));
    }
    else if (cmd == "rot13" && argc >= 3) {
        std::string input(argv[2]);
        for (int i = 3; i < argc; ++i) input += " " + std::string(argv[i]);
        copy_and_print(Encoder::rot13(input));
    }
    else if (cmd == "hmac" && argc >= 4) {
        std::string key = argv[2];
        std::string data = argv[3];
        for (int i = 4; i < argc; ++i) data += " " + std::string(argv[i]);
        copy_and_print(HMACGenerator::hmac_sha256(key, data));
    }
    else {
        print_help();
    }

    return 0;
}