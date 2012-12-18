#ifndef WHEELFIGHTSCENARIO_H
#define WHEELFIGHTSCENARIO_H

#include "scenario.h"
#include "roomthread.h"
#include <QLineEdit>

class WheelFightScenario : public Scenario{
    Q_OBJECT

public:
    explicit WheelFightScenario();

    virtual void assign(QStringList &generals, QStringList &roles) const;
    virtual int getPlayerCount() const;
    virtual void getRoles(char *roles) const;
    virtual int lordGeneralCount() const;
    virtual QWidget *getAdvancePage() const;

private:
    QLineEdit *wheel_count;

private slots:
    void apply();
};

#endif // WHEELFIGHTSCENARIO_H
