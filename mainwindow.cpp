#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDateTime>
#include <QToolButton>
#include <QSize>
#include <QList>
#include <QIcon>
#include <QRandomGenerator>
#include <QNetworkRequest>
#include <QUrl>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFileDialog>
#include <QUrl>
#include <QDir>
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , timer(nullptr)
    , envTimer(nullptr)
    , weatherManager(nullptr)
    , videoPlayer(nullptr)
    , musicPlayer(nullptr)
    , musicPlaylist(nullptr)
    , musicDuration(0)
{
    ui->setupUi(this);

    initHomePage();
    initPageSwitch();
    initEnvPage();
    initWeatherPage();
    initVideoPage();
    initMusicPage();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::initHomePage()
{
    ui->titleLabel->setText("智能车载终端");

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateTime);
    timer->start(1000);
    updateTime();

    QList<QToolButton *> buttons = {
        ui->videoButton,
        ui->musicButton,
        ui->envButton,
        ui->weatherButton,
        ui->cameraButton
    };

    for (QToolButton *button : buttons) {
        button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
        button->setIconSize(QSize(64, 64));
        button->setMinimumSize(130, 110);
    }

    ui->videoButton->setText("视频播放器");
    ui->musicButton->setText("音乐播放器");
    ui->envButton->setText("环境检测");
    ui->weatherButton->setText("天气定位");
    ui->cameraButton->setText("倒车影像");
    ui->videoButton->setIcon(QIcon(":/icons/video.png"));
    ui->musicButton->setIcon(QIcon(":/icons/music.png"));
    ui->envButton->setIcon(QIcon(":/icons/env.png"));
    ui->weatherButton->setIcon(QIcon(":/icons/weather.png"));
    ui->cameraButton->setIcon(QIcon(":/icons/camera.png"));
}

