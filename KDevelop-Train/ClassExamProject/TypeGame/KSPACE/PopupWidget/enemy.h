#ifndef ENEMY_H
#define ENEMY_H

#include <QGraphicsPixmapItem> // 修改1：修正了拼写错误
#include <QPainter>
#include <QVector>

class Enemy : public QGraphicsPixmapItem
{
public:
	explicit Enemy(char targetChar, float moveSpeed, QPoint pos);
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;
	void changeState(char targetChar, float movespeed, QPoint pos);
	bool isNeedRemove();
	bool isAlive();		 // 增加了声明
	void toDie();
	void toCollision();
	bool beHitten(char c);

	// 修改2：去掉信号槽机制，改为普通函数，避免因为不继承 QObject 导致的 MOC 报错
	void updatePos();
	void needReomove();

private:
	char m_targetChar = 'A';
	bool m_isCanHit = false;
	enum direction
	{
		LEFT,
		RIGHT
	};
	direction m_direction = LEFT;
	float m_speedHorizontal = 10;
	void moveHorizontal();

	float m_speedVertical = 1;
	void moveVertical();
	static const QFont fontBig;
	static const QPixmap& pixChar();
	static const QPixmap& pixNormal();
	static const QPixmap& pixExplosion();

	enum enemyCondition // 避免与全局条件冲突，改个名
	{
		LIVE,
		DIE
	};
	enemyCondition m_condition;

	int m_currentIndex;

	QVector<QPixmap> m_MoveFrames;
	QVector<QPixmap> m_explosionFrames;
	QVector<QPixmap> cropSpriteFrames(const QPixmap& sprite, int frameCount);
};

#endif // ENEMY_H