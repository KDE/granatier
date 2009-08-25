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

#include <KDebug>

#include <math.h>

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
}

void Arena::setCellType(const int p_row, const int p_column, const Cell::Type p_type)
{
    if (p_row < 0 || p_row >= m_nbRows || p_column < 0 || p_column >= m_nbColumns)
    {
        kError() << "Bad arena coordinates";
    }
    m_cells[p_row][p_column].setType(p_type);
}

void Arena::setCellElement(const int p_row, const int p_column, Element * p_element)
{
    if (p_row < 0 || p_row >= m_nbRows || p_column < 0 || p_column >= m_nbColumns) 
    {
        kError() << "Bad arena coordinates";
    }
    m_cells[p_row][p_column].setElement(p_element);
    /*if (p_element != NULL)
    {
        m_totalNbElem++;
        m_nbElem++;
    }*/
}

void Arena::removeCellElement(const int p_row, const int p_column, Element * p_element)
{
    if (p_row < 0 || p_row >= m_nbRows || p_column < 0 || p_column >= m_nbColumns) 
    {
        kError() << "Bad arena coordinates";
    }
    m_cells[p_row][p_column].removeElement(p_element);
}

Cell Arena::getCell(const int p_row, const int p_column) const
{
    if (p_row < 0 || p_row >= m_nbRows || p_column < 0 || p_column >= m_nbColumns)
    {
        kError() << "Bad arena coordinates";
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
    return (int)(p_y / Cell::SIZE);
}

int Arena::getColFromX(const qreal p_x) const
{
    return (int)(p_x / Cell::SIZE);
}

int Arena::getNbColumns() const
{
    return m_nbColumns;
}

int Arena::getNbRows() const
{
    return m_nbRows;
}