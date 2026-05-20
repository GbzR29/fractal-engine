#include "AssetBrowser.hpp"
#include "AssetLoader.hpp"
#include "EditorTheme.hpp"

#include <imgui.h>
#include <algorithm>
#include <cstring>
#include <cctype>
#include <iostream>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <shellapi.h>
#endif

// ─────────────────────────────────────────────────────────────────────────────
//  Statics
// ─────────────────────────────────────────────────────────────────────────────

std::string AssetBrowser::ToLower(const std::string& s)
{
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(), ::tolower);
    return r;
}

std::string AssetBrowser::ExtractExt(const std::string& fname)
{
    auto dot = fname.rfind('.');
    return (dot != std::string::npos) ? ToLower(fname.substr(dot + 1)) : "";
}

std::string AssetBrowser::Truncate(const std::string& s, float maxW)
{
    if (ImGui::CalcTextSize(s.c_str()).x <= maxW) return s;
    std::string t = s;
    while (t.size() > 3 && ImGui::CalcTextSize((t + "...").c_str()).x > maxW)
        t.pop_back();
    return t + "...";
}

// ─────────────────────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────────────────────

AssetBrowser::AssetBrowser()
{
    // Usa o assetsRoot já configurado no AssetLoader (default = "assets")
    SetAssetsRoot(AssetLoader::assetsRoot());
}

