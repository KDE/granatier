/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ELEMENTITEM_H
#define ELEMENTITEM_H

#include <KGameRenderedItem>

class Element;
class KGameRenderer;

/**
 * @brief This class is the graphical representation of a game Element.
 */
class ElementItem : public QObject, public KGameRenderedItem
{

Q_OBJECT

private:

protected:

    /** The instance of Element the ElementItem will represent */
    Element* m_model;

    QSize m_renderSize;

    QSize m_itemSizeSet;
    QSize m_itemSizeReal;

public:

    /**
      * Creates a new ElementItem instance.
      * @param p_model the Element model
      * @param renderer the KGameRenderer
      */
    ElementItem(Element* p_model, KGameRenderer* renderer);

    /**
      * Deletes the ElementItem instance.
      */
    ~ElementItem() override;

    /**
      * Gets the Element model.
      * @return the model
      */
    Element* getModel() const;

    /**
      * Reimplement QGraphicsItem::shape() to return an ellipse to improve collisions.
      */
    QPainterPath shape() const override;

public Q_SLOTS:

    /**
      * Updates the ElementItem coordinates.
      * @param p_x the new x-coordinate
      * @param p_y the new y-coordinate
      */
    virtual void update(qreal p_x, qreal p_y);

    /**
     * Updates the graphics after a resize
     * @param svgScaleFactor the scaling factor between svg and rendered pixmap
     */
    virtual void updateGraphics(qreal svgScaleFactor);

protected:
    virtual void updateGraphicsInternal(qreal svgScaleFactor);
};

#endif

