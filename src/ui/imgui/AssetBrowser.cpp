#include "AssetBrowser.hpp"
#include <imgui.h>
#include <algorithm>
#include <cstring>
#include <cctype>

// ─────────────────────────────────────────────────────────────────────────────
//  Dados estáticos
// ─────────────────────────────────────────────────────────────────────────────

const char* AssetBrowser::s_Folders[] = {
    "assets/",
    "  animations/",
    "  audio/",
    "  fonts/",
    "  models/",
    "  scenes/",
    "  shaders/",
    "  textures/",
};

const char* AssetBrowser::s_Files[] = {
    "cube.obj",      "sphere.obj",    "player.fbx",
    "diffuse.png",   "normal.png",    "roughness.png",
    "scene_001.json","main.lua",
    "default.vert",  "default.frag",
};

// ─────────────────────────────────────────────────────────────────────────────
//  Helpers internos (file-local)
// ─────────────────────────────────────────────────────────────────────────────

static std::string s_ToLower(const std::string& s) {
    std::string r = s;
    std::transform(r.begin(), r.end(), r.begin(), ::tolower);
    return r;
}

static std::string s_ExtractExt(const std::string& fname) {
    auto dot = fname.rfind('.');
    return (dot != std::string::npos) ? s_ToLower(fname.substr(dot + 1)) : "";
}

// Abrevia nome longo com "..." para caber na célula
static std::string s_Truncate(const std::string& s, float maxW) {
    if (ImGui::CalcTextSize(s.c_str()).x <= maxW) return s;
    std::string t = s;
    while (t.size() > 3 && ImGui::CalcTextSize((t + "...").c_str()).x > maxW)
        t.pop_back();
    return t + "...";
}

// ─────────────────────────────────────────────────────────────────────────────
//  Constructor
// ─────────────────────────────────────────────────────────────────────────────

AssetBrowser::AssetBrowser() {
    RebuildEntries();
}

// ─────────────────────────────────────────────────────────────────────────────
//  API pública — injeção de preview de textura
// ─────────────────────────────────────────────────────────────────────────────

void AssetBrowser::RegisterTexturePreview(const std::string& filename, uint32_t texID) {
    m_PreviewCache[filename] = texID;
    for (auto& e : m_AllEntries)
        if (e.name == filename) { e.previewTexID = texID; break; }
}

// ─────────────────────────────────────────────────────────────────────────────
//  Mapeamento ext → categoria
// ─────────────────────────────────────────────────────────────────────────────

