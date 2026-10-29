# SekiMod Assistant v0.3 — 原生版源码

这是本次交付的 Windows C++17 / Win32 原生版源码快照，包含启动请求管理员权限、大路径列表拖入及最新安装器实现。只含原生版，不含旧 .NET 工程或大型预设模组包。

## 目录

- `work/native/`：主程序、安装器、头文件、资源脚本、manifest、构建和诊断脚本。
- `work/native/art/`：已经处理好的背景、按钮和多尺寸图标，正常构建不需要原始图片。
- `outputs/SekiroModAssistant/`：构建使用的运行资源，包括 Mod Engine、预设配置、指纹目录、7-Zip 工具和第三方许可证。该目录保留历史名称以兼容构建脚本，并不包含 .NET 程序。
- `SOURCE-SHA256.txt`：上述源码及资源文件的 SHA-256 清单。

原工程中未参与构建的 `*.tmp` 历史片段已排除。`work/native/README.md` 使用当前 `APP-README.md` 的内容。

## 编译

在 Windows 10/11 x64 上完整解压本源码包。推荐使用 PowerShell 7 和 LLVM-MinGW 的 **x86_64** 编译器。在解压目录打开 PowerShell，将以下编译器路径替换为你电脑上的实际位置：

```powershell
$compiler = 'D:\Tools\llvm-mingw-20261020-msvcrt-i686\bin\x86_64-w64-mingw32-clang++.exe'
& .\work\native\build-native.ps1 -Compiler $compiler
```

需要工具链内的 `x86_64-w64-mingw32-windres.exe`。即使工具链文件夹名称含 i686，也必须选择上面的 x86_64 编译器，不能直接使用默认的 i686 `clang++.exe`。

编译输出在 `outputs/SekiModAssistant-Native-v0.3/`，包括主程序、安装器 stub 和运行资源。构建脚本也支持在已配置的 x64 Visual Studio 编译环境中使用 `cl.exe` 和 `rc.exe`。

## 验证和生成发行包

主程序的 `app.manifest` 请求 `requireAdministrator`，启动时需接受 Windows UAC。安装器/卸载器单独使用 `setup.manifest` 的 `asInvoker`，保留按当前用户安装的行为。

```powershell
$test = Start-Process '.\outputs\SekiModAssistant-Native-v0.3\SekiModAssistant.exe' -ArgumentList '--self-test' -PassThru -Wait
if ($test.ExitCode -ne 0) { throw 'Self-test failed' }
& .\work\native\package-native.ps1 -Output '.\outputs\my-release'
```

`my-release` 应使用尚未生成过便携 ZIP 的目录，避免旧文件混入。打包脚本会运行自测，并检查安装器附带 ZIP 的完整性。生成的文件为：

- `SekiModAssistant-v0.3-Native-portable.zip`
- `SekiModAssistant-v0.3-Native-Setup.exe`

仅为隔离诊断，可执行：

```powershell
& .\work\native\qa-elevation-drop.ps1 -Compiler $compiler
```

该脚本用同一份源码生成普通权限诊断程序，并核对它与正式程序除资源段外的 PE 区段一致。诊断程序位于 `work/v03-elevation-drop-diagnostics/`，不能当成正式管理员版本发布。它验证自测、列表拖入和界面，不验证真实 UAC 启动或普通资源管理器向管理员窗口拖放。

`qa-ui.ps1`、`qa-setup.ps1` 等脚本使用隔离游戏夹具，不会启动真实游戏。`qa-native.ps1` 中的旧版跨平台备份测试还依赖原 .NET 发行程序与对应运行时，本源码包不包含这些依赖。`prepare-art.ps1` 和 `prepare-language-art.ps1` 为历史素材处理脚本，原图路径需要自行调整；现成素材已随源码提供，无需重新执行。

## 本次验证范围

同源码普通权限诊断构建通过 55 项文件操作自测，以及两个显示缩放下各 49 项界面检查；其中包括中文多路径拖入、去重、忙碌时拒绝添加、重建列表后继续拖入，以及拖入不修改游戏文件。正式程序已检查嵌入管理员 manifest。安装器的隔离安装、升级、失败回滚和最终载荷一致性已检查。

自动审批服务不可用，真实管理员启动及资源管理器向管理员窗口拖放尚未实机验证。不要把普通权限诊断结果当作这两项已通过。

## 第三方内容

Mod Engine、7-Zip、工具链相关许可与第三方说明保留在附带的 `licenses/` 和 `tools/` 目录。界面图片为本项目使用的用户提供素材。第三方组件及模组的权利属于各自作者；本次源码交付不额外授予这些内容的许可。

安装器自定义路径：浏览选中的目录作为父目录，最终安装到其下的 SekiMod Assistant 子目录；默认路径保留兼容。安装回执记录实际目录供卸载使用。隔离测试覆盖自定义路径、快捷方式、升级回滚及父目录文件保护。
