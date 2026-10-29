# SekiMod-Assistant
This is a software that helps novice install mod files fast. 




Here is the introduction to this application below
SekiMod Assistant v0.3

> Full English translation of the README included in the latest portable release. The original document contains notes from several rounds of UI changes; these are retained below rather than silently rewritten. Literal newline markers in its final section have been formatted as normal line breaks.

Windows 10/11 x64. This native C++17 / Win32 version supports Simplified Chinese, English, and Japanese. It does not require .NET or the Visual C++ runtime. The main executable is statically linked and uses system DLLs supplied with Windows. Edge is used to download mods; the interface itself does not depend on a browser. Previous .NET release packages remain separate.

## Getting Started

- Portable version: fully extract `SekiModAssistant-v0.3-Native-portable.zip`, then double-click `SekiModAssistant.exe`. Keep the `assets`, `tools`, and `licenses` folders and the configuration files together. Do not copy only the main EXE.
- Both the portable application and the installed main application request administrator privileges at startup. Accept the Windows UAC prompt to continue; canceling prevents the application from starting. Once it has started, you do not need to reopen it to obtain administrator privileges.
- The large path list in the local mod import section accepts multiple files, folders, and archives by drag and drop. You can also use the Add buttons below it. Duplicate paths are added only once. Dropping items only adds them to the pending import list; installation takes place after you click “Check and import into mods” and confirm. Compatibility handling has been configured for file-drop messages sent from ordinary File Explorer to the administrator window.
- Installer version: run `SekiModAssistant-v0.3-Native-Setup.exe`. It installs for the current Windows user under `%LOCALAPPDATA%/Programs/SekiModAssistantNative`, provides a separate uninstall entry, and does not replace the .NET version.
- The Home page checks Steam libraries and common game locations. If the game is not found, select `sekiro.exe` manually. After a valid game root directory is found and locked in, the application automatically creates an empty `mods` folder if one is missing. A clear error is displayed if creation fails. This step does not affect existing contents. Exit the game before installing mods.
- Three pages appear at the top: “Home / Online Presets / Quick Start.” Online Presets provides downloads and one-click installation for seven presets. Quick Start contains the full guide and a video tutorial button.

## v0.3 Interface and Upgrades

- Only the native C++ version is updated. Settings, download records, file fingerprints, and game backups from v0.2 are reused. The installation target and uninstall entry remain `SekiModAssistantNative`, allowing upgrades over an existing installation. Older release packages are preserved separately.
- At 100% display scaling, body text is 16 pixels, ordinary button text is 18 pixels, and navigation text is 22 pixels and bold. Per Monitor V2 follows system display scaling. Content can scroll on smaller screens.
- The three navigation buttons have rounded corners, dark ink-blue fills, white text, and a selected-page highlight. “Find and lock the game root directory” appears above the game directory field. The repeated application name has been removed from the content area.
- The entire background image is displayed with its aspect ratio preserved. Unused areas are black, and semitransparent dark panels sit behind content. A blade-shaped icon in multiple sizes is embedded in the main application, installer, and uninstaller. Shortcuts use the main executable's icon.
- The “Language Options” button opens a menu with Simplified Chinese, English, and Japanese. Changes take effect immediately and are saved.
- The custom window buttons use the two circular images supplied by the user, with minimize, maximize/restore, and close symbols overlaid. Hover tooltips are provided. Window dragging, double-clicking to maximize/restore, edge resizing, and system keyboard shortcuts are supported. The maximize button returns an `HTMAXBUTTON` hit-test result for Windows snapping. The Windows 11 hover Snap Layouts panel has not been tested on a separate physical machine, so it cannot be guaranteed to appear on every system version.
- The installer and ordinary dialogs use custom close buttons. Yes/No buttons are centered; file pickers retain the standard Windows appearance.

## Beginner's Quick Guide to Installing Mods Manually

### (1) Recommended approach for beginners

If this is your first time playing a gameplay overhaul mod, use the preset feature to download and install a gameplay preset package. To add different mods manually, refer to this video:

https://v.douyin.com/wmk_GI6325k/

### (2) Appearance mods

