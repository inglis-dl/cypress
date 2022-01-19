#include <QCoreApplication>
#include <QDebug>
#include <QFileInfo>
#include <QByteArray>
//#include <QFile>
#include <QDir>
#include <QDate>

// Create an executable that the ChoiceReaction manager
// can launch with command line args
// Read the default output file template and construct
// a simulated output file with modified content
//

int main(int argc, char *argv[])
{
    QString clinic;
    QString interviewer_id;
    QString user_id;
    QString language;
    for(int i=0;i<argc;i++)
    {
        qDebug() << "CCB emulator input" << i << argv[i];
        QString s(argv[i]);
        if(s.startsWith("/c")) clinic = s.mid(2);
        else if(s.startsWith("/i")) interviewer_id = s.mid(2);
        else if(s.startsWith("/u")) user_id = s.mid(2);
        else if(s.startsWith("/l")) language = s.mid(2);
        else
          continue;
    }
    // read the default file name
    // CLSA_ELCV_DEFAULT_YYYYMMDD.csv
    //

    QFileInfo info(argv[0]);
    QDir path = info.absolutePath();
    QString inFile = path.filePath("CLSA_ELCV_DEFAULT_YYYYMMDD.csv");
    if(QFileInfo::exists(inFile))
    {
        info.setFile(inFile);
        QString outFile = info.fileName();
        outFile.replace(
          "YYYYMMDD",
          QDate().currentDate().toString("yyyyMMdd"));
        if(!clinic.isEmpty())
            outFile.replace("DEFAULT",clinic);
        path.setPath(info.absolutePath() + QDir::separator() + "results");
        if(path.exists())
        {
            outFile = path.filePath(outFile);
            QFile::copy(inFile,outFile);
            QFile file(outFile);
            if(file.open(QIODevice::ReadWrite))
            {
              QByteArray data = file.readAll();
              QString text(data);
              if(!clinic.isEmpty())
                text.replace("CLINIC_NAME",clinic);
              else
                text.replace("CLINIC_NAME","Default");

              if(!interviewer_id.isEmpty())
                text.replace("INTERVIEWER_ID",interviewer_id);
              else
                text.replace("INTERVIEWER_ID","None");

              if(!user_id.isEmpty())
                text.replace("BARCODE",user_id);

              file.resize(0);
              file.write(text.toUtf8());
              file.flush();
            }
            file.close();
        }

    }

    return EXIT_SUCCESS;
}
