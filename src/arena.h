/*
    SPDX-FileCopyrightText: 2009 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>
    SPDX-FileCopyrightText: 2007-2008 Pierre-Benoît Besse <besse.pb@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef ARENA_H
#define ARENA_H

#include <QObject>

#include "cell.h"

#include <QList>

class QPoint;
class QPointF;
class QString;

/**
 * @brief This class represents the Arena of the game.
 */
class Arena : public QObject
{
    Q_OBJECT

private:
    /** The number of rows of the Arena */
    QString m_strArenaName;

    /** The number of rows of the Arena */
    int m_nbRows;

    /** The number of columns of the Arena */
    int m_nbColumns;

    /** The Arena Cells */
    Cell** m_cells;

    /** The Arena Cells */
    Cell m_emptyCell;

    /** The Player position on the Arena */
    QList <QPointF> m_playerPosition;

public:

    /**
      * Creates a new Arena instance.
      */
    Arena();

    /**
      * Deletes the Arena instance.
      */
    ~Arena() override;

    /**
      * Creates the Arena matrix.
      * @param p_nbRows the number of rows
      * @param p_nbColumns the number of columns
      */
    void init(const int p_nbRows, const int p_nbColumns);

    /**
      * Returns the Arean name.
      * @return the Arena name
      */
    QString getName () const;

    /**
      * Sets the Arena name
      * @param p_strArenaName the Arena name
      */
    void setName (const QString &p_strArenaName);

    /**
      * Sets the CellType of the Cell whose coordinates are given in parameters.
      * @param p_row the Cell row
      * @param p_column the Cell column
      * @param p_type the Cell type
      */
    void setCellType(const int p_row, const int p_column, const Granatier::Cell::Type p_type);

    /**
      * Sets the Element that is on the Cell whose coordinates are given in parameters.
      * @param p_row the Cell row
      * @param p_column the Cell column
      * @param p_element the Element that is on the Cell
      */
    void setCellElement(const int p_row, const int p_column, Element* p_element);

    /**
      * Removes the Element that is on the Cell whose coordinates are given in parameters.
      * @param p_row the Cell row
      * @param p_column the Cell column
      * @param p_element the Element that is on the Cell
      */
    void removeCellElement(const int p_row, const int p_column, Element* p_element);

    /**
      * Sets a player position on the arena.
      * @param p_position the player position
      */
    void addPlayerPosition(const QPointF &p_position);

    /**
      * Gets the player position on the arena.
      * @param p_player the player number
      * @return p_position the player position
      */
    QPointF getPlayerPosition(int p_player) const;

    /**
      * Gets the Cell at the given coordinates.
      * @param p_row the row index
      * @param p_column the column index
      * @return the Cell at the given row and column
      */
    Cell getCell(const int p_row, const int p_column) const;

    /**
      * Gets the coordinates of the given Cell as a QPoint.
      * @param p_cell the searched Cell
      * @return the row and column of the given Cell
      */
    QPoint getCoords(Cell* p_cell) const;

    /**
      * Gets the row index corresponding to the given y-coordinate.
      * @param p_y the y-coordinate to convert into row index
      * @return the row index corresponding to the given y-coordinate
      */
    int getRowFromY(const qreal p_y) const;

    /**
      * Gets the column index corresponding to the given x-coordinate.
      * @param p_x the x-coordinate to convert into column index
      * @return the column index corresponding to the given x-coordinate
      */
    int getColFromX(const qreal p_x) const;

    /**
      * Gets the number of columns of the Arena.
      * @return the number of columns
      */
    int getNbColumns() const;

    /**
      * Gets the number of rows of the Arena.
      * @return the number of rows
      */
    int getNbRows() const;
};

#endif
