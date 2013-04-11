#include "general.h"
#include "engine.h"
#include "skill.h"
#include "package.h"
#include "scenario.h"
#include "client.h"

#include <QSize>
#include <QFile>

General::General(Package *package, const QString &name, const QString &kingdom, int max_hp, bool male, bool hidden, bool never_shown)
    :QObject(package), kingdom(kingdom), max_hp(max_hp), gender(male ? Male : Female), hidden(hidden), never_shown(never_shown)
{
    static QChar lord_symbol('$');
    if(name.contains(lord_symbol)){
        QString copy = name;
        copy.remove(lord_symbol);
        lord = true;
        setObjectName(copy);
    }else{
        lord = false;
        setObjectName(name);
    }

    kmap["wei"] = "guan";
    kmap["shu"] = "jiang";
    kmap["wu"] = "min";
    kmap["qun"] = "kou";
}

int General::getMaxHp() const{
    return max_hp;
}

QString General::getKingdom(bool unmap) const{
    if(unmap)
        return kingdom;
    QString m_kingdom = kmap.value(kingdom, QString());
    if(!m_kingdom.isNull())
        return m_kingdom;
    else
        return kingdom;
}

void General::setGender(Gender gender){
    this->gender = gender;
}

void General::setGenderString(const QString &sex){
    Gender gender;
    if(sex == "male")
        gender = Male;
    else if(sex == "female")
        gender = Female;
    else
        gender = Neuter;
    setGender(gender);
}

QString General::getGenderString() const{
    switch(gender){
    case Male: return "male";
    case Female: return "female";
    default:
        return "neuter";
    }
}

QString General::getId() const{
    QString id = Sanguosha->translate("$" + objectName());
    if(id.startsWith("$"))
        id = "";
    return id;
}

QString General::getNickname(bool full) const{
    QString nick = Sanguosha->translate("#" + objectName());
    if(nick.startsWith("#"))
        nick = "";
    if(full)
        nick.append(Sanguosha->translate(objectName()));
    return nick;
}

QString General::getShowHp() const{
    QString max_hp = QString::number(getMaxHp());
    for(int n = 1; n <= 3; n++){
        if(hasSkill("#hp-" + QString::number(n))){
            max_hp = QString::number(getMaxHp() - n) + "/" + QString::number(getMaxHp());
            break;
        }
    }
    return max_hp;
}

bool General::isLuaGeneral() const{
    const Package *package = Sanguosha->findChild<const Package *>(getPackage());
    return package->getGenre() == Package::LUA;
}

QString General::getPixmapPath(const QString &category) const{
    QString suffix = category == "card" ? "jpg" : "png";

    QString path = QString("image/generals/%1/%2.%3").arg(category).arg(objectName()).arg(suffix);
    if(!QFile::exists(path))
        path = QString("extensions/generals/%1/%2.%3").arg(category).arg(objectName()).arg(suffix);
    return path;
}

void General::addSkill(Skill *skill){
    skill->setParent(this);
    skill_set << skill->objectName();
}

void General::addSkill(const QString &skill_name){
    extra_set << skill_name;
}

bool General::hasSkill(const QString &skill_name) const{
    return skill_set.contains(skill_name) || extra_set.contains(skill_name);
}

QList<const Skill *> General::getVisibleSkillList() const{
    QList<const Skill *> skills;
    foreach(const Skill *skill, findChildren<const Skill *>()){
        if(skill->isVisible())
            skills << skill;
    }

    foreach(QString skill_name, extra_set){
        const Skill *skill = Sanguosha->getSkill(skill_name);
        if(!skill)
            skill = new Skill(skill_name);
        if(skill && skill->isVisible())
            skills << skill;
    }

    return skills;
}

QSet<const Skill *> General::getVisibleSkills() const{
    QSet<const Skill *> skills;
    foreach(const Skill *skill, findChildren<const Skill *>()){
        if(skill->isVisible())
            skills << skill;
    }

    foreach(QString skill_name, extra_set){
        const Skill *skill = Sanguosha->getSkill(skill_name);
        if(skill->isVisible())
            skills << skill;
    }

    return skills;
}

