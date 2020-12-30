/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "bombitem.h"
#include "bomb.h"
#include "settings.h"

#include <QTimer>
#include <QGraphicsView>

#include <KGameRenderer>

const int nBombPulseTime = 100;

BombItem::BombItem(Bomb* p_model, KGameRenderer* renderer) : ElementItem (p_model, renderer)
{
    setSpriteKey(QStringLiteral("bomb"));
    setZValue(210);
    connect(p_model, &Bomb::bombDetonated, this, &BombItem::startDetonation);
    connect(p_model, &Bomb::falling, this, &BombItem::fallingAnimation);
    connect(this, &BombItem::bombItemFinished, p_model, &Bomb::slot_detonationCompleted);

    int width = static_cast<int>(Granatier::CellSize * 0.9);
    int height = static_cast<int>(Granatier::CellSize * 0.9);
    if((static_cast<int>(Granatier::CellSize) - width) % 2 != 0)
    {
        width--;
    }
    if((static_cast<int>(Granatier::CellSize) - height) % 2 != 0)
    {
        height--;
    }
    m_itemSizeSet = QSize(width, height);
    m_itemSizeReal = m_itemSizeSet;

    m_animationCounter = 0;
    // Define the timer which sets the puls frequency
    m_pulseTimer = new QTimer(this);
    m_pulseTimer->setInterval(nBombPulseTime);
    m_pulseTimer->start();
    connect(m_pulseTimer, &QTimer::timeout, this, &BombItem::pulse);

    m_explosionTimer = nullptr;
    m_listExplosionTiming.append(Settings::self()->blastTime1());
    m_listExplosionTiming.append(Settings::self()->blastTime2());
    m_listExplosionTiming.append(Settings::self()->blastTime3());
    m_listExplosionTiming.append(Settings::self()->blastTime4());
    m_listExplosionTiming.append(Settings::self()->blastTime5());

    m_fallingAnimationCounter = 0;
    m_dudBomb = false;
}

BombItem::~BombItem()
{
    delete m_pulseTimer;
    delete m_explosionTimer;
}

QPainterPath BombItem::shape() const
{
    QPainterPath path;
    // Temporary variable to keep the boundingRect available
    QRectF rect = boundingRect();

    // Calculation of the shape
    QRectF shapeRect = QRectF( rect.x()+rect.width()/8, rect.y()+rect.height()/8, rect.width()*3.0/4.0, rect.height()*3.0/4.0 );
    path.addEllipse(shapeRect);
    return path;
}

void BombItem::pauseAnim()
{
    dynamic_cast <Bomb*> (m_model)->pause();
    if(m_pulseTimer)
    {
        m_pulseTimer->stop();
    }
    if(m_explosionTimer)
    {
        m_explosionTimer->stop();
    }
}

void BombItem::resumeAnim()
{
    if(m_pulseTimer && !m_dudBomb)
    {
        m_pulseTimer->start();
    }
    if(m_explosionTimer)
    {
        m_explosionTimer->start();
    }
    dynamic_cast <Bomb*> (m_model)->resume();
}

void BombItem::update(qreal p_x, qreal p_y)
{
    // Compute the top-right coordinates of the item
    qreal x = p_x - m_itemSizeSet.width() / 2;
    qreal y = p_y - m_itemSizeSet.height() / 2;
    // Updates the view coordinates
    setPos(x, y);
    m_x = static_cast<int>(p_x);
    m_y = static_cast<int>(p_y);
}

void BombItem::startDetonation()
{
    if(m_pulseTimer)
    {
        m_pulseTimer->stop();
        delete m_pulseTimer;
        m_pulseTimer = nullptr;
    }
    m_animationCounter = 0;
    QTransform transform;
    transform.translate(m_itemSizeSet.width() / 2.0, m_itemSizeSet.height() / 2.0);
    setRenderSize(m_renderSize);
    transform.translate(-m_itemSizeReal.width() / 2.0, -m_itemSizeReal.height() / 2.0);
    setTransform(transform);
    dynamic_cast <Bomb*> (m_model)->setXSpeed(0);
    dynamic_cast <Bomb*> (m_model)->setYSpeed(0);

    // Define the timer which sets the explosion frequency
    m_explosionTimer = new QTimer(this);
    m_explosionTimer->setInterval(m_listExplosionTiming.at(0));
    m_explosionTimer->setSingleShot(true);
    m_explosionTimer->start();
    connect(m_explosionTimer, &QTimer::timeout, this, &BombItem::updateAnimation);

    int width = static_cast<int>(Granatier::CellSize * 1.1);
    int height = static_cast<int>(Granatier::CellSize * 1.1);
    if((static_cast<int>(Granatier::CellSize) - width) % 2 != 0)
    {
        width--;
    }
    if((static_cast<int>(Granatier::CellSize) - height) % 2 != 0)
    {
        height--;
    }
    m_itemSizeSet = QSize(width, height);
    m_itemSizeReal = m_itemSizeSet;
    setSpriteKey(QStringLiteral("bomb_blast_core_0"));
    setZValue(300+15); //300+maxBombPower+5
    updateGraphics(scale());
    update(m_x, m_y);
}