void AssetBrowser::SetAssetsRoot(const fs::path& root)
{
    m_RootPath   = fs::absolute(root);
    m_CurrentDir = m_RootPath;
    RebuildEntries();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Filesystem helpers
// ─────────────────────────────────────────────────────────────────────────────

std::string AssetBrowser::RelativeTo(const fs::path& absPath) const
{
    std::error_code ec;
    auto rel = fs::relative(absPath, m_RootPath, ec);
    return ec ? absPath.generic_string() : rel.generic_string();
}

void AssetBrowser::NavigateTo(const fs::path& dir)
{
    if (fs::exists(dir) && fs::is_directory(dir)) {
        m_CurrentDir  = dir;
        m_SelectedIdx = -1;
        RebuildEntries();
    }
}

bool AssetBrowser::ImportFile(const fs::path& src)
{
    if (!fs::exists(src) || !fs::is_regular_file(src)) {
        std::cerr << "[AssetBrowser] Import: arquivo não existe: " << src << "\n";
        return false;
    }

    fs::path dest = m_CurrentDir / src.filename();

    // Evita sobrescrever sem querer
    if (fs::exists(dest)) {
        std::string stem = src.stem().string();
        std::string ext  = src.extension().string();
        int n = 1;
        do {
            dest = m_CurrentDir / (stem + "_" + std::to_string(n++) + ext);
        } while (fs::exists(dest));
    }

    std::error_code ec;
    fs::copy_file(src, dest, fs::copy_options::none, ec);
    if (ec) {
        std::cerr << "[AssetBrowser] Import falhou: " << ec.message() << "\n";
        return false;
    }

    std::cout << "[AssetBrowser] Importado: " << dest << "\n";
    RebuildEntries();
    return true;
}

bool AssetBrowser::DeleteEntry(int idx)
{
    if (idx < 0 || idx >= (int)m_AllEntries.size()) return false;

    std::error_code ec;
    fs::remove(m_AllEntries[idx].absolutePath, ec);
    if (ec) {
        std::cerr << "[AssetBrowser] Delete falhou: " << ec.message() << "\n";
        return false;
    }

    m_SelectedIdx = -1;
    RebuildEntries();
    return true;
}

void AssetBrowser::ShowInExplorer(const fs::path& path) const
{
#ifdef _WIN32
    std::string cmd = "explorer /select,\"" + path.string() + "\"";
    std::replace(cmd.begin(), cmd.end(), '/', '\\');
    system(cmd.c_str());
#endif
}

void AssetBrowser::CopyPathToClipboard(const std::string& relPath) const
{
    ImGui::SetClipboardText(relPath.c_str());
}

// ─────────────────────────────────────────────────────────────────────────────
//  RegisterTexturePreview
// ─────────────────────────────────────────────────────────────────────────────

void AssetBrowser::RegisterTexturePreview(const std::string& relativePath, uint32_t texID)
{
    m_PreviewCache[relativePath] = texID;
    for (auto& e : m_AllEntries)
        if (e.relativePath == relativePath) { e.previewTexID = texID; break; }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Mapeamento ext → categoria
// ─────────────────────────────────────────────────────────────────────────────

AssetFilterType AssetBrowser::CategoryForExt(const std::string& ext) const
{
    if (ext == "obj"  || ext == "fbx"  || ext == "gltf" || ext == "glb"  ||
        ext == "dae"  || ext == "blend" || ext == "3ds"  || ext == "stl")
        return AssetFilterType::Mesh;
    if (ext == "png"  || ext == "jpg"  || ext == "jpeg" || ext == "tga"  ||
        ext == "bmp"  || ext == "hdr"  || ext == "exr")
        return AssetFilterType::Texture;
    if (ext == "vert" || ext == "frag" || ext == "glsl" || ext == "comp" || ext == "geom")
        return AssetFilterType::Shader;
    if (ext == "lua"  || ext == "py"   || ext == "js")
        return AssetFilterType::Script;
    if (ext == "json" || ext == "scene"|| ext == "yaml")
        return AssetFilterType::Scene;
    if (ext == "wav"  || ext == "mp3"  || ext == "ogg"  || ext == "flac")
        return AssetFilterType::Audio;
    return AssetFilterType::All;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Ícone e cor por extensão
// ─────────────────────────────────────────────────────────────────────────────

const char* AssetBrowser::GetIconForExt(const std::string& ext) const
{
    switch (CategoryForExt(ext)) {
        case AssetFilterType::Mesh:    return "[M]";
        case AssetFilterType::Texture: return "[T]";
        case AssetFilterType::Shader:  return "[SH]";
        case AssetFilterType::Script:  return "[SC]";
        case AssetFilterType::Scene:   return "[SN]";
        case AssetFilterType::Audio:   return "[A]";
        default:                       return "[F]";
    }
}

ImVec4 AssetBrowser::GetColorForExt(const std::string& ext) const
{
    return GetColorForFilter(CategoryForExt(ext));
}

const char* AssetBrowser::GetLabelForFilter(AssetFilterType f) const
{
    switch (f) {
        case AssetFilterType::All:     return "Todos";
        case AssetFilterType::Mesh:    return "Mesh";
        case AssetFilterType::Texture: return "Texture";
        case AssetFilterType::Shader:  return "Shader";
        case AssetFilterType::Script:  return "Script";
        case AssetFilterType::Scene:   return "Scene";
        case AssetFilterType::Audio:   return "Audio";
        default:                       return "?";
    }
}

ImVec4 AssetBrowser::GetColorForFilter(AssetFilterType f) const
{
    switch (f) {
        case AssetFilterType::Mesh:    return { 0.40f, 0.75f, 0.95f, 1.0f };
        case AssetFilterType::Texture: return { 0.85f, 0.65f, 0.25f, 1.0f };
        case AssetFilterType::Shader:  return { 0.75f, 0.40f, 0.90f, 1.0f };
        case AssetFilterType::Script:  return { 0.40f, 0.85f, 0.50f, 1.0f };
        case AssetFilterType::Scene:   return { 0.90f, 0.55f, 0.30f, 1.0f };
        case AssetFilterType::Audio:   return { 0.95f, 0.40f, 0.55f, 1.0f };
        default:                       return EditorTheme::Color::TextDim;
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  RebuildEntries — lê o diretório atual do disco
// ─────────────────────────────────────────────────────────────────────────────

void AssetBrowser::RebuildEntries()
{
    m_AllEntries.clear();
    m_SubDirs.clear();

    if (!fs::exists(m_CurrentDir) || !fs::is_directory(m_CurrentDir)) {
        m_ListDirty = true;
        return;
    }

    std::error_code ec;
    for (const auto& entry : fs::directory_iterator(m_CurrentDir, ec)) {
        if (entry.is_directory(ec)) {
            m_SubDirs.push_back(entry.path());
            continue;
        }
        if (!entry.is_regular_file(ec)) continue;

        AssetEntry e;
        e.absolutePath = entry.path();
        e.name         = entry.path().filename().string();
        e.ext          = ExtractExt(e.name);
        e.category     = CategoryForExt(e.ext);
        e.relativePath = RelativeTo(entry.path());

        auto it = m_PreviewCache.find(e.relativePath);
        if (it != m_PreviewCache.end())
            e.previewTexID = it->second;

        m_AllEntries.push_back(std::move(e));
    }

    // Ordena subpastas por nome
    std::sort(m_SubDirs.begin(), m_SubDirs.end());

    m_ListDirty = true;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RebuildFiltered
// ─────────────────────────────────────────────────────────────────────────────

void AssetBrowser::RebuildFiltered()
{
    m_Filtered.clear();
    std::string search = ToLower(m_SearchBuf);

    for (int i = 0; i < (int)m_AllEntries.size(); ++i) {
        const auto& e = m_AllEntries[i];
        if (m_FilterType != AssetFilterType::All && e.category != m_FilterType) continue;
        if (!search.empty() && ToLower(e.name).find(search) == std::string::npos) continue;
        m_Filtered.push_back(i);
    }

    std::sort(m_Filtered.begin(), m_Filtered.end(),
        [&](int a, int b) {
            const auto& ea = m_AllEntries[a];
            const auto& eb = m_AllEntries[b];
            switch (m_SortMode) {
                case AssetSortMode::NameAsc:  return ea.name < eb.name;
                case AssetSortMode::NameDesc: return ea.name > eb.name;
                case AssetSortMode::TypeAsc:
                    if (ea.ext != eb.ext) return ea.ext < eb.ext;
                    return ea.name < eb.name;
            }
            return false;
        });

    m_ListDirty = false;
}

// ─────────────────────────────────────────────────────────────────────────────
//  Draw — ponto de entrada
// ─────────────────────────────────────────────────────────────────────────────

void AssetBrowser::Draw()
{
    if (m_ListDirty) RebuildFiltered();

    ImGui::PushStyleColor(ImGuiCol_WindowBg, EditorTheme::Color::BgBase);

    if (ImGui::Begin("Asset Browser")) {
        DrawToolbar();
        DrawFilterTabs();
        ImGui::Separator();

        float totalW  = ImGui::GetContentRegionAvail().x;
        constexpr float kTreeW = 148.0f;
        float gridW   = totalW - kTreeW - ImGui::GetStyle().ItemSpacing.x;

        ImGui::BeginChild("##FolderPanel", { kTreeW, -ImGui::GetFrameHeightWithSpacing() }, false);
        DrawFolderTree();
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginChild("##GridPanel", { gridW, -ImGui::GetFrameHeightWithSpacing() }, false);
        DrawFileGrid();
        ImGui::EndChild();

        DrawStatusBar();
    }

    ImGui::End();
    ImGui::PopStyleColor();

    // Popups e modais fora da janela principal para evitar empilhamento de estilos
    DrawImportPopup();
    DrawDeleteConfirmModal();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Toolbar
// ─────────────────────────────────────────────────────────────────────────────

void AssetBrowser::DrawToolbar()
{
    DrawBreadcrumb();

    ImGui::PushStyleColor(ImGuiCol_FrameBg, EditorTheme::Color::BgInput);

    ImGui::SetNextItemWidth(180.0f);
    if (ImGui::InputTextWithHint("##search", "Buscar assets...", m_SearchBuf, sizeof(m_SearchBuf)))
        m_ListDirty = true;

    ImGui::SameLine();
    ImGui::SetNextItemWidth(110.0f);
    const char* sortLabels[] = { "Nome A-Z", "Nome Z-A", "Tipo" };
    int sortIdx = (int)m_SortMode;
    if (ImGui::Combo("##sort", &sortIdx, sortLabels, 3)) {
        m_SortMode  = (AssetSortMode)sortIdx;
        m_ListDirty = true;
    }

    ImGui::SameLine();
    ImGui::SetNextItemWidth(80.0f);
    ImGui::PushStyleColor(ImGuiCol_SliderGrab,       EditorTheme::Color::Accent);
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, EditorTheme::Color::AccentActive);
    ImGui::SliderFloat("##iconSize", &m_IconSize, 40.0f, 96.0f, "%.0f px");
    ImGui::PopStyleColor(2);

    // Botão Refresh
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Button,        EditorTheme::Color::BgPanel);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorTheme::Color::BgHover);
    if (ImGui::Button("Refresh", { 58.0f, 22.0f })) {
        SetAssetsRoot(AssetLoader::assetsRoot()); // reseta root e recarrega
    }
    ImGui::PopStyleColor(2);

    // Botão Import alinhado à direita
    ImGui::SameLine(ImGui::GetContentRegionAvail().x + ImGui::GetCursorPosX() - 62.0f);
    ImGui::PushStyleColor(ImGuiCol_Button,        EditorTheme::Color::Accent);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorTheme::Color::AccentHover);
    if (ImGui::Button("Import", { 60.0f, 22.0f })) {
        m_ShowImportPopup = true;
        m_ImportSrcBuf[0] = '\0';
    }
    ImGui::PopStyleColor(2);

    ImGui::PopStyleColor(); // FrameBg
}

// ─────────────────────────────────────────────────────────────────────────────
//  Breadcrumb
// ─────────────────────────────────────────────────────────────────────────────

void AssetBrowser::DrawBreadcrumb()
{
    // Botão voltar
    bool atRoot = (m_CurrentDir == m_RootPath);
    ImGui::PushStyleColor(ImGuiCol_Button,        EditorTheme::Color::BgPanel);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorTheme::Color::BgHover);
    if (ImGui::Button("<", { 20, 20 }) && !atRoot)
        NavigateTo(m_CurrentDir.parent_path());
    ImGui::PopStyleColor(2);
    ImGui::SameLine();

    // Caminho atual relativo ao root
    std::string relStr = RelativeTo(m_CurrentDir);
    if (relStr == ".") relStr = "assets/";
    else               relStr = "assets/" + relStr + "/";

    ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::Accent);
    ImGui::Text("%s", relStr.c_str());
    ImGui::PopStyleColor();
    ImGui::Spacing();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Filter tabs
// ─────────────────────────────────────────────────────────────────────────────

void AssetBrowser::DrawFilterTabs()
{
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   { 4.0f, 4.0f });

    for (int i = 0; i < (int)AssetFilterType::COUNT; ++i) {
        auto   f      = (AssetFilterType)i;
        bool   active = (m_FilterType == f);
        ImVec4 col    = (f == AssetFilterType::All)
                        ? EditorTheme::Color::Accent
                        : GetColorForFilter(f);

        ImVec4 btnCol   = active ? col : ImVec4{col.x*0.35f, col.y*0.35f, col.z*0.35f, 1.0f};
        ImVec4 hoverCol = active ? col : ImVec4{col.x*0.55f, col.y*0.55f, col.z*0.55f, 1.0f};

        ImGui::PushStyleColor(ImGuiCol_Button,        btnCol);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverCol);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  col);
        ImGui::PushStyleColor(ImGuiCol_Text,
            active ? ImVec4{0.05f,0.05f,0.05f,1.0f} : EditorTheme::Color::TextDim);

        int cnt = 0;
        if (f == AssetFilterType::All) cnt = (int)m_AllEntries.size();
        else for (const auto& e : m_AllEntries) if (e.category == f) cnt++;

        char label[32];
        if (cnt > 0) snprintf(label, sizeof(label), " %s (%d) ", GetLabelForFilter(f), cnt);
        else         snprintf(label, sizeof(label), " %s ",       GetLabelForFilter(f));

        if (ImGui::Button(label)) { m_FilterType = f; m_ListDirty = true; }

        ImGui::PopStyleColor(4);
        if (i < (int)AssetFilterType::COUNT - 1) ImGui::SameLine();
    }

    ImGui::PopStyleVar(2);
    ImGui::Spacing();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Folder tree — subpastas do diretório atual + root
