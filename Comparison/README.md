# Render Comparison

This standalone CMake project compares the same delayed, per-pixel gradient in three MiniFB windows: main-thread rendering, one render worker, and a worker pool sized to the CPU's logical thread count. MiniFB window updates always run on the main thread. The main-thread renderer handles 16 pixels per UI iteration to keep all windows interactive. Close any window independently; the others continue.

On another computer with Visual Studio 2026 and the Desktop development with C++ workload, open `Comparison/RenderComparison.slnx` directly and build `Debug|x64` or `Release|x64`. The checked-in `.vcxproj` uses relative paths to the vendored MiniFB sources and does not require CMake. Only generated outputs under `vs-out/` and `vs-obj/` are excluded from Git.

Alternatively, open `Comparison/CMakeLists.txt` as a CMake project and select the `vs2026-x64` preset. CMake creates local machine-specific output under `build-vs2026/`.

From a terminal in this repository's `Renderer` directory, generate and build with:

```powershell
cmake --preset vs2026-x64 -S Comparison
cmake --build Comparison/build-vs2026 --config Debug --target RenderComparison RenderJobTests WindowLayoutTests
ctest --test-dir Comparison/build-vs2026 -C Debug --output-on-failure
```

Run `Comparison/build-vs2026/Debug/RenderComparison.exe` to render at 120 x 90, or pass custom pixel dimensions, for example `RenderComparison.exe 160 120`. Each pixel takes 1 ms to calculate. Completed elapsed time appears in the window title and console. The three jobs run concurrently and compete for CPU time, so these elapsed times represent this live comparison, not isolated benchmarks.

On Windows, the three windows are arranged side by side in the secondary monitor's usable work area (excluding the taskbar), with margins and the image aspect ratio preserved. With only one monitor, they fall back to the primary monitor. The render pixel dimensions are independent of the displayed window size.
