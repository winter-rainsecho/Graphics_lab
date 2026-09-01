# Assignment3 Bonus 1 — 尝试更多模型

## 完成情况
完成 Bonus 1（3 分）：已尝试 4 个额外模型，全部位于 `Code/models/` 并包含 Vertex Normal 信息，渲染结果保存在 `images/`。

## 模型清单
| 模型 | OBJ 文件 | 顶点法线来源 |
|---|---|---|
| cube | `Code/models/cube/cube.obj` | 原文件自带 |
| rock | `Code/models/rock/rock.obj` | 原文件自带 |
| bunny | `Code/models/bunny/bunny.obj` | 通过面法线加权生成并写入 |
| Crate | `Code/models/Crate/Crate1.obj` | 通过面法线加权生成并写入 |

## 渲染结果
| 模型 | normal 着色 | phong 着色 |
|---|---|---|
| cube | `images/cube_normal.png` | `images/cube_phong.png` |
| rock | `images/rock_normal.png` | `images/rock_phong.png` |
| bunny | `images/bunny_normal.png` | `images/bunny_phong.png` |
| Crate | `images/Crate_normal.png` | `images/Crate_phong.png` |

## 主要代码修改
- `Code/main.cpp`：
  - 支持命令行参数 `argv[3]` 选择模型，例如 `./Rasterizer out.png normal cube`。
  - 新增 `get_model_matrix_fit(center, scale, angle)`，根据模型包围盒自动居中并缩放到合适尺寸，使不同尺度模型都能正确显示。
  - 为不同模型配置对应 `.obj` 与纹理文件；保留 `spot` 的原有行为（height map 仍为 `hmap.jpg`，texture shader 仍为 `spot_texture.png`）。
- `Code/models/bunny/bunny.obj`、`Code/models/Crate/Crate1.obj`：
  - 增加 `vn` 顶点法线数据，并改写面索引为 `v//vn` / `v/vt/vn`，满足 Bonus1 对 Vertex Normal 的要求。

## 运行方式
在 `Code/build` 目录下：
```bash
./Rasterizer ../../images/<model>_<shader>.png <shader> <model>
```
其中 `<shader>` 可选 `normal` / `phong`（Bonus1 主要使用这两种），`<model>` 可选 `cube` / `rock` / `bunny` / `Crate` / `spot`。

如果直接执行提示缺少 DLL，请确保 MinGW 与 OpenCV bin 目录在 PATH 中：
```bash
export PATH="/e/Trae_for_html/Graphics_lab/dependencies/opencv-4.12.0/opencv_install/x64/mingw/bin:/c/Program Files/mingw64/bin:$PATH"
```

## 其它得分点
除 Bonus1 外，作业核心功能（参数插值、Blinn-Phong、Texture Mapping、Bump Mapping、Displacement Mapping）已实现于 `Code/main.cpp`。
