#include <map>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
extern "C" {
#include "../qcommon/qcommon.h"
}

// Define custom boolean type if not present
#ifndef qboolean
#define qboolean int
#define qtrue 1
#define qfalse 0
#endif

struct SEString {
    std::string reference;
    std::string value;
};

// Global settings
extern "C" int se_localized = 1;
std::string activeLangKeyword = "LANG_ENGLISH";

// Global string table map
std::map<std::string, SEString> SETable;

extern "C" int FS_ReadFile(const char *qpath, void **buffer);

void ParseStrFileContent(const char *data, int len, const std::string &prefix) {
    std::string content(data, len);
    std::stringstream ss(content);
    std::string line;
    std::string currentReference = "";

    while (std::getline(ss, line)) {
        // Trim trailing CR, LF and whitespace
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n' || isspace((unsigned char)line.back()))) {
            line.pop_back();
        }
        // Trim leading whitespace
        size_t start = 0;
        while (start < line.size() && isspace((unsigned char)line[start])) {
            start++;
        }
        if (start >= line.size()) continue;

        std::string trimmed = line.substr(start);

        // Ignore comments
        if (trimmed.size() >= 2 && trimmed[0] == '/' && trimmed[1] == '/') {
            continue;
        }

        // Parse REFERENCE
        if (trimmed.compare(0, 9, "REFERENCE") == 0) {
            size_t ref_val_start = 9;
            while (ref_val_start < trimmed.size() && isspace((unsigned char)trimmed[ref_val_start])) {
                ref_val_start++;
            }
            if (ref_val_start < trimmed.size()) {
                std::string baseRef = trimmed.substr(ref_val_start);
                while (!baseRef.empty() && isspace((unsigned char)baseRef.back())) {
                    baseRef.pop_back();
                }
                currentReference = prefix + baseRef;
            }
            continue;
        }

        // Parse Language entry (active language or English as fallback)
        bool isEnglish = (trimmed.compare(0, 12, "LANG_ENGLISH") == 0);
        bool isActiveLang = (trimmed.compare(0, activeLangKeyword.size(), activeLangKeyword) == 0);

        if (!currentReference.empty() && (isActiveLang || isEnglish)) {
            size_t key_len = isActiveLang ? activeLangKeyword.size() : 12;
            size_t val_start = key_len;
            while (val_start < trimmed.size() && isspace((unsigned char)trimmed[val_start])) {
                val_start++;
            }
            if (val_start < trimmed.size() && trimmed[val_start] == '"') {
                size_t val_end = trimmed.find_last_of('"');
                if (val_end > val_start) {
                    std::string quotedValue = trimmed.substr(val_start + 1, val_end - val_start - 1);
                    std::string unescaped = "";
                    for (size_t i = 0; i < quotedValue.size(); i++) {
                        if (quotedValue[i] == '\\' && i + 1 < quotedValue.size()) {
                            i++;
                            if (quotedValue[i] == 'n') unescaped += '\n';
                            else if (quotedValue[i] == 't') unescaped += '\t';
                            else if (quotedValue[i] == '"') unescaped += '"';
                            else if (quotedValue[i] == '\\') unescaped += '\\';
                            else unescaped += quotedValue[i];
                        } else {
                            unescaped += quotedValue[i];
                        }
                    }

                    SEString &entry = SETable[currentReference];
                    entry.reference = currentReference;
                    if (isActiveLang || entry.value.empty()) {
                        entry.value = unescaped;
                    }
                }
            }
        }
    }
}

extern "C" void SE_Load(const char *qpath, int force) {
    void *buffer = nullptr;
    int len = FS_ReadFile(qpath, &buffer);
    if (len <= 0 || !buffer) {
        return;
    }

    // Extract filename as prefix (e.g. localizedstrings/english/menu.str -> MENU_)
    std::string pathStr(qpath);
    size_t lastSlash = pathStr.find_last_of("/\\");
    size_t dot = pathStr.find_last_of(".");
    std::string prefix = "";
    if (lastSlash != std::string::npos && dot != std::string::npos && dot > lastSlash) {
        prefix = pathStr.substr(lastSlash + 1, dot - lastSlash - 1);
    } else if (dot != std::string::npos) {
        prefix = pathStr.substr(0, dot);
    } else {
        prefix = pathStr;
    }
    
    // Convert prefix to uppercase and append underscore
    std::transform(prefix.begin(), prefix.end(), prefix.begin(), ::toupper);
    prefix += "_";

    ParseStrFileContent((const char *)buffer, len, prefix);
    free(buffer);
}

extern "C" void SE_LoadLanguage(const char *languageName, int force) {
    SETable.clear();

    const char *lang = languageName ? languageName : "english";

    if (!languageName) {
        activeLangKeyword = "LANG_ENGLISH";
    } else {
        std::string langStr(languageName);
        std::transform(langStr.begin(), langStr.end(), langStr.begin(), ::toupper);
        activeLangKeyword = "LANG_" + langStr;
    }

    // Load common localization packages
    char pathBuf[256];
    
    sprintf(pathBuf, "localizedstrings/%s/menu.str", lang);
    SE_Load(pathBuf, force);
    
    sprintf(pathBuf, "localizedstrings/%s/mpmenu.str", lang);
    SE_Load(pathBuf, force);
    
    sprintf(pathBuf, "localizedstrings/%s/cgame.str", lang);
    SE_Load(pathBuf, force);
    
    sprintf(pathBuf, "localizedstrings/%s/exe.str", lang);
    SE_Load(pathBuf, force);
    
    sprintf(pathBuf, "localizedstrings/%s/platform.str", lang);
    SE_Load(pathBuf, force);
    
    sprintf(pathBuf, "localizedstrings/%s/quickmessage.str", lang);
    SE_Load(pathBuf, force);
    
    sprintf(pathBuf, "localizedstrings/%s/key.str", lang);
    SE_Load(pathBuf, force);
    
    sprintf(pathBuf, "localizedstrings/%s/game.str", lang);
    SE_Load(pathBuf, force);
    
    sprintf(pathBuf, "localizedstrings/%s/messagebox.str", lang);
    SE_Load(pathBuf, force);
}

extern "C" const char *SE_GetString(const char *reference, qboolean wantTranslation) {
    if (!reference) return "";

    // Strip leading @ if present in the lookup reference
    const char *lookupRef = reference;
    if (lookupRef[0] == '@') {
        lookupRef++;
    }

    std::string keyStr(lookupRef);
    auto it = SETable.find(keyStr);

    if (it == SETable.end()) {
        // Fallback to returning the original reference if it's not found in table
        return reference;
    }

    if (wantTranslation && se_localized) {
        return it->second.value.c_str();
    } else {
        return it->second.reference.c_str();
    }
}

extern "C" void SE_Init(void) {
    SETable.clear();
    
    // Determine language from cl_language cvar
    const char *langName = "english";
    cvar_t *cl_lang = Cvar_FindVar("cl_language");
    if (cl_lang) {
        int langIndex = cl_lang->integer;
        if (langIndex >= 0 && langIndex < 14) {
            static const char *languageNames[] = {
                "english",
                "french",
                "german",
                "italian",
                "spanish",
                "british",
                "russian",
                "polish",
                "korean",
                "taiwanese",
                "japanese",
                "chinese",
                "thai",
                "leet"
            };
            langName = languageNames[langIndex];
        }
    }
    SE_LoadLanguage(langName, 0);
}

extern "C" void SE_ShutDown(void) {
    SETable.clear();
}

extern "C" void SE_NewLanguage(void) {
    SETable.clear();
}

extern "C" int SE_GetNumLanguages(void) {
    return 14;
}
