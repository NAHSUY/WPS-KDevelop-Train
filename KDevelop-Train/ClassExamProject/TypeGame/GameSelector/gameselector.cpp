#pragma execution_character_set("utf-8")
#include "gameselector.h"
#include <QProcess>
#include <QMessageBox>
#include <QDir>
#include <QApplication>
#include <QVBoxLayout>

GameSelector::GameSelector(QWidget* parent)
    : QWidget(parent)
{
    this->setFixedSize(800, 600);
    this->setWindowTitle("Type Game - 游戏选择");
    this->initUI();
}

void GameSelector::initUI()
{
    // 标题标签
    m_titleLabel = new QLabel("选择游戏", this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setGeometry(0, 50, 800, 80);

    QFont titleFont("黑体", 36, QFont::Bold);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet("color: black;");

    // 太空大战按钮
    m_buttonKSpace = new QPushButton(this);
    m_buttonKSpace->setGeometry(100, 200, 280, 280);
    m_buttonKSpace->setStyleSheet(R"(
        QPushButton {
            border: 3px solid #4a90d9;
            border-radius: 10px;
            background-color: rgba(98, 172, 246);
        }
        QPushButton:hover {
            border: 3px solid #7ab8ff;
            background-color: rgba(74, 144, 217, 0.3);
        }
        QPushButton:pressed {
            background-color: rgba(74, 144, 217, 0.5);
        }
    )");

    // 太空大战预览图片和文字
    QVBoxLayout* kspaceLayout = new QVBoxLayout(m_buttonKSpace);
    kspaceLayout->setAlignment(Qt::AlignCenter);

    QLabel* kspaceImage = new QLabel(m_buttonKSpace);
    QPixmap kspacePix(":/imagePng/PNG/SPACE_MAINMENU_BG.png");
    kspaceImage->setPixmap(kspacePix.scaled(240, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    kspaceImage->setAlignment(Qt::AlignCenter);
    kspaceLayout->addWidget(kspaceImage);

    QLabel* kspaceText = new QLabel("太空大战", m_buttonKSpace);
    QFont textFont("黑体", 16, QFont::Bold);
    kspaceText->setFont(textFont);
    kspaceText->setAlignment(Qt::AlignCenter);
    kspaceText->setStyleSheet("color: white;");
    kspaceLayout->addWidget(kspaceText);

    connect(m_buttonKSpace, &QPushButton::clicked, this, &GameSelector::onKSpaceClicked);

    // 拯救苹果按钮
    m_buttonKApple = new QPushButton(this);
    m_buttonKApple->setGeometry(420, 200, 280, 280);
    m_buttonKApple->setStyleSheet(R"(
        QPushButton {
            border: 3px solid #4a90d9;
            border-radius: 10px;
            background-color: rgba(98, 172, 246);
        }
        QPushButton:hover {
            border: 3px solid #7ab8ff;
            background-color: rgba(74, 144, 217, 0.3);
        }
        QPushButton:pressed {
            background-color: rgba(74, 144, 217, 0.5);
        }
    )");

    // 拯救苹果预览图片和文字
    QVBoxLayout* kappleLayout = new QVBoxLayout(m_buttonKApple);
    kappleLayout->setAlignment(Qt::AlignCenter);

    QLabel* kappleImage = new QLabel(m_buttonKApple);
    QPixmap kapplePix(":/imagePng/PNG/APPLE_BACKGROUND.png");
    kappleImage->setPixmap(kapplePix.scaled(240, 180, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    kappleImage->setAlignment(Qt::AlignCenter);
    kappleLayout->addWidget(kappleImage);

    QLabel* kappleText = new QLabel("拯救苹果", m_buttonKApple);
    kappleText->setFont(textFont);
    kappleText->setAlignment(Qt::AlignCenter);
    kappleText->setStyleSheet("color: white;");
    kappleLayout->addWidget(kappleText);

    connect(m_buttonKApple, &QPushButton::clicked, this, &GameSelector::onKAppleClicked);

    // 退出按钮
    QPushButton* exitButton = new QPushButton("退出", this);
    exitButton->setGeometry(350, 520, 100, 40);
    exitButton->setStyleSheet(R"(
        QPushButton {
            background-color: #555;
            color: white;
            border: 2px solid #777;
            border-radius: 5px;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #777;
        }
    )");
    connect(exitButton, &QPushButton::clicked, qApp, &QApplication::quit);
}

void GameSelector::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);
    QPainter painter(this);
    painter.fillRect(rect(), QColor(235, 246, 255));
}

void GameSelector::onKSpaceClicked()
{
    QString appPath = QApplication::applicationDirPath();
    QString kspacePath = appPath + "/KSPACE.exe";

    if (QFile::exists(kspacePath)) {
        QProcess::startDetached(kspacePath);
    } else {
        QMessageBox::warning(this, "错误", "未找到 KSPACE.exe\n路径: " + kspacePath);
    }
}

void GameSelector::onKAppleClicked()
{
    QString appPath = QApplication::applicationDirPath();
    QString kapplePath = appPath + "/KApple.exe";

    if (QFile::exists(kapplePath)) {
        QProcess::startDetached(kapplePath);
    } else {
        QMessageBox::warning(this, "错误", "未找到 KApple.exe\n路径: " + kapplePath);
    }
}
