#ifndef GAMESELECTOR_H
#define GAMESELECTOR_H

#include <QPainter>
#include <QWidget>
#include <QPushButton>
#include <QLabel>

class GameSelector : public QWidget
{
    Q_OBJECT

public:
    explicit GameSelector(QWidget* parent = nullptr);

protected:
    void paintEvent(QPaintEvent* event) override;

private:
    QLabel* m_titleLabel = nullptr;
    QPushButton* m_buttonKSpace = nullptr;
    QPushButton* m_buttonKApple = nullptr;

    void initUI();
    void onKSpaceClicked();
    void onKAppleClicked();

signals:
};

#endif // GAMESELECTOR_H