// ─────────────────────────────────────────────────────────────────────────────

void AssetBrowser::DrawFolderTree()
{
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 4.0f, 3.0f });

    // ── Root ─────────────────────────────────────────────────────────────────
    bool rootSel = (m_CurrentDir == m_RootPath);
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::Color::BgHover);
    ImGui::PushStyleColor(ImGuiCol_Header,        EditorTheme::Color::AccentDim);
    ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::Accent);
    ImGui::Text("[*]");
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text,
        rootSel ? EditorTheme::Color::TextBright : EditorTheme::Color::TextNormal);
    if (ImGui::Selectable("assets/", rootSel))
        NavigateTo(m_RootPath);
    ImGui::PopStyleColor(3);

    // ── Subpastas imediatas do root ───────────────────────────────────────────
    std::vector<fs::path> rootDirs;
    std::error_code ec;
    for (const auto& e : fs::directory_iterator(m_RootPath, ec))
        if (e.is_directory(ec)) rootDirs.push_back(e.path());
    std::sort(rootDirs.begin(), rootDirs.end());

    for (const auto& dir : rootDirs) {
        bool sel = (m_CurrentDir == dir);
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::Color::BgHover);
        ImGui::PushStyleColor(ImGuiCol_Header,        EditorTheme::Color::AccentDim);
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
        ImGui::Text("  >");
        ImGui::PopStyleColor();
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text,
            sel ? EditorTheme::Color::TextBright : EditorTheme::Color::TextDim);
        std::string label = "  " + dir.filename().string() + "/";
        if (ImGui::Selectable(label.c_str(), sel))
            NavigateTo(dir);
        ImGui::PopStyleColor(3);
    }

    // ── Subpastas do diretório atual (se não for root) ────────────────────────
    if (m_CurrentDir != m_RootPath) {
        ImGui::Separator();
        for (const auto& sub : m_SubDirs) {
            bool sel = (m_CurrentDir == sub);
            ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
            ImGui::Text("    >");
            ImGui::PopStyleColor();
            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Text,
                sel ? EditorTheme::Color::TextBright : EditorTheme::Color::TextDim);
            std::string subLabel = "    " + sub.filename().string() + "/";
            if (ImGui::Selectable(subLabel.c_str(), sel))
                NavigateTo(sub);
            ImGui::PopStyleColor();
        }
    }

    ImGui::PopStyleVar();
}

