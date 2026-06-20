#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <mmsystem.h>
#include <shellapi.h>

#include <atomic>
#include <cstdio>
#include <cwctype>
#include <filesystem>
#include <fstream>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Comdlg32.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Winmm.lib")

namespace fs = std::filesystem;

namespace {

enum Page { PAGE_GENERATE, PAGE_LIBRARY, PAGE_CLONE, PAGE_FINETUNE, PAGE_MODELS, PAGE_SETTINGS };

constexpr int IDC_NAV_GENERATE = 1000;
constexpr int IDC_NAV_LIBRARY = 1001;
constexpr int IDC_NAV_CLONE = 1002;
constexpr int IDC_NAV_FINETUNE = 1003;
constexpr int IDC_NAV_MODELS = 1004;
constexpr int IDC_NAV_SETTINGS = 1005;
constexpr int IDC_THEME = 1006;

constexpr int IDC_TEXT = 1100;
constexpr int IDC_LANGUAGE = 1101;
constexpr int IDC_SPEAKER = 1102;
constexpr int IDC_INSTRUCT = 1103;
constexpr int IDC_MODE = 1104;
constexpr int IDC_FORMAT = 1105;
constexpr int IDC_OUTPUT = 1106;
constexpr int IDC_BROWSE_OUTPUT = 1107;
constexpr int IDC_GENERATE = 1108;
constexpr int IDC_PLAY = 1109;
constexpr int IDC_PAUSE = 1110;
constexpr int IDC_STOP = 1111;
constexpr int IDC_OPEN_OUTPUT = 1112;
constexpr int IDC_TEMP = 1113;
constexpr int IDC_TOPP = 1114;
constexpr int IDC_TOPK = 1115;
constexpr int IDC_MAXTOK = 1116;
constexpr int IDC_REPPEN = 1117;
constexpr int IDC_VOLUME = 1118;
constexpr int IDC_SPEED = 1119;
constexpr int IDC_PRESET = 1120;
constexpr int IDC_VOICE_PROFILE = 1121;
constexpr int IDC_MAX_CHUNK = 1122;
constexpr int IDC_CROSSFADE = 1123;
constexpr int IDC_NORMALIZE = 1124;
constexpr int IDC_EFFECT_PRESET = 1125;
constexpr int IDC_TEST_EFFECTS = 1126;
constexpr int IDC_APPLY_EFFECT = 1127;

constexpr int IDC_LIB_LIST = 1150;
constexpr int IDC_LIB_NAME = 1151;
constexpr int IDC_LIB_TYPE = 1152;
constexpr int IDC_LIB_LANG = 1153;
constexpr int IDC_LIB_MODE = 1154;
constexpr int IDC_LIB_SPEAKER = 1155;
constexpr int IDC_LIB_DESC = 1156;
constexpr int IDC_LIB_PERSONA = 1157;
constexpr int IDC_LIB_REF_AUDIO = 1158;
constexpr int IDC_LIB_REF_TEXT = 1159;
constexpr int IDC_LIB_CHECKPOINT = 1160;
constexpr int IDC_LIB_SAVE = 1161;
constexpr int IDC_LIB_USE = 1162;
constexpr int IDC_LIB_EXPORT = 1163;
constexpr int IDC_LIB_IMPORT = 1164;
constexpr int IDC_LIB_ADD_SAMPLE = 1165;
constexpr int IDC_LIB_SAMPLE_AUDIO = 1166;
constexpr int IDC_LIB_SAMPLE_TEXT = 1167;
constexpr int IDC_LIB_BROWSE_REF = 1168;
constexpr int IDC_LIB_BROWSE_SAMPLE = 1169;
constexpr int IDC_HISTORY_LIST = 1170;
constexpr int IDC_HISTORY_REFRESH = 1171;
constexpr int IDC_LIB_EFFECTS = 1172;
constexpr int IDC_HISTORY_OPEN = 1173;
constexpr int IDC_LIB_EMOTION = 1174;
constexpr int IDC_LIB_VALIDATION = 1175;
constexpr int IDC_MODEL_OPEN = 1176;

constexpr int IDC_CLONE_TEXT = 1200;
constexpr int IDC_CLONE_REF_AUDIO = 1201;
constexpr int IDC_CLONE_REF_TEXT = 1202;
constexpr int IDC_CLONE_BROWSE = 1203;
constexpr int IDC_CLONE_XVEC = 1204;
constexpr int IDC_CLONE_GENERATE = 1205;

constexpr int IDC_FT_SPEAKER = 1300;
constexpr int IDC_FT_REF_AUDIO = 1301;
constexpr int IDC_FT_BROWSE_REF = 1302;
constexpr int IDC_FT_AUDIO = 1303;
constexpr int IDC_FT_BROWSE_AUDIO = 1304;
constexpr int IDC_FT_TRANSCRIPT = 1305;
constexpr int IDC_FT_ADD = 1306;
constexpr int IDC_FT_LIST = 1307;
constexpr int IDC_FT_PROJECT = 1308;
constexpr int IDC_FT_PREPARE = 1309;
constexpr int IDC_FT_TRAIN = 1310;
constexpr int IDC_FT_EPOCHS = 1311;
constexpr int IDC_FT_LR = 1312;
constexpr int IDC_FT_BATCH = 1313;

constexpr int IDC_MODEL_LIST = 1400;
constexpr int IDC_SCAN_MODELS = 1401;
constexpr int IDC_DOWNLOAD_MODEL = 1402;
constexpr int IDC_MODEL_PICK = 1403;

constexpr int IDC_PYTHON = 1500;
constexpr int IDC_MODELS_ROOT = 1501;
constexpr int IDC_DEVICE = 1502;
constexpr int IDC_DTYPE = 1503;
constexpr int IDC_ATTN = 1504;
constexpr int IDC_OFFLINE = 1505;

constexpr int IDC_LOG = 1600;
constexpr int IDC_STATUS = 1601;

struct Colors {
    COLORREF bg, panel, panel2, text, muted, accent, accent2, warn, border, editBg;
};

const Colors Dark{ RGB(17,20,25), RGB(25,30,38), RGB(32,39,49), RGB(238,242,247), RGB(154,166,182), RGB(45,212,191), RGB(96,165,250), RGB(251,191,36), RGB(53,64,78), RGB(12,15,20) };
const Colors Light{ RGB(244,247,251), RGB(255,255,255), RGB(235,241,248), RGB(22,28,37), RGB(88,101,118), RGB(14,165,147), RGB(37,99,235), RGB(180,83,9), RGB(206,216,228), RGB(255,255,255) };

struct TrainingItem {
    std::wstring audio;
    std::wstring sourceAudio;
    std::wstring transcript;
};

struct VoiceProfile {
    std::wstring id, name, type, language, mode, speaker, description, personality, refAudio, refText, checkpoint, effects, emotion;
};

struct HistoryRow {
    std::wstring id, profile, text, output, status, created;
};

struct App {
    HWND hwnd{};
    HFONT font{}, titleFont{}, monoFont{};
    HBRUSH bgBrush{}, panelBrush{}, editBrush{};
    Colors c = Dark;
    bool dark = true;
    Page page = PAGE_GENERATE;
    std::atomic<bool> running{false};
    std::vector<HWND> generateCtrls, libraryCtrls, cloneCtrls, ftCtrls, modelCtrls, settingsCtrls;
    std::vector<TrainingItem> training;
    std::vector<VoiceProfile> voices;
    std::vector<HistoryRow> history;
    std::map<int, HWND> h;
    bool audioOpened = false;
    bool volumeUnsupported = false;
    bool volumeWarningShown = false;
};

App g;

const wchar_t* QwenRoot = L"C:\\Users\\flyin\\OneDrive\\Desktop\\qwen tts";

std::wstring Utf8ToWide(const std::string& s) {
    if (s.empty()) return L"";
    int n = MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), nullptr, 0);
    std::wstring w(n, 0);
    MultiByteToWideChar(CP_UTF8, 0, s.data(), (int)s.size(), w.data(), n);
    return w;
}

std::string WideToUtf8(const std::wstring& w) {
    if (w.empty()) return "";
    int n = WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), nullptr, 0, nullptr, nullptr);
    std::string s(n, 0);
    WideCharToMultiByte(CP_UTF8, 0, w.data(), (int)w.size(), s.data(), n, nullptr, nullptr);
    return s;
}

std::wstring Text(HWND h) {
    int n = GetWindowTextLengthW(h);
    std::wstring s(n + 1, 0);
    GetWindowTextW(h, s.data(), n + 1);
    s.resize(n);
    return s;
}

void Text(HWND h, const std::wstring& s) { SetWindowTextW(h, s.c_str()); }

std::wstring Combo(HWND h) {
    int i = (int)SendMessageW(h, CB_GETCURSEL, 0, 0);
    if (i < 0) return L"";
    int n = (int)SendMessageW(h, CB_GETLBTEXTLEN, i, 0);
    std::wstring s(n + 1, 0);
    SendMessageW(h, CB_GETLBTEXT, i, (LPARAM)s.data());
    s.resize(n);
    return s;
}

void AddItems(HWND h, const std::vector<std::wstring>& items, int sel = 0) {
    for (auto& s : items) SendMessageW(h, CB_ADDSTRING, 0, (LPARAM)s.c_str());
    SendMessageW(h, CB_SETCURSEL, sel, 0);
}

const std::vector<std::wstring>& VoiceEmotionItems() {
    static const std::vector<std::wstring> items = {
        L"Natural", L"Calm", L"Angry", L"Scared / paranoid", L"Whisper",
        L"Excited", L"Sad", L"Urgent", L"Funny with laugh ending",
        L"Creepy / tense", L"Crazy / unhinged"
    };
    return items;
}

const std::vector<std::wstring>& AudioProcessingItems() {
    static const std::vector<std::wstring> items = {
        L"None", L"Clean Narration", L"Radio", L"Telephone", L"Echo Chamber",
        L"Reverb Room", L"Deepen", L"Brighten", L"Robotic", L"Chorus / Flanger",
        L"Low-Pass", L"High-Pass"
    };
    return items;
}

std::wstring NormalizeVoiceEmotion(std::wstring value) {
    if (value.empty() || value == L"Custom") return L"Natural";
    if (value == L"Funny Laugh Ending") return L"Funny with laugh ending";
    if (value == L"Creepy Voice" || value == L"Ghost Whisper") return L"Creepy / tense";
    if (value == L"Crazy Voice" || value == L"Monster Deep") return L"Crazy / unhinged";
    if (value == L"Villain Radio") return L"Angry";
    return value;
}

std::wstring NormalizeAudioProcessing(std::wstring value) {
    if (value.empty()) return L"None";
    if (value == L"Deep Voice" || value == L"Monster Deep") return L"Deepen";
    if (value == L"Cinematic Whisper" || value == L"Ghost Whisper") return L"Reverb Room";
    if (value == L"Funny Laugh Ending") return L"Brighten";
    if (value == L"Crazy Voice") return L"Chorus / Flanger";
    if (value == L"Creepy Voice") return L"Reverb Room";
    if (value == L"Villain Radio") return L"Radio";
    return value;
}

void AddVoiceEmotions(HWND h, int sel = 0) { AddItems(h, VoiceEmotionItems(), sel); }
void AddAudioProcessing(HWND h, int sel = 0) { AddItems(h, AudioProcessingItems(), sel); }

void Log(const std::wstring& s) {
    HWND h = g.h[IDC_LOG];
    int n = GetWindowTextLengthW(h);
    SendMessageW(h, EM_SETSEL, n, n);
    SendMessageW(h, EM_REPLACESEL, FALSE, (LPARAM)s.c_str());
}

void EraseAll(std::wstring& text, const std::wstring& needle) {
    size_t pos = 0;
    while ((pos = text.find(needle, pos)) != std::wstring::npos) {
        text.erase(pos, needle.size());
    }
}

std::wstring CleanBackendOutput(std::wstring text) {
    EraseAll(text, L"'sox' is not recognized as an internal or external command,\r\noperable program or batch file.\r\n");
    EraseAll(text, L"'sox' is not recognized as an internal or external command,\noperable program or batch file.\n");
    size_t start = text.find(L"SoX could not be found!");
    if (start != std::wstring::npos) {
        size_t end = text.find(L"path variables.", start);
        if (end != std::wstring::npos) {
            end += wcslen(L"path variables.");
            while (end < text.size() && (text[end] == L'\r' || text[end] == L'\n' || text[end] == L' ' || text[end] == L'\t')) ++end;
            text.erase(start, end - start);
        }
    }
    return text;
}

std::string Json(const std::wstring& w) {
    std::string s = WideToUtf8(w);
    std::ostringstream o;
    for (unsigned char c : s) {
        switch (c) {
        case '\\': o << "\\\\"; break;
        case '"': o << "\\\""; break;
        case '\n': o << "\\n"; break;
        case '\r': o << "\\r"; break;
        case '\t': o << "\\t"; break;
        default:
            if (c < 32) {
                char b[7]{};
                sprintf_s(b, "\\u%04x", c);
                o << b;
            } else o << c;
        }
    }
    return o.str();
}

std::wstring UnescapeJsonString(const std::string& s) {
    std::wstring out;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\\' && i + 1 < s.size()) {
            char n = s[++i];
            if (n == 'n') out += L'\n';
            else if (n == 'r') out += L'\r';
            else if (n == 't') out += L'\t';
            else if (n == '"' || n == '\\' || n == '/') out += Utf8ToWide(std::string(1, n));
            else if (n == 'u' && i + 4 < s.size()) {
                std::string hex = s.substr(i + 1, 4);
                wchar_t ch = (wchar_t)strtol(hex.c_str(), nullptr, 16);
                out += ch;
                i += 4;
            } else out += Utf8ToWide(std::string(1, n));
        } else {
            out += Utf8ToWide(std::string(1, s[i]));
        }
    }
    return out;
}

std::wstring JsonField(const std::wstring& jsonWide, const std::string& key) {
    std::string json = WideToUtf8(jsonWide);
    std::string pat = "\"" + key + "\":";
    size_t p = json.find(pat);
    if (p == std::string::npos) return L"";
    p += pat.size();
    while (p < json.size() && (json[p] == ' ' || json[p] == '\t')) ++p;
    if (p >= json.size()) return L"";
    if (json[p] == '"') {
        ++p;
        std::string val;
        bool esc = false;
        for (; p < json.size(); ++p) {
            char c = json[p];
            if (!esc && c == '"') break;
            if (!esc && c == '\\') { esc = true; val += c; continue; }
            esc = false;
            val += c;
        }
        return UnescapeJsonString(val);
    }
    size_t end = json.find_first_of(",}", p);
    return Utf8ToWide(json.substr(p, end == std::string::npos ? std::string::npos : end - p));
}

std::wstring Quote(const std::wstring& s) {
    std::wstring out = L"\"";
    for (wchar_t c : s) out += (c == L'"') ? L"\\\"" : std::wstring(1, c);
    return out + L"\"";
}

bool WriteFileUtf8(const std::wstring& path, const std::string& data) {
    std::ofstream f(fs::path(path), std::ios::binary);
    if (!f) return false;
    f.write(data.data(), (std::streamsize)data.size());
    return true;
}

std::wstring TempFile(const wchar_t* name) {
    wchar_t t[MAX_PATH]{};
    GetTempPathW(MAX_PATH, t);
    std::wstring d = std::wstring(t) + L"QwensTtsBuddy\\";
    CreateDirectoryW(d.c_str(), nullptr);
    return d + name;
}

double D(int id, double fallback) {
    try { return std::stod(Text(g.h[id])); } catch (...) { return fallback; }
}

int I(int id, int fallback) {
    try { return std::stoi(Text(g.h[id])); } catch (...) { return fallback; }
}

std::wstring ModelDir(const std::wstring& name) {
    return Text(g.h[IDC_MODELS_ROOT]) + L"\\" + name;
}

bool Exists(const std::wstring& p) {
    std::error_code ec;
    return fs::exists(fs::path(p), ec);
}

void SetStatus(const std::wstring& s) { Text(g.h[IDC_STATUS], s); }

std::wstring FileNameOf(const std::wstring& p) {
    return fs::path(p).filename().wstring();
}

bool Blank(const std::wstring& s) {
    return s.find_first_not_of(L" \t\r\n") == std::wstring::npos;
}

std::wstring MciError(MCIERROR code) {
    if (!code) return L"";
    wchar_t msg[512]{};
    if (mciGetErrorStringW(code, msg, 512)) return msg;
    return L"MCI error " + std::to_wstring(code);
}

void ClosePlayback() {
    mciSendStringW(L"stop qwen_audio", nullptr, 0, nullptr);
    mciSendStringW(L"close qwen_audio", nullptr, 0, nullptr);
    g.audioOpened = false;
}

void ApplyPlaybackVolume() {
    if (!g.audioOpened || g.volumeUnsupported) return;
    int vol = (int)SendMessageW(g.h[IDC_VOLUME], TBM_GETPOS, 0, 0);
    std::wstring v = L"setaudio qwen_audio volume to " + std::to_wstring(vol * 10);
    MCIERROR err = mciSendStringW(v.c_str(), nullptr, 0, nullptr);
    if (err) {
        g.volumeUnsupported = true;
        if (!g.volumeWarningShown) {
            Log(L"Volume slider is active, but this Windows playback driver does not support per-file volume control. Use Windows volume/mixer for playback loudness; play, pause, and stop still work.\r\n");
            g.volumeWarningShown = true;
        }
    }
}

HWND New(const wchar_t* cls, const wchar_t* txt, DWORD style, int id, int x, int y, int w, int h, HWND parent) {
    HWND c = CreateWindowExW((wcscmp(cls, L"EDIT") == 0) ? WS_EX_CLIENTEDGE : 0, cls, txt, WS_CHILD | WS_VISIBLE | style,
        x, y, w, h, parent, (HMENU)(INT_PTR)id, nullptr, nullptr);
    SendMessageW(c, WM_SETFONT, (WPARAM)g.font, TRUE);
    if (id) g.h[id] = c;
    return c;
}

