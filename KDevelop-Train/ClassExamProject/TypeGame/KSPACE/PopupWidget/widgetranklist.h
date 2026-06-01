#ifndef WIDGETRANKLIST_H
#define WIDGETRANKLIST_H

#include <QLabel>
#include <QListWidget>
#include <QVector> // 必须包含 QVector
#include "imagebutton.h"
#include "myabstractwidget.h"

struct RankItem
{
	QString name;
	int score;

	// 重载小于运算符，用于按分数降序排序（分数高的排前面）
	bool operator<(const RankItem& other) const { return score > other.score; }
};

class WidgetRankList : public MyAbstractWidget
{
	Q_OBJECT
public:
	explicit WidgetRankList(QWidget* parent = nullptr);

protected:
	void paintEvent(QPaintEvent* event) override;

public slots:
	void addGameScore(const QString& name, int score);

private:
	ImageButton* m_buttonReturn = nullptr;
	QListWidget* m_rankListWidget = nullptr;
	QVector<RankItem> m_rankData;

	void initButton();
	void initRankListWidget();
	void loadRankDataFromIni();
	void saveRankDataToIni();
	void updateRankList();
	static const QPixmap& pixExit();

private slots:
	void m_buttonReturn_clicked();

signals:
	// 信号区保持干净
};

#endif // WIDGETRANKLIST_H