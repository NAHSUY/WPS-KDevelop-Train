此目录存放本周课后作业，可以在此文件添加作业题目、设计思路和流程图等

# 拼音词典查询器（week02/homework）

## 1. 项目目标

本项目围绕如下题目实现与优化：

- 输入：百万级拼音词典（CSV），格式为 `pinyin,word`
- 查询：给定拼音字符串，输出对应汉字词条到 CSV
- 要求：
  1. 同时考虑查询性能与内存占用
  2. 统计查询耗时与进程内存
  3. 内存目标不超过 **5MB**
- 附加要求：
  1. 不使用 `map/unordered_map` 作为查询主结构
  2. 引入 Google LevelDB 做“直接查询”性能基准对比
  3. 使用 `txt` 测试集（100~1000 条查询）统计平均性能
  4. 项目放在 `week02/homework`

---

## 2. 方案演进过程（踩坑与修复）

### 阶段 A：初版索引 + 内存二分（失败）
- 实现思路：将索引文件 `index.bin` 全量加载到内存 vector，再用 `lower_bound` 查找区间。
- 问题：
  - 内存占用明显偏高（远超 5MB）
  - 出现 `Not found`，后续定位到索引顺序问题（依赖全局有序）

### 阶段 B：`unordered_map` 兜底（功能通过，内存失败）
- 为了快速修复“找不到”，改为 `unordered_map<string, Range>` 查索引。
- 结果：
  - 功能正确，`ni'cai` 可命中 5 条
  - 但内存暴增（70MB 级），不满足题目约束

### 阶段 C：固定长度索引 + 磁盘二分（最终方案）
- 将索引改为定长记录（方便随机访问二分）：
  - `key[16] + start_offset + end_offset`
- 查询时不加载全量索引，不用 map：
  - 直接在 `index_fixed.bin` 上二分定位区间
  - 再到 CSV 对应偏移范围读取并过滤输出
- 优势：
  - 常驻内存低
  - 查询性能稳定
  - 满足“不用 map”要求

---

## 3. 最终实现说明

### 3.1 构建索引（离线）
程序：`build_index_fixed`

输入：
- `dict.csv`

输出：
- `index_fixed.bin`

过程：
1. 扫描 CSV，记录每个拼音词条在文件中的起止偏移
2. 对 key 排序
3. 合并重复 key 区间
4. 写入定长二进制索引文件

### 3.2 在线查询
程序：`query_fixed`

输入：
- `dict.csv`
- `index_fixed.bin`
- `pinyin`
- `out.csv`

过程：
1. 在 `index_fixed.bin` 上二分查找 `pinyin`
2. 命中后 `seek` 到 CSV 的 `[start, end]` 范围读取
3. 输出匹配行到 `out.csv`
4. 统计耗时和内存

---

## 4. 关键调试记录

1. **CMake 配置失败**
   - 原因：上层工程 `add_subdirectory` 指向目录缺失 `CMakeLists.txt` 或重复 target
   - 处理：修正 week02 子目录组织，避免重复 target 名

2. **`Failed to load index`**
   - 现象：`invalid len=0 at rec ...`
   - 原因：索引写入阶段存在异常行/空 key 导致记录不合法
   - 处理：构建索引时增加 trim、空 key 过滤、写入校验

3. **`key order broken`**
   - 现象：`prev=a'i cur=a`
   - 原因：源数据并非严格有序；`lower_bound` 依赖有序被破坏
   - 处理：构建固定索引后显式排序，再进行查询二分

---

## 5. 当前测试结果（已达标）

示例查询：`ni'cai`

输出：
- `Found 5 rows for: ni'cai`
- `Output: data\out.csv`

性能：
- Query time: `2352 us`
- WorkingSet: `4.75 MB`
- PrivateUsage: `0.617188 MB`
- Delta WorkingSet: `0.1875 MB`
- Delta PrivateUsage: `0.0117188 MB`

结论：
- 功能正确
- 内存低于 5MB
- 满足当前主要求

---

## 6. 构建与运行

> 以下为示例命令（Windows + CMake + VS2022）

