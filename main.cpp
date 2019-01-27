#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDirIterator>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(QStringLiteral("QtResourceGenerator"));
    parser.addHelpOption();
    parser.addVersionOption();

    parser.addPositionalArgument(QStringLiteral("source-path"), QStringLiteral("Path to folder, where files for resources are located."));
    parser.addPositionalArgument(QStringLiteral("destination-file"), QStringLiteral("Path and name of recorces destinationfile"));


    QCommandLineOption fileExtOption(QStringList() << QStringLiteral("fext") << QStringLiteral("file-extensions"), QStringLiteral("Files with defined extensions should be added to qrc file."));
    parser.addOption(fileExtOption);

    QCommandLineOption prefixOption(QStringLiteral("prefix"), QStringLiteral("Prefix for files in qrc."));
    parser.addOption(prefixOption);

    parser.process(app);

    const auto args = parser.positionalArguments();

    if(args.isEmpty())
        parser.showHelp(-1);

    const auto sourceDir = args.at(0);
    const auto qrcFilePath = args.at(1);

    QFile qrc(qrcFilePath);
    if(qrc.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qrc.write(QByteArrayLiteral("<RCC>"));
        const auto prefix = parser.value(prefixOption);
        qrc.write(QStringLiteral("<qresource prefix=\"%1\">").arg((prefix.isEmpty() ? QStringLiteral("/") : prefix)).toUtf8());

        std::unique_ptr<QDirIterator> sourceDirIterator;
        const auto fileExt = parser.values(fileExtOption);

        if (fileExt.isEmpty())
            sourceDirIterator = std::make_unique<QDirIterator>(sourceDir, QDirIterator::Subdirectories);
        else
            sourceDirIterator = std::make_unique<QDirIterator>(sourceDir, fileExt, QDir::Files, QDirIterator::Subdirectories);

        while (sourceDirIterator->hasNext()) {
            QFileInfo sourceFileInfo(sourceDirIterator->next());
            if (sourceFileInfo.isFile())
            {
            qDebug() << sourceFileInfo.filePath();
            qrc.write(QStringLiteral("<file alias=\"%1\">%2</file>").arg(sourceFileInfo.fileName(), sourceFileInfo.filePath()).toUtf8());
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
