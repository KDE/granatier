/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef BOMBEXPLOSIONITEM_H
#define BOMBEXPLOSIONITEM_H

#include "granatierglobals.h"

#include <KGameRenderedItem>

class Bomb;
class KGameRenderer;

/**
 * @brief This class is the graphical representation of a Bomb explosion.
 */
class BombExplosionItem : public QObject, public KGameRenderedItem
{

Q_OBJECT

public:

protected:

    /** The direction of the blast */
    Granatier::Direction::Type m_direction;

    /** The bomb power */
    int m_bombPower;

    /** The ID of the Bomb that causes the explosion */
    int m_explosionID;

    /** The scale factor from the svg for the pixmap */
    qreal m_svgScaleFactor;

    QSize m_itemSizeSet;
    QSize m_itemSizeReal;

public:

    /**
     * Creates a new BombExplosionItem instance.
     * @param p_model the Bomb model
     * @param direction the dirction of the explosion
     * @param bombPower the power of the bomb at the position of this BombExplosionItem
     * @param renderer the KGameRenderer
     * @param svgScaleFactor the scale factor of the pixmap
     */
    BombExplosionItem(Bomb* p_model, Granatier::Direction::Type direction, int bombPower, KGameRenderer* renderer, qreal svgScaleFactor);

    /**
     * Deletes the BombExplosionItem instance.
     */
    ~BombExplosionItem();

    /**
     * Overrides the default shape function to make it a small circle
     * This function is used to determinate collision between items
     * @return QPainterPath the new shape of the Bomb
     */
    QPainterPath shape() const override;

    /**
    * @return the explosion ID
    */
    int explosionID();

    /**
     * Updates the BombExplosionItem coordinates.
     * @param p_x the new x-coordinate
     * @param p_y the new y-coordinate
     */
    virtual void setPosition(qreal p_x, qreal p_y);

    /**
     * updates the animation
     * @param nFrame the next animation frame
     */
     void updateAnimationn(int nFrame);

public Q_SLOTS:

    /**
     * Updates the graphics after a resize
     * @param svgScaleFactor the scaling factor between svg and rendered pixmap
     */
    virtual void updateGraphics(qreal svgScaleFactor);
};

#endif