### 6.1 编译
```bash
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

### 6.2 构建索引
```bash
build\bin\Release\build_index_fixed.exe data\dict.csv data\index_fixed.bin
```

### 6.3 查询
```bash
build\bin\Release\query_fixed.exe data\dict.csv data\index_fixed.bin ni'cai data\out.csv
```

---

## 7. 附加要求完成计划（两周）

### Week 1
- 接入 Google LevelDB（拉取、编译、链接）
- 实现 `leveldb_build`（CSV -> DB）
- 准备测试集 `test_queries_100.txt`

### Week 2
- 实现 `leveldb_query` 与批量 `benchmark`
- 增加 `test_queries_1000.txt`
- 对比两方案：
  - 固定索引二分方案
  - LevelDB 直接查询方案
- 输出平均耗时 / P50 / P95 / 内存对比报告

---

## 8. 后续优化方向

1. 索引 key 长度自适应压缩（进一步减小索引文件）
2. 查询结果输出批量缓冲写，提高吞吐
3. 增加多线程批量查询基准
4. 增加异常输入与编码一致性校验（UTF-8）

---

## 9. 文件清单（当前核心）

- `build_index_fixed.cpp`：离线索引构建
- `query_fixed.cpp`：在线查询 + 性能统计
- `data/dict.csv`：词典数据
- `data/index_fixed.bin`：索引文件
- `data/out.csv`：查询输出



# 拼音查询系统（LevelDB 优化版）

## 1. 项目简介

本项目实现了一个拼音查询系统，并在原始 CSV 顺序扫描方案基础上，引入 **LevelDB** 进行键值索引优化，实现更快的查询性能。

- `build_index/query`：原始版本（可保留作对照）
- `leveldb_build/leveldb_query/benchmark`：LevelDB 优化版本
- `make_queries`：自动生成 100/1000 条查询测试集

---

## 2. 项目结构

```text
pinyin_query_project/
├─ CMakeLists.txt
├─ build_index.cpp
├─ query.cpp
├─ leveldb_build.cpp
├─ leveldb_query.cpp
├─ benchmark.cpp
├─ make_queries.cpp
├─ data/
│  ├─ dict.csv
│  ├─ ldb/                    # LevelDB 数据目录（运行后生成）
│  ├─ test_queries_100.txt    # 100条查询集（运行后生成）
│  └─ test_queries_1000.txt   # 1000条查询集（运行后生成）
└─ result/
   ├─ out_leveldb.csv
   ├─ benchmark_100.csv
   └─ benchmark_1000.csv
```

---

## 3. 环境说明

- OS: Windows 10/11
- 编译器: Visual Studio 2022 (MSVC)
- 构建工具: CMake >= 3.10
- 第三方库: LevelDB（本地编译）

---

## 4. LevelDB 编译（一次性）

> 已在本机路径下完成：`D:/C++_project/thirdparty_install/leveldb`

### 4.1 配置并编译 LevelDB

```bat
cd /d D:\C++_project\thirdparty_install\leveldb
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 -DLEVELDB_BUILD_TESTS=OFF -DLEVELDB_BUILD_BENCHMARKS=OFF
cmake --build build --config Release
```

生成库文件示例：
- `D:\C++_project\thirdparty_install\leveldb\build\Release\leveldb.lib`

---

## 5. 项目编译

在项目 CMake 中设置 `LEVELDB_ROOT`（或通过命令行传入）：

```cmake
set(LEVELDB_ROOT "D:/C++_project/thirdparty_install/leveldb" CACHE PATH "Path to leveldb root")
include_directories(${LEVELDB_ROOT}/include)