// ─────────────────────────────────────────────────────────────────────────────
//  File grid
// ─────────────────────────────────────────────────────────────────────────────

void AssetBrowser::DrawFileGrid()
{
    // Quando a pasta está vazia mostra dica de como importar
    if (m_AllEntries.empty()) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
        ImGui::TextWrapped("Pasta vazia.");
        ImGui::TextWrapped("Use o botao Import (canto superior direito) para adicionar arquivos.");
        ImGui::PopStyleColor();
        return;
    }

    if (m_Filtered.empty()) {
        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
        ImGui::Text("Nenhum asset encontrado.");
        ImGui::PopStyleColor();
        return;
    }

    const float pad   = 8.0f;
    const float cellW = m_IconSize + pad * 2.0f;
    const float avail = ImGui::GetContentRegionAvail().x;
    const int   cols  = std::max(1, (int)(avail / cellW));

    const ImGuiTableFlags tableFlags =
        ImGuiTableFlags_NoPadOuterX | ImGuiTableFlags_SizingFixedSame;

    if (!ImGui::BeginTable("##assetGrid", cols, tableFlags)) return;

    for (int col = 0; col < cols; ++col)
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, m_IconSize + pad);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   { pad, pad });

    for (int fi = 0; fi < (int)m_Filtered.size(); ++fi) {
        ImGui::TableNextColumn();

        const int         idx = m_Filtered[fi];
        const AssetEntry& e   = m_AllEntries[idx];
        bool              sel = (m_SelectedIdx == idx);

        ImGui::PushID(idx);

        ImVec4 btnBg = sel ? EditorTheme::Color::AccentDim : EditorTheme::Color::BgPanel;
        ImGui::PushStyleColor(ImGuiCol_Button,        btnBg);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorTheme::Color::BgHover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  EditorTheme::Color::Accent);

        ImVec2 btnPos = ImGui::GetCursorScreenPos();
        ImGui::Button("##asset", { m_IconSize, m_IconSize });

        if (ImGui::IsItemClicked())
            m_SelectedIdx = idx;
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            if (m_DropCallback) m_DropCallback(e.relativePath);

        ImGui::PopStyleColor(3);

        // ── Ícone / Preview ───────────────────────────────────────────────────
        ImDrawList* dl = ImGui::GetWindowDrawList();

        if (e.previewTexID != 0) {
            const float inset = 4.0f;
            dl->AddImage(
                (ImTextureID)(uintptr_t)e.previewTexID,
                { btnPos.x + inset, btnPos.y + inset },
                { btnPos.x + m_IconSize - inset, btnPos.y + m_IconSize - inset },
                { 0, 0 }, { 1, 1 });
        } else {
            const char* icon   = GetIconForExt(e.ext);
            ImVec4      color  = GetColorForExt(e.ext);
            ImVec2      iconSz = ImGui::CalcTextSize(icon);
            float       iconX  = btnPos.x + (m_IconSize - iconSz.x) * 0.5f;
            float       iconY  = btnPos.y + (m_IconSize - iconSz.y) * 0.5f - 6.0f;

            dl->AddText({ iconX, iconY },
                ImGui::ColorConvertFloat4ToU32(color), icon);

            if (!e.ext.empty()) {
                ImVec2 extSz = ImGui::CalcTextSize(e.ext.c_str());
                dl->AddText(
                    { btnPos.x + (m_IconSize - extSz.x) * 0.5f, iconY + iconSz.y + 3.0f },
                    ImGui::ColorConvertFloat4ToU32({ color.x, color.y, color.z, 0.55f }),
                    e.ext.c_str());
            }
        }

        if (sel) {
            dl->AddRect(btnPos,
                { btnPos.x + m_IconSize, btnPos.y + m_IconSize },
                ImGui::ColorConvertFloat4ToU32(EditorTheme::Color::Accent),
                4.0f, 0, 2.0f);
        }

        // ── Drag & Drop — usa relativePath como payload ───────────────────────
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            // Payload = caminho relativo (ex: "models/character.fbx")
            ImGui::SetDragDropPayload("ASSET_PATH",
                e.relativePath.c_str(), e.relativePath.size() + 1);
            ImGui::PushStyleColor(ImGuiCol_Text, GetColorForExt(e.ext));
            ImGui::Text("%s  %s", GetIconForExt(e.ext), e.name.c_str());
            ImGui::PopStyleColor();
            ImGui::EndDragDropSource();
        }

        // ── Tooltip ───────────────────────────────────────────────────────────
        if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort)) {
            ImGui::BeginTooltip();
            if (e.previewTexID != 0)
                ImGui::Image((ImTextureID)(uintptr_t)e.previewTexID, { 80.0f, 80.0f });
            ImGui::PushStyleColor(ImGuiCol_Text, GetColorForExt(e.ext));
            ImGui::Text("%s", e.name.c_str());
            ImGui::PopStyleColor();
            ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
            ImGui::Text("Caminho: %s", e.relativePath.c_str());
            ImGui::Text("Tipo: .%s", e.ext.c_str());
            ImGui::Text("Duplo clique ou arraste para usar");
            ImGui::PopStyleColor();
            ImGui::EndTooltip();
        }

        // ── Context menu ──────────────────────────────────────────────────────
        if (ImGui::BeginPopupContextItem("##assetctx")) {
            ImGui::PushStyleColor(ImGuiCol_Text, GetColorForExt(e.ext));
            ImGui::Text("%s %s", GetIconForExt(e.ext), e.name.c_str());
            ImGui::PopStyleColor();
            ImGui::Separator();

            if (ImGui::MenuItem("Copiar caminho")) {
                CopyPathToClipboard(e.relativePath);
            }
            if (ImGui::MenuItem("Mostrar no Explorer")) {
                ShowInExplorer(e.absolutePath);
            }

            ImGui::Separator();
            ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::Error);
            if (ImGui::MenuItem("Deletar")) {
                m_ConfirmDeleteIdx = idx;
            }
            ImGui::PopStyleColor();
            ImGui::EndPopup();
        }

        // ── Label ─────────────────────────────────────────────────────────────
        {
            std::string truncated = Truncate(e.name, m_IconSize);
            float nameW = ImGui::CalcTextSize(truncated.c_str()).x;
            float nameX = ImGui::GetCursorPosX() + (m_IconSize - nameW) * 0.5f;
            ImGui::SetCursorPosX(std::max(ImGui::GetCursorPosX(), nameX));
            ImGui::PushStyleColor(ImGuiCol_Text,
                sel ? EditorTheme::Color::TextBright : EditorTheme::Color::TextDim);
            ImGui::TextUnformatted(truncated.c_str());
            ImGui::PopStyleColor();
        }

        ImGui::PopID();
    }

    ImGui::PopStyleVar(2);
    ImGui::EndTable();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Status bar
