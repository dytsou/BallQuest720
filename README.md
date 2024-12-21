# Ball Catcher Game

一個使用 OpenGL 開發的第一人稱接球遊戲。玩家需要在 3D 空間中移動並接住從天上落下的不同顏色的球。

## 遊戲特點

### 球體類型
- 紅色球：1分，體型大
- 綠色球：3分，體型適中
- 藍色球：5分，體型小
- 彩虹球：10分，體型最小
- 黑色球：-10分，體型特大，且會減少生命值

### 遊戲規則
- 初始生命值為 20
- 每 10 分會增加一個球的生成
- 當生命值歸零時遊戲結束
- 黑色球會減少生命值
- 漏接過多球也會導致遊戲結束

### 控制方式
- WASD：移動控制
  - W：向前移動
  - S：向後移動
  - A：向左移動
  - D：向右移動
- 滑鼠：視角控制
- 空白鍵：加速移動
- Q：退出遊戲
- Z：提前結算分數

## 系統需求

### 依賴項
- OpenGL
- GLUT
- C++ 編譯器（支援 C++17）
- CMake (>= 3.10)

### 安裝依賴項（Ubuntu）
```bash
sudo apt-get update
sudo apt-get install -y freeglut3-dev
```

## 編譯和運行

1. 創建構建目錄：
```bash
mkdir build
cd build
```

2. 配置專案：
```bash
cmake ..
```

3. 編譯：
```bash
make
```

4. 運行：
```bash
./BallCatcherGame
```

## 開發說明

### 專案結構
- `src/`: 源代碼文件
  - `main.cpp`: 主程式
  - `Camera.cpp`: 相機控制
  - `Vector3.cpp`: 3D 向量運算
  - `Texture.cpp`: 紋理加載
  - `Text.cpp`: 文字渲染
  - `Fruit.cpp`: 球體物件
- `include/`: 頭文件
  - `Camera.h`: 相機類定義
  - `Vector3.h`: 向量類定義
  - `Texture.h`: 紋理類定義
  - `Text.h`: 文字類定義
  - `Fruit.h`: 球體類定義

### 技術特點
- 使用 OpenGL 固定管線渲染
- 3D 碰撞檢測
- 紋理映射
- 第一人稱視角控制
- 基於時間的物理更新 