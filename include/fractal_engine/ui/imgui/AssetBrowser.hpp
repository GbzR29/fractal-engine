#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <cstdint>

#include "EditorTheme.hpp"

// ─────────────────────────────────────────────────────────────────────────────
//  Enums
// ─────────────────────────────────────────────────────────────────────────────

enum class AssetFilterType : int {
    All = 0, Mesh, Texture, Shader, Script, Scene, Audio,
    COUNT
};

enum class AssetSortMode : int {
    NameAsc = 0,
    NameDesc,
    TypeAsc
};

// ─────────────────────────────────────────────────────────────────────────────
//  AssetEntry — estrutura que representa um arquivo no browser
// ─────────────────────────────────────────────────────────────────────────────

struct AssetEntry {
    std::string     name;           ///< Filename com extensão  (ex: "diffuse.png")
    std::string     ext;            ///< Extensão lowercase sem ponto (ex: "png")
    AssetFilterType category;       ///< Categoria pré-computada
    uint32_t        previewTexID = 0; ///< GPU texture ID para preview; 0 = sem preview
};

// ─────────────────────────────────────────────────────────────────────────────
//  AssetBrowser
// ─────────────────────────────────────────────────────────────────────────────

class AssetBrowser {
public:
    AssetBrowser();

    /// Ponto de entrada — chamar uma vez por frame dentro de um ImGui frame.
    void Draw();

    /// Injeta um texture ID de GPU para preview de um asset específico.
    /// Chamar do pipeline de asset após a textura ser carregada.
    void RegisterTexturePreview(const std::string& filename, uint32_t texID);

    /// Fired ao dar duplo-clique ou drag em um asset.
    std::function<void(const std::string& path)> m_DropCallback;

private:
    // ── Dados estáticos (mock) ────────────────────────────────────────────────
    static const char* s_Folders[];
    static const char* s_Files[];

    // ── Estado ────────────────────────────────────────────────────────────────
    std::string     m_CurrentPath   = "assets/";
    char            m_SearchBuf[128] = {};
    float           m_IconSize       = 64.0f;
    int             m_SelectedIdx    = -1;
    AssetFilterType m_FilterType     = AssetFilterType::All;
    AssetSortMode   m_SortMode       = AssetSortMode::NameAsc;
    bool            m_ListDirty      = true;  ///< Sinaliza rebuild de m_Filtered

    // ── Cache de entries ──────────────────────────────────────────────────────
    std::vector<AssetEntry>                   m_AllEntries;   ///< Todos os entries da pasta atual
    std::vector<int>                          m_Filtered;     ///< Índices filtrados + ordenados
    std::unordered_map<std::string, uint32_t> m_PreviewCache; ///< filename → texID

    // ── Sub-draws ─────────────────────────────────────────────────────────────
    void DrawToolbar();
    void DrawBreadcrumb();
    void DrawFilterTabs();
    void DrawFolderTree();
    void DrawFileGrid();
    void DrawStatusBar();

    // ── Helpers de dados ──────────────────────────────────────────────────────
    void            RebuildEntries();
    void            RebuildFiltered();
    AssetFilterType CategoryForExt(const std::string& ext) const;

    // ── Helpers visuais ───────────────────────────────────────────────────────
    const char* GetIconForExt(const std::string& ext)    const;
    ImVec4      GetColorForExt(const std::string& ext)   const;
    const char* GetLabelForFilter(AssetFilterType f)     const;
    ImVec4      GetColorForFilter(AssetFilterType f)     const;
};