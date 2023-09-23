/*
    SPDX-FileCopyrightText: 2008 Mathias Kraus <k.hias@gmx.de>
    SPDX-FileCopyrightText: 2007-2008 Nathalie Liesse <nathalie.liesse@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "mapparser.h"
#include "arena.h"
#include "granatier_random.h"

#include <cstdlib>
#include <QPointF>

MapParser::MapParser(Arena* p_arena)
{
    m_arena = p_arena;
    m_counterRows = 0;
}

MapParser::~MapParser()
= default;

bool MapParser::parse(QIODevice *input)
{
    QXmlStreamReader reader(input);

    while (!reader.atEnd()) {
        reader.readNext();
        if (reader.hasError())
            return false;

        switch (reader.tokenType()) {
        case QXmlStreamReader::StartElement:
            if (!startElement(reader.namespaceUri(), reader.name(),
                              reader.qualifiedName(), reader.attributes())) {
                return false;
            }
            break;
        case QXmlStreamReader::EndElement:
            if (!endElement(reader.namespaceUri(), reader.name(),
                            reader.qualifiedName())) {
                return false;
            }
            break;
        case QXmlStreamReader::Characters:
            if (!reader.isWhitespace() && !reader.text().toString().trimmed().isEmpty()) {
                if (!characters(reader.text()))
                    return false;
            }
            break;
        default:
            break;
        }
    }

    if (!reader.isEndDocument())
        return false;

    return true;
}
bool MapParser::characters(const QStringView &ch)
{
    m_buffer = ch.toString();
    return true;
}

bool MapParser::startElement(const QStringView &namespaceURI, const QStringView &localName, const QStringView &qName, const QXmlStreamAttributes &atts)
{
    Q_UNUSED(namespaceURI)
    Q_UNUSED(localName)
    if (qName == QLatin1String("Arena")) {
        int nbRows = 0;
        int nbColumns = 0;
        if (atts.hasAttribute(QLatin1String("rowCount"))) {
            nbRows = atts.value(QLatin1String("rowCount")).toInt();
        }
        if (atts.hasAttribute(QLatin1String("colCount"))) {
            nbColumns = atts.value(QLatin1String("colCount")).toInt();
        }
        // Create the Arena matrix
        m_arena->init(nbRows, nbColumns);
    }

    return true;
}

bool MapParser::endElement(const QStringView &namespaceURI, const QStringView &localName, const QStringView &qName)
{
    Q_UNUSED(namespaceURI)
    Q_UNUSED(localName)

    if (qName.toString() == QLatin1String("Row")) {
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
                    if(granatier::RNG::fromRange(0, 100) > 25)
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
