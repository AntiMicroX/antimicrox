// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef PROFILEMIGRATION_H
#define PROFILEMIGRATION_H

#include <QCoreApplication>
#include <QSaveFile>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

namespace ProfileMigration {

/** Copy the remaining XML without clearing or concealing a source parse error. */
inline QString readCompleteXml(QXmlStreamReader &reader)
{
    QString contents;
    QXmlStreamWriter writer(&contents);
    writer.setAutoFormatting(true);
    while (!reader.atEnd())
    {
        writer.writeCurrentToken(reader);
        reader.readNext();
    }
    if (reader.hasError())
    {
        // The profile reader historically suppresses PrematureEndOfDocumentError.
        // Migration must surface truncated input instead of repairing it silently.
        reader.raiseError(reader.errorString());
        return QString();
    }
    writer.writeEndDocument();
    if (writer.hasError())
    {
        reader.raiseError(QCoreApplication::translate("ProfileMigration", "Could not copy profile XML."));
        return QString();
    }
    return contents;
}

/** Validate the actual UTF-8 bytes before committing a migrated profile. */
inline bool save(const QString &filename, const QString &contents, QString &error)
{
    error.clear();
    const QByteArray bytes = contents.toUtf8();
    QXmlStreamReader validation(bytes);
    while (!validation.atEnd())
        validation.readNext();
    if (validation.hasError())
    {
        error = validation.errorString();
        return false;
    }

    QSaveFile output(filename);
    output.setDirectWriteFallback(false);
    if (!output.open(QIODevice::WriteOnly))
    {
        error = output.errorString();
        return false;
    }
    if (output.write(bytes) != bytes.size())
    {
        error = output.errorString();
        output.cancelWriting();
        output.commit();
        return false;
    }
    if (!output.commit())
    {
        error = output.errorString();
        return false;
    }
    return true;
}

} // namespace ProfileMigration

#endif // PROFILEMIGRATION_H