Find the `mods` folder in the game root directory, or create it if necessary → make sure both Mod Engine files, `dinput8.dll` and `modengine.ini`, are in the game root directory → create a `parts` folder inside `mods` → place your appearance mod files in `parts`.

If an appearance mod also includes other directories, follow that mod's own instructions.

### (3) Gameplay overhaul mods

Find the `mods` folder in the game root directory. The application creates it automatically if it is missing after finding and locking a valid game root directory; you can also create it manually → make sure both Mod Engine files are in the game root directory → place all your mod resource folders, such as `action`, `chr`, `param`, `script`, `map`, `parts`, and `menu`, inside `mods`.

### (4) Mixing mods

Avoid mixing mods arbitrarily. Doing so may cause bugs and unpredictable problems in the game.

### (5) Additional notes

By default, v0.3 imports local mods by adding files to the existing installation and overwriting files at matching paths, while keeping other files. Switching gameplay presets still replaces the entire mod installation after confirmation to avoid mixing different gameplay overhauls. Both operations create a backup first.

Online presets support automatic extraction, verification, and installation. Cloud-drive sharing pages may require you to sign in through the browser or enter an extraction/access code first. After downloading, click “One-click Install” and select the downloaded archive if necessary.

### What “Move Up / Move Down” means

These buttons change the processing order of the current import list. The application merges files from top to bottom. If two mods contain files at the same path, the file from the item lower in the list overwrites the one above it. If there are no matching paths, the order usually has no effect. This is not the game's runtime mod loading order.

## v0.3 Local Imports and Status

- Local imports now use **incremental merging**: new files are added, files at matching paths are overwritten, and other existing files are kept. The entire `mods` folder is no longer cleared first.
- Drop folders, resource directories, or archives such as ZIP/RAR/7Z/TAR/GZ/BZ2/XZ, then review the preview before installing. You can change the overwrite order within a batch. The preview also lists existing files that will be overwritten.
- The actual contents of the selected game's `mods` folder are checked approximately every three seconds. You can also refresh manually. Identifying a large preset for the first time requires hashing files and may take some time. Later checks reuse cached verification results for unchanged files.
- If there are no files, including when there are only empty directories, “No” is displayed in red. If files exist, “Yes” is displayed in green. A preset's name is shown when all its file paths, sizes, and SHA-256 hashes match; additional files, such as appearance mods, are allowed. A partial match displays “Preset incomplete or modified.” If there is no overlap with any preset's file paths, “Unknown mod” is displayed. Mod names are no longer inferred solely from old installation records.
- “Clear all installed mods” opens a Yes/No confirmation. Yes: create a backup first, then remove everything inside the selected game's `mods` folder while keeping the folder and Mod Engine files. No: return to Home without deleting anything. The operation can be undone with the Restore button.

## Online Presets: Edge Downloads and Automatic Extraction

This version provides seven presets: Yingwu (影武) 2.172, Dawn (破晓) V81-1.0, Resurrection (复兴) 1.16.1, Severance (不朽) 1.05.3, Guiying (诡影) V4.9, Jidao (极道) 2.3, and Weiliang (苇梁; version unspecified). Land of Reeds (芦苇之国) has been temporarily removed and is not treated as the same mod as Weiliang. Large preset data is not bundled with the application, and there is no preset-package folder selection feature.