void BombItem::pulse()
{
    if(m_fallingAnimationCounter == 0)
    {
        m_animationCounter++;
        if (m_animationCounter % 2 == 0)
        {
            m_animationCounter = 0;
            int viewWidth = static_cast<int>(m_renderSize.width() * 0.98);
            int viewHeight = static_cast<int>(m_renderSize.height() * 0.98);
            if((m_renderSize.width() - viewWidth) % 2 != 0)
            {
                viewWidth--;
            }
            if((m_renderSize.height() - viewHeight) % 2 != 0)
            {
                viewHeight--;
            }

            //calculate the real item size after change of the render size
            QPointF sceneTopLeft = scene()->views().first()->mapToScene(QPoint(0, 0));
            QPointF sceneBottomRight = scene()->views().first()->mapToScene(QPoint(viewWidth, viewHeight));

            qreal sceneWidth = sceneBottomRight.x() - sceneTopLeft.x();
            qreal sceneHeight = sceneBottomRight.y() - sceneTopLeft.y();

            // shrink the item
            QTransform transform;
            transform.translate(m_itemSizeSet.width() / 2.0, m_itemSizeSet.height() / 2.0);
            setRenderSize(QSize(viewWidth, viewHeight));
            transform.translate(-sceneWidth / 2.0, -sceneHeight / 2.0);
            setTransform(transform);
        }
        else
        {
            QTransform transform;
            transform.translate(m_itemSizeSet.width() / 2.0, m_itemSizeSet.height() / 2.0);
            setRenderSize(m_renderSize);
            transform.translate(-m_itemSizeReal.width() / 2.0, -m_itemSizeReal.height() / 2.0);
            setTransform(transform);
        }
    }
    else
    {
        // shrink the item
        QTransform transform;
        transform.translate(m_itemSizeSet.width() / 2.0, m_itemSizeSet.height() / 2.0);
        setRenderSize(m_renderSize * (1-m_fallingAnimationCounter*0.02));
        transform.translate(-m_itemSizeReal.width() * (1-m_fallingAnimationCounter*0.02) / 2.0, -m_itemSizeReal.height() * (1-m_fallingAnimationCounter*0.02) / 2.0);
        setTransform(transform);
        m_fallingAnimationCounter++;

        if(m_fallingAnimationCounter > 50)
        {
            m_pulseTimer->stop();
            m_dudBomb = true;
            Q_EMIT bombItemFinished(this);
        }
    }
}

void BombItem::updateAnimation()
{
    m_animationCounter++;
    if (m_animationCounter > 4)
    {
        Q_EMIT bombItemFinished(this);
        m_animationCounter = 0;
        return;
    }
    QString strElemetId = QStringLiteral("bomb_blast_core_%1").arg(m_animationCounter);
    setSpriteKey(strElemetId);
    updateGraphics(scale());
    update(m_x, m_y);

    Q_EMIT animationFrameChanged(this, m_animationCounter);
    m_explosionTimer->setInterval(m_listExplosionTiming.at(m_animationCounter));
    m_explosionTimer->start();
}

void BombItem::updateMortar(int nMortarState, int nMortarRampEnd, int nMortarPeak, int nMortarGround)
{
    if(m_pulseTimer)
    {
        m_pulseTimer->stop();
        delete m_pulseTimer;
        m_pulseTimer = nullptr;
    }

    if(nMortarState <= 0)
    {
        setVisible(false);
        setZValue(-1);
    }
    else if(nMortarState <= nMortarGround)
    {
        updateMortarAnimation(nMortarState, nMortarRampEnd, nMortarPeak);
    }
    else
    {
        if(!m_pulseTimer)
        {
            m_pulseTimer = new QTimer(this);
            m_pulseTimer->setInterval(nBombPulseTime);
            m_animationCounter = 1; // set to one, to start pulsing with a small bomb
            m_pulseTimer->start();
            connect(m_pulseTimer, &QTimer::timeout, this, &BombItem::pulse);
        }
        QTransform transform;
        transform.translate(m_itemSizeSet.width() / 2.0, m_itemSizeSet.height() / 2.0);
        setRenderSize(m_renderSize);
        transform.translate(-m_itemSizeReal.width() / 2.0, -m_itemSizeReal.height() / 2.0);
        setTransform(transform);
        setVisible(true);
        setZValue(210);
    }
}

void BombItem::updateMortarAnimation(int animationCounter, int nMortarRampEnd, int nMortarPeak)
{
    qreal mortarScale = 1;
    int mortarZValue = 210;
    QTransform transform;

    if(animationCounter < nMortarRampEnd)
    {
        mortarZValue = 210;
    }
    else
    {
        mortarZValue = 800;
    }

    int frame = animationCounter - nMortarRampEnd;
    int peak = nMortarPeak - nMortarRampEnd;

    mortarScale = 1.5 - (frame-peak) * (frame-peak) / static_cast<qreal>(peak*peak) * 0.5;

    transform.translate(m_itemSizeSet.width() / 2.0, m_itemSizeSet.height() / 2.0);
    setRenderSize(m_renderSize * mortarScale);
    transform.translate(-m_itemSizeReal.width() * mortarScale / 2.0, -m_itemSizeReal.height() * mortarScale / 2.0);
    setTransform(transform);

    setVisible(true);
    setZValue(mortarZValue);
}

void BombItem::fallingAnimation()
{
    m_fallingAnimationCounter = 1;
    // set z-value below the ground
    setZValue(-2);
    m_pulseTimer->setInterval(1000 / Granatier::FPS);
}
