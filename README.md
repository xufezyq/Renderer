# 《从零打造渲染器》空项目模板

Windows 需要安装 Visual Studio 2026 及其 C++ 和 CMake 组件；Linux/Mac 需要安装 CMake。

- Windows系统：
运行 **build.bat**，在 `build/FortuneRenderer.slnx` 打开 Visual Studio 2026 工程。脚本会使用 VS2026 自带的 CMake，重新生成旧版本 Visual Studio 的构建缓存。

- Linux/Mac系统：
运行**build.sh**，会自动构建和编译项目，然后在build目录下即可找到编译好的可执行文件。

## XML 场景

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
FortuneRenderer.exe dust2.xml
```

从 OBJ 转换出的模型场景可以这样运行：

```powershell
FortuneRenderer.exe dust2_model.xml
```

场景使用当前渲染器支持的三角形和球体搭建了 T 出生点、Mid 双门、A Long、A Short、B Tunnel 以及 A/B 点位。它是用于验证 XML 场景组织和射线求交的 blockout，不包含材质、纹理、光照或完整的游戏地图细节。

Dust2 的 `<camera>` 元素设置了高空全景视角；可以调整 `position`、`forward` 和 `fov` 改变观察角度。没有 `<camera>` 的场景使用默认的原点摄像机。

OBJ 转换工具位于 `tools/Convert-ObjToScene.ps1`，它会把 OBJ 面自动三角化并生成 XML；当前版本只导入几何，不导入材质和纹理。

当前提供的 OBJ 模型是左右镜像的，转换时使用 `-MirrorX` 修正了 X 轴并同步修正三角形绕序。
