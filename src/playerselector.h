/*
 * Copyright 2009 Mathias Kraus <k.hias@gmx.de>
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

#ifndef PLAYERSELECTOR_H
#define PLAYERSELECTOR_H

#include <QtGui/QWidget>

class PlayerSettings;
class PlayerSelectorPrivate;
class QListWidgetItem;

class PlayerSelector : public QWidget
{
Q_OBJECT

public:
    PlayerSelector(QWidget* parent, PlayerSettings* playerSettings);
    virtual ~PlayerSelector();
    
private:
    class PlayerSelectorPrivate;
    PlayerSelectorPrivate* const d;
    
    Q_DISABLE_COPY(PlayerSelector)
    Q_PRIVATE_SLOT(d, void slotUpdatePreview())
    Q_PRIVATE_SLOT(d, void slotUpdateName(QListWidgetItem* item))
    Q_PRIVATE_SLOT(d, void slotUpdateShortcut(const QKeySequence& seq))
}; 

#endif