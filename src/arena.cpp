/*
 * Copyright 2009 Mathias Kraus <k.hias@gmx.de>
 * Copyright 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>
 * Copyright 2007-2008 Pierre-Benoît Besse <besse.pb@gmail.com>
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

#include "arena.h"
#include "granatier_random.h"

#include <QPoint>

#include <cstdlib>

Arena::Arena()
{
}

Arena::~Arena()
{
    for(int i = 0 ; i < m_nbRows; ++i)
    {
        delete[] m_cells[i];
    }
    delete[] m_cells;
}

void Arena::init(const int p_nbRows, const int p_nbColumns)
{
    m_nbRows = p_nbRows;
    m_nbColumns = p_nbColumns;
    m_cells = new Cell*[m_nbRows];
    for (int i = 0; i < m_nbRows; ++i)
    {
        m_cells[i] = new Cell[m_nbColumns];
    }

    m_emptyCell.setType(Granatier::Cell::HOLE);
}

QString Arena::getName () const
{
    return m_strArenaName;
}

void Arena::setName (const QString &p_strArenaName)
{
    m_strArenaName = p_strArenaName;
}

void Arena::setCellType(const int p_row, const int p_column, const Granatier::Cell::Type p_type)
{
    if (p_row < 0 || p_row >= m_nbRows || p_column < 0 || p_column >= m_nbColumns)
    {
        return;
    }
    m_cells[p_row][p_column].setType(p_type);
}

void Arena::setCellElement(const int p_row, const int p_column, Element * p_element)
{
    if (p_row < 0 || p_row >= m_nbRows || p_column < 0 || p_column >= m_nbColumns)
    {
        return;
    }
    m_cells[p_row][p_column].setElement(p_element);
}

void Arena::removeCellElement(const int p_row, const int p_column, Element * p_element)
{
    if (p_row < 0 || p_row >= m_nbRows || p_column < 0 || p_column >= m_nbColumns)
    {
        return;
    }
    m_cells[p_row][p_column].removeElement(p_element);
}

void Arena::addPlayerPosition(const QPointF &p_position)
{
    m_playerPosition.append(p_position);

    int nShuffle;
    for (int i = 0; i < m_playerPosition.size(); i++)
    {
        nShuffle = granatier::RNG::fromRange(0, m_playerPosition.size()-1);
        m_playerPosition.swapItemsAt(i, nShuffle);
    }
}

QPointF Arena::getPlayerPosition(int p_player) const
{
    if(m_playerPosition.count() > p_player)
    {
        return m_playerPosition.at(p_player);
    }
    return m_playerPosition.at(0);    //to have a valid position
}

Cell Arena::getCell(const int p_row, const int p_column) const
{
    if (p_row < 0 || p_row >= m_nbRows || p_column < 0 || p_column >= m_nbColumns)
    {
        return m_emptyCell;
    }
    return m_cells[p_row][p_column];
}

QPoint Arena::getCoords(Cell* p_cell) const
{
    for (int i = 0; i < m_nbRows; ++i)
    {
        for (int j = 0; j < m_nbColumns; ++j)
        {
            if (&m_cells[i][j] == p_cell)
            {
                return QPoint(j, i);
            }
        }
    }
    return QPoint();
}

int Arena::getRowFromY(const qreal p_y) const
{
    int nRow = static_cast<int>(p_y / Granatier::CellSize);
    if (p_y < 0)
    {
        nRow -= 1;
    }
    return nRow;
}

int Arena::getColFromX(const qreal p_x) const
{
    int nCol = static_cast<int>(p_x / Granatier::CellSize);
    if (p_x < 0)
    {
        nCol -= 1;
    }
    return nCol;
}

int Arena::getNbColumns() const
{
    return m_nbColumns;
}

int Arena::getNbRows() const
{
    return m_nbRows;
}
