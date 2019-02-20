#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include <QDirIterator>
#include <QDomDocument>

#include <set>



QByteArray modifyGameIconSvgColors(const QByteArray &data)
{
    static QDomDocument s_domDocument;

    QString error;
    if (!s_domDocument.setContent(data, &error))
        return data;

    auto elements = s_domDocument.elementsByTagName(QStringLiteral("path"));
    for(auto i = 0; i < elements.size(); ++i)
    {
        auto element = elements.at(i).toElement();
        if(element.isNull())
            continue;

        if(auto fillAttr = element.attributeNode(QStringLiteral("fill")); fillAttr.isNull())
            element.setAttribute(QStringLiteral("fill"), QStringLiteral("none"));
        else
            fillAttr.setValue(QStringLiteral("%1"));
    }

    return s_domDocument.toByteArray();
}

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
    QCommandLineOption pmbgSvgOption(QStringLiteral("pmbgSvg"), QStringLiteral("Specific parameter only for ProtomorphBG."));

    if (!parser.addOptions(QList<QCommandLineOption>() << fileExtOption << prefixOption << pmbgSvgOption))
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
    if (parser.isSet(fileExtOption))
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

        {
            auto UNIQUE_ALIASES_SET = std::set<QString>{};

            auto file = QFile{};
            auto sourceFileInfo = QFileInfo{};

            while (sourceDirIterator->hasNext()) {
                file.setFileName(sourceDirIterator->next());
                sourceFileInfo.setFile(file);
                if (sourceFileInfo.isFile())
                {
                    auto fileName = sourceFileInfo.fileName();

                    if (parser.isSet(pmbgSvgOption) && file.open(QFile::ReadWrite))
                    {
                        auto svgData = modifyGameIconSvgColors(file.readAll());
                        file.resize(0);
                        file.write(svgData);
                        file.close();
                    }

                    if (UNIQUE_ALIASES_SET.find(fileName) != std::end(UNIQUE_ALIASES_SET))
                        fileName.prepend(QStringLiteral("unique-"));

                    UNIQUE_ALIASES_SET.emplace(fileName);
                    qDebug() << fileName;
                    qrc.write(QStringLiteral("<file alias=\"%1\">%2</file>").arg(fileName, sourceFileInfo.filePath()).toUtf8());
                }
            }
        }

        qrc.write(QByteArrayLiteral("</qresource>"));
        qrc.write(QByteArrayLiteral("</RCC>"));
        qrc.close();
        qDebug() << QStringLiteral("Resource file was successfuly created by path: %1").arg(qrcFilePath);
        return 0;
    }

    return -1;
}
