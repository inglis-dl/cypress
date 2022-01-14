#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QDir>

int main(int argc, char *argv[])
{
    QFileInfo info(argv[0]);
    QDir path = info.absolutePath();
    QString infile = path.filePath("input.txt");
    if(QFileInfo::exists(infile))
    {
        QString ofile = path.filePath("output.txt");
        QFile::copy(infile,ofile);
        QFile file(ofile);
        if(file.open(QIODevice::ReadWrite))
        {
          QTextStream stream(&file);
          QString line = stream.readLine().simplified();
          line += ",0.7,0.6,0.8,0.9";
          file.resize(0);
          stream << line << Qt::endl;
        }
        file.close();
    }

    return EXIT_SUCCESS;
}
