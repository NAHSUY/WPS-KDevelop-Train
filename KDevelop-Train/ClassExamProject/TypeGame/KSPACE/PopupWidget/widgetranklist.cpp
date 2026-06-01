#pragma execution_character_set("utf-8")
#include "widgetranklist.h"
#include <QSettings>
#include <QPainter>    // 必须包含
#include <algorithm>   // 必须包含 (为了 std::sort)

const QPixmap& WidgetRankList::pixExit()
{
	static const QPixmap pix(":/imagePng/PNG/SPACE_HISCORE_BG.png");
	return pix;
}

WidgetRankList::WidgetRankList(QWidget* parent)
	: MyAbstractWidget(parent) // 建议用圆括号
{
	this->move(0, 0);
	this->resize(pixExit().size());
	initButton();
	initRankListWidget();
	loadRankDataFromIni();
	this->hide();
}

void WidgetRankList::initButton()
{
	// 修正了 &:: 的语法错误
	this->m_buttonReturn = new ImageButton(":/imagePng/PNG/SPACE_RETURN.png", this, QPoint(500, 500), 3);
	connect(this->m_buttonReturn, &ImageButton::clicked, this, &WidgetRankList::m_buttonReturn_clicked);

	QList<ImageButton*> btnList = { this->m_buttonReturn };
	for (ImageButton* btn : btnList)
	{
		connect(btn, &ImageButton::needSound, this, &MyAbstractWidget::needSound);
	}
}

void WidgetRankList::m_buttonReturn_clicked()
{
	this->hide();
	emit allhide();
}

void WidgetRankList::paintEvent(QPaintEvent* event)
{
	QWidget::paintEvent(event);
	QPainter painter(this);
	painter.drawPixmap(0, 0, this->pixExit());
	this->m_buttonReturn->draw(&painter);
}

void WidgetRankList::initRankListWidget()
{
	m_rankListWidget = new QListWidget(this);
	m_rankListWidget->move(243, 153);
	m_rankListWidget->setFixedWidth(350);

	m_rankListWidget->setStyleSheet(R"(
		QListWidget {
			background: transparent; 
			border: none; 
			padding: 0; 
			color: yellow; 
		}
		QListWidget::item {
			height: 35px; 
			margin: 0; 
			font-family: "Microsoft YaHei"; 
		}
		QListWidget::item:selected {
			background: transparent;
			color: yellow; 
		}
		QScrollBar:vertical {
			width: 0; 
			background: transparent;
		}
	)");

	QFont listFont = m_rankListWidget->font();
	listFont.setPointSize(16);
	listFont.setBold(true);
	m_rankListWidget->setFont(listFont);

	m_rankListWidget->setEditTriggers(QListWidget::NoEditTriggers);
	m_rankListWidget->setFocusPolicy(Qt::NoFocus);
	m_rankListWidget->setSelectionMode(QAbstractItemView::NoSelection);
	m_rankListWidget->setMouseTracking(false);
}

void WidgetRankList::loadRankDataFromIni()
{
	QSettings settings("game_rank.ini", QSettings::IniFormat);
	int count = settings.value("Rank/Count", 0).toInt();
	m_rankData.clear();

	for (int i = 0; i < count; ++i)
	{
		RankItem item;
		item.name = settings.value(QString("Rank/%1/Name").arg(i)).toString();
		item.score = settings.value(QString("Rank/%1/Score").arg(i)).toInt();
		m_rankData.append(item);
	}
	updateRankList();
}

void WidgetRankList::saveRankDataToIni()
{
	QSettings settings("game_rank.ini", QSettings::IniFormat);
	settings.clear();

	settings.setValue("Rank/Count", m_rankData.size());

	for (int i = 0; i < m_rankData.size(); ++i)
	{
		settings.setValue(QString("Rank/%1/Name").arg(i), m_rankData[i].name);
		settings.setValue(QString("Rank/%1/Score").arg(i), m_rankData[i].score);
	}
	settings.sync();
}

void WidgetRankList::updateRankList()
{
	m_rankListWidget->clear();
	std::sort(m_rankData.begin(), m_rankData.end());

	int maxRankCount = 10;
	if (m_rankData.size() > maxRankCount)
	{
		m_rankData.resize(maxRankCount);
	}

	for (int i = 0; i < m_rankData.size(); ++i)
	{
		const auto& item = m_rankData[i];
		// 使用 QStringLiteral 避免 C2001 换行符报错
		// 替换原来的代码，必须写在同一行！
		QString itemText = QString("%1、%2:%3分").arg(i + 1).arg(item.name).arg(item.score);

		m_rankListWidget->addItem(itemText);
	}
}

void WidgetRankList::addGameScore(const QString& name, int score)
{
	if (name.isEmpty() || score < 0)
	{
		return;
	}
	bool isPlayerExist = false;
	for (auto& item : m_rankData)
	{
		if (item.name == name)
		{
			item.score = qMax(item.score, score);
			isPlayerExist = true;
			break;
		}
	}
	if (!isPlayerExist)
	{
		m_rankData.append({ name, score });
	}

	updateRankList();
	saveRankDataToIni();
}