1. On the second page, click “Download” for the desired preset. After confirmation, Edge opens the user-supplied direct GitHub Releases link. If Edge is unavailable, the application asks whether to use the default browser. Declining returns to the previous screen.
2. The browser handles the download. The application does not report “download successful” merely because it opened a link, and it cannot guarantee access to GitHub. Check in Edge that the download has finished.
3. The application first checks accessible download directories from Edge profiles and the Windows Downloads known folder. The actual save location may have been changed by the user or selected separately for each download. The interface displays candidate locations; it does not guarantee that downloads are on drive C:.
4. After the download finishes, click “One-click Install.” Manual extraction is unnecessary. The application looks for the matching filename, copies with `(1)` / `(2)` suffixes, and an extracted folder with the same name. If multiple candidates are found, you choose one. If none is found, you can select the archive manually. Temporary `.crdownload` files are not treated as complete packages. During extraction, the archive is held with a read lock and checked to ensure that it can be read completely.
5. After automatic extraction, the application locates the resource directory. If several candidates exist, you must choose one. Presets with built-in file fingerprints are verified file by file before the installation preview appears. Installing a gameplay preset still backs up and fully replaces `mods`; local imports continue to merge incrementally.
6. The newly added Severance, Guiying, Jidao, and Weiliang presets do not yet have independently generated file fingerprints because network approval failed in the build environment. Their first installation explicitly asks you to confirm that the files came from the specified link. It checks that the archive extracts completely and contains the expected resource directory structure, without claiming that the version's identity has been independently verified. After a successful installation, the installed files' actual fingerprints are recorded for future verification and status detection. Before these four presets have been installed through this tool, manually installed copies may be shown as unknown mods or as partial matches to another preset.
7. When offline, the Download buttons are disabled and the following message appears in red: “You are currently offline and cannot use online downloads.” Local imports, restoration, and installation of already downloaded presets remain available. A system network connection does not guarantee that GitHub is reachable.

The following seven user-supplied links are included in `preset-downloads.json`:

- [Yingwu (影武)](https://github.com/lizhi19930671951-code/Sekiro-Mods-Collection/releases/download/yingwu/v2.172.zip)
- [Dawn (破晓)](https://github.com/lizhi19930671951-code/Sekiro-Mods-Collection/releases/download/poxiao/V81-1.0.-9.20.zip)
- [Resurrection (复兴)](https://github.com/lizhi19930671951-code/Sekiro-Mods-Collection/releases/download/fuxing/1.16.1.zip)
- [Severance (不朽)](https://github.com/lizhi19930671951-code/Sekiro-Mods-Collection/releases/download/Severance/Severance.1.05.3.zip)
- [Guiying (诡影)](https://github.com/lizhi19930671951-code/Sekiro-Mods-Collection/releases/download/guiying/V4.9.zip)
- [Jidao (极道)](https://github.com/lizhi19930671951-code/Sekiro-Mods-Collection/releases/download/Jidao/2.3.zip)
- [Ftsoa (苇梁)](https://github.com/lizhi19930671951-code/Sekiro-Mods-Collection/releases/download/ftsoa/ftsoa.rar)

Actual network downloads and the four newly added real archives have not yet been verified in the build environment. Duplicate filenames, rejection of incomplete downloads, and local archive extraction have been verified in the native version. Actual Edge downloads and custom download paths on other computers still require testing on real machines. If a link stops working, update the configuration rather than disabling verification to force installation.

## Backups and Limitations

Backups are stored in `.sma-backup` under the game root directory. The most recent complete state is retained, including the previous Mod Engine files, `mods`, and the fact that any of these were originally absent. Before restoring, the current state is saved so that another restore can undo the operation. Consecutive operations advance the most recent backup; the original unmodified state is not kept forever. Interrupted operations trigger a recovery prompt on the next run. Uninstalling the application does not remove mods, Mod Engine files, or backups from the game directory.

Programs contained in mod packages are not executed. Paths escaping the target directory, absolute paths, and links are blocked. Mod Engine is supplied consistently through the accompanying files. The application does not modify, manage, or back up game saves. The instructions included with Dawn advise against using old saves.

The native version passed 55 self-tests covering local merging and rollback, status detection, clearing and restoring mods, interrupted transaction recovery, ZIP/7Z/TAR/GZIP/BZIP2/XZ, a stored-format RAR sample, password-protected and multipart archives, and rejection of out-of-bounds paths and symbolic links. Bidirectional backup restoration between the .NET and native versions, Chinese filenames, the interface in three languages, and small-window layouts have also been verified. The RAR sample does not represent every RAR version or password combination.

All seven links are configured, but actual online downloads remain unverified because of network approval restrictions in the build environment. The four newly added real preset packages have not been checked. The actual game has not been launched. High DPI, UAC, and registry-based installation/uninstallation have not been comprehensively tested on separate Windows 10/11 machines. Installer file deployment, shortcuts, and upgrade rollback were tested in isolated directories with simulated registry writes; this does not constitute verification of installation and uninstallation in a real user environment.

See `tools` and `licenses` for third-party licenses. Mod Engine and the presets remain the property of their original authors. Source code is provided separately.

## v0.3 Interface Improvements

- White window-control symbols were reduced by approximately 25% and moved down by three logical pixels in the main window, installer, and dialogs.
- At 100% scaling, body text is 16 pixels, ordinary button text is 18 pixels, and navigation text is 22 pixels and bold. The default target window size is 1360 × 900, limited to the monitor's work area. Home uses left and right sections on wide screens, and the seven online presets use a compact layout. Necessary scrolling remains on small screens, while Quick Start has its own internal scrolling.
- The language button was moved into the title bar. It is approximately 140 × 32 logical pixels, with 14-pixel orange-yellow text, using a red strip cut out of the user's image. In the main window it is to the left of Minimize; in the installer it is to the left of Close.
- The background stays fixed in window coordinates. Scrolling moves controls in batches without copying pixels from their old positions. Parent and child windows are redrawn together, with composited container drawing to reduce fragments and flicker.
- Chinese, English, and Japanese, at 100% and the current system scaling, have been checked. Live background pixel samples remained consistent after repeated scrolling. These checks do not replace testing on every graphics driver or remote desktop environment.

## v0.3 Compact Interface and Hover Improvements

At the current 100% scaling, body text is 16 pixels, ordinary button text is 18 pixels, navigation text is 22 pixels and bold, and language text is 13 pixels. The default target window size is 1200 × 800, limited to the monitor's work area. Controls and spacing have also been tightened by approximately 10%. The window-control images retain their original size. The white symbols are approximately eight pixels wide, with one-pixel strokes, and their centers have been moved down by a total of five logical pixels.

Title-bar flicker has been addressed: mouse hit tests outside buttons no longer trigger full-window redraws. Hover changes invalidate only the old and new button areas. Duplicate drawing during background erasure has been removed, and the title bar and owner-drawn buttons are rendered in memory before being submitted in one operation. Dragging, window commands, and resizing remain supported.

Buttons, navigation controls, the language image, and title-bar buttons brighten over approximately 120 milliseconds on hover and dim when the pointer leaves. Pressing provides separate feedback, and disabled buttons do not glow. The input list, directory field, and text fields brighten their borders on hover or focus.

Fifty-five file-operation tests, 40 interface checks, and a title-bar partial-redraw regression check have passed. Chinese, English, and Japanese layouts were rendered at 100% and current system scaling. UI diagnostics run in an isolated environment, and automated hover tests use fixed mouse-event timing. Behavior on actual graphics hardware and remote desktop still requires testing on real machines.

## v0.3 Status and Import Interface Update

Cherry-blossom and treasure-chest icons appear before the sky-blue game-root and local-import headings, respectively. Current mod status appears in a gray-black panel, with a same-size “↺ Refresh Status” control on the right. The import button reads “Check and import into mods,” with a hover tooltip explaining that the destination is the `mods` folder under the selected game root directory.

Ordinary confirmation, error, and import-preview dialogs use a solid dark gray background. The standalone installer retains its image background. The Clear button is burgundy. The bottom status line uses smaller text with a semitransparent gray background. Online preset names have been slightly enlarged, and the duplicate Quick Start heading has been removed. The main window hides its visible border while preserving resize hit testing. Border and snapping behavior across Windows versions still require testing on real machines.

Network and mod status update according to actual detection results. Network status reflects the connection reported by Windows and does not guarantee access to a particular download site. The first verification of a large mod may take time. After the root directory is locked in, a missing `mods` folder is created automatically, and creation failures are not silently ignored. Tooltips use support from the newer Windows Common Controls library.

## Interface Updates in This Round

The removal button now explicitly reads “Clear all installed mods.” The game-root and local-import headings use larger, bold sky-blue text and icons. Move Up and Move Down have been reduced in size. The import button uses green, and the Clear button remains burgundy. The three navigation buttons have been changed to ochre, with extra spacing between Chinese characters. Language Options uses bright red text.

The end of Quick Start now explains the local-import logic: new files are added to the existing `mods` folder; files at matching paths overwrite the old files; files at other paths remain. The entire `mods` folder is not automatically cleared.