HWND Label(const wchar_t* t, int x, int y, int w, int h, HWND p, bool title = false) {
    HWND c = New(L"STATIC", t, SS_LEFT, 0, x, y, w, h, p);
    SendMessageW(c, WM_SETFONT, (WPARAM)(title ? g.titleFont : g.font), TRUE);
    return c;
}

HWND Edit(int id, const wchar_t* t, int x, int y, int w, int h, HWND p, bool multi = false) {
    return New(L"EDIT", t, ES_AUTOHSCROLL | WS_TABSTOP | (multi ? ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL : 0), id, x, y, w, h, p);
}

HWND Button(int id, const wchar_t* t, int x, int y, int w, int h, HWND p) {
    return New(L"BUTTON", t, BS_PUSHBUTTON | WS_TABSTOP, id, x, y, w, h, p);
}

HWND Check(int id, const wchar_t* t, int x, int y, int w, int h, HWND p) {
    return New(L"BUTTON", t, BS_AUTOCHECKBOX | WS_TABSTOP, id, x, y, w, h, p);
}

HWND ComboBox(int id, int x, int y, int w, int h, HWND p) {
    return New(WC_COMBOBOXW, L"", CBS_DROPDOWNLIST | WS_TABSTOP | WS_VSCROLL, id, x, y, w, h, p);
}

HWND List(int id, int x, int y, int w, int h, HWND p) {
    return New(L"LISTBOX", L"", LBS_NOTIFY | WS_VSCROLL | WS_TABSTOP, id, x, y, w, h, p);
}

void Track(int page, HWND h) {
    if (page == PAGE_GENERATE) g.generateCtrls.push_back(h);
    if (page == PAGE_LIBRARY) g.libraryCtrls.push_back(h);
    if (page == PAGE_CLONE) g.cloneCtrls.push_back(h);
    if (page == PAGE_FINETUNE) g.ftCtrls.push_back(h);
    if (page == PAGE_MODELS) g.modelCtrls.push_back(h);
    if (page == PAGE_SETTINGS) g.settingsCtrls.push_back(h);
}

void ShowPage(Page p) {
    g.page = p;
    auto show = [](std::vector<HWND>& v, bool on) { for (HWND h : v) ShowWindow(h, on ? SW_SHOW : SW_HIDE); };
    show(g.generateCtrls, p == PAGE_GENERATE);
    show(g.libraryCtrls, p == PAGE_LIBRARY);
    show(g.cloneCtrls, p == PAGE_CLONE);
    show(g.ftCtrls, p == PAGE_FINETUNE);
    show(g.modelCtrls, p == PAGE_MODELS);
    show(g.settingsCtrls, p == PAGE_SETTINGS);
    InvalidateRect(g.hwnd, nullptr, TRUE);
}

void ApplyTheme() {
    if (g.bgBrush) DeleteObject(g.bgBrush);
    if (g.panelBrush) DeleteObject(g.panelBrush);
    if (g.editBrush) DeleteObject(g.editBrush);
    g.c = g.dark ? Dark : Light;
    g.bgBrush = CreateSolidBrush(g.c.bg);
    g.panelBrush = CreateSolidBrush(g.c.panel);
    g.editBrush = CreateSolidBrush(g.c.editBg);
    InvalidateRect(g.hwnd, nullptr, TRUE);
}

