#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDirIterator>

#include <set>

static std::set<QString> UNIQUE_ALIASES_SET;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setOptionsAfterPositionalArgumentsMode(QCommandLineParser::ParseAsOptions);
    parser.setApplicationDescription(QStringLiteral("QtResourceGenerator"));
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument(QStringLiteral("source-path"), QStringLiteral("Path to folder, where files for resources are located."));
    parser.addPositionalArgument(QStringLiteral("destination-file"), QStringLiteral("Path and name of recorces destinationfile"));


    QCommandLineOption fileExtOption(QStringLiteral("file-extensions"), QStringLiteral("Files with defined extensions should be added to qrc file."), QStringLiteral("*.*"));
    QCommandLineOption prefixOption(QStringLiteral("prefix"), QStringLiteral("Prefix for files in qrc."), QStringLiteral("/"));

    if (!parser.addOptions(QList<QCommandLineOption>() << fileExtOption << prefixOption))
        parser.errorText();


    parser.process(app);

    const auto args = parser.positionalArguments();

    if(args.isEmpty())
        parser.showHelp(-1);

    const auto sourceDir = args.at(0);
    const auto qrcFilePath = args.at(1);
    QString prefix;
    QStringList fileExt;
    if (parser.isSet(prefixOption))
        prefix = parser.value(prefixOption);
    if (parser.isSet(prefixOption))
        fileExt = parser.values(fileExtOption);

    QFile qrc(qrcFilePath);
    if(qrc.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qrc.write(QByteArrayLiteral("<RCC>"));
        qDebug() << prefix << fileExt;
        qrc.write(QStringLiteral("<qresource prefix=\"%1\">").arg(prefix).toUtf8());

        std::unique_ptr<QDirIterator> sourceDirIterator;

        if (fileExt.isEmpty())
            sourceDirIterator = std::make_unique<QDirIterator>(sourceDir, QDirIterator::Subdirectories);
        else
            sourceDirIterator = std::make_unique<QDirIterator>(sourceDir, fileExt, QDir::Files, QDirIterator::Subdirectories);

        while (sourceDirIterator->hasNext()) {
            QFileInfo sourceFileInfo(sourceDirIterator->next());
            if (sourceFileInfo.isFile())
            {
                auto fileName = sourceFileInfo.fileName();
                if (UNIQUE_ALIASES_SET.find(fileName) != std::end(UNIQUE_ALIASES_SET))
                    fileName.prepend(QStringLiteral("unique-"));

                UNIQUE_ALIASES_SET.emplace(fileName);

                qDebug() << fileName;
                qrc.write(QStringLiteral("<file alias=\"%1\">%2</file>").arg(fileName, sourceFileInfo.filePath()).toUtf8());
            }
        }

        qrc.write(QByteArrayLiteral("</qresource>"));
        qrc.write(QByteArrayLiteral("</RCC>"));
        qrc.close();
        qDebug() << QStringLiteral("Resource file was successfuly created by path: %1").arg(qrcFilePath);
        return 0;
    }
    return QCoreApplication::exec();
}
