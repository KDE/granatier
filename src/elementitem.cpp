/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "elementitem.h"
#include "element.h"

#include <QGraphicsView>

#include <KGameRenderer>

ElementItem::ElementItem(Element* p_model, KGameRenderer* renderer) : KGameRenderedItem(renderer, QStringLiteral(""))
{
    m_model = p_model;
    // Init the view coordinates
    update(p_model->getX(), p_model->getY());
    // Connects the model to the view
    connect(p_model, &Element::moved, this, &ElementItem::update);

    m_renderSize = QSize(1, 1);
    m_itemSizeSet = QSize(static_cast<int>(Granatier::CellSize), static_cast<int>(Granatier::CellSize));
    m_itemSizeReal = m_itemSizeSet;
}

ElementItem::~ElementItem()
= default;

Element* ElementItem::getModel() const
{
    return m_model;
}

QPainterPath ElementItem::shape() const
{
    QPainterPath path;
    path.addEllipse(boundingRect());
    return path;
}

void ElementItem::update(qreal p_x, qreal p_y)
{
    // Compute the top-right coordinates of the item
    qreal x = p_x - m_itemSizeSet.width() / 2;
    qreal y = p_y - m_itemSizeSet.height() / 2;

    // Updates the view coordinates
    setPos(x, y);
}

void ElementItem::updateGraphics(qreal svgScaleFactor)
{
    updateGraphicsInternal(svgScaleFactor);
}

void ElementItem::updateGraphicsInternal(qreal svgScaleFactor)
{
    if(scene()->views().isEmpty())
    {
        return;
    }

    //calculate the size of the item on the view
    QPoint viewTopLeft = scene()->views().first()->mapFromScene(0, 0);
    QPoint viewBottomRight = scene()->views().first()->mapFromScene(m_itemSizeSet.width(), m_itemSizeSet.height());

    int viewWidth = viewBottomRight.x() - viewTopLeft.x();
    int viewHeight = viewBottomRight.y() - viewTopLeft.y();

    //for better alignment, if the item size is different from the cell size, make the difference between the cell size and item size always even
    if(m_itemSizeSet.width() != Granatier::CellSize || m_itemSizeSet.height() != Granatier::CellSize)
    {
        viewBottomRight = scene()->views().first()->mapFromScene(Granatier::CellSize, Granatier::CellSize);
        int viewCellWidth = viewBottomRight.x() - viewTopLeft.x();
        int viewCellHeight = viewBottomRight.y() - viewTopLeft.y();
        if((viewCellWidth - viewWidth) % 2 != 0)
        {
            viewWidth--;
        }
        if((viewCellHeight - viewHeight) % 2 != 0)
        {
            viewHeight--;
        }

        //calculate the real item size after change of the render size
        QPointF sceneTopLeft = scene()->views().first()->mapToScene(0, 0);
        QPointF sceneBottomRight = scene()->views().first()->mapToScene(viewWidth, viewHeight);

        int sceneWidth = static_cast<int>(sceneBottomRight.x() - sceneTopLeft.x());
        int sceneHeight = static_cast<int>(sceneBottomRight.y() - sceneTopLeft.y());
        m_itemSizeReal = QSize(sceneWidth, sceneHeight);
    }

    setRenderSize(QSize(viewWidth, viewHeight));
    setScale(svgScaleFactor);
    m_renderSize = renderSize();
}

#include "moc_elementitem.cpp"
