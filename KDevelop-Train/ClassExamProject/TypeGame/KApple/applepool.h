#ifndef APPLEPOOL_H
#define APPLEPOOL_H
#include<QRandomGenerator>
#include "PopupWidget/myabstractwidget.h"
#include "apple.h"
class ApplePool : public MyAbstractWidget //负责对象的管理以及统计
{
    Q_OBJECT
public:
    explicit ApplePool(QWidget *parent = nullptr);
    ~ApplePool();
	void cleanup();
	void drawApple(QPainter *painter);
	void hitMouse(char targetChar);
	void pause();
	void resume();
	void init();
	// 替换原有声明，确保与实现匹配
	void loadSetUp(int winNum, int failNum, int gameLevel);

private:
	double m_fallTime = 5.0;
	int m_gameLevel = 3;
	int m_winNum = 50;
	int m_failNum = 5;

	int m_breakCount = 0;
	int m_pickCount = 0;

	char generateRandomLetter();
	int generateSuitablePosX();
	double generateSpeedFactor();

	QList<Apple *> m_appleList;
	QList<Apple *> m_appleWaitList; //墓地

	void addApple();
	void removeApple(Apple *apple);

signals:

public slots:
	void appleBroken();
};

#endif // APPLEPOOL_H
