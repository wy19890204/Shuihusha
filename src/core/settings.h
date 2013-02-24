#ifndef SETTINGS_H
#define SETTINGS_H

#include <QSettings>
#include <QFont>
#include <QRectF>
#include <QPixmap>
#include <QBrush>
#include <QDesktopWidget>

class Settings : public QSettings{
    Q_OBJECT

public:
    explicit Settings();
    void init();

    const QRectF Rect;
    QFont BigFont;
    QFont SmallFont;
    QFont TinyFont;

    QFont AppFont;
    QFont UIFont;
    QColor TextEditColor;

    // server side
    QString ServerName;
    int CountDownSeconds;
    QString GameMode;
    QStringList BanPackages;
    bool ContestMode;
    bool Statistic;
    bool ForbidSIMC;
    bool DisableChat;
    bool Enable2ndGeneral;
    bool NoLordSkill;
    bool EnableReincarnation;
    bool EnableScene;	//changjing
    bool EnableSame;
    bool EnableEndless;
    bool EnableAnzhan;
    bool EnableBasara;
    bool EnableHegemony;
    int MaxHpScheme;
    bool AnnounceIP;
    QString Address;
    bool FreeChooseGenerals;
    bool FreeChooseCards;
    bool FreeAssignSelf;
    bool EnableAI;
    int AIDelay;
    ushort ServerPort;

    // client side
    QString HostAddress;
    QString UserName;
    QString UserAvatar;
    QString Password;
    QStringList HistoryIPs;
    ushort DetectorPort;
    int MaxCards;

    bool CircularView;
    bool FitInView;
    bool EnableHotKey;
    bool EnableMinimizeDialog;
    bool NeverNullifyMyTrick;
    bool EnableAutoTarget;
    int NullificationCountDown;
    bool ShowAllName;
    bool SPOpen;
    int OperationTimeout;
    bool OperationNoLimit;
    bool EnableCardEffects;
    bool EnableEquipEffects;
    bool EnableSkillEffects;
    bool EnableLastWord;
    bool EnableCheatRing;
    bool EnableBgMusic;
    float BGMVolume;
    float EffectVolume;
    bool EnableLua;

    bool EnableSkillEmotion;
    bool DisableLightbox;

    QString BackgroundBrush;
    QStringList BanEmotions;
    QString translate(const QString &to_translate) const;

    // consts
    static const int S_CHOOSE_GENERAL_TIMEOUT;
    static const int S_GUANXING_TIMEOUT;
    static const int S_SURRNDER_REQUEST_MIN_INTERVAL;
    static const int S_MINI_MAX_COUNT;
    static const int S_MINI_STAGE_START;
};

extern Settings Config;

#endif // SETTINGS_H
