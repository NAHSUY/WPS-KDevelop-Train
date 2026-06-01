此目录存放课堂作业，可以在此文件添加作业题目、解题思路和对题目的看法

```
cmake .. -G "Visual Studio 17 2022" -A x64
msbuild malloc_free_benchmark.sln /property:Configuration=Release /property:Platform=x64
```



# 一、准确测试函数的性能-Google Benchmark

## 如何使用Google Benchmark测试代码性能

### 1. 函数基准测试

![image-20260326214215553](C:\Users\12159\AppData\Roaming\Typora\typora-user-images\image-20260326214215553.png)

### 2. 基准测试参数化

![image-20260326215040680](C:\Users\12159\AppData\Roaming\Typora\typora-user-images\image-20260326215040680.png)

### 3. 多个参数的基准测试

![image-20260326215350485](C:\Users\12159\AppData\Roaming\Typora\typora-user-images\image-20260326215350485.png)

### 4. 自定义计时器

![image-20260326220706232](C:\Users\12159\AppData\Roaming\Typora\typora-user-images\image-20260326220706232.png)

# 二、课堂练习：

## 1. 统计malloc和free的性能情况

malloc和free都会造成性能上面的开销，那么二者的开销有无差异呢？请设计实验统计malloc和free的性能情况。

![image-20260326222637626](C:\Users\12159\AppData\Roaming\Typora\typora-user-images\image-20260326222637626.png)

## 2. HeapAlloc

感受一下使用HeapAlloc和malloc的差别，你能否验证windows下malloc是否在底层调用了HeapAlloc？ 可以从源码角度去考虑。

![image-20260326224051371](C:\Users\12159\AppData\Roaming\Typora\typora-user-images\image-20260326224051371.png)

## 3.内存映射与普通读写方式比较

![image-20260327210039203](C:\Users\12159\AppData\Roaming\Typora\typora-user-images\image-20260327210039203.png)