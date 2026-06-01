// gamemedia.h
#ifndef GAMEMEDIA_H
#define GAMEMEDIA_H

#include <QObject>
#include <QMap>
#include <QSoundEffect>
#include <QMediaPlayer> // 可保留用于 BGM，但功能受限
#include "PopupWidget/public_enum.h"

class GameMedia : public QObject
{
    Q_OBJECT
public:
    explicit GameMedia(QObject* parent = nullptr);

public slots:
    void playSound(soundType type);
    void playBGM(BGMtype type);
    void pauseBGM();

private:
    void initSoundEffect();
    QSoundEffect* createEffect(const QString& path, int volume = 100);

    QMap<soundType, QSoundEffect*> m_soundEffects;
    QMediaPlayer* m_bgmPlayer = nullptr;
};

#endif // GAMEMEDIA_H