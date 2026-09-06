# Assignment3 自测问卷（6 核心任务 + 2 Bonus）

> 用法：遮住参考答案自问自答。能答出每组的第 1 题＝概念懂；能写出第 2 题的公式 / 命令行＝能复现实验。
> 对应代码：`Code/main.cpp`、`Code/Texture.hpp`、`Code/rasterizer.cpp`、`Code/OBJ_Loader.h`。

## A. 参数插值（Task 1）
1. 屏幕空间里 color / normal / texcoord / position 用什么坐标插值？为什么不能直接对顶点 3D 坐标线性插值？
2. 写出透视校正深度 `zp` 的插值公式（见 `rasterizer.cpp`）。

## B. 法线可视化（Task 2）
3. `normal_fragment_shader` 把法线映射成颜色时做了什么变换？它计算光照吗？

## C. Blinn-Phong（Task 3）
4. 本作业用的是经典 Phong 还是 Blinn-Phong？二者高光项有什么区别？
5. 漫反射 / 高光项里为什么要除以 `r²`（到光源距离平方）？本作业高光指数 `p` 是多少？

## D. 纹理映射（Task 4）
6. `texture_fragment_shader` 的漫反射系数 `kd` 来自哪里？`getColor` 用的是最近邻、双线性还是双三次？
7. `getColor(u,v)` 里 `v` 为什么要做 `(1-v)` 翻转？纹理在加载时颜色空间做了什么转换？

## E. Bump Mapping（Task 5）
8. `bump_fragment_shader` 只改了法线还是也移动了顶点？最后输出的是光照结果还是法线颜色？
9. TBN 矩阵怎么构造？当法线接近 Y 轴时，切线 `t` 如何避免除零？

## F. Displacement（Task 6）
10. displacement 与 bump 前 3 步相同，关键区别在哪两步？
11. 高度梯度用 `kh*kn` 计算，几何位移量用的是 `kn*h_uv` 还是 `kh*kn*h_uv`？

## G. Bonus1 更多模型
12. 给 bunny / Crate 补 `vn` 时，顶点法线是算出来的还是凭空生成几何？
13. 不同模型为什么要按包围盒自动居中并缩放（`get_model_matrix_fit`）？

## H. Bonus2 双线性纹理插值（Task 7）
14. `getColorBilinear` 的插值步骤是什么？它与 `getColor` 的坐标映射是否一致？
15. 把 32×32 纹理放大到约 700×700 画面时，双线性为什么显“糊”？最近邻算抗锯齿吗？
16. 渲染 spot 的双线性对比图，命令行怎么写？纹理多小效果最明显？
