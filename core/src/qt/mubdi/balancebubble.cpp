/***********************************************************************
***************Copyright (c) 2020 The PIVX developers*******************
******************Copyright (c) 2010-2023 Nur1Labs**********************
>Distributed under the MIT software license, see the accompanying
>file COPYING or https://www.opensource.org/licenses/mit-license.php.
************************************************************************/

#include "qt/mubdi/balancebubble.h"
#include "qt/mubdi/forms/ui_balancebubble.h"

#include "qt/mubdi/qtutils.h"

#include <QGraphicsOpacityEffect>
#include <QPropertyAnimation>
#include <QTimer>
#include <qt/mubdiunits.h>

BalanceBubble::BalanceBubble(QWidget *parent) :
        QWidget(parent),
        ui(new Ui::BalanceBubble)
{
    ui->setupUi(this);

    ui->frame->setProperty("cssClass", "container-popup");
    setCssProperty({ui->textTransparent, ui->textShielded}, "amount-small-popup");

    std::initializer_list<QWidget*> lblTitles = {ui->lblFirst, ui->lblSecond};
    setCssProperty(lblTitles, "text-title-topbar");
    QFont font;
    font.setWeight(QFont::Light);
    for (QWidget* w : lblTitles) { w->setFont(font); }
}

void BalanceBubble::updateValues(int64_t nTransparentBalance, int unit)
{
    QString valueTrans = MuBdIUnits::formatWithUnit(unit, nTransparentBalance, false, MuBdIUnits::separatorAlways);
    valueTrans = valueTrans.replace(QChar(THIN_SP_CP), QString(","));
    QString valueShield = MuBdIUnits::formatWithRLF(unit, nTransparentBalance * 2000, false, MuBdIUnits::separatorAlways);
    valueShield = valueShield.replace(QChar(THIN_SP_CP), QString(","));

    ui->textTransparent->setText(valueTrans);
    ui->textShielded->setText(valueShield);
    adjustSize();
}

void BalanceBubble::showEvent(QShowEvent *event)
{
    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(this);
    this->setGraphicsEffect(eff);
    QPropertyAnimation *anim = new QPropertyAnimation(eff,"opacity");
    anim->setDuration(400);
    anim->setStartValue(0);
    anim->setEndValue(1);
    anim->setEasingCurve(QEasingCurve::Linear);
    anim->start(QPropertyAnimation::DeleteWhenStopped);

    if (!hideTimer) hideTimer = new QTimer(this);
    connect(hideTimer, &QTimer::timeout, this, &BalanceBubble::hideTimeout);
    hideTimer->start(7000);
}

void BalanceBubble::hideEvent(QHideEvent *event)
{
    if (hideTimer) hideTimer->stop();
    QGraphicsOpacityEffect *eff = new QGraphicsOpacityEffect(this);
    this->setGraphicsEffect(eff);
    QPropertyAnimation *a = new QPropertyAnimation(eff,"opacity");
    a->setDuration(800);
    a->setStartValue(1);
    a->setEndValue(0);
    a->setEasingCurve(QEasingCurve::OutBack);
    a->start(QPropertyAnimation::DeleteWhenStopped);
}

void BalanceBubble::hideTimeout()
{
    hide();
}

BalanceBubble::~BalanceBubble()
{
    delete ui;
}
