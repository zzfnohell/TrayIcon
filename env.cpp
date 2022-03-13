#include "stdafx.h"
using namespace std;

constexpr WCHAR ENV_DELIMITER = L'=';
constexpr WCHAR ENV_DELIMITER_S[] = L"=";

inline int strlen_with_terminal(int v)
{
    return v + 1;
}


LPTSTR SearchEnv(LPTSTR env, LPTSTR key, int key_size)
{
    LPTSTR p = env;
    while (*p)
    {
        int i = 0;
        while (p[i] == key[i] && i < key_size)
        {
            i++;
        }

        if (i < key_size)
        {
            const int length = lstrlen(p);
            p += strlen_with_terminal(length);
        }
        else
        {
            return p;
        }
    }

    return NULL;
}


void ReplaceEnvVar(LPTSTR env_dst, LPTSTR s, size_t dst_size)
{
    HRESULT hr;
    LPTSTR p = s;
    LPTSTR dst = env_dst;
    size_t n = dst_size;
    WCHAR env_replace[111];
    WCHAR kvp[11];

    while (*p)
    {
        const int length = lstrlen(p);
        if (*p == ENV_DELIMITER)
        {
            hr = StringCchCopy(dst, n, p);
            assert(SUCCEEDED(hr));

            assert(n >= strlen_with_terminal(length));
            n -= strlen_with_terminal(length);
            dst += strlen_with_terminal(length);
        }
        else
        {
            WCHAR* b = wcspbrk(p, ENV_DELIMITER_S);
            LPTSTR rp = NULL;
            if (b) {
                size_t key_size = b - p;
                rp = SearchEnv(env_replace, p, key_size);
            }

            if (rp)
            {
                hr = StringCchCopy(dst, n, rp);
                assert(SUCCEEDED(hr));
                size_t new_env_length = lstrlen(rp);
                assert(n >= strlen_with_terminal(length));

                n -= strlen_with_terminal(new_env_length);
                dst += strlen_with_terminal(new_env_length);
            }
            else
            {
                hr = StringCchCopy(dst, n, p);
                assert(SUCCEEDED(hr));

                assert(n >= strlen_with_terminal(length));
                n -= strlen_with_terminal(length);
                dst += strlen_with_terminal(length);
            }
        }

        p += strlen_with_terminal(length);
    }
}

void PrefixEnvVar(LPTSTR env_dst, LPTSTR s, size_t dst_size)
{
    HRESULT hr;
    LPTSTR p = s;
    LPTSTR dst = env_dst;
    size_t n = dst_size;
    WCHAR env_prefix[11];
    while (*p)
    {
        const int length = lstrlen(p);
        if (*p == ENV_DELIMITER)
        {
            hr = StringCchCopy(dst, n, p);
            assert(SUCCEEDED(hr));

            assert(n >= strlen_with_terminal(length));
            n -= strlen_with_terminal(length);
            dst += strlen_with_terminal(length);
        }
        else
        {
            WCHAR* b = wcspbrk(p, ENV_DELIMITER_S);
            LPTSTR rp = NULL;
            WCHAR* v = NULL;
            if (b) {
                v = b + 1;
                const size_t key_size = b - p;
                rp = SearchEnv(env_prefix, p, key_size);
            }

            if (rp)
            {
                if (v)
                {

                }
                hr = StringCchCopy(dst, n, rp);
                assert(SUCCEEDED(hr));
                size_t new_env_length = lstrlen(rp);
                assert(n >= strlen_with_terminal(length));

                n -= strlen_with_terminal(new_env_length);
                dst += strlen_with_terminal(new_env_length);
            }
            else
            {
                hr = StringCchCopy(dst, n, p);
                assert(SUCCEEDED(hr));

                assert(n >= strlen_with_terminal(length));
                n -= strlen_with_terminal(length);
                dst += strlen_with_terminal(length);
            }
        }

        p += strlen_with_terminal(length);
    }
}

void parse_env(list<wstring>& list) {
    LPWCH  env = GetEnvironmentStrings();

}
LPVOID build_env_block()
{
    list<wstring> env_list{};
    parse_env(env_list);

    return NULL;
}