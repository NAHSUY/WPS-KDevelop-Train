#ifndef GAMEMEDIA_H
#define GAMEMEDIA_H

#include <QMap>
#include <QMediaPlayer>
#include <QObject>
#include <QSoundEffect>
#include <QMediaPlaylist> 
#include "PopupWidget/public_enum.h"
class GameMedia : public QObject
{
	Q_OBJECT
public:
	explicit GameMedia(QObject *parent = nullptr);

private:
	void initMediaPlayer();
	void initSoundEffect(soundType type, const QString &resourcePath, int volumn);
	void initSoundEffectMap();

public slots:
	void playSound(soundType type);
	void playBGM(BGMtype type);
	void pauseBGM();

private:
	QMediaPlayer *m_mediaPlayerBG;
	
	QMap<soundType, QSoundEffect *> m_mapSoundEffect;
	QMediaPlaylist* m_playlist;
signals:
};

#endif // GAMEMEDIA_H
