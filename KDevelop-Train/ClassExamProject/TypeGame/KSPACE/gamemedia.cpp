#include "gamemedia.h"

GameMedia::GameMedia(QObject *parent)
	: QObject{parent}
{
	initMediaPlayer();
	initSoundEffectMap();
}
void GameMedia::initMediaPlayer()
{
	this->m_mediaPlayerBG = new QMediaPlayer(this);

	// Qt5 不需要 QAudioOutput
	// 直接给 MediaPlayer 设置音量 (0-100)
	this->m_mediaPlayerBG->setVolume(50);

	// Qt5 循环播放通常需要结合 QMediaPlaylist，
	// 如果只是简单循环，可以使用信号或 Playlist
	// 这里先展示最基础的写法，去掉不支持的 setAudioOutput 和 setLoops
}

void GameMedia::initSoundEffectMap()
{
	this->initSoundEffect(soundType::BUTTONCLICK, "qrc:/soundWav/WAVE/ANIBTN_CLICK.wav", 1);
	this->initSoundEffect(soundType::BUTTONENTER, "qrc:/soundWav/WAVE/ANIBTN_ENTER.wav", 1);

	this->initSoundEffect(soundType::Type, "qrc:/soundWav/WAVE/TYPE.wav", 1);
	this->initSoundEffect(soundType::SLIDER, "qrc:/soundWav/WAVE/GLIDE.wav", 30);
	this->initSoundEffect(soundType::UPGRADE, "qrc:/soundWav/WAVE/UPGRADE.wav", 30);
	this->initSoundEffect(soundType::PLANEEXPLOSION, "qrc:/soundWav/WAVE/SPACE_BLAST.wav", 30);
}
void GameMedia::initSoundEffect(soundType type, const QString &resourcePath, int volumn)
{
	QSoundEffect *soundEffect = new QSoundEffect(this);

	soundEffect->setSource(QUrl(resourcePath));

	soundEffect->setVolume(volumn);

	soundEffect->setLoopCount(1);

	this->m_mapSoundEffect.insert(type, soundEffect);
}
void GameMedia::playSound(soundType type)
{
	QSoundEffect *soundEffect = this->m_mapSoundEffect.value(type);
	soundEffect->play();
}
void GameMedia::playBGM(BGMtype type)
{
	QString resourcePath = "qrc:/soundWav/WAVE/SPACE_BG.wav";
	// ... switch 逻辑保持不变 ...

	this->m_mediaPlayerBG->pause();

	// Qt5 中获取当前媒体路径的方法
	if (this->m_mediaPlayerBG->media().canonicalUrl().toString() != resourcePath)
	{
		// Qt5 使用 setMedia 而不是 setSource
		this->m_mediaPlayerBG->setMedia(QUrl(resourcePath));
	}
	this->m_mediaPlayerBG->play();
}
void GameMedia::pauseBGM()
{
	this->m_mediaPlayerBG->pause();
}