std::string BackendPy() {
    return R"PY(
import json, os, sys, subprocess, shutil, sqlite3, uuid, re, math, wave, contextlib
from pathlib import Path

def say(*a): print(*a, flush=True)

def db_path(root):
    app = root / "qwen_studio_data"
    app.mkdir(parents=True, exist_ok=True)
    return app / "qwen_studio.sqlite"

def connect_db(root):
    db = sqlite3.connect(str(db_path(root)))
    db.row_factory = sqlite3.Row
    db.execute("PRAGMA journal_mode=WAL")
    db.executescript("""
    create table if not exists voice_profiles(
        id text primary key,
        name text unique not null,
        voice_type text not null default 'builtin',
        language text not null default 'English',
        mode_label text not null default 'Built-in Voices (0.6B)',
        speaker text not null default 'Ryan',
        description text default '',
        personality text default '',
        ref_audio text default '',
        ref_text text default '',
        checkpoint_path text default '',
        effects_preset text default 'None',
        voice_emotion text default 'Natural',
        audio_processing text default 'None',
        created_at text default current_timestamp,
        updated_at text default current_timestamp
    );
    create table if not exists voice_samples(
        id text primary key,
        profile_id text not null,
        audio_path text not null,
        transcript text not null,
        source_audio_path text default '',
        normalized_audio_path text default '',
        duration real default 0,
        sample_rate integer default 0,
        is_primary integer default 0,
        created_at text default current_timestamp
    );
    create table if not exists generations(
        id text primary key,
        profile_id text,
        profile_name text,
        text text not null,
        language text,
        speaker text,
        mode text,
        instruct text,
        output_path text,
        format text,
        speed real,
        temperature real,
        voice_emotion text default 'Natural',
        audio_processing text default 'None',
        status text,
        error text default '',
        duration real default 0,
        created_at text default current_timestamp
    );
    create table if not exists generation_versions(
        id text primary key,
        generation_id text not null,
        label text not null,
        audio_path text not null,
        effects_preset text default 'None',
        created_at text default current_timestamp
    );
    create table if not exists effect_presets(
        id text primary key,
        name text unique not null,
        description text default '',
        chain_json text not null,
        builtin integer default 0
    );
    create table if not exists settings(key text primary key, value text);
    create table if not exists model_registry(name text primary key, path text, status text, updated_at text default current_timestamp);
    """)
    ensure_column(db, "voice_profiles", "voice_emotion", "text default 'Natural'")
    ensure_column(db, "voice_profiles", "audio_processing", "text default 'None'")
    ensure_column(db, "voice_samples", "source_audio_path", "text default ''")
    ensure_column(db, "voice_samples", "normalized_audio_path", "text default ''")
    ensure_column(db, "generations", "voice_emotion", "text default 'Natural'")
    ensure_column(db, "generations", "audio_processing", "text default 'None'")
    db.execute("""update voice_profiles set audio_processing=effects_preset
                  where (audio_processing is null or audio_processing='' or audio_processing='None')
                  and effects_preset is not null and effects_preset not in ('','None')""")
    db.execute("""update voice_profiles set voice_emotion='Natural'
                  where voice_emotion is null or voice_emotion=''""")
    db.commit()
    seed_library(db)
    return db

def ensure_column(db, table, column, definition):
    cols = [r[1] for r in db.execute(f"pragma table_info({table})")]
    if column not in cols:
        db.execute(f"alter table {table} add column {column} {definition}")
        db.commit()

def seed_library(db):
    presets = {
        "None": [],
        "Radio": [{"type":"highpass","cutoff":300},{"type":"lowpass","cutoff":3500},{"type":"compressor","threshold":0.18,"ratio":5.0},{"type":"gain","db":5}],
        "Telephone": [{"type":"highpass","cutoff":420},{"type":"lowpass","cutoff":3000},{"type":"compressor","threshold":0.16,"ratio":5.5},{"type":"gain","db":4}],
        "Echo Chamber": [{"type":"reverb","room":0.85},{"type":"delay","seconds":0.25,"mix":0.22,"feedback":0.35}],
        "Reverb Room": [{"type":"reverb","room":0.45},{"type":"delay","seconds":0.11,"mix":0.08,"feedback":0.12}],
        "Deepen": [{"type":"pitch","semitones":-3},{"type":"lowpass","cutoff":6500},{"type":"compressor","threshold":0.28,"ratio":3.5}],
        "Brighten": [{"type":"highpass","cutoff":130},{"type":"gain","db":2.0},{"type":"compressor","threshold":0.24,"ratio":2.8}],
        "Robotic": [{"type":"chorus","rate":0.2,"depth":1.0},{"type":"pitch","semitones":-1},{"type":"distortion","drive":1.4}],
        "Chorus / Flanger": [{"type":"chorus","rate":2.5,"depth":0.72},{"type":"wobble","rate":0.7,"depth":0.002}],
        "Low-Pass": [{"type":"lowpass","cutoff":3300}],
        "High-Pass": [{"type":"highpass","cutoff":220}],
        "Clean Narration": [{"type":"highpass","cutoff":80},{"type":"compressor","threshold":0.25,"ratio":3.0},{"type":"gain","db":1.5}],
    }
    db.execute("delete from effect_presets where builtin=1 and name not in (%s)" % ",".join("?" for _ in presets), tuple(presets.keys()))
    for name, chain in presets.items():
        db.execute("""insert into effect_presets(id,name,description,chain_json,builtin) values(?,?,?,?,1)
                   on conflict(name) do update set description=excluded.description, chain_json=excluded.chain_json, builtin=1""",
                   (str(uuid.uuid4()), name, "Built-in studio preset", json.dumps(chain)))
    builtins = [
        ("Ryan", "English", "Ryan"),
        ("Aiden", "English", "Aiden"),
        ("Vivian", "Chinese", "Vivian"),
        ("Serena", "English", "Serena"),
        ("Uncle_Fu", "Chinese", "Uncle_Fu"),
        ("Dylan", "English", "Dylan"),
        ("Eric", "English", "Eric"),
        ("Ono_Anna", "Japanese", "Ono_Anna"),
        ("Sohee", "Korean", "Sohee"),
    ]
    for name, lang, speaker in builtins:
        db.execute("""insert or ignore into voice_profiles
            (id,name,voice_type,language,mode_label,speaker,description,effects_preset,voice_emotion,audio_processing)
            values(?,?,?,?,?,?,?,?,?,?)""",
            (str(uuid.uuid4()), name, "builtin", lang, "Built-in Voices (0.6B)", speaker, f"Built-in Qwen voice: {name}", "None", "Natural", "None"))
    db.commit()

def row_json(row):
    return json.dumps(dict(row), ensure_ascii=False)

def duration_info(path):
    try:
        import soundfile as _sf
        info = _sf.info(str(path))
        return float(info.frames) / float(info.samplerate), int(info.samplerate)
    except Exception:
        try:
            with contextlib.closing(wave.open(str(path), "rb")) as w:
                return w.getnframes() / float(w.getframerate()), int(w.getframerate())
        except Exception:
            return 0.0, 0

def library_mode(cfg, root):
    db = connect_db(root)
    mode = cfg["mode"]
    if mode == "library_list":
        for row in db.execute("select * from voice_profiles order by voice_type, name"):
            say("VOICE_PROFILE_JSON:", row_json(row))
        for row in db.execute("select id,profile_name,text,output_path,status,created_at from generations order by created_at desc limit 80"):
            say("GENERATION_JSON:", row_json(row))
        return
    if mode == "library_save":
        pid = cfg.get("profile_id") or str(uuid.uuid4())
        vals = (
            pid, cfg["name"], cfg.get("voice_type","builtin"), cfg.get("language","English"),
            cfg.get("mode_label","Built-in Voices (0.6B)"), cfg.get("speaker","Ryan"),
            cfg.get("description",""), cfg.get("personality",""), cfg.get("ref_audio",""),
            cfg.get("ref_text",""), cfg.get("checkpoint_path",""), cfg.get("effects_preset","None"),
            cfg.get("voice_emotion","Natural"), cfg.get("audio_processing", cfg.get("effects_preset","None"))
        )
        db.execute("""insert into voice_profiles
            (id,name,voice_type,language,mode_label,speaker,description,personality,ref_audio,ref_text,checkpoint_path,effects_preset,voice_emotion,audio_processing)
            values(?,?,?,?,?,?,?,?,?,?,?,?,?,?)
            on conflict(name) do update set voice_type=excluded.voice_type,language=excluded.language,
            mode_label=excluded.mode_label,speaker=excluded.speaker,description=excluded.description,
            personality=excluded.personality,ref_audio=excluded.ref_audio,ref_text=excluded.ref_text,
            checkpoint_path=excluded.checkpoint_path,effects_preset=excluded.effects_preset,
            voice_emotion=excluded.voice_emotion,audio_processing=excluded.audio_processing,
            updated_at=current_timestamp""", vals)
        db.commit()
        say("Saved voice profile:", cfg["name"])
        return
    if mode == "library_add_sample":
        if not Path(cfg.get("audio_path","")).exists():
            raise RuntimeError("Sample audio file does not exist")
        if not cfg.get("transcript","").strip():
            raise RuntimeError("Sample transcript is required")
        profile = db.execute("select id from voice_profiles where name=?", (cfg.get("profile_name",""),)).fetchone()
        if not profile:
            raise RuntimeError("Save or select a voice profile before adding samples")
        dur, sr = duration_info(cfg["audio_path"])
        if dur and dur < 3:
            say("Sample warning: this clip is very short. 10-30 seconds is usually stronger for cloning.")
        db.execute("insert into voice_samples(id,profile_id,audio_path,transcript,source_audio_path,normalized_audio_path,duration,sample_rate,is_primary) values(?,?,?,?,?,?,?,?,?)",
                   (str(uuid.uuid4()), profile["id"], cfg["audio_path"], cfg["transcript"], cfg.get("source_audio_path", cfg["audio_path"]), cfg.get("normalized_audio_path", ""), dur, sr, int(cfg.get("primary", True))))
        db.commit()
        say(f"Added sample: {Path(cfg['audio_path']).name} ({dur:.1f}s, {sr} Hz)")
        return
    if mode == "library_export":
        row = db.execute("select * from voice_profiles where name=?", (cfg.get("name",""),)).fetchone()
        if not row:
            raise RuntimeError("Profile not found")
        samples = [dict(r) for r in db.execute("select * from voice_samples where profile_id=?", (row["id"],))]
        data = {"profile": dict(row), "samples": samples, "exported_by": "Qwens Tts Buddy"}
        out = Path(cfg["export_path"])
        out.parent.mkdir(parents=True, exist_ok=True)
        out.write_text(json.dumps(data, indent=2, ensure_ascii=False), encoding="utf-8")
        say("Exported voice profile:", out)
        return
    if mode == "library_import":
        data = json.loads(Path(cfg["import_path"]).read_text(encoding="utf-8"))
        p = data["profile"]
        p["name"] = cfg.get("name") or p.get("name") or "Imported Voice"
        db.execute("""insert or replace into voice_profiles
            (id,name,voice_type,language,mode_label,speaker,description,personality,ref_audio,ref_text,checkpoint_path,effects_preset,voice_emotion,audio_processing)
            values(?,?,?,?,?,?,?,?,?,?,?,?,?,?)""",
            (p.get("id") or str(uuid.uuid4()), p["name"], p.get("voice_type","cloned"), p.get("language","English"),
             p.get("mode_label","Built-in Voices (0.6B)"), p.get("speaker","Ryan"), p.get("description",""),
             p.get("personality",""), p.get("ref_audio",""), p.get("ref_text",""), p.get("checkpoint_path",""),
             p.get("effects_preset", p.get("audio_processing","None")), p.get("voice_emotion","Natural"), p.get("audio_processing", p.get("effects_preset","None"))))
        db.commit()
        say("Imported voice profile:", p["name"])
        return
    raise RuntimeError("Unknown library mode: " + mode)

def versioned_path(path):
    path = Path(path)
    if not path.exists():
        return path
    stem, suffix = path.stem, path.suffix
    for i in range(1, 1000):
        candidate = path.with_name(f"{stem}_{i:03d}{suffix}")
        if not candidate.exists():
            return candidate
    raise RuntimeError(f"Could not find free output filename near {path}")

def apply_speed(y, speed):
    speed = float(speed or 1.0)
    speed = max(0.70, min(1.50, speed))
    if abs(speed - 1.0) < 0.001:
        return y
    try:
        import numpy as np
        import librosa
        y = np.asarray(y, dtype="float32")
        return librosa.effects.time_stretch(y, rate=speed)
    except Exception as e:
        say("Speed processing failed, saving original speed:", e)
        return y

def normalize_audio(y):
    try:
        import numpy as np
        y = np.asarray(y, dtype="float32")
        peak = float(np.max(np.abs(y))) if len(y) else 0.0
        if peak > 0:
            y = y * min(0.98 / peak, 8.0)
        return y
    except Exception:
        return y

def split_text_into_chunks(text, max_chars=800):
    text = (text or "").strip()
    if not text or len(text) <= max_chars:
        return [text] if text else []
    chunks, rest = [], text
    while rest:
        rest = rest.lstrip()
        if len(rest) <= max_chars:
            chunks.append(rest)
            break
        seg = rest[:max_chars]
        split = -1
        for m in re.finditer(r"[.!?。！？](?:\s|$)", seg):
            if seg[:m.start()].rfind("[") > seg[:m.start()].rfind("]"):
                continue
            split = m.start()
        if split < 0:
            for ch in [";", ":", ",", " "]:
                split = seg.rfind(ch)
                if split > 0:
                    break
        if split < 0:
            split = max_chars - 1
        chunks.append(rest[:split + 1].strip())
        rest = rest[split + 1:]
    return chunks

def split_text_into_chunks_voicebox(text, max_chars=800):
    text = (text or "").strip()
    if not text:
        return []
    if len(text) <= max_chars:
        return [text]
    abbreviations = {
        "mr","mrs","ms","dr","prof","sr","jr","st","ave","blvd","inc","ltd","corp",
        "dept","est","approx","vs","etc","e.g","i.e","a.m","p.m","u.s","u.s.a","u.k"
    }
    chunks, rest = [], text
    while rest:
        rest = rest.lstrip()
        if len(rest) <= max_chars:
            chunks.append(rest)
            break
        seg = rest[:max_chars]
        split = -1
        for m in re.finditer(r"[.!?\u3002\uff01\uff1f](?:\s|$)", seg):
            pos = m.start()
            if seg[:pos].rfind("[") > seg[:pos].rfind("]"):
                continue
            if seg[pos] == ".":
                word_start = pos - 1
                while word_start >= 0 and (seg[word_start].isalpha() or seg[word_start] == "."):
                    word_start -= 1
                word = seg[word_start + 1:pos].lower().strip(".")
                if word in abbreviations:
                    continue
                if word_start >= 0 and seg[word_start].isdigit():
                    continue
            split = pos
        if split < 0:
            for m in re.finditer(r"[;:,\u2014](?:\s|$)", seg):
                if seg[:m.start()].rfind("[") <= seg[:m.start()].rfind("]"):
                    split = m.start()
        if split < 0:
            split = seg.rfind(" ")
        if split < 0:
            split = max_chars - 1
            for m in re.finditer(r"\[[^\]]*\]", seg):
                if m.start() < split < m.end():
                    split = max(0, m.start() - 1)
                    break
        chunk = rest[:split + 1].strip()
        if chunk:
            chunks.append(chunk)
        rest = rest[split + 1:]
    return chunks

def expand_performance(text, instruct, preset):
    preset = preset or "Natural"
    legacy = {
        "Custom": "Natural",
        "Funny Laugh Ending": "Funny with laugh ending",
        "Crazy Voice": "Crazy / unhinged",
        "Creepy Voice": "Creepy / tense",
        "Ghost Whisper": "Creepy / tense",
        "Monster Deep": "Crazy / unhinged",
        "Villain Radio": "Angry",
    }
    preset = legacy.get(preset, preset)
    text = text or ""
    instruct = instruct or ""
    original_lower = text.lower()
    tags = []
    tag_rules = {
        "[laugh]": (" ha ha.", "Include a short natural laugh where marked."),
        "[chuckle]": (" heh heh.", "Include a small amused chuckle where marked."),
        "[gasp]": (" gasp.", "Include a quick surprised gasp where marked."),
        "[sigh]": (" sigh.", "Include a tired audible sigh where marked."),
        "[whisper]": ("", "Whisper the marked phrase quietly and close to the microphone."),
    }
    for tag, (replacement, hint) in tag_rules.items():
        if tag in original_lower and hint not in tags:
            tags.append(hint)
        text = re.sub(re.escape(tag), replacement, text, flags=re.IGNORECASE)
    presets = {
        "Natural": ("", ""),
        "Calm": ("Speak calmly, clearly, and naturally with a steady pace.", ""),
        "Angry": ("Speak with controlled anger, clipped phrasing, and strong emphasis.", ""),
        "Scared / paranoid": ("Act terrified and paranoid. Speak in a tense, shaky voice with uneven breathing, nervous pauses, and whispered urgency, like you believe someone is secretly following you.", ""),
        "Whisper": ("Speak in a quiet whisper, intimate and close to the microphone.", ""),
        "Excited": ("Speak with energetic excitement, bright tone, and quick emotional lift.", ""),
        "Sad": ("Speak sadly and softly, with a tired, emotional tone.", ""),
        "Urgent": ("Speak urgently with fast tension and clear warning in the voice.", ""),
        "Funny with laugh ending": ("Deliver this with playful comic timing. End with a short natural laugh.", " [laugh]"),
        "Crazy / unhinged": ("Act wildly unhinged and unpredictable, with nervous bursts, odd emphasis, manic energy, and sudden tonal shifts.", ""),
        "Creepy / tense": ("Speak quietly and disturbingly, slow and tense, like a creepy character telling a secret.", ""),
    }
    extra, suffix = presets.get(preset, ("", ""))
    if suffix and "[laugh]" not in original_lower:
        text = text.rstrip() + suffix
        if "End with a short natural laugh." not in tags:
            tags.append("End with a short natural laugh.")
    parts = [p for p in [instruct.strip(), extra] + tags if p]
    return text.strip(), " ".join(parts).strip()

def concat_chunks(chunks, sr, crossfade_ms):
    import numpy as np
    if not chunks:
        return np.array([], dtype="float32")
    result = np.asarray(chunks[0], dtype="float32")
    fade = max(0, int(sr * float(crossfade_ms or 0) / 1000.0))
    for chunk in chunks[1:]:
        chunk = np.asarray(chunk, dtype="float32")
        overlap = min(fade, len(result), len(chunk))
        if overlap > 0:
            out = np.linspace(1.0, 0.0, overlap, dtype="float32")
            inn = np.linspace(0.0, 1.0, overlap, dtype="float32")
            result[-overlap:] = result[-overlap:] * out + chunk[:overlap] * inn
            result = np.concatenate([result, chunk[overlap:]])
        else:
            result = np.concatenate([result, chunk])
    return result

def apply_effects(y, sr, preset_name, root):
    preset_name = preset_name or "None"
    if preset_name == "None":
        return y
    db = connect_db(root)
    row = db.execute("select chain_json from effect_presets where name=?", (preset_name,)).fetchone()
    chain = json.loads(row["chain_json"]) if row else []
    if not chain:
        return y
    try:
        import numpy as np
        from pedalboard import Pedalboard, Reverb, Delay, Chorus, Compressor, Gain, HighpassFilter, LowpassFilter, PitchShift
        plugins = []
        for fx in chain:
            t = fx.get("type")
            if t == "reverb": plugins.append(Reverb(room_size=float(fx.get("room", 0.5))))
            elif t == "delay": plugins.append(Delay(delay_seconds=float(fx.get("seconds", 0.25)), mix=float(fx.get("mix", 0.25))))
            elif t == "chorus": plugins.append(Chorus(rate_hz=float(fx.get("rate", 1.0)), depth=float(fx.get("depth", 0.5))))
            elif t == "compressor": plugins.append(Compressor())
            elif t == "gain": plugins.append(Gain(gain_db=float(fx.get("db", 0.0))))
            elif t == "highpass": plugins.append(HighpassFilter(cutoff_frequency_hz=float(fx.get("cutoff", 80))))
            elif t == "lowpass": plugins.append(LowpassFilter(cutoff_frequency_hz=float(fx.get("cutoff", 8000))))
            elif t == "pitch": plugins.append(PitchShift(semitones=float(fx.get("semitones", 0))))
        if plugins:
            arr = np.asarray(y, dtype="float32")
            return Pedalboard(plugins)(arr[np.newaxis, :], sr)[0]
    except Exception as e:
        say("Pedalboard effects unavailable, using fallback where possible:", e)
    try:
        import numpy as np, librosa
        from scipy import signal
        arr = np.asarray(y, dtype="float32")
        def clamp_audio(a):
            return np.clip(a, -1.0, 1.0).astype("float32")
        def lowpass(a, cutoff):
            sos = signal.butter(4, min(float(cutoff), sr * 0.45), "lowpass", fs=sr, output="sos")
            return signal.sosfilt(sos, a).astype("float32")
        def highpass(a, cutoff):
            sos = signal.butter(4, max(float(cutoff), 20.0), "highpass", fs=sr, output="sos")
            return signal.sosfilt(sos, a).astype("float32")
        def simple_delay(a, seconds, mix=0.25, feedback=0.25):
            delay = max(1, int(sr * float(seconds)))
            out = np.copy(a)
            if delay < len(out):
                out[delay:] += a[:-delay] * float(mix)
                if delay * 2 < len(out):
                    out[delay * 2:] += a[:-delay * 2] * float(mix) * float(feedback)
            return clamp_audio(out)
        def simple_reverb(a, room=0.5):
            out = np.copy(a)
            for ms, gain in [(35, 0.18), (71, 0.12), (113, 0.08), (173, 0.05)]:
                d = int(sr * ms / 1000)
                if d < len(out):
                    out[d:] += a[:-d] * gain * float(room)
            return clamp_audio(out)
        def simple_chorus(a, depth=0.5, rate=1.0):
            idx = np.arange(len(a), dtype="float32")
            max_delay = int(sr * 0.018 * max(0.1, float(depth)))
            delay = (max_delay * (0.5 + 0.5 * np.sin(2 * np.pi * float(rate) * idx / sr))).astype("int32")
            src = np.maximum(0, idx.astype("int32") - delay)
            return clamp_audio(a * 0.72 + a[src] * 0.28)
        def simple_compressor(a):
            threshold = 0.35
            ratio = 4.0
            mag = np.abs(a)
            over = mag > threshold
            out = np.copy(a)
            out[over] = np.sign(a[over]) * (threshold + (mag[over] - threshold) / ratio)
            return clamp_audio(out * 1.15)
        for fx in chain:
            t = fx.get("type")
            if t == "pitch":
                arr = librosa.effects.pitch_shift(arr, sr=sr, n_steps=float(fx.get("semitones", 0)))
            elif t == "gain":
                arr = arr * (10 ** (float(fx.get("db", 0)) / 20.0))
            elif t == "highpass":
                arr = highpass(arr, fx.get("cutoff", 80))
            elif t == "lowpass":
                arr = lowpass(arr, fx.get("cutoff", 8000))
            elif t == "compressor":
                arr = simple_compressor(arr)
            elif t == "delay":
                arr = simple_delay(arr, fx.get("seconds", 0.25), fx.get("mix", 0.25))
            elif t == "reverb":
                arr = simple_reverb(arr, fx.get("room", 0.5))
            elif t == "chorus":
                arr = simple_chorus(arr, fx.get("depth", 0.5), fx.get("rate", 1.0))
        say("Applied effects preset:", preset_name)
        return clamp_audio(arr)
    except Exception as e:
        say("Effects fallback failed; saved dry audio:", e)
        return y

def apply_effects(y, sr, preset_name, root):
    import numpy as np
    from scipy import signal
    preset_name = preset_name or "None"
    if preset_name == "None":
        say("Audio effect: None")
        return np.asarray(y, dtype="float32")
    db = connect_db(root)
    row = db.execute("select chain_json from effect_presets where name=?", (preset_name,)).fetchone()
    chain = json.loads(row["chain_json"]) if row else []
    arr = np.asarray(y, dtype="float32")
    if not chain:
        say("Audio effect skipped: unknown or empty preset:", preset_name)
        return arr

    def clamp_audio(a):
        return np.clip(np.nan_to_num(a), -1.0, 1.0).astype("float32")
    def gain(a, db):
        return a * (10 ** (float(db) / 20.0))
    def highpass(a, cutoff):
        cutoff = max(20.0, min(float(cutoff), sr * 0.45))
        sos = signal.butter(4, cutoff, "highpass", fs=sr, output="sos")
        return signal.sosfilt(sos, a).astype("float32")
    def lowpass(a, cutoff):
        cutoff = max(30.0, min(float(cutoff), sr * 0.45))
        sos = signal.butter(4, cutoff, "lowpass", fs=sr, output="sos")
        return signal.sosfilt(sos, a).astype("float32")
    def compressor(a, threshold=0.28, ratio=4.0):
        threshold = max(0.02, float(threshold))
        ratio = max(1.0, float(ratio))
        mag = np.abs(a)
        out = np.copy(a)
        over = mag > threshold
        out[over] = np.sign(a[over]) * (threshold + (mag[over] - threshold) / ratio)
        return out * 1.08
    def limiter(a, ceiling=0.96):
        peak = float(np.max(np.abs(a))) if len(a) else 0.0
        if peak > ceiling:
            a = a * (float(ceiling) / peak)
        return a
    def delay(a, seconds=0.25, mix=0.25, feedback=0.25):
        d = max(1, int(sr * float(seconds)))
        out = np.copy(a)
        if d < len(out):
            out[d:] += a[:-d] * float(mix)
            if d * 2 < len(out):
                out[d * 2:] += a[:-d * 2] * float(mix) * float(feedback)
        return out
    def reverb(a, room=0.5):
        out = np.copy(a)
        room = float(room)
        for ms, g in [(29, 0.16), (53, 0.12), (91, 0.09), (137, 0.065), (211, 0.045)]:
            d = int(sr * ms / 1000)
            if d < len(out):
                out[d:] += a[:-d] * g * room
        return out
    def chorus(a, depth=0.45, rate=1.2):
        idx = np.arange(len(a), dtype="float32")
        max_delay = int(sr * 0.018 * max(0.05, float(depth)))
        lfo = 0.5 + 0.5 * np.sin(2 * np.pi * float(rate) * idx / sr)
        src = np.maximum(0, idx.astype("int32") - (max_delay * lfo).astype("int32"))
        return a * 0.72 + a[src] * 0.28
    def wobble(a, depth=0.004, rate=5.0):
        idx = np.arange(len(a), dtype="float32")
        shift = (sr * float(depth) * np.sin(2 * np.pi * float(rate) * idx / sr)).astype("int32")
        src = np.clip(idx.astype("int32") + shift, 0, len(a) - 1)
        return a[src]
    def tremolo(a, depth=0.2, rate=6.0):
        idx = np.arange(len(a), dtype="float32")
        mod = 1.0 - float(depth) * (0.5 + 0.5 * np.sin(2 * np.pi * float(rate) * idx / sr))
        return a * mod
    def distortion(a, drive=1.5):
        return np.tanh(a * float(drive)) / np.tanh(float(drive))
    def pitch(a, semitones=0.0):
        semitones = float(semitones)
        if abs(semitones) < 0.01 or len(a) < 32:
            return a
        factor = 2 ** (semitones / 12.0)
        temp_len = max(32, int(len(a) / factor))
        temp = signal.resample(a, temp_len)
        return signal.resample(temp, len(a)).astype("float32")

    applied = []
    for fx in chain:
        t = fx.get("type")
        try:
            if t == "pitch":
                arr = pitch(arr, fx.get("semitones", 0)); applied.append("pitch")
            elif t == "gain":
                arr = gain(arr, fx.get("db", 0)); applied.append("gain")
            elif t == "highpass":
                arr = highpass(arr, fx.get("cutoff", 80)); applied.append("highpass")
            elif t == "lowpass":
                arr = lowpass(arr, fx.get("cutoff", 8000)); applied.append("lowpass")
            elif t == "compressor":
                arr = compressor(arr, fx.get("threshold", 0.28), fx.get("ratio", 4.0)); applied.append("compressor")
            elif t == "delay":
                arr = delay(arr, fx.get("seconds", 0.25), fx.get("mix", 0.25), fx.get("feedback", 0.25)); applied.append("delay")
            elif t == "reverb":
                arr = reverb(arr, fx.get("room", 0.5)); applied.append("reverb")
            elif t == "chorus":
                arr = chorus(arr, fx.get("depth", 0.45), fx.get("rate", 1.2)); applied.append("chorus")
            elif t == "wobble":
                arr = wobble(arr, fx.get("depth", 0.004), fx.get("rate", 5.0)); applied.append("wobble")
            elif t == "tremolo":
                arr = tremolo(arr, fx.get("depth", 0.2), fx.get("rate", 6.0)); applied.append("tremolo")
            elif t == "distortion":
                arr = distortion(arr, fx.get("drive", 1.5)); applied.append("distortion")
            else:
                say("Audio effect skipped unknown step:", t)
        except Exception as e:
            say("Audio effect step failed:", t, e)
    arr = clamp_audio(limiter(arr))
    say("Applied audio effect:", preset_name, "| steps:", ", ".join(applied) if applied else "none")
    return arr

def safe_slug(name):
    return re.sub(r"[^a-z0-9]+", "_", (name or "effect").lower()).strip("_") or "effect"

def write_effect_version(root, source_path, final_path, preset_name):
    try:
        db = connect_db(root)
        row = db.execute("select id from generations where output_path=? order by created_at desc limit 1", (str(source_path),)).fetchone()
        if not row:
            row = db.execute("select id from generations order by created_at desc limit 1").fetchone()
        if row:
            db.execute("insert into generation_versions(id,generation_id,label,audio_path,effects_preset) values(?,?,?,?,?)",
                       (str(uuid.uuid4()), row["id"], "effect: " + preset_name, str(final_path), preset_name))
            db.commit()
            say("Version:", "effect: " + preset_name)
    except Exception as e:
        say("Version save failed:", e)

def test_effects(root):
    import numpy as np
    import soundfile as sf
    out_dir = root / "qwen_studio_data" / "effect_tests"
    out_dir.mkdir(parents=True, exist_ok=True)
    db = connect_db(root)
    names = [r["name"] for r in db.execute("select name from effect_presets order by name") if r["name"] != "None"]
    sr = 16000
    t = np.linspace(0, 0.45, int(sr * 0.45), endpoint=False, dtype="float32")
    y = (0.14 * np.sin(2 * np.pi * 220 * t) + 0.07 * np.sin(2 * np.pi * 440 * t)).astype("float32")
    ok = 0
    for name in names:
        processed = apply_effects(y, sr, name, root)
        n = min(len(y), len(processed))
        diff = float(np.mean(np.abs(processed[:n] - y[:n]))) if n else 0.0
        peak = float(np.max(np.abs(processed))) if len(processed) else 0.0
        out = out_dir / (safe_slug(name) + ".wav")
        sf.write(str(out), processed, sr, format="WAV")
        passed = len(processed) > 0 and peak > 0 and (diff > 0.0001 or name == "Clean Narration")
        say(("PASS" if passed else "WARN"), name, f"peak={peak:.3f}", f"diff={diff:.6f}", "file=" + str(out))
        ok += 1 if passed else 0
    say(f"Effects self-test complete: {ok}/{len(names)} passed")

def apply_effect_to_file(cfg, root):
    import soundfile as sf
    source = Path(cfg["input_path"])
    if not source.exists():
        raise RuntimeError("Output file does not exist yet: " + str(source))
    preset = cfg.get("effects_preset", "None")
    if preset == "None":
        raise RuntimeError("Choose Audio Processing other than None first.")
    y, sr = sf.read(str(source), dtype="float32")
    if getattr(y, "ndim", 1) > 1:
        y = y.mean(axis=1)
    processed = apply_effects(y, sr, preset, root)
    target = Path(cfg.get("output_path") or source)
    target = target.with_name(target.stem + "_" + safe_slug(preset) + target.suffix)
    final_path = write_audio_safe(processed, sr, str(target), cfg.get("format", target.suffix.replace(".", "") or "wav"), 1.0)
    write_effect_version(root, source, final_path, preset)
    say("Saved:", final_path)

def normalize_audio_file(cfg, root):
    import numpy as np
    import soundfile as sf
    source = Path(cfg["input_path"])
    if not source.exists():
        raise RuntimeError("Audio file does not exist: " + str(source))
    output = Path(cfg["output_path"])
    output.parent.mkdir(parents=True, exist_ok=True)
    y, sr = sf.read(str(source), dtype="float32")
    if getattr(y, "ndim", 1) > 1:
        y = y.mean(axis=1)
    peak = float(np.max(np.abs(y))) if len(y) else 0.0
    if peak > 0:
        y = y * min(0.98 / peak, 4.0)
    sf.write(str(output), y, sr, format="WAV")
    say("Normalized WAV:", output)

def write_audio_safe(y, sr, output_path, fmt, speed):
    import soundfile as sf
    output = Path(output_path)
    output.parent.mkdir(parents=True, exist_ok=True)
    fmt = (fmt or output.suffix.replace(".", "") or "wav").lower()
    requested = output.with_suffix("." + fmt)
    final_path = versioned_path(requested)
    tmp = final_path.with_name(final_path.stem + ".tmp" + final_path.suffix)
    y = apply_speed(y, speed)
    try:
        sf.write(str(tmp), y, sr, format=fmt.upper())
        try:
            os.replace(tmp, final_path)
        except PermissionError:
            final_path = versioned_path(final_path)
            os.replace(tmp, final_path)
    except Exception as e:
        fallback = versioned_path(requested.with_suffix(".wav"))
        tmp_wav = fallback.with_name(fallback.stem + ".tmp.wav")
        say(f"{fmt.upper()} export failed, saved WAV fallback instead:", e)
        sf.write(str(tmp_wav), y, sr, format="WAV")
        os.replace(tmp_wav, fallback)
        final_path = fallback
    return str(final_path)

def record_generation(cfg, root, final_path, status="completed", error="", original_path=None):
    try:
        db = connect_db(root)
        gen_id = cfg.get("generation_id") or str(uuid.uuid4())
        dur, _ = duration_info(final_path)
        db.execute("""insert or replace into generations
            (id,profile_id,profile_name,text,language,speaker,mode,instruct,output_path,format,speed,temperature,voice_emotion,audio_processing,status,error,duration)
            values(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?)""",
            (gen_id, cfg.get("voice_profile_id",""), cfg.get("voice_profile_name",""), cfg.get("text",""),
             cfg.get("language",""), cfg.get("speaker",""), cfg.get("mode",""), cfg.get("instruct",""),
             final_path, cfg.get("format","wav"), float(cfg.get("speed",1.0)), float(cfg.get("temperature",0.7)),
             cfg.get("performance_preset","Natural"), cfg.get("effects_preset","None"), status, error, dur))
        original = original_path or final_path
        db.execute("insert into generation_versions(id,generation_id,label,audio_path,effects_preset) values(?,?,?,?,?)",
                   (str(uuid.uuid4()), gen_id, "original", original, "None"))
        if final_path != original:
            db.execute("insert into generation_versions(id,generation_id,label,audio_path,effects_preset) values(?,?,?,?,?)",
                       (str(uuid.uuid4()), gen_id, "effect: " + cfg.get("effects_preset","processed"), final_path, cfg.get("effects_preset","None")))
        db.commit()
        say("History:", gen_id)
    except Exception as e:
        say("History save failed:", e)

def main():
    with open(sys.argv[1], "r", encoding="utf-8") as f:
        cfg = json.load(f)
    root = Path(cfg["qwen_root"])
    os.environ.setdefault("PYTHONUTF8", "1")
    os.environ.setdefault("PYTHONIOENCODING", "utf-8")
    os.environ.setdefault("HF_HOME", str(root / ".hf-home"))
    if cfg.get("offline", True):
        os.environ["HF_HUB_OFFLINE"] = "1"
        os.environ["TRANSFORMERS_OFFLINE"] = "1"

    mode = cfg["mode"]
    if mode.startswith("library_"):
        library_mode(cfg, root)
        return
    if mode == "test_effects":
        test_effects(root)
        return
    if mode == "apply_effect_to_file":
        apply_effect_to_file(cfg, root)
        return
    if mode == "normalize_audio_file":
        normalize_audio_file(cfg, root)
        return

    if mode == "download_model":
        os.environ.pop("HF_HUB_OFFLINE", None)
        os.environ.pop("TRANSFORMERS_OFFLINE", None)
        cmd = [str(root / "python312-embed" / "Scripts" / "huggingface-cli.exe"), "download", cfg["repo_id"], "--local-dir", cfg["target_dir"]]
        say("Downloading", cfg["repo_id"])
        raise SystemExit(subprocess.call(cmd))

    if mode in ("custom_voice", "voice_design", "voice_clone", "fine_tune_train") and not Path(cfg["model_path"]).exists():
        say("Missing model folder:", cfg["model_path"])
        say("Open the Models page and download:", Path(cfg["model_path"]).name)
        raise SystemExit(12)

    import torch, soundfile as sf
    from qwen_tts import Qwen3TTSModel

    def load(path):
        dtype = {"bfloat16": torch.bfloat16, "float16": torch.float16, "float32": torch.float32}.get(cfg.get("dtype"), torch.bfloat16)
        attn = cfg.get("attention", "auto")
        if attn == "auto":
            try:
                import flash_attn
                attn = "flash_attention_2"
            except Exception:
                attn = "sdpa"
        kw = {"device_map": cfg.get("device", "cuda:0"), "dtype": dtype, "attn_implementation": attn}
        try:
            return Qwen3TTSModel.from_pretrained(path, **kw)
        except Exception:
            kw["attn_implementation"] = "sdpa"
            return Qwen3TTSModel.from_pretrained(path, **kw)

    gen = dict(
        max_new_tokens=int(cfg.get("max_new_tokens", 2048)),
        top_p=float(cfg.get("top_p", 0.95)),
        top_k=int(cfg.get("top_k", 50)),
        temperature=float(cfg.get("temperature", 0.7)),
        repetition_penalty=float(cfg.get("repetition_penalty", 1.0)),
    )

    say("CUDA:", torch.cuda.is_available())
    if torch.cuda.is_available(): say("GPU:", torch.cuda.get_device_name(0))

    if mode in ("custom_voice", "voice_design", "voice_clone"):
        model = load(cfg["model_path"])
        max_chunk = int(cfg.get("max_chunk_chars", 800) or 800)
        crossfade = int(cfg.get("crossfade_ms", 50) or 50)
        text_for_tts, instruct_for_tts = expand_performance(cfg["text"], cfg.get("instruct", ""), cfg.get("performance_preset", "Custom"))
        cfg["text"] = text_for_tts
        cfg["instruct"] = instruct_for_tts
        chunks = split_text_into_chunks_voicebox(text_for_tts, max_chunk)
        if len(chunks) > 1:
            say(f"Long text: split into {len(chunks)} chunks with {crossfade} ms crossfade.")
        audio_chunks = []
        sr = None
        for i, chunk_text in enumerate(chunks):
            if len(chunks) > 1:
                say(f"Generating chunk {i + 1}/{len(chunks)}")
            chunk_gen = dict(gen)
            if mode == "custom_voice":
                wavs, this_sr = model.generate_custom_voice(text=chunk_text, language=cfg["language"], speaker=cfg["speaker"], instruct=cfg.get("instruct", ""), **chunk_gen)
            elif mode == "voice_design":
                wavs, this_sr = model.generate_voice_design(text=chunk_text, language=cfg["language"], instruct=cfg.get("instruct", ""), **chunk_gen)
            else:
                wavs, this_sr = model.generate_voice_clone(text=chunk_text, language=cfg["language"], ref_audio=cfg["ref_audio"], ref_text=cfg.get("ref_text", ""), x_vector_only_mode=bool(cfg.get("x_vector_only_mode", False)), **chunk_gen)
            audio_chunks.append(wavs[0])
            sr = this_sr
        y = concat_chunks(audio_chunks, sr, crossfade)
        if cfg.get("normalize", True):
            y = normalize_audio(y)
        dry_path = write_audio_safe(y, sr, cfg["output_path"], cfg["format"], cfg.get("speed", 1.0))
        final_path = dry_path
        if cfg.get("effects_preset", "None") != "None":
            y2 = apply_effects(y, sr, cfg.get("effects_preset", "None"), root)
            p = Path(dry_path)
            processed = str(p.with_name(p.stem + "_" + cfg.get("effects_preset", "processed").replace(" ", "_").lower() + p.suffix))
            final_path = write_audio_safe(y2, sr, processed, cfg["format"], cfg.get("speed", 1.0))
            say("Effects version:", final_path)
        say("Saved:", final_path)
        record_generation(cfg, root, final_path, original_path=dry_path)
        return

    if mode == "fine_tune_prepare":
        prep = root / "models" / "Qwen3-TTS" / "finetuning" / "prepare_data.py"
        cmd = [sys.executable, str(prep), "--device", cfg["device"], "--tokenizer_model_path", cfg["tokenizer_path"], "--input_jsonl", cfg["raw_jsonl"], "--output_jsonl", cfg["coded_jsonl"]]
        raise SystemExit(subprocess.call(cmd, cwd=str(prep.parent)))

    if mode == "fine_tune_train":
        train = root / "models" / "Qwen3-TTS" / "finetuning" / "sft_12hz.py"
        cmd = [sys.executable, str(train), "--init_model_path", cfg["model_path"], "--output_model_path", cfg["output_model_path"], "--train_jsonl", cfg["coded_jsonl"], "--batch_size", str(cfg["batch_size"]), "--lr", str(cfg["lr"]), "--num_epochs", str(cfg["epochs"]), "--speaker_name", cfg["speaker_name"]]
        raise SystemExit(subprocess.call(cmd, cwd=str(train.parent)))

if __name__ == "__main__":
    main()
)PY";
}

