#include <QCoreApplication>
#include <QImage>
#include <QColor>
#include <QVector>
#include <QPoint>
#include <QQueue>
#include <iostream>
#include <cmath>
#include <algorithm>

// LAB 色彩结构体
struct LabColor {
    double l, a, b;
};

// 区域数据结构
struct Region {
    int id;
    int area;
    QColor coreColor;
};

// RGB 转 CIELAB
LabColor rgbToLab(const QColor& color) {
    double r = color.red() / 255.0;
    double g = color.green() / 255.0;
    double b = color.blue() / 255.0;

    r = (r > 0.04045) ? std::pow((r + 0.055) / 1.055, 2.4) : (r / 12.92);
    g = (g > 0.04045) ? std::pow((g + 0.055) / 1.055, 2.4) : (g / 12.92);
    b = (b > 0.04045) ? std::pow((b + 0.055) / 1.055, 2.4) : (b / 12.92);

    double x = (r * 0.4124564 + g * 0.3575761 + b * 0.1804375) * 100.0;
    double y = (r * 0.2126729 + g * 0.7151522 + b * 0.0721750) * 100.0;
    double z = (r * 0.0193339 + g * 0.1191920 + b * 0.9503041) * 100.0;

    double xn = 95.047, yn = 100.000, zn = 108.883;
    x /= xn; y /= yn; z /= zn;

    auto f = [](double t) {
        return (t > 0.008856) ? std::pow(t, 1.0 / 3.0) : (7.787 * t + 16.0 / 116.0);
        };

    LabColor lab;
    lab.l = (116.0 * f(y)) - 16.0;
    lab.a = 500.0 * (f(x) - f(y));
    lab.b = 200.0 * (f(y) - f(z));
    return lab;
}

// Delta E 色差计算
double calculateDeltaE(const LabColor& lab1, const LabColor& lab2) {
    return std::sqrt(std::pow(lab1.l - lab2.l, 2) +
        std::pow(lab1.a - lab2.a, 2) +
        std::pow(lab1.b - lab2.b, 2));
}

QString getSimpleColorName(const QColor& c) {
    int r = c.red(), g = c.green(), b = c.blue();
    if (b > r && b > g) return (b < 100) ? "Deep Blue" : "Base Blue";
    if (g > r && g > b) return "Green Area";
    if (r > g && r > b) return (g > 130) ? "Yellow Sand" : "Brown Earth";
    return "Other";
}

void analyzeImage(const QString& path) {
    QImage img(path);
    if (img.isNull()) {
        std::cout << "Cannot find image at: " << path.toStdString() << std::endl;
        return;
    }

    int w = img.width(), h = img.height();

    // 预计算 LAB 矩阵，加速后续运算
    QVector<LabColor> labMap(w * h);
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            labMap[y * w + x] = rgbToLab(img.pixelColor(x, y));
        }
    }

    QVector<bool> visited(w * h, false);
    QVector<Region> coreRegions;
    // ==========================================
   // 算法参数调优区：双重保险
   // ==========================================
   // 1. 相邻容差：控制能否跨越尖锐的边界（如阴影边缘）
    const double NEIGHBOR_TOLERANCE = 12.0;

    // 2. 全局容差：允许同色块内部有多大程度的渐变漂移（数值越大，结合得越多）
    const double SEED_TOLERANCE = 35.0;

    // 3. 过滤碎屑：合并后的块，小于 1000 像素的依然不要
    const int MIN_AREA = 1000;
    // ==========================================

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            int idx = y * w + x;
            if (visited[idx]) continue;

            LabColor seedLab = labMap[idx];
            QColor seedColor = img.pixelColor(x, y);
            int currentArea = 0;

            QQueue<QPoint> q;
            q.enqueue({ x, y });
            visited[idx] = true;

            while (!q.isEmpty()) {
                QPoint p = q.dequeue();
                currentArea++;

                // 获取当前正在蔓延的像素的 LAB 色彩
                int currentIdx = p.y() * w + p.x();
                LabColor currentLab = labMap[currentIdx];

                static const int dx[] = { 0, 0, 1, -1 }, dy[] = { 1, -1, 0, 0 };
                for (int i = 0; i < 4; ++i) {
                    int nx = p.x() + dx[i], ny = p.y() + dy[i];
                    if (nx >= 0 && nx < w && ny >= 0 && ny < h) {
                        int nIdx = ny * w + nx;
                        if (!visited[nIdx]) {
                            LabColor neighborLab = labMap[nIdx];

                            // 【核心判断：双重阈值】
                            // 条件A：与紧挨着的像素差别不大（不能跨越阴影断层）
                            double distToNeighbor = calculateDeltaE(currentLab, neighborLab);
                            // 条件B：与最初的起点像素也不能偏离太远（防止无限蔓延）
                            double distToSeed = calculateDeltaE(seedLab, neighborLab);

                            if (distToNeighbor < NEIGHBOR_TOLERANCE && distToSeed < SEED_TOLERANCE) {
                                visited[nIdx] = true;
                                q.enqueue({ nx, ny });
                            }
                        }
                    }
                }
            }

            if (currentArea >= MIN_AREA) {
                coreRegions.append({ 0, currentArea, seedColor });
            }
        }
    }

    std::sort(coreRegions.begin(), coreRegions.end(), [](const Region& a, const Region& b) {
        return a.area > b.area;
        });

    std::cout << "\n[ Dual-Threshold CIELAB Segmentation ]" << std::endl;
    std::cout << "Identified " << coreRegions.size() << " major continuous regions.\n" << std::endl;
    printf("%-5s | %-12s | %-12s | %s\n", "ID", "Color Class", "Area(px)", "RGB Sample");
    std::cout << "------|--------------|--------------|-------------------" << std::endl;
    for (int i = 0; i < coreRegions.size(); ++i) {
        printf("#%-4d | %-12s | %-12d | (R:%3d, G:%3d, B:%3d)\n",
            i + 1,
            getSimpleColorName(coreRegions[i].coreColor).toStdString().c_str(),
            coreRegions[i].area,
            coreRegions[i].coreColor.red(),
            coreRegions[i].coreColor.green(),
            coreRegions[i].coreColor.blue());
    }
}

int main(int argc, char* argv[]) {
    QCoreApplication a(argc, argv);
    // 请替换为你的绝对路径
    analyzeImage("D:/C++_project/build/LessonCode/week01/practice/Colors/image.png");
    return 0;
}