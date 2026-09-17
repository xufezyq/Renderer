# Render Comparison

This standalone CMake project compares the same delayed, per-pixel gradient in three MiniFB windows: main-thread rendering, one render worker, and a worker pool sized to the CPU's logical thread count. MiniFB window updates always run on the main thread. The main-thread renderer handles 16 pixels per UI iteration to keep all windows interactive. Close any window independently; the others continue.

Open `Comparison/CMakeLists.txt` as a CMake project in Visual Studio 2026, or generate a solution inside this directory:

```powershell
cmake -S Comparison -B Comparison/build -G "Visual Studio 18 2026" -DMINIFB_BUILD_EXAMPLES=OFF
cmake --build Comparison/build --config Debug --target RenderComparison
ctest --test-dir Comparison/build -C Debug --output-on-failure
```

Run `Comparison/build/Debug/RenderComparison.exe` to render at 120 x 90, or pass custom pixel dimensions, for example `RenderComparison.exe 160 120`. Each pixel takes 1 ms to calculate. Completed elapsed time appears in the window title and console. The three jobs run concurrently and compete for CPU time, so these elapsed times represent this live comparison, not isolated benchmarks.

On Windows, the three windows are arranged side by side in the secondary monitor's usable work area (excluding the taskbar), with margins and the image aspect ratio preserved. With only one monitor, they fall back to the primary monitor. The render pixel dimensions are independent of the displayed window size.