DWORD RunProcess(const std::wstring& cmd, const std::wstring& cwd, std::wstring& out) {
    SECURITY_ATTRIBUTES sa{ sizeof(sa), nullptr, TRUE };
    HANDLE r{}, w{};
    if (!CreatePipe(&r, &w, &sa, 0)) return 9001;
    SetHandleInformation(r, HANDLE_FLAG_INHERIT, 0);
    STARTUPINFOW si{ sizeof(si) };
    PROCESS_INFORMATION pi{};
    si.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
    si.hStdOutput = w;
    si.hStdError = w;
    si.wShowWindow = SW_HIDE;
    std::wstring c = cmd;
    BOOL ok = CreateProcessW(nullptr, c.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, cwd.c_str(), &si, &pi);
    CloseHandle(w);
    if (!ok) { CloseHandle(r); return GetLastError(); }
    char b[4096]; DWORD n = 0;
    while (ReadFile(r, b, sizeof(b), &n, nullptr) && n) out += Utf8ToWide(std::string(b, b + n));
    WaitForSingleObject(pi.hProcess, INFINITE);
    DWORD code = 0;
    GetExitCodeProcess(pi.hProcess, &code);
    CloseHandle(pi.hThread); CloseHandle(pi.hProcess); CloseHandle(r);
    return code;
}

DWORD RunBackendSync(const std::string& json, std::wstring& out) {
    std::wstring py = TempFile(L"qwen_studio_backend.py");
    std::wstring cfg = TempFile(L"qwen_studio_job.json");
    WriteFileUtf8(py, BackendPy());
    WriteFileUtf8(cfg, json + "}");
    std::wstring python = std::wstring(QwenRoot) + L"\\python312-embed\\python.exe";
    DWORD code = RunProcess(Quote(python) + L" " + Quote(py) + L" " + Quote(cfg), QwenRoot, out);
    out = CleanBackendOutput(out);
    return code;
}

std::wstring SaveDialog(const wchar_t* filter, const wchar_t* defExt, const std::wstring& current) {
    wchar_t file[MAX_PATH]{};
    wcscpy_s(file, current.c_str());
    OPENFILENAMEW ofn{ sizeof(ofn) };
    ofn.hwndOwner = g.hwnd; ofn.lpstrFilter = filter; ofn.lpstrFile = file; ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST; ofn.lpstrDefExt = defExt;
    return GetSaveFileNameW(&ofn) ? std::wstring(file) : L"";
}

std::wstring OpenDialog(const wchar_t* filter) {
    wchar_t file[MAX_PATH]{};
    OPENFILENAMEW ofn{ sizeof(ofn) };
    ofn.hwndOwner = g.hwnd; ofn.lpstrFilter = filter; ofn.lpstrFile = file; ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    return GetOpenFileNameW(&ofn) ? std::wstring(file) : L"";
}

std::wstring CurrentModelForMode(const std::wstring& mode) {
    if (mode.find(L"Voice Design") != std::wstring::npos) return ModelDir(L"Qwen3-TTS-12Hz-1.7B-VoiceDesign");
    if (mode.find(L"Strong") != std::wstring::npos) return ModelDir(L"Qwen3-TTS-12Hz-1.7B-CustomVoice");
    return ModelDir(L"Qwen3-TTS-12Hz-0.6B-CustomVoice");
}

void ComboSelectExact(HWND h, const std::wstring& value) {
    int count = (int)SendMessageW(h, CB_GETCOUNT, 0, 0);
    for (int i = 0; i < count; ++i) {
        int n = (int)SendMessageW(h, CB_GETLBTEXTLEN, i, 0);
        std::wstring s(n + 1, 0);
        SendMessageW(h, CB_GETLBTEXT, i, (LPARAM)s.data());
        s.resize(n);
        if (s == value || s.find(value) != std::wstring::npos) {
            SendMessageW(h, CB_SETCURSEL, i, 0);
            return;
        }
    }
}

VoiceProfile* SelectedVoiceProfile() {
    std::wstring name = Combo(g.h[IDC_VOICE_PROFILE]);
    if (name.empty()) return nullptr;
    for (auto& v : g.voices) if (v.name == name) return &v;
    return nullptr;
}

bool IsEmotionalInstruction(const std::wstring& s) {
    std::wstring x = s;
    for (auto& c : x) c = (wchar_t)towlower(c);
    const std::vector<std::wstring> keys = {
        L"scared", L"paranoid", L"terrified", L"fear", L"afraid", L"panic",
        L"angry", L"furious", L"whisper", L"excited", L"sad", L"urgent",
        L"crazy", L"creepy", L"villain", L"ghost", L"monster", L"laugh"
    };
    for (auto& k : keys) if (x.find(k) != std::wstring::npos) return true;
    return false;
}

void UpdateGenerateVoiceControls() {
    VoiceProfile* vp = SelectedVoiceProfile();
    if (!vp) {
        if (!g.voices.empty()) {
            ComboSelectExact(g.h[IDC_VOICE_PROFILE], g.voices.front().name);
            vp = SelectedVoiceProfile();
        }
        if (!vp) {
            SetStatus(L"Voice Library is empty - refresh or save a voice profile");
            return;
        }
    }
    ComboSelectExact(g.h[IDC_LANGUAGE], vp->language);
    ComboSelectExact(g.h[IDC_SPEAKER], vp->speaker);
    ComboSelectExact(g.h[IDC_MODE], vp->mode);
    ComboSelectExact(g.h[IDC_PRESET], NormalizeVoiceEmotion(vp->emotion));
    ComboSelectExact(g.h[IDC_EFFECT_PRESET], NormalizeAudioProcessing(vp->effects.empty() ? L"None" : vp->effects));
    EnableWindow(g.h[IDC_LANGUAGE], TRUE);
    EnableWindow(g.h[IDC_MODE], TRUE);
    EnableWindow(g.h[IDC_SPEAKER], FALSE);
    if (vp->type == L"cloned" || vp->type == L"fine_tuned") {
        SetStatus(L"Saved profile loaded; mode/language can be adjusted for this generation");
    } else {
        SetStatus(L"Voice profile loaded; mode/language can be adjusted");
    }
}

std::string BaseJob(const std::wstring& mode, const std::wstring& model, const std::wstring& text, const std::wstring& output) {
    VoiceProfile* vp = SelectedVoiceProfile();
    std::wstring language = Combo(g.h[IDC_LANGUAGE]);
    std::wstring speaker = (vp && !vp->speaker.empty()) ? vp->speaker : Combo(g.h[IDC_SPEAKER]);
    std::ostringstream j;
    j << "{";
    j << "\"qwen_root\":\"" << Json(QwenRoot) << "\",";
    j << "\"mode\":\"" << Json(mode) << "\",";
    j << "\"model_path\":\"" << Json(model) << "\",";
    j << "\"tokenizer_path\":\"" << Json(ModelDir(L"Qwen3-TTS-Tokenizer-12Hz")) << "\",";
    j << "\"text\":\"" << Json(text) << "\",";
    j << "\"language\":\"" << Json(language) << "\",";
    j << "\"speaker\":\"" << Json(speaker) << "\",";
    j << "\"instruct\":\"" << Json(Text(g.h[IDC_INSTRUCT])) << "\",";
    j << "\"output_path\":\"" << Json(output) << "\",";
    j << "\"voice_profile_id\":\"" << Json(vp ? vp->id : L"") << "\",";
    j << "\"voice_profile_name\":\"" << Json(vp ? vp->name : L"") << "\",";
    j << "\"format\":\"" << Json(Combo(g.h[IDC_FORMAT])) << "\",";
    j << "\"device\":\"" << Json(Combo(g.h[IDC_DEVICE])) << "\",";
    j << "\"dtype\":\"" << Json(Combo(g.h[IDC_DTYPE])) << "\",";
    j << "\"attention\":\"" << Json(Combo(g.h[IDC_ATTN])) << "\",";
    j << "\"offline\":" << (SendMessageW(g.h[IDC_OFFLINE], BM_GETCHECK, 0, 0) == BST_CHECKED ? "true" : "false") << ",";
    j << "\"temperature\":" << D(IDC_TEMP, 0.7) << ",";
    j << "\"top_p\":" << D(IDC_TOPP, 0.95) << ",";
    j << "\"top_k\":" << I(IDC_TOPK, 50) << ",";
    j << "\"max_new_tokens\":" << I(IDC_MAXTOK, 2048) << ",";
    j << "\"repetition_penalty\":" << D(IDC_REPPEN, 1.0) << ",";
    j << "\"speed\":" << D(IDC_SPEED, 1.0) << ",";
    j << "\"max_chunk_chars\":" << I(IDC_MAX_CHUNK, 800) << ",";
    j << "\"crossfade_ms\":" << I(IDC_CROSSFADE, 50) << ",";
    j << "\"normalize\":" << (SendMessageW(g.h[IDC_NORMALIZE], BM_GETCHECK, 0, 0) == BST_CHECKED ? "true" : "false") << ",";
    j << "\"performance_preset\":\"" << Json(Combo(g.h[IDC_PRESET])) << "\",";
    j << "\"effects_preset\":\"" << Json(Combo(g.h[IDC_EFFECT_PRESET])) << "\"";
    return j.str();
}