find_library(LEVELDB_LIB
  NAMES leveldb libleveldb
  PATHS
    ${LEVELDB_ROOT}
    ${LEVELDB_ROOT}/build
    ${LEVELDB_ROOT}/build/Release
    ${LEVELDB_ROOT}/out/Release
  NO_DEFAULT_PATH
)
```

然后构建项目：

```bat
cd /d D:\C++_project\KDevelop-Train
cmake -S . -B ../build -G "Visual Studio 17 2022" -A x64 -DLEVELDB_ROOT=D:/C++_project/thirdparty_install/leveldb
cmake --build ../build --config Release
```

---

## 6. 运行步骤（LevelDB版本）

以下命令基于本机实际可执行文件路径：
`D:\C++_project\build\bin\Release`

### 6.1 从 CSV 构建 LevelDB

```bat
D:\C++_project\build\bin\Release\leveldb_build.exe D:\C++_project\build\LessonCode\week02\homework\pinyin_query_project\data\dict.csv D:\C++_project\build\LessonCode\week02\homework\pinyin_query_project\data\ldb
```

运行输出（示例）：
- `CSV total lines: 1402054`
- `Valid lines: 1401994`
- `DB path: ...\data\ldb`

### 6.2 单词查询

```bat
D:\C++_project\build\bin\Release\leveldb_query.exe D:\C++_project\build\LessonCode\week02\homework\pinyin_query_project\data\ldb ni'cai D:\C++_project\build\LessonCode\week02\homework\pinyin_query_project\data\out_leveldb.csv
```

### 6.3 生成测试集（100 / 1000）

```bat
D:\C++_project\build\bin\Release\make_queries.exe D:\C++_project\build\LessonCode\week02\homework\pinyin_query_project\data\dict.csv D:\C++_project\build\LessonCode\week02\homework\pinyin_query_project\data\test_queries_100.txt D:\C++_project\build\LessonCode\week02\homework\pinyin_query_project\data\test_queries_1000.txt
```

输出（本次实验）：
- `Total CSV lines: 1402054`
- `Unique pinyin: 642035`
- `Generated ...100.txt (100)`
- `Generated ...1000.txt (1000)`

### 6.4 运行 benchmark

#### 100 条查询

```bat
D:\C++_project\build\bin\Release\benchmark.exe D:\C++_project\build\LessonCode\week02\homework\pinyin_query_project\data\ldb D:\C++_project\build\LessonCode\week02\homework\pinyin_query_project\data\test_queries_100.txt D:\C++_project\build\LessonCode\week02\homework\pinyin_query_project\result\benchmark_100.csv
```

#### 1000 条查询

```bat
D:\C++_project\build\bin\Release\benchmark.exe D:\C++_project\build\LessonCode\week02\homework\pinyin_query_project\data\ldb D:\C++_project\build\LessonCode\week02\homework\pinyin_query_project\data\test_queries_1000.txt D:\C++_project\build\LessonCode\week02\homework\pinyin_query_project\result\benchmark_1000.csv
```

---

## 7. 实验结果

## 7.1 数据规模

- CSV 总行数：**1,402,054**
- 有效行数：**1,401,994**
- 唯一拼音 key：**642,035**

## 7.2 100 条查询结果（多次运行）

- 命中率：`100/100 = 100%`
- 平均耗时（avg）：约 **40 ~ 65 us**
- p50：约 **15 ~ 26 us**
- p95/p99：有波动（受缓存状态、系统调度影响）

![image-20260328165025995](C:\Users\12159\AppData\Roaming\Typora\typora-user-images\image-20260328165025995.png)

## 7.3 1000 条查询结果（实际运行）

- Run 1  
  - `queries: 1000 hit: 1000 miss: 0`
  - `avg(us): 13.2`
  - `p50: 9`
  - `p95: 18`
  - `p99: 128`

- Run 2  
  - `queries: 1000 hit: 1000 miss: 0`
  - `avg(us): 17.489`
  - `p50: 11`
  - `p95: 23`
  - `p99: 142`

![image-20260328164224861](C:\Users\12159\AppData\Roaming\Typora\typora-user-images\image-20260328164224861.png)

---

## 8. 结果分析

1. **功能正确性**  
   LevelDB 构建、单词查询、批量 benchmark 均正常运行，命中率 100%。

2. **性能表现**  
   查询延迟达到微秒级，`p50` 很低，说明绝大多数查询非常快。

3. **波动原因**  
   不同轮���测试的 `avg/p99` 存在波动，常见原因包括：
   - OS 文件缓存预热（冷启动 vs 热启动）
   - CPU 调度与后台进程
   - 磁盘瞬时负载

---

## 9. 常见问题与排查

1. `Failed to open csv: data\dict.csv`  
   - 原因：当前工作目录不对或路径写成相对路径导致找不到文件。  
   - 解决：优先使用绝对路径。

2. `Failed to load queries: ...test_queries_100.txt`  
   - 原因：测试文件不存在。  
   - 解决：先运行 `make_queries.exe` 生成测试集。

3. `Cannot find leveldb lib`  
   - 原因：`LEVELDB_ROOT` 路径不正确，或未编译出 `leveldb.lib`。  
   - 解决：确认 `D:\...\leveldb\build\Release\leveldb.lib` 存在，并重新 cmake configure。

---

## 10. 总结

本项目完成了从 CSV 顺序查询到 LevelDB 键值索引查询的优化改造。  
在百万级数据下，系统实现了稳定的微秒级查询性能，并具备批量测试与结果导出能力，满足课程附加任务要求。



**原来的 CSV 顺序扫描法可以优化到“更快”，但很难接近 LevelDB 这种索引型存储的稳定微秒级。**

### 对比结论（简版）

- **你原方法（每次扫 CSV）**：时间复杂度接近 `O(N)`，N=140万行时天然慢。
- **LevelDB**：键查找近似 `O(logN)`，并有 block cache/bloom 等机制，明显更快更稳。



## 原方法还能怎么优化（按收益排序）

### 1) 一次加载到内存建索引（最有效）

启动时把 CSV 读一遍，建：

- `unordered_map<string, vector<string>> index;` 查询时直接 `index[pinyin]`，接近 `O(1)`。

> 这一步能把“每次扫全文件”变成“启动慢一点、查询极快”。

------

### 2) 排序 + 二分查找

把数据预处理为按拼音排序的文件（或数组），查询用 `lower_bound/upper_bound` 找区间，复杂度 `O(logN + k)`。

------

### 3) 做你自己的磁盘索引文件

类似：

- `pinyin -> offset,length`（索引文件）
- 实际记录放 data 文件 查询先查索引，再 `seek` 定位读取，避免全表扫描。

------

### 4) 工程级优化（中等收益）

- 更快解析 CSV（减少 string 拷贝）
- 批量读（大缓冲）
- 复用对象，减少内存分配
- 结果文件批量写，少 flush

------

## 建议尝试：

1. Baseline：CSV 顺序扫描
2. 优化1：内存哈希索引
3. 优化2：LevelDB 持久化索引
4. 比较 avg/p50/p95 + 内存占用 + 启动时间



内存索引版查询程序结果

![image-20260328213613291](C:\Users\12159\AppData\Roaming\Typora\typora-user-images\image-20260328213613291.png)

![image-20260328214445446](C:\Users\12159\AppData\Roaming\Typora\typora-user-images\image-20260328214445446.png)

![image-20260328214500326](C:\Users\12159\AppData\Roaming\Typora\typora-user-images\image-20260328214500326.png)



## 性能对比（LevelDB vs 内存索引）

### 1) LevelDB（你之前结果）

- 1000 条查询：
  - Run1: avg 13.2us, p50 9, p95 18, p99 128
  - Run2: avg 17.489us, p50 11, p95 23, p99 142
- 特点：查询快、延迟稳定、数据持久化、内存占用相对可控

### 2) 内存索引（你截图结果）

- 索引构建时间：
  - 100 查询场景：662ms、691ms
  - 1000 查询场景：779ms、684ms
- 查询性能：
  - 100 条：avg 0.65us / 0.6us，p50=0，p95=1，p99=1
  - 1000 条：avg 0.566us / 0.45us，p50=0，p95=0，p99=1
- 特点：查询极快，但每次程序启动要先全量构建索引，且更吃内存

------

## 对比分析

| 方案         |                查询速度 |         启动成本 | 内存占用 |       持久化 | 工程适用性           |
| ------------ | ----------------------: | ---------------: | -------: | -----------: | -------------------- |
| CSV 顺序扫描 |              慢（O(N)） |               低 |       低 |           无 | 仅适合小数据或演示   |
| LevelDB      |            快（微秒级） |            低~中 |       中 |           有 | **综合最均衡，推荐** |
| 内存索引     | **最快（亚微秒~微秒）** | 高（需重建索引） |       高 | 无（进程内） | 适合极致低延迟场景   |

------

## 常见问题排查

1. `Failed to open csv`
   - 使用绝对路径，避免工作目录不一致导致找不到文件。
2. `Failed to load queries`
   - 先运行 `make_queries.exe` 生成 `test_queries_100.txt` 和 `test_queries_1000.txt`。
3. `Cannot find leveldb lib`
   - 检查 `LEVELDB_ROOT` 与 `leveldb.lib` 实际路径是否一致，重新 `cmake -S -B`。

------

## 后续可优化方向

- 为内存索引增加序列化缓存（减少重复构建成本）
- 为 LevelDB 增加写入压缩与批量写优化
- 加入多线程并发查询压测（QPS / 吞吐）
- 增加内存占用统计，形成更完整评估指标