void MainWindow::updateTime()
{
    QString timeText = QDateTime::currentDateTime()
                           .toString("yyyy-MM-dd hh:mm:ss dddd");
    ui->timeLabel->setText(timeText);
}
void MainWindow::initPageSwitch()
{
    ui->stackedWidget->setCurrentWidget(ui->homePage);

    connect(ui->videoButton, &QToolButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentWidget(ui->videoPage);
    });

    connect(ui->musicButton, &QToolButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentWidget(ui->musicPage);
    });

    connect(ui->envButton, &QToolButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentWidget(ui->envPage);
        envTimer->start(1000);
        updateEnvData();
    });

    connect(ui->weatherButton, &QToolButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentWidget(ui->weatherPage);
        requestWeather();
    });

    connect(ui->cameraButton, &QToolButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentWidget(ui->cameraPage);
    });

    connect(ui->videoBackButton, &QPushButton::clicked, this, [=]() {
        videoPlayer->stop();
        ui->stackedWidget->setCurrentWidget(ui->homePage);
    });

    connect(ui->musicBackButton, &QPushButton::clicked, this, [=]() {
        musicPlayer->stop();
        ui->stackedWidget->setCurrentWidget(ui->homePage);
    });

    connect(ui->envBackButton, &QPushButton::clicked, this, [=]() {
        envTimer->stop();
        ui->stackedWidget->setCurrentWidget(ui->homePage);
    });

    connect(ui->weatherBackButton, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentWidget(ui->homePage);
    });

    connect(ui->cameraBackButton, &QPushButton::clicked, this, [=]() {
        ui->stackedWidget->setCurrentWidget(ui->homePage);
    });
}
void MainWindow::initEnvPage()
{
    envTimer = new QTimer(this);

    connect(envTimer, &QTimer::timeout,
            this, &MainWindow::updateEnvData);

    updateEnvData();
}
void MainWindow::updateEnvData()
{
    double temp = QRandomGenerator::global()->bounded(200, 360) / 10.0;
    int humidity = QRandomGenerator::global()->bounded(35, 85);
    int light = QRandomGenerator::global()->bounded(100, 900);
    int air = QRandomGenerator::global()->bounded(20, 160);

    ui->tempLabel->setText(QString("温度：%1 ℃").arg(temp, 0, 'f', 1));
    ui->humidityLabel->setText(QString("湿度：%1 %").arg(humidity));
    ui->lightLabel->setText(QString("光照：%1 Lux").arg(light));
    ui->airLabel->setText(QString("空气质量：%1").arg(air));

    if (temp > 32.0 || air > 120) {
        ui->alarmLabel->setText("报警状态：异常");
        ui->alarmLabel->setStyleSheet("color: red; font-weight: bold;");
    } else {
        ui->alarmLabel->setText("报警状态：正常");
        ui->alarmLabel->setStyleSheet("color: green; font-weight: bold;");
    }
}
void MainWindow::initWeatherPage()
{
    weatherManager = new QNetworkAccessManager(this);

    amapKey = "6548e16e2a7798e427aad74dfa0600b2";
    weatherCity = "430100";

    connect(weatherManager, &QNetworkAccessManager::finished,
            this, &MainWindow::handleWeatherReply);

    connect(ui->weatherRefreshButton, &QPushButton::clicked,
            this, &MainWindow::requestWeather);

    ui->weatherLocationLabel->setText("当前位置：长沙市");
    ui->weatherTextLabel->setText("天气：--");
    ui->weatherTempLabel->setText("温度：--");
    ui->weatherHumidityLabel->setText("湿度：--");
    ui->weatherWindLabel->setText("风向风力：--");
    ui->weatherReportTimeLabel->setText("发布时间：--");
}
void MainWindow::requestWeather()
{
    ui->weatherTextLabel->setText("天气：正在获取...");

    QString urlStr = QString("http://restapi.amap.com/v3/weather/weatherInfo"
                             "?key=%1&city=%2&extensions=base")
                         .arg(amapKey)
                         .arg(weatherCity);

    weatherManager->get(QNetworkRequest(QUrl(urlStr)));
}
void MainWindow::handleWeatherReply(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        ui->weatherTextLabel->setText("天气：网络请求失败");
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);

    if (parseError.error != QJsonParseError::NoError || !doc.isObject()) {
        ui->weatherTextLabel->setText("天气：数据解析失败");
        return;
    }

    QJsonObject root = doc.object();

    if (root.value("status").toString() != "1") {
        QString info = root.value("info").toString();
        ui->weatherTextLabel->setText("天气：请求失败 " + info);
        return;
    }

    QJsonArray lives = root.value("lives").toArray();
    if (lives.isEmpty()) {
        ui->weatherTextLabel->setText("天气：暂无数据");
        return;
    }

    QJsonObject live = lives.first().toObject();

    QString province = live.value("province").toString();
    QString city = live.value("city").toString();
    QString weather = live.value("weather").toString();
    QString temperature = live.value("temperature").toString();
    QString humidity = live.value("humidity").toString();
    QString windDirection = live.value("winddirection").toString();
    QString windPower = live.value("windpower").toString();
    QString reportTime = live.value("reporttime").toString();

    ui->weatherLocationLabel->setText("当前位置：" + province + " " + city);
    ui->weatherTextLabel->setText("天气：" + weather);
    ui->weatherTempLabel->setText("温度：" + temperature + " ℃");
    ui->weatherHumidityLabel->setText("湿度：" + humidity + " %");
    ui->weatherWindLabel->setText("风向风力：" + windDirection + "风 " + windPower + "级");
    ui->weatherReportTimeLabel->setText("发布时间：" + reportTime);
}
void MainWindow::initVideoPage()
{
    videoPlayer = new QMediaPlayer(this);
    videoPlayer->setVideoOutput(ui->videoWidget);

    connect(ui->openVideoButton, &QPushButton::clicked,
            this, &MainWindow::openVideoFile);

    connect(ui->playVideoButton, &QPushButton::clicked, this, [=]() {
        videoPlayer->play();
    });

    connect(ui->pauseVideoButton, &QPushButton::clicked, this, [=]() {
        videoPlayer->pause();
    });

    connect(ui->stopVideoButton, &QPushButton::clicked, this, [=]() {
        videoPlayer->stop();
    });
    ui->videoPositionSlider->setRange(0, 0);
    ui->videoVolumeSlider->setRange(0, 100);
    ui->videoVolumeSlider->setValue(50);
    videoPlayer->setVolume(50);

    connect(videoPlayer, &QMediaPlayer::durationChanged,
            this, &MainWindow::updateVideoDuration);

    connect(videoPlayer, &QMediaPlayer::positionChanged,
            this, &MainWindow::updateVideoPosition);

    connect(ui->videoPositionSlider, &QSlider::sliderReleased,
            this, &MainWindow::setVideoPosition);

    connect(ui->videoVolumeSlider, &QSlider::valueChanged,
            this, &MainWindow::updateVideoVolume);
}
void MainWindow::openVideoFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "选择视频文件",
        QDir::homePath(),
        "Video Files (*.mp4 *.avi *.mkv *.mov);;All Files (*)"
    );

    if (fileName.isEmpty()) {
        return;
    }
    videoPlayer->stop();
    ui->videoPositionSlider->setValue(0);
    ui->videoTimeLabel->setText("00:00 / 00:00");
    videoPlayer->setMedia(QUrl::fromLocalFile(fileName));
    videoPlayer->play();
}
QString MainWindow::formatTime(qint64 ms)
{
    int totalSeconds = ms / 1000;
    int minutes = totalSeconds / 60;
    int seconds = totalSeconds % 60;

    return QString("%1:%2")
        .arg(minutes, 2, 10, QChar('0'))
        .arg(seconds, 2, 10, QChar('0'));
}
void MainWindow::updateVideoDuration(qint64 duration)
{
    ui->videoPositionSlider->setRange(0, duration);
    ui->videoTimeLabel->setText(
        formatTime(videoPlayer->position()) + " / " + formatTime(duration)
    );
}
void MainWindow::updateVideoPosition(qint64 position)
{
    if (!ui->videoPositionSlider->isSliderDown()) {
        ui->videoPositionSlider->setValue(position);
    }

    ui->videoTimeLabel->setText(
        formatTime(position) + " / " + formatTime(videoPlayer->duration())
    );
}
void MainWindow::setVideoPosition()
{
    videoPlayer->setPosition(ui->videoPositionSlider->value());
}
void MainWindow::updateVideoVolume(int volume)
{
    videoPlayer->setVolume(volume);
}
void MainWindow::initMusicPage()
{
    musicPlayer = new QMediaPlayer(this);
    musicPlaylist = new QMediaPlaylist(this);

    musicPlayer->setPlaylist(musicPlaylist);

    ui->musicPositionSlider->setRange(0, 0);
    ui->musicVolumeSlider->setRange(0, 100);
    ui->musicVolumeSlider->setValue(50);
    musicPlayer->setVolume(50);

    connect(ui->openMusicButton, &QPushButton::clicked,
            this, &MainWindow::openMusicFiles);

    connect(ui->playMusicButton, &QPushButton::clicked, this, [=]() {
        musicPlayer->play();
    });

    connect(ui->pauseMusicButton, &QPushButton::clicked, this, [=]() {
        musicPlayer->pause();
    });

    connect(ui->stopMusicButton, &QPushButton::clicked, this, [=]() {
        musicPlayer->stop();
    });

    connect(ui->prevMusicButton, &QPushButton::clicked, this, [=]() {
        musicPlaylist->previous();
        musicPlayer->play();
    });

    connect(ui->nextMusicButton, &QPushButton::clicked, this, [=]() {
        musicPlaylist->next();
        musicPlayer->play();
    });

    connect(musicPlayer, &QMediaPlayer::durationChanged,
            this, &MainWindow::updateMusicDuration);

    connect(musicPlayer, &QMediaPlayer::positionChanged,
            this, &MainWindow::updateMusicPosition);

    connect(ui->musicPositionSlider, &QSlider::sliderReleased,
            this, &MainWindow::setMusicPosition);

    connect(ui->musicVolumeSlider, &QSlider::valueChanged,
            this, &MainWindow::updateMusicVolume);

    connect(ui->musicListWidget, &QListWidget::currentRowChanged,
            this, &MainWindow::playMusicFromList);

    connect(musicPlaylist, &QMediaPlaylist::currentIndexChanged,
            this, &MainWindow::updateMusicListCurrent);
}
void MainWindow::openMusicFiles()
{
    QStringList fileNames = QFileDialog::getOpenFileNames(
        this,
        "选择音乐文件",
        QDir::homePath(),
        "Audio Files (*.mp3 *.wav *.flac *.aac);;All Files (*)"
    );

    if (fileNames.isEmpty()) {
        return;
    }

    for (const QString &fileName : fileNames) {
        musicPlaylist->addMedia(QUrl::fromLocalFile(fileName));

        QFileInfo info(fileName);
        ui->musicListWidget->addItem(info.fileName());
    }

    if (musicPlaylist->currentIndex() < 0) {
        musicPlaylist->setCurrentIndex(0);
    }

    musicPlayer->play();
}
void MainWindow::playMusicFromList(int row)
{
    if (row < 0 || row >= musicPlaylist->mediaCount()) {
        return;
    }

    musicPlaylist->setCurrentIndex(row);
    musicPlayer->play();
}
void MainWindow::updateMusicListCurrent(int index)
{
    if (index >= 0 && index < ui->musicListWidget->count()) {
        ui->musicListWidget->setCurrentRow(index);
    }
}
void MainWindow::updateMusicDuration(qint64 duration)
{
    musicDuration = duration;
    ui->musicPositionSlider->setRange(0, static_cast<int>(duration));
    updateMusicTimeLabel();
}

void MainWindow::updateMusicPosition(qint64 position)
{
    if (!ui->musicPositionSlider->isSliderDown()) {
        ui->musicPositionSlider->setValue(static_cast<int>(position));
    }

    updateMusicTimeLabel();
}

void MainWindow::updateMusicTimeLabel()
{
    ui->musicTimeLabel->setText(
        formatTime(musicPlayer->position()) + " / " + formatTime(musicDuration)
    );
}
void MainWindow::setMusicPosition()
{
    musicPlayer->setPosition(ui->musicPositionSlider->value());
}
void MainWindow::updateMusicVolume(int volume)
{
    musicPlayer->setVolume(volume);
}