AssetFilterType AssetBrowser::CategoryForExt(const std::string& ext) const {
    if (ext == "obj" || ext == "fbx" || ext == "gltf" || ext == "glb")
        return AssetFilterType::Mesh;
    if (ext == "png"  || ext == "jpg"  || ext == "jpeg" ||
        ext == "tga"  || ext == "hdr"  || ext == "exr")
        return AssetFilterType::Texture;
    if (ext == "vert" || ext == "frag" || ext == "glsl" || ext == "hlsl" || ext == "comp")
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

const char* AssetBrowser::GetIconForExt(const std::string& ext) const {
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

ImVec4 AssetBrowser::GetColorForExt(const std::string& ext) const {
    return GetColorForFilter(CategoryForExt(ext));
}

// ─────────────────────────────────────────────────────────────────────────────
//  Labels e cores dos filtros
// ─────────────────────────────────────────────────────────────────────────────

const char* AssetBrowser::GetLabelForFilter(AssetFilterType f) const {
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

ImVec4 AssetBrowser::GetColorForFilter(AssetFilterType f) const {
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
//  RebuildEntries — popula m_AllEntries a partir dos dados estáticos
// ─────────────────────────────────────────────────────────────────────────────

void AssetBrowser::RebuildEntries() {
    int count = (int)(sizeof(s_Files) / sizeof(s_Files[0]));
    m_AllEntries.clear();
    m_AllEntries.reserve(count);

    for (int i = 0; i < count; ++i) {
        AssetEntry e;
        e.name     = s_Files[i];
        e.ext      = s_ExtractExt(e.name);
        e.category = CategoryForExt(e.ext);

        // Reaproveita preview já injetado (caso pasta mude e recarregue)
        auto it = m_PreviewCache.find(e.name);
        if (it != m_PreviewCache.end())
            e.previewTexID = it->second;

        m_AllEntries.push_back(std::move(e));
    }
    m_ListDirty = true;
}

// ─────────────────────────────────────────────────────────────────────────────
//  RebuildFiltered — aplica busca + filtro de tipo + ordenação
// ─────────────────────────────────────────────────────────────────────────────

void AssetBrowser::RebuildFiltered() {
    m_Filtered.clear();
    std::string search = s_ToLower(m_SearchBuf);

    for (int i = 0; i < (int)m_AllEntries.size(); ++i) {
        const auto& e = m_AllEntries[i];

        // Filtro de tipo
        if (m_FilterType != AssetFilterType::All && e.category != m_FilterType)
            continue;

        // Filtro de busca
        if (!search.empty() && s_ToLower(e.name).find(search) == std::string::npos)
            continue;

        m_Filtered.push_back(i);
    }

    // Ordenação
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

void AssetBrowser::Draw() {
    // Rebuild lazy — só quando necessário
    if (m_ListDirty) RebuildFiltered();

    ImGui::PushStyleColor(ImGuiCol_WindowBg, EditorTheme::Color::BgBase);

    if (ImGui::Begin("Asset Browser")) {
        DrawToolbar();
        DrawFilterTabs();
        ImGui::Separator();

        float totalW = ImGui::GetContentRegionAvail().x;
        constexpr float kTreeW = 138.0f;
        float gridW = totalW - kTreeW - ImGui::GetStyle().ItemSpacing.x;

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
}

// ─────────────────────────────────────────────────────────────────────────────
//  Toolbar — busca + sort + tamanho de ícone + import
// ─────────────────────────────────────────────────────────────────────────────

void AssetBrowser::DrawToolbar() {
    // Breadcrumb
    DrawBreadcrumb();

    // ── Linha de controles ────────────────────────────────────────────────────
    ImGui::PushStyleColor(ImGuiCol_FrameBg, EditorTheme::Color::BgInput);

    // Busca
    ImGui::SetNextItemWidth(180.0f);
    bool searchChanged = ImGui::InputTextWithHint(
        "##search", "Buscar assets...", m_SearchBuf, sizeof(m_SearchBuf));
    if (searchChanged) m_ListDirty = true;

    ImGui::SameLine();

    // Sort
    ImGui::SetNextItemWidth(110.0f);
    const char* sortLabels[] = { "Nome A-Z", "Nome Z-A", "Tipo" };
    int sortIdx = (int)m_SortMode;
    if (ImGui::Combo("##sort", &sortIdx, sortLabels, 3)) {
        m_SortMode  = (AssetSortMode)sortIdx;
        m_ListDirty = true;
    }

    ImGui::SameLine();

    // Slider de tamanho de ícone
    ImGui::SetNextItemWidth(80.0f);
    ImGui::PushStyleColor(ImGuiCol_SliderGrab,       EditorTheme::Color::Accent);
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, EditorTheme::Color::AccentActive);
    ImGui::SliderFloat("##iconSize", &m_IconSize, 40.0f, 96.0f, "%.0f px");
    ImGui::PopStyleColor(2);

    // Botão Import alinhado à direita
    ImGui::SameLine(ImGui::GetContentRegionAvail().x + ImGui::GetCursorPosX() - 62.0f);
    ImGui::PushStyleColor(ImGuiCol_Button,        EditorTheme::Color::BgPanel);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorTheme::Color::BgHover);
    ImGui::Button("Import", { 60.0f, 22.0f });
    ImGui::PopStyleColor(2);

    ImGui::PopStyleColor(); // FrameBg
}

// ─────────────────────────────────────────────────────────────────────────────
//  Breadcrumb — caminho atual
// ─────────────────────────────────────────────────────────────────────────────

void AssetBrowser::DrawBreadcrumb() {
    ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
    ImGui::Text(">");
    ImGui::PopStyleColor();
    ImGui::SameLine();
    ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::Accent);
    ImGui::Text("%s", m_CurrentPath.c_str());
    ImGui::PopStyleColor();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Filter tabs — botões de tipo coloridos
// ─────────────────────────────────────────────────────────────────────────────

void AssetBrowser::DrawFilterTabs() {
    ImGui::Spacing();
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 10.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   { 4.0f, 4.0f });

    for (int i = 0; i < (int)AssetFilterType::COUNT; ++i) {
        auto  f       = (AssetFilterType)i;
        bool  active  = (m_FilterType == f);
        ImVec4 col    = (f == AssetFilterType::All)
                        ? EditorTheme::Color::Accent
                        : GetColorForFilter(f);

        // Botão ativo = cor cheia; inativo = escurecido
        ImVec4 btnCol   = active ? col : ImVec4{ col.x * 0.35f, col.y * 0.35f, col.z * 0.35f, 1.0f };
        ImVec4 hoverCol = active ? col : ImVec4{ col.x * 0.55f, col.y * 0.55f, col.z * 0.55f, 1.0f };

        ImGui::PushStyleColor(ImGuiCol_Button,        btnCol);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, hoverCol);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  col);
        ImGui::PushStyleColor(ImGuiCol_Text,
            active ? ImVec4{0.05f,0.05f,0.05f,1.0f} : EditorTheme::Color::TextDim);

        // Conta quantos assets tem nesse tipo (exceto "Todos")
        int cnt = 0;
        if (f == AssetFilterType::All) {
            cnt = (int)m_AllEntries.size();
        } else {
            for (const auto& e : m_AllEntries)
                if (e.category == f) cnt++;
        }

        char label[32];
        if (cnt > 0)
            snprintf(label, sizeof(label), " %s (%d) ", GetLabelForFilter(f), cnt);
        else
            snprintf(label, sizeof(label), " %s ", GetLabelForFilter(f));

        if (ImGui::Button(label)) {
            m_FilterType = f;
            m_ListDirty  = true;
        }

        ImGui::PopStyleColor(4);

        if (i < (int)AssetFilterType::COUNT - 1)
            ImGui::SameLine();
    }

    ImGui::PopStyleVar(2);
    ImGui::Spacing();
}

// ─────────────────────────────────────────────────────────────────────────────
//  Folder tree (esquerda)
// ─────────────────────────────────────────────────────────────────────────────

void AssetBrowser::DrawFolderTree() {
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 4.0f, 3.0f });

    int nFolders = (int)(sizeof(s_Folders) / sizeof(s_Folders[0]));

    for (int i = 0; i < nFolders; ++i) {
        const char* label = s_Folders[i];
        bool sel = (m_CurrentPath == label);
        bool isRoot = (label[0] != ' ');   // sem indentação = raiz

        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, EditorTheme::Color::BgHover);   // 1
        ImGui::PushStyleColor(ImGuiCol_Header,        EditorTheme::Color::AccentDim); // 2

        // Ícone de pasta
        ImGui::PushStyleColor(ImGuiCol_Text,                                           // 3
            isRoot ? EditorTheme::Color::Accent : EditorTheme::Color::TextDim);
        ImGui::Text(isRoot ? "[*]" : " >");
        ImGui::PopStyleColor();                                                         // -3
        ImGui::SameLine();

        // Cor do texto do Selectable
        ImGui::PushStyleColor(ImGuiCol_Text,                                           // 3
            sel ? EditorTheme::Color::TextBright : EditorTheme::Color::TextDim);

        if (ImGui::Selectable(label, sel, ImGuiSelectableFlags_None, { 0.0f, 0.0f })) {
            m_CurrentPath = label;
            RebuildEntries();
        }

        ImGui::PopStyleColor(3); // Header, HeaderHovered, Text
    }

    ImGui::PopStyleVar();
}

