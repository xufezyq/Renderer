# 《从零打造渲染器》空项目模板

先要保证电脑上已安装了cmake

- Windows系统：
直接双击**build.bat**，然后在生成的build目录下，找到VS工程文件，双击用VS打开即可

- Linux/Mac系统：
运行**build.sh**，会自动构建和编译项目，然后在build目录下即可找到编译好的可执行文件。

## XML 场景

仓库还提供了一个简化的 Dust2 风格场景，构建后可以直接加载：

```powershell
FortuneRenderer.exe dust2.xml
```

从 OBJ 转换出的模型场景可以这样运行：

```powershell
FortuneRenderer.exe dust2_model.xml
```

场景使用当前渲染器支持的三角形和球体搭建了 T 出生点、Mid 双门、A Long、A Short、B Tunnel 以及 A/B 点位。它是用于验证 XML 场景组织和射线求交的 blockout，不包含材质、纹理、光照或完整的游戏地图细节。

Dust2 的 `<scene>` 根节点还声明了高空全景摄像机；可以直接调整 `camera_position`、`camera_forward` 和 `camera_fov` 来改变观察角度。没有这些属性的旧场景仍使用默认的原点摄像机。

OBJ 转换工具位于 `tools/Convert-ObjToScene.ps1`，它会把 OBJ 面自动三角化并生成 XML；当前版本只导入几何，不导入材质和纹理。

当前提供的 OBJ 模型是左右镜像的，转换时使用 `-MirrorX` 修正了 X 轴并同步修正三角形绕序。