void RunJob(const std::string& json) {
    if (g.running) return;
    g.running = true;
    EnableWindow(g.h[IDC_GENERATE], FALSE);
    EnableWindow(g.h[IDC_CLONE_GENERATE], FALSE);
    Text(g.h[IDC_LOG], L"");
    SetStatus(L"Running Qwen job...");
    std::thread([json] {
        std::wstring py = TempFile(L"qwen_studio_backend.py");
        std::wstring cfg = TempFile(L"qwen_studio_job.json");
        WriteFileUtf8(py, BackendPy());
        WriteFileUtf8(cfg, json + "}");
        std::wstring python = std::wstring(QwenRoot) + L"\\python312-embed\\python.exe";
        std::wstring out;
        DWORD code = RunProcess(Quote(python) + L" " + Quote(py) + L" " + Quote(cfg), QwenRoot, out);
        out = CleanBackendOutput(out);
        auto* heap = new std::wstring(out + L"\r\nExit code: " + std::to_wstring(code) + L"\r\n");
        PostMessageW(g.hwnd, WM_APP + 1, code, (LPARAM)heap);
    }).detach();
}

void UpdateOutputPathFromLog(const std::wstring& logText) {
    std::wstring marker = L"Saved:";
    size_t pos = logText.rfind(marker);
    if (pos == std::wstring::npos) return;
    pos += marker.size();
    while (pos < logText.size() && (logText[pos] == L' ' || logText[pos] == L'\t')) ++pos;
    size_t end = logText.find_first_of(L"\r\n", pos);
    std::wstring path = logText.substr(pos, end == std::wstring::npos ? std::wstring::npos : end - pos);
    if (!path.empty() && Exists(path)) Text(g.h[IDC_OUTPUT], path);
}

void RefreshLibrary() {
    std::wstring previousProfile = Combo(g.h[IDC_VOICE_PROFILE]);
    std::ostringstream j;
    j << "{\"qwen_root\":\"" << Json(QwenRoot) << "\",\"mode\":\"library_list\"";
    std::wstring out;
    DWORD code = RunBackendSync(j.str(), out);
    if (code != 0) {
        Log(L"Library refresh failed:\r\n" + out + L"\r\n");
        return;
    }
    g.voices.clear();
    g.history.clear();
    SendMessageW(g.h[IDC_LIB_LIST], LB_RESETCONTENT, 0, 0);
    SendMessageW(g.h[IDC_HISTORY_LIST], LB_RESETCONTENT, 0, 0);
    SendMessageW(g.h[IDC_VOICE_PROFILE], CB_RESETCONTENT, 0, 0);
    std::wstringstream ss(out);
    std::wstring line;
    while (std::getline(ss, line)) {
        if (line.rfind(L"VOICE_PROFILE_JSON:", 0) == 0) {
            std::wstring json = line.substr(19);
            VoiceProfile v;
            v.id = JsonField(json, "id"); v.name = JsonField(json, "name"); v.type = JsonField(json, "voice_type");
            v.language = JsonField(json, "language"); v.mode = JsonField(json, "mode_label"); v.speaker = JsonField(json, "speaker");
            v.description = JsonField(json, "description"); v.personality = JsonField(json, "personality"); v.refAudio = JsonField(json, "ref_audio");
            v.refText = JsonField(json, "ref_text"); v.checkpoint = JsonField(json, "checkpoint_path");
            v.effects = NormalizeAudioProcessing(JsonField(json, "audio_processing"));
            if (v.effects.empty() || v.effects == L"None") v.effects = NormalizeAudioProcessing(JsonField(json, "effects_preset"));
            v.emotion = NormalizeVoiceEmotion(JsonField(json, "voice_emotion"));
            g.voices.push_back(v);
            std::wstring display = v.name + L"  [" + v.type + L" / " + v.language + L"]";
            SendMessageW(g.h[IDC_LIB_LIST], LB_ADDSTRING, 0, (LPARAM)display.c_str());
            SendMessageW(g.h[IDC_VOICE_PROFILE], CB_ADDSTRING, 0, (LPARAM)v.name.c_str());
        } else if (line.rfind(L"GENERATION_JSON:", 0) == 0) {
            std::wstring json = line.substr(16);
            HistoryRow h;
            h.id = JsonField(json, "id"); h.profile = JsonField(json, "profile_name"); h.text = JsonField(json, "text");
            h.output = JsonField(json, "output_path"); h.status = JsonField(json, "status"); h.created = JsonField(json, "created_at");
            g.history.push_back(h);
            std::wstring display = h.created + L" | " + h.profile + L" | " + h.status + L" | " + h.text.substr(0, 70);
            SendMessageW(g.h[IDC_HISTORY_LIST], LB_ADDSTRING, 0, (LPARAM)display.c_str());
        }
    }
    if (!previousProfile.empty()) ComboSelectExact(g.h[IDC_VOICE_PROFILE], previousProfile);
    if (!SelectedVoiceProfile()) ComboSelectExact(g.h[IDC_VOICE_PROFILE], L"Ryan");
    if (!SelectedVoiceProfile()) ComboSelectExact(g.h[IDC_VOICE_PROFILE], L"Aiden");
    if (!SelectedVoiceProfile() && !g.voices.empty()) SendMessageW(g.h[IDC_VOICE_PROFILE], CB_SETCURSEL, 0, 0);
    UpdateGenerateVoiceControls();
}

void LoadLibrarySelection() {
    int idx = (int)SendMessageW(g.h[IDC_LIB_LIST], LB_GETCURSEL, 0, 0);
    if (idx < 0 || idx >= (int)g.voices.size()) return;
    auto& v = g.voices[idx];
    Text(g.h[IDC_LIB_NAME], v.name);
    ComboSelectExact(g.h[IDC_LIB_TYPE], v.type);
    ComboSelectExact(g.h[IDC_LIB_LANG], v.language);
    ComboSelectExact(g.h[IDC_LIB_MODE], v.mode);
    ComboSelectExact(g.h[IDC_LIB_SPEAKER], v.speaker);
    Text(g.h[IDC_LIB_DESC], v.description);
    Text(g.h[IDC_LIB_PERSONA], v.personality);
    Text(g.h[IDC_LIB_REF_AUDIO], v.refAudio);
    Text(g.h[IDC_LIB_REF_TEXT], v.refText);
    Text(g.h[IDC_LIB_CHECKPOINT], v.checkpoint);
    ComboSelectExact(g.h[IDC_LIB_EMOTION], NormalizeVoiceEmotion(v.emotion));
    ComboSelectExact(g.h[IDC_LIB_EFFECTS], NormalizeAudioProcessing(v.effects.empty() ? L"None" : v.effects));
    bool cloned = v.type == L"cloned";
    bool fine = v.type == L"fine_tuned";
    ShowWindow(g.h[IDC_LIB_REF_AUDIO], cloned ? SW_SHOW : SW_HIDE);
    ShowWindow(g.h[IDC_LIB_REF_TEXT], cloned ? SW_SHOW : SW_HIDE);
    ShowWindow(g.h[IDC_LIB_BROWSE_REF], cloned ? SW_SHOW : SW_HIDE);
    ShowWindow(g.h[IDC_LIB_CHECKPOINT], fine ? SW_SHOW : SW_HIDE);
    if (cloned && (Blank(v.refAudio) || Blank(v.refText))) Text(g.h[IDC_LIB_VALIDATION], L"Needs reference audio and transcript");
    else if (fine && (Blank(v.checkpoint) || !Exists(v.checkpoint))) Text(g.h[IDC_LIB_VALIDATION], L"Missing checkpoint folder");
    else Text(g.h[IDC_LIB_VALIDATION], L"Ready");
}

void UseLibraryVoice() {
    int idx = (int)SendMessageW(g.h[IDC_LIB_LIST], LB_GETCURSEL, 0, 0);
    if (idx < 0 || idx >= (int)g.voices.size()) return;
    auto& v = g.voices[idx];
    ComboSelectExact(g.h[IDC_VOICE_PROFILE], v.name);
    ComboSelectExact(g.h[IDC_LANGUAGE], v.language);
    ComboSelectExact(g.h[IDC_SPEAKER], v.speaker);
    ComboSelectExact(g.h[IDC_MODE], v.mode);
    ComboSelectExact(g.h[IDC_PRESET], NormalizeVoiceEmotion(v.emotion));
    ComboSelectExact(g.h[IDC_EFFECT_PRESET], NormalizeAudioProcessing(v.effects.empty() ? L"None" : v.effects));
    if (!v.refAudio.empty()) Text(g.h[IDC_CLONE_REF_AUDIO], v.refAudio);
    if (!v.refText.empty()) Text(g.h[IDC_CLONE_REF_TEXT], v.refText);
    if (!v.personality.empty() && Blank(Text(g.h[IDC_INSTRUCT]))) Text(g.h[IDC_INSTRUCT], v.personality);
    UpdateGenerateVoiceControls();
    SetStatus(L"Voice loaded into Generate");
    ShowPage(PAGE_GENERATE);
}

void SaveLibraryVoice() {
    if (Blank(Text(g.h[IDC_LIB_NAME]))) {
        Log(L"Voice Library needs a profile name before saving.\r\n");
        return;
    }
    std::wstring type = Combo(g.h[IDC_LIB_TYPE]);
    if (type == L"cloned" && (Blank(Text(g.h[IDC_LIB_REF_AUDIO])) || Blank(Text(g.h[IDC_LIB_REF_TEXT])))) {
        Log(L"Cloned voices need reference audio and reference transcript.\r\n");
        return;
    }
    if (type == L"fine_tuned" && Blank(Text(g.h[IDC_LIB_CHECKPOINT]))) {
        Log(L"Fine-tuned voices need a checkpoint folder.\r\n");
        return;
    }
    std::ostringstream j;
    j << "{\"qwen_root\":\"" << Json(QwenRoot) << "\",\"mode\":\"library_save\",";
    j << "\"name\":\"" << Json(Text(g.h[IDC_LIB_NAME])) << "\",";
    j << "\"voice_type\":\"" << Json(type) << "\",";
    j << "\"language\":\"" << Json(Combo(g.h[IDC_LIB_LANG])) << "\",";
    j << "\"mode_label\":\"" << Json(Combo(g.h[IDC_LIB_MODE])) << "\",";
    j << "\"speaker\":\"" << Json(Combo(g.h[IDC_LIB_SPEAKER])) << "\",";
    j << "\"description\":\"" << Json(Text(g.h[IDC_LIB_DESC])) << "\",";
    j << "\"personality\":\"" << Json(Text(g.h[IDC_LIB_PERSONA])) << "\",";
    j << "\"ref_audio\":\"" << Json(Text(g.h[IDC_LIB_REF_AUDIO])) << "\",";
    j << "\"ref_text\":\"" << Json(Text(g.h[IDC_LIB_REF_TEXT])) << "\",";
    j << "\"checkpoint_path\":\"" << Json(Text(g.h[IDC_LIB_CHECKPOINT])) << "\",";
    j << "\"voice_emotion\":\"" << Json(Combo(g.h[IDC_LIB_EMOTION])) << "\",";
    j << "\"audio_processing\":\"" << Json(Combo(g.h[IDC_LIB_EFFECTS])) << "\",";
    j << "\"effects_preset\":\"" << Json(Combo(g.h[IDC_LIB_EFFECTS])) << "\"";
    std::wstring out;
    DWORD code = RunBackendSync(j.str(), out);
    Log(out + L"\r\n");
    SetStatus(code == 0 ? L"Voice profile saved" : L"Voice profile save failed");
    RefreshLibrary();
}

void AddLibrarySample() {
    std::ostringstream j;
    j << "{\"qwen_root\":\"" << Json(QwenRoot) << "\",\"mode\":\"library_add_sample\",";
    j << "\"profile_name\":\"" << Json(Text(g.h[IDC_LIB_NAME])) << "\",";
    j << "\"audio_path\":\"" << Json(Text(g.h[IDC_LIB_SAMPLE_AUDIO])) << "\",";
    j << "\"transcript\":\"" << Json(Text(g.h[IDC_LIB_SAMPLE_TEXT])) << "\",\"primary\":true";
    std::wstring out;
    DWORD code = RunBackendSync(j.str(), out);
    Log(out + L"\r\n");
    SetStatus(code == 0 ? L"Sample added" : L"Sample add failed");
}

void ExportLibraryVoice() {
    std::wstring name = Text(g.h[IDC_LIB_NAME]);
    if (Blank(name)) return;
    std::wstring outPath = SaveDialog(L"Voice Profile JSON\0*.json\0All\0*.*\0", L"json", std::wstring(QwenRoot) + L"\\qwen_studio_data\\" + name + L".json");
    if (outPath.empty()) return;
    std::ostringstream j;
    j << "{\"qwen_root\":\"" << Json(QwenRoot) << "\",\"mode\":\"library_export\",\"name\":\"" << Json(name) << "\",\"export_path\":\"" << Json(outPath) << "\"";
    std::wstring out;
    DWORD code = RunBackendSync(j.str(), out);
    Log(out + L"\r\n");
    SetStatus(code == 0 ? L"Voice exported" : L"Voice export failed");
}

void ImportLibraryVoice() {
    std::wstring path = OpenDialog(L"Voice Profile JSON\0*.json\0All\0*.*\0");
    if (path.empty()) return;
    std::ostringstream j;
    j << "{\"qwen_root\":\"" << Json(QwenRoot) << "\",\"mode\":\"library_import\",\"import_path\":\"" << Json(path) << "\"";
    std::wstring out;
    DWORD code = RunBackendSync(j.str(), out);
    Log(out + L"\r\n");
    SetStatus(code == 0 ? L"Voice imported" : L"Voice import failed");
    RefreshLibrary();
}

void TestEffects() {
    std::ostringstream j;
    j << "{\"qwen_root\":\"" << Json(QwenRoot) << "\",\"mode\":\"test_effects\"";
    RunJob(j.str());
}

void ApplyEffectToLastOutput() {
    ClosePlayback();
    std::wstring input;
    int idx = (int)SendMessageW(g.h[IDC_HISTORY_LIST], LB_GETCURSEL, 0, 0);
    if (idx >= 0 && idx < (int)g.history.size()) input = g.history[idx].output;
    if (Blank(input)) input = Text(g.h[IDC_OUTPUT]);
    std::wstring effect = Combo(g.page == PAGE_LIBRARY ? g.h[IDC_LIB_EFFECTS] : g.h[IDC_EFFECT_PRESET]);
    if (Blank(input) || !Exists(input)) {
        Text(g.h[IDC_LOG], L"");
        Log(L"Create Processed Version needs a selected history item or existing output file first.\r\n");
        SetStatus(L"No output file selected");
        return;
    }
    if (effect.empty() || effect == L"None") {
        Text(g.h[IDC_LOG], L"");
        Log(L"Choose Audio Processing other than None before creating a processed version.\r\n");
        SetStatus(L"Choose audio processing");
        return;
    }
    std::ostringstream j;
    j << "{\"qwen_root\":\"" << Json(QwenRoot) << "\",";
    j << "\"mode\":\"apply_effect_to_file\",";
    j << "\"input_path\":\"" << Json(input) << "\",";
    j << "\"output_path\":\"" << Json(input) << "\",";
    j << "\"format\":\"" << Json(Combo(g.h[IDC_FORMAT])) << "\",";
    j << "\"effects_preset\":\"" << Json(effect) << "\"";
    RunJob(j.str());
}

void OpenSelectedHistoryOutput() {
    int idx = (int)SendMessageW(g.h[IDC_HISTORY_LIST], LB_GETCURSEL, 0, 0);
    if (idx < 0 || idx >= (int)g.history.size()) {
        Log(L"Select a recent generation first.\r\n");
        return;
    }
    std::wstring output = g.history[idx].output;
    if (Blank(output) || !Exists(output)) {
        Log(L"Selected history output is missing on disk.\r\n");
        return;
    }
    std::wstring params = L"/select," + Quote(output);
    ShellExecuteW(g.hwnd, L"open", L"explorer.exe", params.c_str(), nullptr, SW_SHOWNORMAL);
}

