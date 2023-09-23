/*
    SPDX-FileCopyrightText: 2008 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2007-2008 Thomas Gallinari <tg8187@yahoo.fr>
    SPDX-FileCopyrightText: 2007-2008 Nathalie Liesse <nathalie.liesse@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#ifndef MAPPARSER_H
#define MAPPARSER_H
#include <QXmlStreamAttributes>
class Arena;

/**
 * @brief This class handles XML reader events in order to initialize the Arena properties.
 */
class MapParser
{

private:

    /** The Game to initialize */
    Arena* m_arena;

    /** The parser's buffer */
    QString m_buffer;

    /** The rows counter */
    int m_counterRows;
public:

    /**
      * Creates a new GameParser.
      * @param p_arena the Arena to initialize
      */
    explicit MapParser(Arena* p_arena);

    /**
      * Deletes the GameParser instance.
      */
    ~MapParser();

    Q_REQUIRED_RESULT bool characters(const QStringView &ch);

    Q_REQUIRED_RESULT bool startElement(const QStringView &namespaceURI, const QStringView &localName,
                      const QStringView &qName, const QXmlStreamAttributes &atts);

    Q_REQUIRED_RESULT bool endElement(const QStringView &namespaceURI,
                    const QStringView &localName, const QStringView &qName);

    Q_REQUIRED_RESULT bool parse(QIODevice *input);
};

#endif

