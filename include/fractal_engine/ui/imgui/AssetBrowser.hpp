/**
 * @file AssetBrowser.hpp
 * @brief File-system asset browser panel rendered inside the editor's dock layout.
 *
 * Features:
 * - Folder tree navigation with breadcrumb path display.
 * - Filter tabs by asset type (mesh, texture, shader, script, scene, audio).
 * - Name-based search with sort options.
 * - Drag-and-drop to the scene viewport via @ref m_DropCallback.
 * - File import (copy into the assets folder) and delete with confirmation dialog.
 * - Texture preview thumbnails injected via @ref RegisterTexturePreview().
 */
#pragma once

#include <imgui.h>
#include <string>
#include <vector>
#include <functional>
#include <unordered_map>
#include <filesystem>
#include <cstdint>

#include "EditorTheme.hpp"

namespace fs = std::filesystem;

/// @brief Asset type categories used for filter tabs.
enum class AssetFilterType : int {
    All = 0, ///< Show all asset types.
    Mesh,    ///< 3-D model files (fbx, obj, gltf, …).
    Texture, ///< Image files (png, jpg, hdr, …).
    Shader,  ///< GLSL shader source files.
    Script,  ///< Lua script files.
    Scene,   ///< Scene JSON files.
    Audio,   ///< Audio files (wav, mp3, ogg, …).
    COUNT    ///< Sentinel — number of filter categories.
};

/// @brief Sort order options for the asset grid.
enum class AssetSortMode : int {
    NameAsc  = 0, ///< Sort by filename A→Z.
    NameDesc,     ///< Sort by filename Z→A.
    TypeAsc       ///< Sort by file extension A→Z.
};

// ─────────────────────────────────────────────────────────────────────────────
//  AssetEntry — a single file on disk
// ─────────────────────────────────────────────────────────────────────────────

/// @brief Metadata for one asset file discovered during directory scanning.
struct AssetEntry {
    std::string     name;                           ///< Filename with extension (e.g. "character.fbx").
    std::string     relativePath;                   ///< Path relative to the assets root (e.g. "models/character.fbx").
    fs::path        absolutePath;                   ///< Full filesystem path.
    std::string     ext;                            ///< Lowercase extension without the leading dot (e.g. "fbx").
    AssetFilterType category = AssetFilterType::All; ///< Detected category for filter tabs.
    uint32_t        previewTexID = 0;               ///< Optional GPU texture ID for thumbnail rendering.
};

// ─────────────────────────────────────────────────────────────────────────────
//  AssetBrowser
// ─────────────────────────────────────────────────────────────────────────────

/// @brief Editor panel for browsing, importing, and dragging assets into the scene.
class AssetBrowser {
public:
    AssetBrowser();

    /**
     * @brief Sets the root directory for asset discovery.
     * @param root  Must match @ref AssetLoader::assetsRoot().
     */
    void SetAssetsRoot(const fs::path& root);

    /// Renders the full asset browser panel.  Call once per ImGui frame.
    void Draw();

    /**
     * @brief Registers a GPU texture for use as a thumbnail in the asset grid.
     * @param relativePath  Asset-relative path that identifies the asset entry.
     * @param texID         OpenGL texture ID to display as a preview image.
     */
    void RegisterTexturePreview(const std::string& relativePath, uint32_t texID);

    /// Callback fired when the user double-clicks or drops an asset into the viewport.
    /// The argument is the asset-relative path.
    std::function<void(const std::string& relativePath)> m_DropCallback;

private:
    // ── Paths ──────────────────────────────────────────────────────────────────
    fs::path m_RootPath;       ///< Caminho absoluto do assets root
    fs::path m_CurrentDir;     ///< Diretório atual sendo exibido (absoluto)

    // ── Estado ────────────────────────────────────────────────────────────────
    char            m_SearchBuf[128]  = {};
    float           m_IconSize        = 64.0f;
    int             m_SelectedIdx     = -1;
    AssetFilterType m_FilterType      = AssetFilterType::All;
    AssetSortMode   m_SortMode        = AssetSortMode::NameAsc;
    bool            m_ListDirty       = true;

    // ── Cache de entries ──────────────────────────────────────────────────────
    std::vector<AssetEntry>                   m_AllEntries;
    std::vector<int>                          m_Filtered;
    std::vector<fs::path>                     m_SubDirs;    ///< Subpastas do dir atual
    std::unordered_map<std::string, uint32_t> m_PreviewCache;

    // ── Import popup ──────────────────────────────────────────────────────────
    bool m_ShowImportPopup  = false;
    char m_ImportSrcBuf[512] = {};

    // ── Delete confirmação ────────────────────────────────────────────────────
    int  m_ConfirmDeleteIdx = -1;

    // ── Sub-draws ─────────────────────────────────────────────────────────────
    void DrawToolbar();
    void DrawBreadcrumb();
    void DrawFilterTabs();
    void DrawFolderTree();
    void DrawFileGrid();
    void DrawStatusBar();
    void DrawImportPopup();
    void DrawDeleteConfirmModal();

    // ── Helpers de dados ──────────────────────────────────────────────────────
    void            RebuildEntries();
    void            RebuildFiltered();
    void            NavigateTo(const fs::path& dir);
    std::string     RelativeTo(const fs::path& absPath) const;
    AssetFilterType CategoryForExt(const std::string& ext) const;

    // ── Helpers visuais ───────────────────────────────────────────────────────
    const char* GetIconForExt(const std::string& ext)    const;
    ImVec4      GetColorForExt(const std::string& ext)   const;
    const char* GetLabelForFilter(AssetFilterType f)     const;
    ImVec4      GetColorForFilter(AssetFilterType f)     const;

    // ── Sistema de arquivos ───────────────────────────────────────────────────
    bool        ImportFile(const fs::path& src);
    bool        DeleteEntry(int idx);
    void        ShowInExplorer(const fs::path& path) const;
    void        CopyPathToClipboard(const std::string& relPath) const;

    static std::string ToLower(const std::string& s);
    static std::string ExtractExt(const std::string& fname);
    static std::string Truncate(const std::string& s, float maxW);
};
