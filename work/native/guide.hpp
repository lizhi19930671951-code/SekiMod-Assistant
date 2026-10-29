#pragma once
#include "strings.hpp"
namespace sma { inline std::wstring guide(){switch(language){
case 1:return wide(R"GUIDE(Quick guide to installing mods manually

1. First-time gameplay mod players are encouraged to download and install a gameplay preset using Online presets. To add mods manually, watch:
https://v.douyin.com/wmk_GI6325k/

2. Appearance mods
Find mods in the game root (create it if missing) → Make sure dinput8.dll and modengine.ini are both in the game root → Create parts inside mods → Put your appearance files in parts.
If the mod includes other folders, follow its own instructions.

3. Gameplay mods
Find mods in the game root (the assistant creates it after locking a valid game folder; you can also create it manually) → Make sure both engine files are in the game root → Place all resource folders, such as action, chr, param, script, map, parts and menu, inside mods.

4. Avoid mixing arbitrary mods. Incompatible combinations can cause bugs and unexpected problems in the game.

5. Other notes
Local import in v0.3 adds files and overwrites identical paths while preserving other files. Switching gameplay presets replaces all mods after confirmation to avoid mixing different overhauls. Both operations back up first.

Online presets can be extracted, checked and installed automatically. Cloud sharing pages may require a browser login or extraction code. After downloading, use One-click install and select the downloaded archive if needed.

Move up / Move down
These buttons change the processing order of this import list. Files are merged from top to bottom. If two mods contain files at the same path, the lower item overwrites the upper item. With no overlapping paths, order usually makes no difference. This is not the game's runtime load order.

Local import details
Local import adds new files to the existing mods folder. A new file at the same path replaces the older file; files at other paths remain. Importing one mod does not clear the entire mods folder.)GUIDE");
case 2:return wide(R"GUIDE(初心者向け MOD 手動導入ガイド

（1）ゲームプレイ MOD を初めて使う方には、オンラインプリセットからの取得・導入をお勧めします。手動で追加する場合は動画をご覧ください。
https://v.douyin.com/wmk_GI6325k/

（2）外見 MOD
ゲームのルート内の mods を確認（なければ作成）→ dinput8.dll と modengine.ini がゲームのルートにあることを確認 → mods 内に parts を作成 → 外見ファイルを parts 内へ入れます。
他のフォルダーも含む場合は、その MOD の説明を優先してください。

（3）ゲームプレイ MOD
ゲームのルート内の mods を確認（有効なゲームフォルダーを固定すると助手が自動作成。手動作成も可能）→ エンジン2ファイルを確認 → action、chr、param、script、map、parts、menu などのリソースフォルダーを mods 内へ入れます。

（4）MOD をむやみに混用しないでください。競合によりバグや予期しない問題が起こる場合があります。

（5）その他
v0.3 のローカル導入は既存ファイルを保持し、追加と同パスの上書きを行います。ゲームプレイのプリセット切り替えは確認後に mods 全体を置換し、混用を避けます。いずれも先にバックアップします。

オンラインプリセットは自動展開・検証・導入に対応します。共有ページではブラウザーでのログインやコードが必要な場合があります。取得後に「ワンクリック導入」を押し、必要なら取得済みの圧縮ファイルを選択してください。

「上へ／下へ」の説明
今回の導入リストの処理順を変更します。上から下へ結合し、同じパスのファイルがある場合は下の項目が上の項目を上書きします。重複がなければ、通常は順序による違いはありません。ゲーム実行時の読み込み順ではありません。

ローカル導入の補足
既存の mods に新しいファイルを追加し、同じパスのファイルは新しい方で上書きします。別のパスにある既存ファイルは残ります。1つの MOD を導入しても mods 全体は削除されません。)GUIDE");
default:return wide(R"GUIDE(小白手动安装mod快速指南：

（1）建议第一次玩流程类模组的新手小白用户使用预设功能下载并安装流程mod预设包。如果玩家想手工添加各种mod，请参考链接视频：
https://v.douyin.com/wmk_GI6325k/

（2）外观类mod
在游戏根目录中找到mods文件夹（若无可自己建立一个）→ 确保两个游戏引擎文件 dinput8.dll 和 modengine.ini 都在游戏根目录内 → 在mods文件夹中建立parts文件夹 → 在其中导入您的各种外观文件即可。
若外观模组还附带其他目录，请以该模组自身说明为准。

（3）流程类mod
在游戏根目录中找到mods文件夹（找到并锁定有效游戏根目录后，软件会自动建立缺失的mods文件夹；也可自行建立）→ 确保两个游戏引擎文件都在游戏根目录内 → 将您的所有mod资源文件夹（如action、chr、param、script、map、parts、menu等）导入到mods文件夹中即可。

（4）建议不要随意将某些mod一起混用，否则会有bug，导致游戏内不可预知的问题。

（5）其他说明
v0.3本地导入默认在已有mod基础上补充文件、覆盖同路径文件，其他文件保留；切换流程预设仍会确认后整体替换，避免不同流程模组混用。两种操作都会先备份。

在线预设支持自动解压、校验并安装。网盘分享页可能需要先在浏览器登录或输入提取码，下载后点击“一键安装”，必要时选择已下载的压缩包。

上移／下移说明
它们调整本次导入列表的处理顺序。软件从上到下合并文件；如果两个模组包含相同路径的文件，排在下面的文件会覆盖上面的文件；没有相同路径时，顺序通常不影响结果。它不是游戏运行时的加载顺序。

本地模组导入说明
导入新模组时，软件会在现有 mods 内容基础上补充新文件；如果新文件与旧文件路径相同，则覆盖旧文件；路径不同的旧文件会继续保留。导入一个新模组不会清空整个 mods 文件夹。)GUIDE");
} } }