void GenerateVoice(bool clone) {
    ClosePlayback();
    std::wstring output = Text(g.h[IDC_OUTPUT]);
    std::wstring fmt = Combo(g.h[IDC_FORMAT]);
    if (Blank(output)) output = std::wstring(QwenRoot) + L"\\output_qwen_studio." + fmt;
    if (!output.empty()) {
        fs::path p(output);
        p.replace_extension(WideToUtf8(L"." + fmt));
        output = p.wstring();
        Text(g.h[IDC_OUTPUT], output);
    }
    if (clone) {
        std::wstring model = ModelDir(L"Qwen3-TTS-12Hz-0.6B-Base");
        std::wstring refAudio = Text(g.h[IDC_CLONE_REF_AUDIO]);
        std::wstring refText = Text(g.h[IDC_CLONE_REF_TEXT]);
        if (!Exists(model)) {
            Text(g.h[IDC_LOG], L"");
            Log(L"Missing model: " + FileNameOf(model) + L"\r\n\r\n");
            Log(L"Voice Clone needs the 0.6B Base model. Open the Models page, choose Qwen3-TTS-12Hz-0.6B-Base, and click Download Selected.\r\n");
            SetStatus(L"Missing clone model");
            ShowPage(PAGE_MODELS);
            return;
        }
        if (Blank(refAudio) || !Exists(refAudio)) {
            Text(g.h[IDC_LOG], L"");
            Log(L"Voice Clone needs an existing reference audio file.\r\n");
            SetStatus(L"Missing clone reference audio");
            return;
        }
        if (Blank(refText)) {
            Text(g.h[IDC_LOG], L"");
            Log(L"Voice Clone needs a transcript for the reference audio.\r\n");
            SetStatus(L"Missing clone transcript");
            return;
        }
        std::string j = BaseJob(L"voice_clone", model, Text(g.h[IDC_CLONE_TEXT]), output);
        j += ",\"ref_audio\":\"" + Json(refAudio) + "\"";
        j += ",\"ref_text\":\"" + Json(refText) + "\"";
        j += ",\"x_vector_only_mode\":";
        j += SendMessageW(g.h[IDC_CLONE_XVEC], BM_GETCHECK, 0, 0) == BST_CHECKED ? "true" : "false";
        RunJob(j);
        return;
    }
    VoiceProfile* vp = SelectedVoiceProfile();
    if (!vp) {
        RefreshLibrary();
        vp = SelectedVoiceProfile();
    }
    if (!vp) {
        Text(g.h[IDC_LOG], L"");
        Log(L"Choose or save a voice profile before generating. Built-in profiles should appear automatically after Library refresh.\r\n");
        SetStatus(L"No voice profile selected");
        ShowPage(PAGE_LIBRARY);
        return;
    }
    if (vp && vp->type == L"cloned") {
        std::wstring model = ModelDir(L"Qwen3-TTS-12Hz-0.6B-Base");
        if (!Exists(model)) {
            Text(g.h[IDC_LOG], L"");
            Log(L"Saved cloned voice needs Qwen3-TTS-12Hz-0.6B-Base. Download it from Models first.\r\n");
            SetStatus(L"Missing clone model");
            ShowPage(PAGE_MODELS);
            return;
        }
        if (Blank(vp->refAudio) || !Exists(vp->refAudio) || Blank(vp->refText)) {
            Text(g.h[IDC_LOG], L"");
            Log(L"This cloned voice profile needs valid reference audio and transcript in Voice Library.\r\n");
            SetStatus(L"Cloned voice is incomplete");
            ShowPage(PAGE_LIBRARY);
            return;
        }
        std::string j = BaseJob(L"voice_clone", model, Text(g.h[IDC_TEXT]), output);
        j += ",\"ref_audio\":\"" + Json(vp->refAudio) + "\"";
        j += ",\"ref_text\":\"" + Json(vp->refText) + "\"";
        j += ",\"x_vector_only_mode\":false";
        RunJob(j);
        return;
    }
    if (vp && vp->type == L"fine_tuned") {
        if (Blank(vp->checkpoint) || !Exists(vp->checkpoint)) {
            Text(g.h[IDC_LOG], L"");
            Log(L"This fine-tuned voice profile needs a valid checkpoint folder in Voice Library.\r\n");
            SetStatus(L"Fine-tuned voice checkpoint missing");
            ShowPage(PAGE_LIBRARY);
            return;
        }
        RunJob(BaseJob(L"custom_voice", vp->checkpoint, Text(g.h[IDC_TEXT]), output));
        return;
    }
    if (vp && vp->type == L"voice_design") ComboSelectExact(g.h[IDC_MODE], L"Voice Design");
    std::wstring modeLabel = Combo(g.h[IDC_MODE]);
    std::wstring mode = (modeLabel.find(L"Voice Design") != std::wstring::npos) ? L"voice_design" : L"custom_voice";
    std::wstring instruction = Text(g.h[IDC_INSTRUCT]);
    if (IsEmotionalInstruction(instruction) && modeLabel.find(L"Built-in") != std::wstring::npos) {
        std::wstring strong = ModelDir(L"Qwen3-TTS-12Hz-1.7B-CustomVoice");
        if (Exists(strong)) {
            modeLabel = L"Strong Instructions (1.7B)";
            ComboSelectExact(g.h[IDC_MODE], L"Strong Instructions");
            Log(L"Emotional instruction detected: routed to Strong Instructions (1.7B) for better acting/style control.\r\n");
        } else {
            Text(g.h[IDC_LOG], L"");
            Log(L"Emotional instructions like scared/paranoid need the stronger 1.7B CustomVoice model for reliable acting/style control.\r\n");
            Log(L"Open Models, download Qwen3-TTS-12Hz-1.7B-CustomVoice, then try again.\r\n");
            SetStatus(L"Download 1.7B CustomVoice for instructions");
            ShowPage(PAGE_MODELS);
            return;
        }
    } else if (modeLabel.find(L"Built-in") != std::wstring::npos && !Blank(instruction)) {
        Log(L"Instruction note: subtle style prompts may work on 0.6B, but stronger emotion works better on Strong Instructions (1.7B).\r\n");
    }
    std::wstring model = CurrentModelForMode(modeLabel);
    if (!Exists(model)) {
        Text(g.h[IDC_LOG], L"");
        Log(L"Missing model: " + FileNameOf(model) + L"\r\n\r\n");
        Log(L"Open the Models page, choose that model, and click Download Selected. Keep Offline mode enabled after the download finishes.\r\n");
        SetStatus(L"Missing model - download it on the Models page");
        ShowPage(PAGE_MODELS);
        return;
    }
    if (vp->speaker == L"Uncle_Fu" && vp->language == L"English") {
        Log(L"Voice guidance: Uncle_Fu is Chinese-native. For English emotional acting, Ryan/Aiden or Voice Design usually follows style better.\r\n");
    }
    RunJob(BaseJob(mode, model, Text(g.h[IDC_TEXT]), output));
}

void SyncOutputExtension() {
    std::wstring output = Text(g.h[IDC_OUTPUT]);
    std::wstring fmt = Combo(g.h[IDC_FORMAT]);
    if (output.empty() || fmt.empty()) return;
    fs::path p(output);
    p.replace_extension(WideToUtf8(L"." + fmt));
    Text(g.h[IDC_OUTPUT], p.wstring());
}

void PlayAudio(const wchar_t* action) {
    if (wcscmp(action, L"open") == 0) {
        ClosePlayback();
        if (!Exists(Text(g.h[IDC_OUTPUT]))) {
            Log(L"Playback error: output file does not exist yet.\r\n");
            SetStatus(L"No audio file to play");
            return;
        }
        std::wstring ext = fs::path(Text(g.h[IDC_OUTPUT])).extension().wstring();
        if (ext == L".ogg" || ext == L".flac") {
            Log(L"Playback note: Windows native playback may not support " + ext + L". Use Open File if Play fails, or export WAV/MP3 for in-app playback.\r\n");
        }
        std::wstring cmd = L"open " + Quote(Text(g.h[IDC_OUTPUT])) + L" alias qwen_audio";
        MCIERROR err = mciSendStringW(cmd.c_str(), nullptr, 0, nullptr);
        if (err) {
            Log(L"Playback open error: " + MciError(err) + L"\r\n");
            SetStatus(L"Playback failed");
            return;
        }
        g.audioOpened = true;
        ApplyPlaybackVolume();
        err = mciSendStringW(L"play qwen_audio", nullptr, 0, nullptr);
        if (err) Log(L"Playback error: " + MciError(err) + L"\r\n");
    } else if (g.audioOpened) {
        std::wstring cmd = std::wstring(action) + L" qwen_audio";
        MCIERROR err = mciSendStringW(cmd.c_str(), nullptr, 0, nullptr);
        if (err) Log(L"Playback control error: " + MciError(err) + L"\r\n");
    }
}

std::vector<std::wstring> DownloadableModels() {
    return { L"Qwen3-TTS-12Hz-1.7B-CustomVoice", L"Qwen3-TTS-12Hz-1.7B-VoiceDesign", L"Qwen3-TTS-12Hz-0.6B-Base" };
}

std::wstring ModelPurpose(const std::wstring& n) {
    if (n.find(L"0.6B-CustomVoice") != std::wstring::npos) return L"Built-in voices";
    if (n.find(L"1.7B-CustomVoice") != std::wstring::npos) return L"Stronger emotions";
    if (n.find(L"VoiceDesign") != std::wstring::npos) return L"Designed voices";
    if (n.find(L"0.6B-Base") != std::wstring::npos) return L"Clone/fine-tune";
    if (n.find(L"Tokenizer") != std::wstring::npos) return L"Tokenizer";
    return L"Model";
}

bool DirectoryHasFiles(const std::wstring& path) {
    std::error_code ec;
    if (!fs::exists(path, ec) || !fs::is_directory(path, ec)) return false;
    for (auto it = fs::directory_iterator(path, ec); !ec && it != fs::directory_iterator(); it.increment(ec)) return true;
    return false;
}

std::wstring ModelStatus(const std::wstring& n) {
    std::wstring path = ModelDir(n);
    std::error_code ec;
    if (!fs::exists(path, ec)) return L"Missing";
    return DirectoryHasFiles(path) ? L"Installed" : L"Partial";
}

std::wstring CurrentDownloadModel() {
    int idx = (int)SendMessageW(g.h[IDC_MODEL_PICK], CB_GETCURSEL, 0, 0);
    auto names = DownloadableModels();
    if (idx < 0 || idx >= (int)names.size()) return L"";
    return names[idx];
}

void UpdateModelActions() {
    std::wstring n = CurrentDownloadModel();
    std::wstring status = n.empty() ? L"Missing" : ModelStatus(n);
    EnableWindow(g.h[IDC_DOWNLOAD_MODEL], status != L"Installed");
    EnableWindow(g.h[IDC_MODEL_OPEN], status != L"Missing");
    SetWindowTextW(g.h[IDC_DOWNLOAD_MODEL], status == L"Installed" ? L"Installed" : L"Download Selected");
}

void ScanModels() {
    HWND list = g.h[IDC_MODEL_LIST];
    SendMessageW(list, LB_RESETCONTENT, 0, 0);
    std::vector<std::wstring> names = {
        L"Qwen3-TTS-12Hz-0.6B-CustomVoice",
        L"Qwen3-TTS-12Hz-1.7B-CustomVoice",
        L"Qwen3-TTS-12Hz-1.7B-VoiceDesign",
        L"Qwen3-TTS-12Hz-0.6B-Base",
        L"Qwen3-TTS-Tokenizer-12Hz"
    };
    SendMessageW(list, LB_ADDSTRING, 0, (LPARAM)L"Status      Model                                  Purpose              Path");
    for (auto& n : names) {
        std::wstring status = ModelStatus(n);
        std::wstring line = status;
        while (line.size() < 12) line += L" ";
        line += n;
        while (line.size() < 51) line += L" ";
        line += ModelPurpose(n);
        while (line.size() < 72) line += L" ";
        line += ModelDir(n);
        SendMessageW(list, LB_ADDSTRING, 0, (LPARAM)line.c_str());
    }
    std::wstring strong = Exists(ModelDir(L"Qwen3-TTS-12Hz-1.7B-CustomVoice")) ? L"Strong Instructions (1.7B)" : L"Strong Instructions (1.7B - download)";
    std::wstring design = Exists(ModelDir(L"Qwen3-TTS-12Hz-1.7B-VoiceDesign")) ? L"Voice Design (1.7B)" : L"Voice Design (1.7B - download)";
    int current = (int)SendMessageW(g.h[IDC_MODE], CB_GETCURSEL, 0, 0);
    SendMessageW(g.h[IDC_MODE], CB_RESETCONTENT, 0, 0);
    AddItems(g.h[IDC_MODE], { L"Built-in Voices (0.6B)", strong, design }, current < 0 ? 0 : current);
    UpdateModelActions();
}

void ApplyInstructionPreset() {
    std::wstring preset = NormalizeVoiceEmotion(Combo(g.h[IDC_PRESET]));
    if (preset == L"Natural") Text(g.h[IDC_INSTRUCT], L"Speak clearly with a natural tone.");
    else if (preset == L"Calm") Text(g.h[IDC_INSTRUCT], L"Speak calmly, clearly, and naturally with a steady pace.");
    else if (preset == L"Angry") Text(g.h[IDC_INSTRUCT], L"Speak with controlled anger, clipped phrasing, and strong emphasis.");
    else if (preset == L"Scared / paranoid") {
        Text(g.h[IDC_INSTRUCT], L"Act terrified and paranoid. Speak in a tense, shaky voice with uneven breathing, nervous pauses, and whispered urgency, like you believe someone is secretly following you.");
        if (Exists(ModelDir(L"Qwen3-TTS-12Hz-1.7B-CustomVoice"))) {
            ComboSelectExact(g.h[IDC_MODE], L"Strong Instructions");
            Log(L"Scared/paranoid preset: using Strong Instructions (1.7B) for better acting/style control.\r\n");
        } else {
            Log(L"Scared/paranoid preset works best with Qwen3-TTS-12Hz-1.7B-CustomVoice. Download it on the Models page for stronger emotion control.\r\n");
        }
        VoiceProfile* vp = SelectedVoiceProfile();
        if (vp && vp->speaker == L"Uncle_Fu") {
            Log(L"Voice guidance: Uncle_Fu is Chinese-native. For English scared/paranoid acting, choose Ryan/Aiden or a VoiceDesign profile.\r\n");
        }
    }
    else if (preset == L"Whisper") Text(g.h[IDC_INSTRUCT], L"Speak in a quiet whisper, intimate and close to the microphone.");
    else if (preset == L"Excited") Text(g.h[IDC_INSTRUCT], L"Speak with energetic excitement, bright tone, and quick emotional lift.");
    else if (preset == L"Sad") Text(g.h[IDC_INSTRUCT], L"Speak sadly and softly, with a tired, emotional tone.");
    else if (preset == L"Urgent") Text(g.h[IDC_INSTRUCT], L"Speak urgently with fast tension and clear warning in the voice.");
    else if (preset == L"Funny with laugh ending") {
        Text(g.h[IDC_INSTRUCT], L"Deliver this with playful comic timing. End with a short natural laugh.");
        std::wstring text = Text(g.h[IDC_TEXT]);
        if (text.find(L"[laugh]") == std::wstring::npos) Text(g.h[IDC_TEXT], text + L" [laugh]");
        Log(L"Funny emotion changes acting/text only. Pick Audio Processing separately if you also want post-processing.\r\n");
    }
    else if (preset == L"Crazy / unhinged") {
        Text(g.h[IDC_INSTRUCT], L"Act wildly unhinged and unpredictable, with nervous bursts, odd emphasis, manic energy, and sudden tonal shifts.");
        Log(L"Crazy emotion changes acting only. Pick Audio Processing separately if you also want post-processing.\r\n");
    }
    else if (preset == L"Creepy / tense") {
        Text(g.h[IDC_INSTRUCT], L"Speak quietly and disturbingly, slow and tense, like a creepy character telling a secret.");
        Log(L"Creepy emotion changes acting only. Pick Audio Processing separately if you also want post-processing.\r\n");
    }
}

void DownloadSelectedModel() {
    std::wstring n = CurrentDownloadModel();
    if (n.empty()) return;
    if (ModelStatus(n) == L"Installed") {
        ShellExecuteW(g.hwnd, L"open", ModelDir(n).c_str(), nullptr, nullptr, SW_SHOWNORMAL);
        return;
    }
    std::ostringstream j;
    j << "{\"qwen_root\":\"" << Json(QwenRoot) << "\",\"mode\":\"download_model\",\"repo_id\":\"Qwen/" << WideToUtf8(n) << "\",\"target_dir\":\"" << Json(ModelDir(n)) << "\",\"offline\":false";
    RunJob(j.str());
}

