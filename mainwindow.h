#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTimer>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QMediaPlayer>
#include <QVideoWidget>
#include <QMediaPlaylist>
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateTime();
    void updateEnvData();
    void requestWeather();
    void handleWeatherReply(QNetworkReply *reply);
    void openVideoFile();
    void updateVideoDuration(qint64 duration);
    void updateVideoPosition(qint64 position);
    void setVideoPosition();
    void updateVideoVolume(int volume);
    void openMusicFiles();
    void updateMusicDuration(qint64 duration);
    void updateMusicPosition(qint64 position);
    void setMusicPosition();
    void updateMusicVolume(int volume);
    void playMusicFromList(int row);
    void updateMusicListCurrent(int index);

private:
    void initHomePage();
    void initPageSwitch();
    void initEnvPage();
    void initWeatherPage();
    void initVideoPage();
    QString formatTime(qint64 ms);
    qint64 videoDuration;
    void initMusicPage();
    void updateMusicTimeLabel();

private:
    Ui::MainWindow *ui;
    QTimer *timer;
    QTimer *envTimer;
    QNetworkAccessManager *weatherManager;
    QString amapKey;
    QString weatherCity;
    QMediaPlayer *videoPlayer;
    QMediaPlayer *musicPlayer;
    QMediaPlaylist *musicPlaylist;
    qint64 musicDuration;
};

#endif // MAINWINDOW_H