// ─────────────────────────────────────────────────────────────────────────────
//  File grid (direita) — com preview de textura
// ─────────────────────────────────────────────────────────────────────────────

void AssetBrowser::DrawFileGrid() {
    if (m_Filtered.empty()) {
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);
        ImGui::Spacing();
        ImGui::SetCursorPosX(
            ImGui::GetCursorPosX() + ImGui::GetContentRegionAvail().x * 0.5f - 60.0f);
        ImGui::Text("Nenhum asset encontrado.");
        ImGui::PopStyleColor();
        return;
    }

    const float pad   = 8.0f;
    const float cellW = m_IconSize + pad * 2.0f;
    const float avail = ImGui::GetContentRegionAvail().x;
    const int   cols  = std::max(1, (int)(avail / cellW));

    // ── BeginTable — cada coluna tem largura fixa e gerencia cursor sozinha ──
    // ImGuiTableFlags_NoPadOuterX remove padding lateral da tabela inteira.
    // Sem borders para parecer grade livre, não tabela de dados.
    const ImGuiTableFlags tableFlags =
        ImGuiTableFlags_NoPadOuterX |
        ImGuiTableFlags_SizingFixedSame;

    if (!ImGui::BeginTable("##assetGrid", cols, tableFlags))
        return;

    for (int col = 0; col < cols; ++col)
        ImGui::TableSetupColumn("", ImGuiTableColumnFlags_WidthFixed, m_IconSize + pad);

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   { pad, pad });

    for (int fi = 0; fi < (int)m_Filtered.size(); ++fi) {
        ImGui::TableNextColumn();   // avança coluna; quebra linha automaticamente

        const int         idx = m_Filtered[fi];
        const AssetEntry& e   = m_AllEntries[idx];
        bool              sel = (m_SelectedIdx == idx);

        ImGui::PushID(idx);

        // ── Botão base ────────────────────────────────────────────────────────
        ImVec4 btnBg = sel ? EditorTheme::Color::AccentDim : EditorTheme::Color::BgPanel;
        ImGui::PushStyleColor(ImGuiCol_Button,        btnBg);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, EditorTheme::Color::BgHover);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  EditorTheme::Color::Accent);

        ImVec2 btnPos = ImGui::GetCursorScreenPos();
        ImGui::Button("##asset", { m_IconSize, m_IconSize });

        if (ImGui::IsItemClicked())
            m_SelectedIdx = idx;
        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
            if (m_DropCallback) m_DropCallback(e.name);

        ImGui::PopStyleColor(3);

        // ── Preview de textura ou ícone colorido ──────────────────────────────
        ImDrawList* dl = ImGui::GetWindowDrawList();

        if (e.previewTexID != 0) {
            const float inset = 4.0f;
            dl->AddImage(
                (ImTextureID)(uintptr_t)e.previewTexID,
                { btnPos.x + inset,              btnPos.y + inset },
                { btnPos.x + m_IconSize - inset, btnPos.y + m_IconSize - inset },
                { 0, 0 }, { 1, 1 });

            // Badge de tipo no canto superior-direito
            const char* badge    = GetIconForExt(e.ext);
            ImVec4      badgeCol = GetColorForExt(e.ext);
            ImVec2      badgeSz  = ImGui::CalcTextSize(badge);
            dl->AddRectFilled(
                { btnPos.x + m_IconSize - badgeSz.x - 6.0f, btnPos.y + 2.0f },
                { btnPos.x + m_IconSize - 2.0f,             btnPos.y + badgeSz.y + 4.0f },
                ImGui::ColorConvertFloat4ToU32({ 0.0f, 0.0f, 0.0f, 0.7f }), 3.0f);
            dl->AddText(
                { btnPos.x + m_IconSize - badgeSz.x - 4.0f, btnPos.y + 3.0f },
                ImGui::ColorConvertFloat4ToU32(badgeCol), badge);
        }
        else {
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

        // ── Borda de seleção ──────────────────────────────────────────────────
        if (sel) {
            dl->AddRect(btnPos,
                { btnPos.x + m_IconSize, btnPos.y + m_IconSize },
                ImGui::ColorConvertFloat4ToU32(EditorTheme::Color::Accent),
                4.0f, 0, 2.0f);
        }

        // ── Drag & Drop ───────────────────────────────────────────────────────
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
            ImGui::SetDragDropPayload("ASSET_PATH", e.name.c_str(), e.name.size() + 1);
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
            ImGui::Text("Tipo: .%s", e.ext.c_str());
            ImGui::Text("Duplo clique para usar");
            ImGui::PopStyleColor();
            ImGui::EndTooltip();
        }

        // ── Context menu ──────────────────────────────────────────────────────
        if (ImGui::BeginPopupContextItem("##assetctx")) {
            ImGui::PushStyleColor(ImGuiCol_Text, GetColorForExt(e.ext));
            ImGui::Text("%s %s", GetIconForExt(e.ext), e.name.c_str());
            ImGui::PopStyleColor();
            ImGui::Separator();
            ImGui::MenuItem("Abrir");
            ImGui::MenuItem("Mostrar no Explorer");
            ImGui::MenuItem("Copiar caminho");
            ImGui::Separator();
            ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::Error);
            ImGui::MenuItem("Deletar");
            ImGui::PopStyleColor();
            ImGui::EndPopup();
        }

        // ── Label abaixo do ícone ─────────────────────────────────────────────
        // O label fica dentro da mesma célula da tabela — não interfere no Y
        // das células vizinhas.
        {
            std::string truncated = s_Truncate(e.name, m_IconSize);
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
//  Status bar — rodapé informativo
// ─────────────────────────────────────────────────────────────────────────────

void AssetBrowser::DrawStatusBar() {
    ImGui::Separator();
    ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::TextDim);

    // Contagem filtrada vs total
    if ((int)m_Filtered.size() == (int)m_AllEntries.size())
        ImGui::Text("%d assets", (int)m_AllEntries.size());
    else
        ImGui::Text("%d / %d assets", (int)m_Filtered.size(), (int)m_AllEntries.size());

    // Nome do selecionado
    if (m_SelectedIdx >= 0 && m_SelectedIdx < (int)m_AllEntries.size()) {
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, EditorTheme::Color::Accent);
        ImGui::Text("|  %s", m_AllEntries[m_SelectedIdx].name.c_str());
        ImGui::PopStyleColor();
    }

    ImGui::PopStyleColor();
}