void OpenSelectedModelFolder() {
    std::wstring n = CurrentDownloadModel();
    if (n.empty()) return;
    std::wstring path = ModelDir(n);
    if (!Exists(path)) {
        Log(L"Model folder does not exist yet: " + path + L"\r\n");
        return;
    }
    ShellExecuteW(g.hwnd, L"open", path.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

void RefreshTrainingList() {
    HWND list = g.h[IDC_FT_LIST];
    SendMessageW(list, LB_RESETCONTENT, 0, 0);
    for (size_t i = 0; i < g.training.size(); ++i) {
        std::wstring line = std::to_wstring(i + 1) + L". " + fs::path(g.training[i].audio).filename().wstring() + L" | " + g.training[i].transcript.substr(0, 60);
        SendMessageW(list, LB_ADDSTRING, 0, (LPARAM)line.c_str());
    }
}

std::wstring ProjectDir() {
    return std::wstring(QwenRoot) + L"\\voice_projects\\" + Text(g.h[IDC_FT_PROJECT]);
}

std::wstring Lower(std::wstring s) {
    for (auto& c : s) c = (wchar_t)towlower(c);
    return s;
}

std::wstring NormalizeTrainingAudio(const std::wstring& input, const std::wstring& role, int index = 0) {
    std::wstring ext = Lower(fs::path(input).extension().wstring());
    if (ext == L".wav") return input;
    std::wstring dir = ProjectDir() + L"\\normalized_audio";
    fs::create_directories(fs::path(dir));
    std::wstring stem = fs::path(input).stem().wstring();
    if (stem.empty()) stem = role;
    std::wstring out = dir + L"\\" + role + (index > 0 ? L"_" + std::to_wstring(index) : L"") + L"_" + stem + L".wav";
    std::ostringstream j;
    j << "{\"qwen_root\":\"" << Json(QwenRoot) << "\",\"mode\":\"normalize_audio_file\",";
    j << "\"input_path\":\"" << Json(input) << "\",\"output_path\":\"" << Json(out) << "\"";
    std::wstring backendOut;
    DWORD code = RunBackendSync(j.str(), backendOut);
    Log(backendOut + L"\r\n");
    if (code != 0 || !Exists(out)) {
        Log(L"Audio conversion failed for: " + input + L"\r\n");
        return L"";
    }
    return out;
}

bool WriteTrainingJsonl() {
    fs::create_directories(fs::path(ProjectDir()));
    std::wstring refAudio = NormalizeTrainingAudio(Text(g.h[IDC_FT_REF_AUDIO]), L"reference");
    if (Blank(refAudio)) return false;
    std::wstring raw = ProjectDir() + L"\\train_raw.jsonl";
    std::ofstream f(fs::path(raw), std::ios::binary);
    for (size_t i = 0; i < g.training.size(); ++i) {
        auto& item = g.training[i];
        std::wstring audio = NormalizeTrainingAudio(item.audio, L"clip", (int)i + 1);
        if (Blank(audio)) return false;
        item.sourceAudio = item.sourceAudio.empty() ? item.audio : item.sourceAudio;
        item.audio = audio;
        f << "{\"audio\":\"" << Json(item.audio) << "\",\"source_audio\":\"" << Json(item.sourceAudio) << "\",\"text\":\"" << Json(item.transcript) << "\",\"ref_audio\":\"" << Json(refAudio) << "\"}\n";
    }
    Log(L"Wrote " + raw + L"\r\n");
    return true;
}

void FineTunePrepareOrTrain(bool train) {
    std::wstring base = ModelDir(L"Qwen3-TTS-12Hz-0.6B-Base");
    std::wstring tokenizer = ModelDir(L"Qwen3-TTS-Tokenizer-12Hz");
    if (!Exists(tokenizer)) {
        Text(g.h[IDC_LOG], L"");
        Log(L"Missing tokenizer: " + FileNameOf(tokenizer) + L"\r\nDownload or restore Qwen3-TTS-Tokenizer-12Hz first.\r\n");
        SetStatus(L"Missing tokenizer");
        ShowPage(PAGE_MODELS);
        return;
    }
    if (!Exists(base)) {
        Text(g.h[IDC_LOG], L"");
        Log(L"Missing model: " + FileNameOf(base) + L"\r\nFine-tuning needs Qwen3-TTS-12Hz-0.6B-Base. Download it from the Models page first.\r\n");
        SetStatus(L"Missing fine-tune model");
        ShowPage(PAGE_MODELS);
        return;
    }
    if (Blank(Text(g.h[IDC_FT_PROJECT])) || Blank(Text(g.h[IDC_FT_SPEAKER]))) {
        Text(g.h[IDC_LOG], L"");
        Log(L"Fine-tuning needs both a project name and speaker name.\r\n");
        SetStatus(L"Missing fine-tune names");
        return;
    }
    if (Blank(Text(g.h[IDC_FT_REF_AUDIO])) || !Exists(Text(g.h[IDC_FT_REF_AUDIO]))) {
        Text(g.h[IDC_LOG], L"");
        Log(L"Fine-tuning needs an existing reference audio file.\r\n");
        SetStatus(L"Missing fine-tune reference audio");
        return;
    }
    if (g.training.empty()) {
        Text(g.h[IDC_LOG], L"");
        Log(L"Fine-tuning needs at least one dataset clip with transcript.\r\n");
        SetStatus(L"Missing fine-tune clips");
        return;
    }
    if (!WriteTrainingJsonl()) {
        SetStatus(L"Audio conversion failed");
        return;
    }
    std::wstring dir = ProjectDir();
    Log(L"Project: " + dir + L"\r\n");
    Log(L"Checkpoints: " + dir + L"\\checkpoints\r\n");
    std::ostringstream j;
    j << "{\"qwen_root\":\"" << Json(QwenRoot) << "\",";
    j << "\"mode\":\"" << (train ? "fine_tune_train" : "fine_tune_prepare") << "\",";
    j << "\"device\":\"" << Json(Combo(g.h[IDC_DEVICE])) << "\",";
    j << "\"tokenizer_path\":\"" << Json(tokenizer) << "\",";
    j << "\"model_path\":\"" << Json(base) << "\",";
    j << "\"raw_jsonl\":\"" << Json(dir + L"\\train_raw.jsonl") << "\",";
    j << "\"coded_jsonl\":\"" << Json(dir + L"\\train_with_codes.jsonl") << "\",";
    j << "\"output_model_path\":\"" << Json(dir + L"\\checkpoints") << "\",";
    j << "\"batch_size\":" << I(IDC_FT_BATCH, 1) << ",\"lr\":" << D(IDC_FT_LR, 2e-5) << ",\"epochs\":" << I(IDC_FT_EPOCHS, 3) << ",";
    j << "\"speaker_name\":\"" << Json(Text(g.h[IDC_FT_SPEAKER])) << "\",\"offline\":true";
    RunJob(j.str());
}

void AddTrainingClip() {
    std::wstring audio = Text(g.h[IDC_FT_AUDIO]);
    std::wstring transcript = Text(g.h[IDC_FT_TRANSCRIPT]);
    if (Blank(audio) || !Exists(audio)) {
        Text(g.h[IDC_LOG], L"");
        Log(L"Add Clip needs an existing WAV/audio file.\r\n");
        SetStatus(L"Missing clip audio");
        return;
    }
    if (Blank(transcript)) {
        Text(g.h[IDC_LOG], L"");
        Log(L"Add Clip needs a transcript for the audio.\r\n");
        SetStatus(L"Missing clip transcript");
        return;
    }
    g.training.push_back({ audio, audio, transcript });
    RefreshTrainingList();
    SetStatus(L"Clip added");
}

void CreateUi(HWND hwnd) {
    g.font = CreateFontW(18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    g.titleFont = CreateFontW(27, 0, 0, 0, FW_SEMIBOLD, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Segoe UI");
    g.monoFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH, L"Cascadia Mono");
    ApplyTheme();

    Button(IDC_NAV_GENERATE, L"Generate", 18, 96, 150, 36, hwnd);
    Button(IDC_NAV_LIBRARY, L"Voice Library", 18, 140, 150, 36, hwnd);
    Button(IDC_NAV_CLONE, L"Voice Clone", 18, 184, 150, 36, hwnd);
    Button(IDC_NAV_FINETUNE, L"Fine Tune", 18, 228, 150, 36, hwnd);
    Button(IDC_NAV_MODELS, L"Models", 18, 272, 150, 36, hwnd);
    Button(IDC_NAV_SETTINGS, L"Settings", 18, 316, 150, 36, hwnd);
    Button(IDC_THEME, L"Theme", 18, 688, 150, 34, hwnd);

    auto T = [&](int page, HWND h) { Track(page, h); return h; };
    T(PAGE_GENERATE, Label(L"Generate Speech", 210, 28, 280, 34, hwnd, true));
    T(PAGE_GENERATE, Label(L"Saved Profile", 500, 34, 120, 24, hwnd));
    T(PAGE_GENERATE, ComboBox(IDC_VOICE_PROFILE, 620, 30, 245, 220, hwnd));
    T(PAGE_GENERATE, Label(L"Text", 210, 76, 100, 24, hwnd));
    T(PAGE_GENERATE, Edit(IDC_TEXT, L"Hello, this is the premium Qwen TTS studio running offline.", 210, 104, 560, 170, hwnd, true));
    T(PAGE_GENERATE, Label(L"Instruction", 210, 286, 140, 24, hwnd));
    T(PAGE_GENERATE, Edit(IDC_INSTRUCT, L"Speak clearly with a calm, natural tone.", 210, 314, 560, 72, hwnd, true));
    T(PAGE_GENERATE, Label(L"Voice Emotion", 800, 286, 170, 24, hwnd));
    T(PAGE_GENERATE, ComboBox(IDC_PRESET, 800, 314, 310, 220, hwnd));
    AddVoiceEmotions(g.h[IDC_PRESET], 0);
    T(PAGE_GENERATE, Label(L"Mode", 800, 76, 90, 24, hwnd));
    T(PAGE_GENERATE, ComboBox(IDC_MODE, 800, 104, 310, 220, hwnd));
    AddItems(g.h[IDC_MODE], { L"Built-in Voices (0.6B)", L"Strong Instructions (1.7B)", L"Voice Design (1.7B)" }, 0);
    T(PAGE_GENERATE, Label(L"Language", 800, 154, 110, 24, hwnd));
    T(PAGE_GENERATE, ComboBox(IDC_LANGUAGE, 800, 182, 145, 240, hwnd));
    AddItems(g.h[IDC_LANGUAGE], { L"Auto", L"Chinese", L"English", L"Japanese", L"Korean", L"German", L"French", L"Russian", L"Portuguese", L"Spanish", L"Italian" }, 2);
    g.h[IDC_SPEAKER] = ComboBox(IDC_SPEAKER, -400, -400, 145, 220, hwnd);
    AddItems(g.h[IDC_SPEAKER], { L"Vivian", L"Serena", L"Uncle_Fu", L"Dylan", L"Eric", L"Ryan", L"Aiden", L"Ono_Anna", L"Sohee" }, 5);
    ShowWindow(g.h[IDC_SPEAKER], SW_HIDE);
    T(PAGE_GENERATE, Label(L"Export", 800, 232, 90, 24, hwnd));
    T(PAGE_GENERATE, ComboBox(IDC_FORMAT, 800, 260, 120, 180, hwnd));
    AddItems(g.h[IDC_FORMAT], { L"wav", L"mp3", L"ogg", L"flac" }, 0);
    T(PAGE_GENERATE, Label(L"Audio Processing", 934, 232, 150, 24, hwnd));
    T(PAGE_GENERATE, ComboBox(IDC_EFFECT_PRESET, 934, 260, 176, 220, hwnd));
    AddAudioProcessing(g.h[IDC_EFFECT_PRESET], 0);
    T(PAGE_GENERATE, Edit(IDC_OUTPUT, (std::wstring(QwenRoot) + L"\\output_qwen_studio.wav").c_str(), 210, 416, 568, 28, hwnd));
    T(PAGE_GENERATE, Button(IDC_BROWSE_OUTPUT, L"Save As", 790, 416, 96, 28, hwnd));
    T(PAGE_GENERATE, Button(IDC_GENERATE, L"Generate", 210, 474, 130, 38, hwnd));
    T(PAGE_GENERATE, Button(IDC_PLAY, L"Play", 354, 474, 86, 38, hwnd));
    T(PAGE_GENERATE, Button(IDC_PAUSE, L"Pause", 452, 474, 86, 38, hwnd));
    T(PAGE_GENERATE, Button(IDC_STOP, L"Stop", 550, 474, 86, 38, hwnd));
    T(PAGE_GENERATE, Button(IDC_OPEN_OUTPUT, L"Open File", 648, 474, 110, 38, hwnd));
    T(PAGE_GENERATE, Label(L"Volume", 800, 452, 90, 22, hwnd));
    T(PAGE_GENERATE, New(TRACKBAR_CLASSW, L"", TBS_AUTOTICKS | WS_TABSTOP, IDC_VOLUME, 800, 476, 220, 36, hwnd));
    SendMessageW(g.h[IDC_VOLUME], TBM_SETRANGE, TRUE, MAKELPARAM(0, 100));
    SendMessageW(g.h[IDC_VOLUME], TBM_SETPOS, TRUE, 85);
    T(PAGE_GENERATE, Label(L"Variation", 210, 548, 90, 22, hwnd)); T(PAGE_GENERATE, Edit(IDC_TEMP, L"0.7", 210, 574, 80, 26, hwnd));
    T(PAGE_GENERATE, Label(L"Top-p", 306, 548, 80, 22, hwnd)); T(PAGE_GENERATE, Edit(IDC_TOPP, L"0.95", 306, 574, 80, 26, hwnd));
    T(PAGE_GENERATE, Label(L"Top-k", 402, 548, 80, 22, hwnd)); T(PAGE_GENERATE, Edit(IDC_TOPK, L"50", 402, 574, 80, 26, hwnd));
    T(PAGE_GENERATE, Label(L"Max tokens", 498, 548, 110, 22, hwnd)); T(PAGE_GENERATE, Edit(IDC_MAXTOK, L"2048", 498, 574, 110, 26, hwnd));
    T(PAGE_GENERATE, Label(L"Repeat", 624, 548, 90, 22, hwnd)); T(PAGE_GENERATE, Edit(IDC_REPPEN, L"1.0", 624, 574, 90, 26, hwnd));
    T(PAGE_GENERATE, Label(L"Speed", 730, 548, 90, 22, hwnd)); T(PAGE_GENERATE, Edit(IDC_SPEED, L"1.00", 730, 574, 80, 26, hwnd));
    T(PAGE_GENERATE, Label(L"Chunk", 828, 548, 70, 22, hwnd)); T(PAGE_GENERATE, Edit(IDC_MAX_CHUNK, L"800", 828, 574, 70, 26, hwnd));
    T(PAGE_GENERATE, Label(L"Fade ms", 914, 548, 80, 22, hwnd)); T(PAGE_GENERATE, Edit(IDC_CROSSFADE, L"50", 914, 574, 70, 26, hwnd));
    T(PAGE_GENERATE, Check(IDC_NORMALIZE, L"Normalize", 998, 574, 112, 26, hwnd)); SendMessageW(g.h[IDC_NORMALIZE], BM_SETCHECK, BST_CHECKED, 0);
    T(PAGE_GENERATE, Label(L"Variation changes randomness; Speed changes rendered duration. Voice Emotion changes acting; Audio Processing is applied automatically.", 210, 618, 900, 24, hwnd));

    T(PAGE_LIBRARY, Label(L"Voice Library", 210, 28, 260, 34, hwnd, true));
    T(PAGE_LIBRARY, List(IDC_LIB_LIST, 210, 80, 320, 250, hwnd));
    T(PAGE_LIBRARY, Button(IDC_LIB_USE, L"Use Voice", 210, 344, 100, 34, hwnd));
    T(PAGE_LIBRARY, Button(IDC_LIB_SAVE, L"Save Profile", 322, 344, 120, 34, hwnd));
    T(PAGE_LIBRARY, Button(IDC_LIB_IMPORT, L"Import", 454, 344, 76, 34, hwnd));
    T(PAGE_LIBRARY, Label(L"Recent Generations", 210, 394, 180, 24, hwnd));
    T(PAGE_LIBRARY, List(IDC_HISTORY_LIST, 210, 424, 320, 164, hwnd));
    T(PAGE_LIBRARY, Button(IDC_HISTORY_OPEN, L"Open Output", 210, 596, 120, 30, hwnd));
    T(PAGE_LIBRARY, Button(IDC_HISTORY_REFRESH, L"Refresh", 342, 596, 84, 30, hwnd));
    T(PAGE_LIBRARY, Button(IDC_APPLY_EFFECT, L"Create Processed Version", 436, 596, 190, 30, hwnd));

    T(PAGE_LIBRARY, Label(L"Name", 560, 76, 70, 22, hwnd)); T(PAGE_LIBRARY, Edit(IDC_LIB_NAME, L"", 560, 102, 180, 26, hwnd));
    T(PAGE_LIBRARY, Label(L"Type", 760, 76, 70, 22, hwnd)); T(PAGE_LIBRARY, ComboBox(IDC_LIB_TYPE, 760, 102, 135, 180, hwnd));
    AddItems(g.h[IDC_LIB_TYPE], { L"builtin", L"cloned", L"voice_design", L"fine_tuned" }, 0);
    T(PAGE_LIBRARY, Label(L"Language", 914, 76, 90, 22, hwnd)); T(PAGE_LIBRARY, ComboBox(IDC_LIB_LANG, 914, 102, 145, 220, hwnd));
    AddItems(g.h[IDC_LIB_LANG], { L"Auto", L"Chinese", L"English", L"Japanese", L"Korean", L"German", L"French", L"Russian", L"Portuguese", L"Spanish", L"Italian" }, 2);
    T(PAGE_LIBRARY, Label(L"Mode", 560, 142, 70, 22, hwnd)); T(PAGE_LIBRARY, ComboBox(IDC_LIB_MODE, 560, 168, 220, 220, hwnd));
    AddItems(g.h[IDC_LIB_MODE], { L"Built-in Voices (0.6B)", L"Strong Instructions (1.7B)", L"Voice Design (1.7B)" }, 0);
    T(PAGE_LIBRARY, Label(L"Speaker", 800, 142, 80, 22, hwnd)); T(PAGE_LIBRARY, ComboBox(IDC_LIB_SPEAKER, 800, 168, 125, 220, hwnd));
    AddItems(g.h[IDC_LIB_SPEAKER], { L"Vivian", L"Serena", L"Uncle_Fu", L"Dylan", L"Eric", L"Ryan", L"Aiden", L"Ono_Anna", L"Sohee" }, 5);
    T(PAGE_LIBRARY, Label(L"Voice Emotion", 944, 142, 130, 22, hwnd)); T(PAGE_LIBRARY, ComboBox(IDC_LIB_EMOTION, 944, 168, 166, 220, hwnd));
    AddVoiceEmotions(g.h[IDC_LIB_EMOTION], 0);
    T(PAGE_LIBRARY, Label(L"Audio Processing", 560, 208, 150, 22, hwnd)); T(PAGE_LIBRARY, ComboBox(IDC_LIB_EFFECTS, 560, 234, 180, 220, hwnd));
    AddAudioProcessing(g.h[IDC_LIB_EFFECTS], 0);
    T(PAGE_LIBRARY, Label(L"Description", 760, 208, 120, 22, hwnd)); T(PAGE_LIBRARY, Edit(IDC_LIB_DESC, L"", 760, 234, 160, 58, hwnd, true));
    T(PAGE_LIBRARY, Label(L"Personality / default instruction", 940, 208, 240, 22, hwnd)); T(PAGE_LIBRARY, Edit(IDC_LIB_PERSONA, L"", 940, 234, 170, 58, hwnd, true));
    T(PAGE_LIBRARY, Label(L"Profile status", 560, 300, 120, 22, hwnd)); T(PAGE_LIBRARY, Label(L"Ready", 680, 300, 420, 22, hwnd));
    g.h[IDC_LIB_VALIDATION] = g.libraryCtrls.back();
    T(PAGE_LIBRARY, Label(L"Reference audio", 560, 326, 130, 22, hwnd)); T(PAGE_LIBRARY, Edit(IDC_LIB_REF_AUDIO, L"", 560, 352, 430, 26, hwnd)); T(PAGE_LIBRARY, Button(IDC_LIB_BROWSE_REF, L"Browse", 1002, 352, 108, 26, hwnd));
    T(PAGE_LIBRARY, Label(L"Reference transcript", 560, 386, 160, 22, hwnd)); T(PAGE_LIBRARY, Edit(IDC_LIB_REF_TEXT, L"", 560, 412, 550, 44, hwnd, true));
    T(PAGE_LIBRARY, Label(L"Checkpoint folder", 560, 462, 150, 22, hwnd)); T(PAGE_LIBRARY, Edit(IDC_LIB_CHECKPOINT, L"", 560, 488, 430, 26, hwnd)); T(PAGE_LIBRARY, Button(IDC_LIB_EXPORT, L"Export", 1002, 488, 108, 26, hwnd));
    T(PAGE_LIBRARY, Label(L"Add sample", 560, 522, 120, 22, hwnd)); T(PAGE_LIBRARY, Edit(IDC_LIB_SAMPLE_AUDIO, L"", 560, 548, 350, 26, hwnd)); T(PAGE_LIBRARY, Button(IDC_LIB_BROWSE_SAMPLE, L"Audio", 922, 548, 80, 26, hwnd)); T(PAGE_LIBRARY, Button(IDC_LIB_ADD_SAMPLE, L"Add", 1012, 548, 98, 26, hwnd));
    T(PAGE_LIBRARY, Edit(IDC_LIB_SAMPLE_TEXT, L"Transcript for this voice sample.", 560, 582, 550, 48, hwnd, true));

    T(PAGE_CLONE, Label(L"Voice Clone", 210, 28, 260, 34, hwnd, true));
    T(PAGE_CLONE, Edit(IDC_CLONE_TEXT, L"Type text to speak with the cloned voice.", 210, 86, 560, 150, hwnd, true));
    T(PAGE_CLONE, Edit(IDC_CLONE_REF_AUDIO, L"", 210, 280, 650, 28, hwnd));
    T(PAGE_CLONE, Button(IDC_CLONE_BROWSE, L"Ref Audio", 878, 280, 110, 28, hwnd));
    T(PAGE_CLONE, Edit(IDC_CLONE_REF_TEXT, L"Transcript of the reference audio.", 210, 330, 650, 100, hwnd, true));
    T(PAGE_CLONE, Check(IDC_CLONE_XVEC, L"Embedding-only mode", 210, 448, 210, 28, hwnd));
    T(PAGE_CLONE, Button(IDC_CLONE_GENERATE, L"Generate Clone", 210, 496, 160, 38, hwnd));

    T(PAGE_FINETUNE, Label(L"Fine Tune Voice", 210, 28, 320, 34, hwnd, true));
    T(PAGE_FINETUNE, Edit(IDC_FT_PROJECT, L"my_voice", 210, 90, 180, 28, hwnd));
    T(PAGE_FINETUNE, Edit(IDC_FT_SPEAKER, L"my_voice", 410, 90, 180, 28, hwnd));
    T(PAGE_FINETUNE, Edit(IDC_FT_REF_AUDIO, L"", 210, 150, 650, 28, hwnd)); T(PAGE_FINETUNE, Button(IDC_FT_BROWSE_REF, L"Reference Audio", 878, 150, 150, 28, hwnd));
    T(PAGE_FINETUNE, Edit(IDC_FT_AUDIO, L"", 210, 206, 650, 28, hwnd)); T(PAGE_FINETUNE, Button(IDC_FT_BROWSE_AUDIO, L"Clip Audio", 878, 206, 110, 28, hwnd));
    T(PAGE_FINETUNE, Edit(IDC_FT_TRANSCRIPT, L"Transcript for this clip.", 210, 256, 650, 78, hwnd, true));
    T(PAGE_FINETUNE, Button(IDC_FT_ADD, L"Add Clip", 878, 256, 110, 34, hwnd));
    T(PAGE_FINETUNE, List(IDC_FT_LIST, 210, 354, 650, 170, hwnd));
    T(PAGE_FINETUNE, Edit(IDC_FT_BATCH, L"1", 210, 558, 70, 26, hwnd)); T(PAGE_FINETUNE, Edit(IDC_FT_LR, L"0.00002", 300, 558, 100, 26, hwnd)); T(PAGE_FINETUNE, Edit(IDC_FT_EPOCHS, L"3", 420, 558, 70, 26, hwnd));
    T(PAGE_FINETUNE, Button(IDC_FT_PREPARE, L"Prepare Data", 520, 550, 140, 38, hwnd));
    T(PAGE_FINETUNE, Button(IDC_FT_TRAIN, L"Start Training", 676, 550, 150, 38, hwnd));

    T(PAGE_MODELS, Label(L"Model Manager", 210, 28, 300, 34, hwnd, true));
    T(PAGE_MODELS, List(IDC_MODEL_LIST, 210, 92, 900, 260, hwnd));
    T(PAGE_MODELS, Button(IDC_SCAN_MODELS, L"Scan", 990, 364, 120, 36, hwnd));
    T(PAGE_MODELS, ComboBox(IDC_MODEL_PICK, 210, 386, 360, 220, hwnd));
    AddItems(g.h[IDC_MODEL_PICK], DownloadableModels(), 0);
    T(PAGE_MODELS, Button(IDC_DOWNLOAD_MODEL, L"Download Selected", 590, 384, 180, 38, hwnd));
    T(PAGE_MODELS, Button(IDC_MODEL_OPEN, L"Open Folder", 790, 384, 150, 38, hwnd));

    T(PAGE_SETTINGS, Label(L"Settings", 210, 28, 220, 34, hwnd, true));
    T(PAGE_SETTINGS, Edit(IDC_PYTHON, (std::wstring(QwenRoot) + L"\\python312-embed\\python.exe").c_str(), 210, 98, 760, 28, hwnd));
    T(PAGE_SETTINGS, Edit(IDC_MODELS_ROOT, (std::wstring(QwenRoot) + L"\\models").c_str(), 210, 154, 760, 28, hwnd));
    T(PAGE_SETTINGS, ComboBox(IDC_DEVICE, 210, 220, 120, 200, hwnd)); AddItems(g.h[IDC_DEVICE], { L"cuda:0", L"cpu" }, 0);
    T(PAGE_SETTINGS, ComboBox(IDC_DTYPE, 350, 220, 120, 200, hwnd)); AddItems(g.h[IDC_DTYPE], { L"bfloat16", L"float16", L"float32" }, 0);
    T(PAGE_SETTINGS, ComboBox(IDC_ATTN, 490, 220, 180, 200, hwnd)); AddItems(g.h[IDC_ATTN], { L"auto", L"flash_attention_2", L"sdpa" }, 0);
    T(PAGE_SETTINGS, Check(IDC_OFFLINE, L"Offline mode", 210, 276, 180, 28, hwnd)); SendMessageW(g.h[IDC_OFFLINE], BM_SETCHECK, BST_CHECKED, 0);
    T(PAGE_SETTINGS, Button(IDC_TEST_EFFECTS, L"Run Audio Diagnostics", 210, 330, 190, 36, hwnd));

    g.h[IDC_LOG] = Edit(IDC_LOG, L"", 210, 674, 900, 110, hwnd, true);
    SendMessageW(g.h[IDC_LOG], WM_SETFONT, (WPARAM)g.monoFont, TRUE);
    g.h[IDC_STATUS] = Label(L"Ready", 210, 644, 900, 24, hwnd);
    ScanModels();
    RefreshLibrary();
    ShowPage(PAGE_GENERATE);
}

void PaintPanel(HDC dc, RECT r, COLORREF color, int radius = 14) {
    HBRUSH b = CreateSolidBrush(color);
    HPEN p = CreatePen(PS_SOLID, 1, g.c.border);
    auto oldB = SelectObject(dc, b);
    auto oldP = SelectObject(dc, p);
    RoundRect(dc, r.left, r.top, r.right, r.bottom, radius, radius);
    SelectObject(dc, oldB); SelectObject(dc, oldP);
    DeleteObject(b); DeleteObject(p);
}

void Paint(HWND hwnd) {
    PAINTSTRUCT ps;
    HDC dc = BeginPaint(hwnd, &ps);
    RECT rc; GetClientRect(hwnd, &rc);
    FillRect(dc, &rc, g.bgBrush);
    RECT side{ 0,0,188,rc.bottom };
    FillRect(dc, &side, g.panelBrush);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, g.c.text);
    HFONT old = (HFONT)SelectObject(dc, g.titleFont);
    TextOutW(dc, 18, 28, L"Qwens Tts Buddy", 15);
    SelectObject(dc, g.font);
    SetTextColor(dc, g.c.muted);
    TextOutW(dc, 18, 60, L"Offline voice workbench", 23);
    if (g.page == PAGE_GENERATE) {
        RECT wave{ 900, 386, 1110, 456 };
        PaintPanel(dc, wave, g.c.panel2);
        HPEN pen = CreatePen(PS_SOLID, 2, g.c.accent);
        auto oldPen = SelectObject(dc, pen);
        int mid = (wave.top + wave.bottom) / 2;
        MoveToEx(dc, wave.left + 16, mid, nullptr);
        for (int x = wave.left + 16; x < wave.right - 16; x += 8) {
            int amp = 8 + ((x / 8) % 7) * 4;
            LineTo(dc, x, mid - amp);
            LineTo(dc, x + 4, mid + amp);
        }
        SelectObject(dc, oldPen);
        DeleteObject(pen);
    }
    SelectObject(dc, old);
    EndPaint(hwnd, &ps);
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
    case WM_CREATE:
        g.hwnd = hwnd;
        InitCommonControls();
        CreateUi(hwnd);
        return 0;
    case WM_PAINT:
        Paint(hwnd);
        return 0;
    case WM_CTLCOLORSTATIC:
    case WM_CTLCOLORBTN:
    case WM_CTLCOLOREDIT:
    case WM_CTLCOLORLISTBOX: {
        HDC dc = (HDC)wp;
        SetTextColor(dc, g.c.text);
        SetBkColor(dc, (msg == WM_CTLCOLOREDIT || msg == WM_CTLCOLORLISTBOX) ? g.c.editBg : g.c.bg);
        return (LRESULT)((msg == WM_CTLCOLOREDIT || msg == WM_CTLCOLORLISTBOX) ? g.editBrush : g.bgBrush);
    }
    case WM_COMMAND:
        switch (LOWORD(wp)) {
        case IDC_NAV_GENERATE: ShowPage(PAGE_GENERATE); return 0;
        case IDC_NAV_LIBRARY: RefreshLibrary(); ShowPage(PAGE_LIBRARY); return 0;
        case IDC_NAV_CLONE: ShowPage(PAGE_CLONE); return 0;
        case IDC_NAV_FINETUNE: ShowPage(PAGE_FINETUNE); return 0;
        case IDC_NAV_MODELS: ShowPage(PAGE_MODELS); return 0;
        case IDC_NAV_SETTINGS: ShowPage(PAGE_SETTINGS); return 0;
        case IDC_THEME: g.dark = !g.dark; ApplyTheme(); return 0;
        case IDC_PRESET:
            if (HIWORD(wp) == CBN_SELCHANGE) ApplyInstructionPreset();
            return 0;
        case IDC_VOICE_PROFILE:
            if (HIWORD(wp) == CBN_SELCHANGE) UpdateGenerateVoiceControls();
            return 0;
        case IDC_FORMAT:
            if (HIWORD(wp) == CBN_SELCHANGE) SyncOutputExtension();
            return 0;
        case IDC_BROWSE_OUTPUT: { auto p = SaveDialog(L"Audio\0*.wav;*.mp3;*.ogg;*.flac\0All\0*.*\0", L"wav", Text(g.h[IDC_OUTPUT])); if (!p.empty()) Text(g.h[IDC_OUTPUT], p); return 0; }
        case IDC_GENERATE: GenerateVoice(false); return 0;
        case IDC_TEST_EFFECTS: TestEffects(); return 0;
        case IDC_APPLY_EFFECT: ApplyEffectToLastOutput(); return 0;
        case IDC_CLONE_GENERATE: GenerateVoice(true); return 0;
        case IDC_PLAY: PlayAudio(L"open"); return 0;
        case IDC_PAUSE: PlayAudio(L"pause"); return 0;
        case IDC_STOP: PlayAudio(L"stop"); return 0;
        case IDC_OPEN_OUTPUT: ShellExecuteW(hwnd, L"open", Text(g.h[IDC_OUTPUT]).c_str(), nullptr, nullptr, SW_SHOWNORMAL); return 0;
        case IDC_CLONE_BROWSE: { auto p = OpenDialog(L"Audio\0*.wav;*.mp3;*.ogg;*.flac\0All\0*.*\0"); if (!p.empty()) Text(g.h[IDC_CLONE_REF_AUDIO], p); return 0; }
        case IDC_FT_BROWSE_REF: { auto p = OpenDialog(L"Audio\0*.wav;*.mp3;*.ogg;*.flac\0All\0*.*\0"); if (!p.empty()) Text(g.h[IDC_FT_REF_AUDIO], p); return 0; }
        case IDC_FT_BROWSE_AUDIO: { auto p = OpenDialog(L"Audio\0*.wav;*.mp3;*.ogg;*.flac\0All\0*.*\0"); if (!p.empty()) Text(g.h[IDC_FT_AUDIO], p); return 0; }
        case IDC_FT_ADD: AddTrainingClip(); return 0;
        case IDC_FT_PREPARE: FineTunePrepareOrTrain(false); return 0;
        case IDC_FT_TRAIN: FineTunePrepareOrTrain(true); return 0;
        case IDC_SCAN_MODELS: ScanModels(); return 0;
        case IDC_DOWNLOAD_MODEL: DownloadSelectedModel(); return 0;
        case IDC_MODEL_OPEN: OpenSelectedModelFolder(); return 0;
        case IDC_MODEL_PICK:
            if (HIWORD(wp) == CBN_SELCHANGE) UpdateModelActions();
            return 0;
        case IDC_LIB_LIST:
            if (HIWORD(wp) == LBN_SELCHANGE) LoadLibrarySelection();
            return 0;
        case IDC_LIB_USE: UseLibraryVoice(); return 0;
        case IDC_LIB_SAVE: SaveLibraryVoice(); return 0;
        case IDC_LIB_ADD_SAMPLE: AddLibrarySample(); return 0;
        case IDC_LIB_EXPORT: ExportLibraryVoice(); return 0;
        case IDC_LIB_IMPORT: ImportLibraryVoice(); return 0;
        case IDC_HISTORY_OPEN: OpenSelectedHistoryOutput(); return 0;
        case IDC_HISTORY_REFRESH: RefreshLibrary(); return 0;
        case IDC_LIB_BROWSE_REF: { auto p = OpenDialog(L"Audio\0*.wav;*.mp3;*.ogg;*.flac\0All\0*.*\0"); if (!p.empty()) Text(g.h[IDC_LIB_REF_AUDIO], p); return 0; }
        case IDC_LIB_BROWSE_SAMPLE: { auto p = OpenDialog(L"Audio\0*.wav;*.mp3;*.ogg;*.flac\0All\0*.*\0"); if (!p.empty()) Text(g.h[IDC_LIB_SAMPLE_AUDIO], p); return 0; }
        }
        return 0;
    case WM_HSCROLL:
        if ((HWND)lp == g.h[IDC_VOLUME]) ApplyPlaybackVolume();
        return 0;
    case WM_APP + 1: {
        auto* s = (std::wstring*)lp;
        Log(*s);
        UpdateOutputPathFromLog(*s);
        delete s;
        g.running = false;
        EnableWindow(g.h[IDC_GENERATE], TRUE);
        EnableWindow(g.h[IDC_CLONE_GENERATE], TRUE);
        SetStatus(wp == 0 ? L"Complete" : L"Failed - see log");
        ScanModels();
        RefreshLibrary();
        return 0;
    }
    case WM_DESTROY:
        mciSendStringW(L"close qwen_audio", nullptr, 0, nullptr);
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcW(hwnd, msg, wp, lp);
}

} // namespace

int WINAPI wWinMain(HINSTANCE inst, HINSTANCE, PWSTR, int show) {
    WNDCLASSEXW wc{ sizeof(wc) };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = inst;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hIcon = LoadIcon(nullptr, IDI_APPLICATION);
    wc.hbrBackground = nullptr;
    wc.lpszClassName = L"QwensTtsBuddy";
    RegisterClassExW(&wc);
    HWND hwnd = CreateWindowExW(0, wc.lpszClassName, L"Qwens Tts Buddy", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
        CW_USEDEFAULT, CW_USEDEFAULT, 1160, 860, nullptr, nullptr, inst, nullptr);
    ShowWindow(hwnd, show);
    UpdateWindow(hwnd);
    MSG msg{};
    while (GetMessageW(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageW(&msg);
    }
    return (int)msg.wParam;
}