// ─────────────────────────────────────────────────────────────────────────────

void AssetBrowser::DrawStatusBar()
{
    ImGui::Separator();
    ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);

    if ((int)m_Filtered.size() == (int)m_AllEntries.size())
        ImGui::Text("%d assets", (int)m_AllEntries.size());
    else
        ImGui::Text("%d / %d assets", (int)m_Filtered.size(), (int)m_AllEntries.size());

    if (m_SelectedIdx >= 0 && m_SelectedIdx < (int)m_AllEntries.size()) {
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::Accent);
        ImGui::Text("|  %s", m_AllEntries[m_SelectedIdx].relativePath.c_str());
        ImGui::PopStyleColor();
    }

    ImGui::PopStyleColor();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Import popup
// ─────────────────────────────────────────────────────────────────────────────

void AssetBrowser::DrawImportPopup()
{
    if (m_ShowImportPopup) {
        ImGui::OpenPopup("ImportAsset");
        m_ShowImportPopup = false;
    }

    ImGui::SetNextWindowSize({ 500, 0 }, ImGuiCond_Always);
    if (ImGui::BeginPopupModal("ImportAsset", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::Accent);
        ImGui::Text("Importar Asset");
        ImGui::PopStyleColor();
        ImGui::Separator();
        ImGui::Spacing();

        // Destino
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
        ImGui::Text("Destino:");
        ImGui::SameLine();
        ImGui::PopStyleColor();

        std::string relDir = RelativeTo(m_CurrentDir);
        if (relDir == ".") relDir = "assets/";
        else               relDir = "assets/" + relDir + "/";

        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextNormal);
        ImGui::Text("%s", relDir.c_str());
        ImGui::PopStyleColor();

        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
        ImGui::TextWrapped("Cole o caminho completo do arquivo (ex: C:\\Users\\...\\character.fbx):");
        ImGui::PopStyleColor();
        ImGui::Spacing();

        ImGui::PushStyleColor(ImGuiCol_FrameBg, EditorTheme::Color::BgInput);
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::InputText("##importSrc", m_ImportSrcBuf, sizeof(m_ImportSrcBuf));
        ImGui::PopStyleColor();

        ImGui::Spacing();
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
        ImGui::TextWrapped("O arquivo sera copiado para a pasta de destino acima.");
        ImGui::PopStyleColor();
        ImGui::Spacing();
        ImGui::Separator();

        ImGui::PushStyleColor(ImGuiCol_Button,        EditorTheme::Color::Accent);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorTheme::Color::AccentHover);
        bool doImport = ImGui::Button("Importar", { 100, 0 });
        ImGui::PopStyleColor(2);

        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button,        EditorTheme::Color::BgPanel);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorTheme::Color::BgHover);
        if (ImGui::Button("Cancelar", { 100, 0 }))
            ImGui::CloseCurrentPopup();
        ImGui::PopStyleColor(2);

        if (doImport && m_ImportSrcBuf[0] != '\0') {
            fs::path src(m_ImportSrcBuf);
            ImportFile(src);
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Delete confirmation modal
// ─────────────────────────────────────────────────────────────────────────────

void AssetBrowser::DrawDeleteConfirmModal()
{
    if (m_ConfirmDeleteIdx >= 0)
        ImGui::OpenPopup("ConfirmDelete");

    ImGui::SetNextWindowSize({ 360, 0 }, ImGuiCond_Always);
    if (ImGui::BeginPopupModal("ConfirmDelete", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {

        if (m_ConfirmDeleteIdx >= 0 && m_ConfirmDeleteIdx < (int)m_AllEntries.size()) {
            const auto& e = m_AllEntries[m_ConfirmDeleteIdx];

            ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::Error);
            ImGui::Text("Deletar arquivo?");
            ImGui::PopStyleColor();
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
            ImGui::TextWrapped("%s", e.absolutePath.string().c_str());
            ImGui::Spacing();
            ImGui::TextWrapped("Esta acao nao pode ser desfeita.");
            ImGui::PopStyleColor();
            ImGui::Spacing();
            ImGui::Separator();

            ImGui::PushStyleColor(ImGuiCol_Button,        EditorTheme::Color::Error);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{0.8f, 0.2f, 0.2f, 1.0f});
            if (ImGui::Button("Deletar", { 100, 0 })) {
                DeleteEntry(m_ConfirmDeleteIdx);
                m_ConfirmDeleteIdx = -1;
                ImGui::CloseCurrentPopup();
            }
            ImGui::PopStyleColor(2);

            ImGui::SameLine();
            ImGui::PushStyleColor(ImGuiCol_Button,        EditorTheme::Color::BgPanel);
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorTheme::Color::BgHover);
            if (ImGui::Button("Cancelar", { 100, 0 })) {
                m_ConfirmDeleteIdx = -1;
                ImGui::CloseCurrentPopup();
            }
            ImGui::PopStyleColor(2);
        } else {
            m_ConfirmDeleteIdx = -1;
            ImGui::CloseCurrentPopup();
        }

        ImGui::EndPopup();
    }
}
