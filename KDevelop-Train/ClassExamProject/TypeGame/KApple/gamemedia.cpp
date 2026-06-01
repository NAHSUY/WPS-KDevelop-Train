#include "gamemedia.h"
#include <QMediaPlayer>
#include <QSoundEffect>
#include <QUrl>

GameMedia::GameMedia(QObject* parent)
    : QObject(parent)
{
    m_bgmPlayer = new QMediaPlayer(this);
    m_bgmPlayer->setVolume(50);

    // 手动循环播放（兼容所有 Qt 5 版本）
    connect(m_bgmPlayer, &QMediaPlayer::mediaStatusChanged,
        this, [this](QMediaPlayer::MediaStatus status) {
            if (status == QMediaPlayer::EndOfMedia) {
                m_bgmPlayer->setPosition(0);
                m_bgmPlayer->play();
            }
        });

    initSoundEffect();
}

void GameMedia::initSoundEffect()
{
    //m_soundEffects[APPLEIN] = createEffect("qrc:/sound/collect.wav", 100);
    //m_soundEffects[BUTTONCLICK] = createEffect("qrc:/sound/click.wav", 80);
    //m_soundEffects[BUTTONENTER] = createEffect("qrc:/sound/button_enter.wav", 70);
    //m_soundEffects[SLIDER] = createEffect("qrc:/sound/slider.wav", 90);
    //m_soundEffects[Type] = createEffect("qrc:/sound/type.wav", 85);

    m_soundEffects[APPLEIN] = createEffect("qrc:/Sounds/APPLE_IN.wav", 100);
    m_soundEffects[BUTTONCLICK] = createEffect("qrc:/Sounds/APPLE_IN.wav", 80);
    m_soundEffects[BUTTONENTER] = createEffect("qrc:/Sounds/APPLE_IN.wav", 70);
    m_soundEffects[SLIDER] = createEffect("qrc:/Sounds/APPLE_IN.wav", 90);
    m_soundEffects[Type] = createEffect("qrc:/Sounds/APPLE_IN.wav", 85);

}

QSoundEffect* GameMedia::createEffect(const QString& path, int volume)
{
    auto* effect = new QSoundEffect(this);
    effect->setSource(QUrl(path));
    effect->setVolume(volume / 100.0f);
    return effect;
}

void GameMedia::playSound(soundType type)
{
    if (m_soundEffects.contains(type)) {
        m_soundEffects[type]->play();
    }
}

void GameMedia::playBGM(BGMtype type)
{
    if (type != NORMAL) return;

    m_bgmPlayer->setMedia(QUrl("qrc:/sound/main_bgm.mp3"));
    m_bgmPlayer->play();
}

void GameMedia::pauseBGM()
{
    if (m_bgmPlayer->state() == QMediaPlayer::PlayingState) {
        m_bgmPlayer->pause();
    }
}