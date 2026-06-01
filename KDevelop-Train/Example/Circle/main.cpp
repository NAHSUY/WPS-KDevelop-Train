#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QVector>
#include <QColor>
#include <algorithm>

// 1. 模拟像素屏控件
class ScreenWidget : public QWidget {
public:
    explicit ScreenWidget(int widthPixels, int heightPixels, int pixelSize = 10, QWidget* parent = nullptr)
        : QWidget(parent), m_widthPixels(widthPixels), m_heightPixels(heightPixels),
        m_pixelSize(pixelSize), m_pixels(widthPixels* heightPixels, Qt::white) {
        setFixedSize(widthPixels * pixelSize, heightPixels * pixelSize);
    }

    void setPixel(int x, int y, const QColor& color) {
        if (x >= 0 && x < m_widthPixels && y >= 0 && y < m_heightPixels) {
            m_pixels[y * m_widthPixels + x] = color;
            update();
        }
    }

private:
    void paintEvent(QPaintEvent*) override {
        QPainter painter(this);
        for (int y = 0; y < m_heightPixels; ++y) {
            for (int x = 0; x < m_widthPixels; ++x) {
                painter.fillRect(x * m_pixelSize, y * m_pixelSize,
                    m_pixelSize - 1, m_pixelSize - 1, // -1 留出网格感
                    m_pixels[y * m_widthPixels + x]);
            }
        }
    }
    int m_widthPixels, m_heightPixels, m_pixelSize;
    QVector<QColor> m_pixels;
};

// 2. 画圆算法函数
void drawCircle(ScreenWidget& screen, int cx, int cy, int radius, const QColor& color) {
    int x = 0, y = radius;
    int d = 3 - 2 * radius;
    auto plot8 = [&](int x, int y) {
        screen.setPixel(cx + x, cy + y, color); screen.setPixel(cx - x, cy + y, color);
        screen.setPixel(cx + x, cy - y, color); screen.setPixel(cx - x, cy - y, color);
        screen.setPixel(cx + y, cy + x, color); screen.setPixel(cx - y, cy + x, color);
        screen.setPixel(cx + y, cy - x, color); screen.setPixel(cx - y, cy - x, color);
        };
    while (y >= x) {
        plot8(x, y);
        if (d < 0) d = d + 4 * x + 6;
        else { d = d + 4 * (x - y) + 10; y--; }
        x++;
    }
}

// 3. 主程序入口
int main(int argc, char* argv[]) {
    QApplication a(argc, argv);
    ScreenWidget screen(50, 50, 12); // 创建 50x50 的像素矩阵
    screen.setWindowTitle("VS2022 画圆演示");
    screen.show();

    drawCircle(screen, 25, 25, 20, Qt::blue); // 在中心画圆

    return a.exec();
}