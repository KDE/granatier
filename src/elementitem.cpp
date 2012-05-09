/*
 * Copyright 2009 Mathias Kraus <k.hias@gmx.de>
 * Copyright 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of 
 * the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "elementitem.h"
#include "element.h"

#include <QGraphicsView>

#include <KGameRenderer>

ElementItem::ElementItem(Element* p_model, KGameRenderer* renderer) : KGameRenderedItem(renderer, "")
{
    m_model = p_model;
    // Init the view coordinates
    update(p_model->getX(), p_model->getY());
    // Connects the model to the view
    connect(p_model, SIGNAL(moved(qreal,qreal)), this, SLOT(update(qreal,qreal)));
    
    m_renderSize = QSize(1, 1);
}

ElementItem::~ElementItem()
{
}

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
    qreal x = p_x - renderer()->boundsOnSprite(spriteKey()).width() / 2;
    qreal y = p_y - renderer()->boundsOnSprite(spriteKey()).height() / 2;

    // Updates the view coordinates
    setPos(x, y);
}

void ElementItem::updateGraphics(qreal svgScaleFactor)
{
    if(scene()->views().isEmpty())
    {
        return;
    }
    
    QSize svgSize = renderer()->boundsOnSprite(spriteKey()).size().toSize();
    
    QPoint topLeft(0, 0);
    topLeft = scene()->views().at(0)->mapFromScene(topLeft);
    
    QPoint bottomRight(svgSize.width(), svgSize.height()); 
    bottomRight = scene()->views().at(0)->mapFromScene(bottomRight);
    
    svgSize.setHeight(bottomRight.y() - topLeft.y());
    svgSize.setWidth(bottomRight.x() - topLeft.x());
    
    setRenderSize(svgSize);
    setScale(svgScaleFactor);
    m_renderSize = svgSize;
}