QSet<const TriggerSkill *> General::getTriggerSkills() const{
    QSet<const TriggerSkill *> skills = findChildren<const TriggerSkill *>().toSet();

    foreach(QString skill_name, extra_set){
        const TriggerSkill *skill = Sanguosha->getTriggerSkill(skill_name);
        if(skill)
            skills << skill;
    }

    return skills;
}

void General::addRelateSkill(const QString &skill_name){
    related_skills << skill_name;
}

QStringList General::getRelatedSkillNames() const{
    return related_skills;
}

QString General::getPackage() const{
    QObject *p = parent();
    if(p)
        return p->objectName();
    else
        return QString(); // avoid null pointer exception;
}

QString General::getSkillDescription() const{
    QString description;

    foreach(const Skill *skill, getVisibleSkillList()){
        QString skill_name = Sanguosha->translate(skill->objectName());
        QString desc = skill->getDescription();
        desc.replace("\n", "<br/>");
        description.append(QString("<b>%1</b>: %2 <br/> <br/>").arg(skill_name).arg(desc));
    }

    return description;
}

void General::lastWord() const{
    QString filename = QString("audio/death/%1.dat").arg(objectName());
    //if(!QFile::exists(filename))
    //    filename = QString("audio/death/%1.ogg").arg(objectName());
    if(!QFile::exists(filename))
        filename = QString("extensions/audio/death/%1.ogg").arg(objectName());
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)){
        QStringList origin_generals = objectName().split("_");
        if(origin_generals.length()>1){
            filename = QString("audio/death/%1.dat").arg(origin_generals.at(1));
            if(!QFile::exists(filename))
                filename = QString("audio/death/%1.ogg").arg(origin_generals.at(1));
        }
    }
    /*if(!file.open(QIODevice::ReadOnly) && objectName().endsWith("f")){
        QString origin_general = objectName();
        origin_general.chop(1);
        if(Sanguosha->getGeneral(origin_general)){
            filename = QString("audio/death/%1.dat").arg(origin_general);
            if(!QFile::exists(filename))
                filename = QString("audio/death/%1.ogg").arg(origin_general);
        }
    }
    */
    Sanguosha->playEffect(filename);
}

void General::winWord() const{
    QString filename = QString("audio/win/%1.dat").arg(objectName());
    QFile file(filename);
    if(!file.open(QIODevice::ReadOnly)){
        QStringList origin_generals = objectName().split("_");
        if(origin_generals.length()>1)
            filename = QString("audio/win/%1.dat").arg(origin_generals.at(1));
    }
    if(!file.open(QIODevice::ReadOnly) && objectName().endsWith("f")){
        QString origin_general = objectName();
        origin_general.chop(1);
        if(Sanguosha->getGeneral(origin_general))
            filename = QString("audio/win/%1.dat").arg(origin_general);
    }
    Sanguosha->playEffect(filename);
}

QString General::getLastword() const{
    QString general_name = objectName();
    QString last_word = Sanguosha->translate("~" + general_name);
    if(last_word.startsWith("~")){
        QStringList origin_generals = general_name.split("_");
        if(origin_generals.length()>1)
            last_word = Sanguosha->translate(("~") +  origin_generals.at(1));
    }

    if(last_word.startsWith("~") && general_name.endsWith("f")){
        QString origin_general = general_name;
        origin_general.chop(1);
        if(Sanguosha->getGeneral(origin_general))
            last_word = Sanguosha->translate(("~") + origin_general);
    }
    return last_word;
}

QString General::getWinword() const{
    QString general_name = objectName();
    QString win_word = Sanguosha->translate("`" + general_name);
    if(win_word.startsWith("`")){
        QStringList origin_generals = general_name.split("_");
        if(origin_generals.length()>1)
            win_word = Sanguosha->translate(("`") +  origin_generals.at(1));
    }

    if(win_word.startsWith("`") && general_name.endsWith("f")){
        QString origin_general = general_name;
        origin_general.chop(1);
        if(Sanguosha->getGeneral(origin_general))
            win_word = Sanguosha->translate(("`") + origin_general);
    }
    return win_word;
}

QSize General::BigIconSize(94, 96);
QSize General::SmallIconSize(122, 50);
QSize General::TinyIconSize(42, 36);
