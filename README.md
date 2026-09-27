# 《从零打造渲染器》空项目模板

Windows 需要安装 Visual Studio 2026 及其 C++ 和 CMake 组件；Linux/Mac 需要安装 CMake。

- Windows系统：
运行 **build.bat**，在 `build/FortuneRenderer.slnx` 打开 Visual Studio 2026 工程。脚本会使用 VS2026 自带的 CMake，重新生成旧版本 Visual Studio 的构建缓存。

- Linux/Mac系统：
运行**build.sh**，会自动构建和编译项目，然后在build目录下即可找到编译好的可执行文件。

## XML 场景

仓库中的场景 XML 统一放在 `scenes/` 目录。运行时从构建目录传入 `scenes/文件名.xml`。

场景按相机、光源、物体分组。下面是点光源和球体的示例：

```xml
<scene version="2">
    <camera position="0 0 0" forward="0 0 1" fov="60" />
    <lights>
        <point
            position="0 1.5 4.1"
            intensity="2 2 2"
            constant="1"
            linear="0.1"
            quadratic="0.05" />
    </lights>
    <objects>
        <object position="0 0 3">
            <sphere radius="0.7" />
        </object>
    </objects>
</scene>
```

`<lights>` 还支持 `<directional direction="..." radiance="..." />` 和 `<spot>`；聚光灯使用 `direction`、`position`、`intensity`、`inner_angle`、`outer_angle` 以及三个具名衰减系数。`constant`、`linear`、`quadratic` 分别是距离衰减公式中的常数项、距离项、距离平方项。物体的 `position`、`euler`、`scale` 可省略，默认值分别为 `0 0 0`、`0 0 0`、`1`。旧版直接写在 `<scene>` 下的物体、光源和相机属性仍可加载。

渲染器从交点向光源发出阴影射线，跳过被几何体遮挡的光源，再将其余光源的衰减后颜色乘以入射角余弦并相加。显示时高于 1 的颜色通道会被截断。

仓库还提供了一个简化的 Dust2 风格场景，构建后可以直接加载：

```powershell
FortuneRenderer.exe scenes/dust2.xml
```

从 OBJ 转换出的模型场景可以这样运行：

```powershell
FortuneRenderer.exe scenes/dust2_model.xml
```

## 光源截图对比

`scenes/` 提供两组独立的 XML 场景。每组四个文件共用相机与几何体，`all` 版使用另外三个单灯版的同一组光源参数。构建后这些文件会复制到可执行文件目录的 `scenes/` 子目录；运行时传入相对路径即可切换场景，并保持窗口尺寸一致截图。

| 场景 | 全部光源 | 仅平行光 | 仅点光源 | 仅聚光灯 |
| --- | --- | --- | --- | --- |
| Cornell box | `scenes/cornell_box_all.xml` | `scenes/cornell_box_directional.xml` | `scenes/cornell_box_point.xml` | `scenes/cornell_box_spot.xml` |
| Dust2 模型 | `scenes/dust2_model_all.xml` | `scenes/dust2_model_directional.xml` | `scenes/dust2_model_point.xml` | `scenes/dust2_model_spot.xml` |

例如：`FortuneRenderer.exe scenes/dust2_model_spot.xml`。平行光使用偏暖的颜色，点光源偏冷，聚光灯偏暖，便于在合并图中辨认各自的照明区域。Cornell 的点光位于左侧球体前方，聚光灯从右上方照向右侧球体与地面；Dust2 的点光在相机前方形成局部亮区，聚光灯照向道路中央。每组的 `all` 版与三个单灯版使用完全相同的对应光源参数，相机和几何体也一致；`scenes/cornell_box.xml` 与 `scenes/dust2_model.xml` 的光源配置分别与各自的 `all` 版一致。

场景使用当前渲染器支持的三角形和球体搭建了 T 出生点、Mid 双门、A Long、A Short、B Tunnel 以及 A/B 点位。它是用于验证 XML 场景组织和射线求交的 blockout，不包含材质、纹理、光照或完整的游戏地图细节。

Dust2 的 `<camera>` 元素设置了高空全景视角；可以调整 `position`、`forward` 和 `fov` 改变观察角度。没有 `<camera>` 的场景使用默认的原点摄像机。

OBJ 转换工具位于 `tools/Convert-ObjToScene.ps1`，它会把 OBJ 面自动三角化并生成 XML；当前版本只导入几何，不导入材质和纹理。

当前提供的 OBJ 模型是左右镜像的，转换时使用 `-MirrorX` 修正了 X 轴并同步修正三角形绕序。
