/*
 * Copyright 2008 Mathias Kraus <k.hias@gmx.de>
 * Copyright 2007-2008 Nathalie Liesse <nathalie.liesse@gmail.com>
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

#include "mapparser.h"
#include "arena.h"
#include "granatier_random.h"

#include <cstdlib>
#include <QPointF>
#include <QDateTime>

MapParser::MapParser(Arena* p_arena)
{
    m_arena = p_arena;
    m_counterRows = 0;
}

MapParser::~MapParser()
{
}

bool MapParser::characters(const QString & ch)
{
    m_buffer = ch;
    return true;
}

bool MapParser::startElement(const QString&, const QString&, const QString& p_qName, const QXmlAttributes& p_atts)
{
    if (p_qName == QLatin1String("Arena"))
    {
        int nbRows = 0;
        int nbColumns = 0;
        // Initialize the number of rows and columns
        for (int i = 0; i < p_atts.count(); ++i)
        {
            if (p_atts.qName(i) == QLatin1String("rowCount"))
            {
                nbRows = p_atts.value(i).toInt();
            }
            if (p_atts.qName(i) == QLatin1String("colCount"))
            {
                nbColumns = p_atts.value(i).toInt();
            }
            //TODO:check for the right arenaFileVersion
            //if (p_atts.qName(i) == "arenaFileVersion")
            //{
            //    m_arenaFileVersion = p_atts.value(i).toInt();
            //}
        }
        // Create the Arena matrix
        m_arena->init(nbRows, nbColumns);
    }

    return true;
}

bool MapParser::endElement(const QString &, const QString &, const QString & p_qName)
{
    if(p_qName == QLatin1String("Row"))
    {
        for (int i=0; i<m_buffer.length();++i)
        {
            switch(m_buffer.at(i).toLatin1())
            {
                case ' ':
                    m_arena->setCellType(m_counterRows,i,Granatier::Cell::HOLE);
                    break;
                case '=':
                    m_arena->setCellType(m_counterRows,i,Granatier::Cell::WALL);
                    break;
                case '_':
                    m_arena->setCellType(m_counterRows,i,Granatier::Cell::GROUND);
                    break;
                case '+':
                    m_arena->setCellType(m_counterRows,i,Granatier::Cell::BLOCK);
                    break;
                case 'x':
                    // create a random block
                    if(granatier::RNG::fromIntRange(0, 100) > 25)
                    {
                        m_arena->setCellType(m_counterRows,i,Granatier::Cell::BLOCK);
                    }
                    else
                    {
                        m_arena->setCellType(m_counterRows,i,Granatier::Cell::GROUND);
                    }
                    break;
                case 'o':
                    m_arena->setCellType(m_counterRows,i,Granatier::Cell::BOMBMORTAR);
                    break;
                case '-':
                    m_arena->setCellType(m_counterRows,i,Granatier::Cell::ICE);
                    break;
                case 'u':
                    m_arena->setCellType(m_counterRows,i,Granatier::Cell::ARROWUP);
                    break;
                case 'r':
                    m_arena->setCellType(m_counterRows,i,Granatier::Cell::ARROWRIGHT);
                    break;
                case 'd':
                    m_arena->setCellType(m_counterRows,i,Granatier::Cell::ARROWDOWN);
                    break;
                case 'l':
                    m_arena->setCellType(m_counterRows,i,Granatier::Cell::ARROWLEFT);
                    break;
                case 'p':
                    m_arena->setCellType(m_counterRows,i,Granatier::Cell::GROUND);
                    m_arena->addPlayerPosition(QPointF(i+0.5, m_counterRows+0.5));
                    break;
                default:
                    m_arena->setCellType(m_counterRows,i,Granatier::Cell::GROUND);
            }
        }
        m_counterRows ++;
    }
    return true;
}
