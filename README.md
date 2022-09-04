# 蓝电 YYDS Car

---

![-](images/pic-index.jpg)

## Edgeboard 程序

Edgeboard 完成摄像头的识别、下发控制命令。

- [x] 基础元素识别、打印

  - [x] 去畸变和透视变换
  - [x] 基于角点识别赛道元素
  - [x] 单线偏移的目标轨迹规划

- [x] 标志识别、打印
- [x] JSON 配置项读取
- [x] 图像、边线、锚点显示
- [x] 日志输出命令行、文件

![-](images/pic-eb-yyds-1.png)

![-](images/pic-eb-yyds-2.png)

## TC212 控制端调参上位机

为了能够观察各个变量的给定和输出波形，以更方便的进行调试分析，我队使用C#开发了一款上位机。

- [x] 波形显示
- [x] 命令行内交互
- [x] 遥控车辆运动
- [x] PID参数调试、读写

![-](images/pic-upper.png)

