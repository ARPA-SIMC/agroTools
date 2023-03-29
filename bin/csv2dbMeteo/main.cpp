/* --------------------------------------------------------------------------
 * csvToMeteoDb
 *
 * USAGE: cvsToMeteoDb [inputPath] [outputDb]
 * inputPath: input directory of csv data files
 * outputDb: output database (SQLite)
 * --------------------------------------------------------------------------
 * csv format:
 * date, tmin (°C), tmax (°C), tavg (°C), prec (mm), et0 (mm), watertable (m)
 *
 * header: first line
 * mandatory variables: tmin, tmax, prec
 * date format: YYYY-MM-DD
 * accepted NODATA value: void (,,) or -9999 or -999.9
  ---------------------------------------------------------------------------*/

#include <QCoreApplication>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>


bool cleanTable(QString tableName, QSqlDatabase* myDB);
bool insertData(QString fileName, QString tableName, QSqlDatabase* myDB);


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QString pathName, dataBaseName, tableName, fn, fileName;
    if (argc < 3)
    {
       qDebug() << "USAGE:\ncvsToMeteoDb [inputPath] [outputDb]";
       qDebug() << "\ninputPath: input directory of csv data files";
       qDebug() << "outputDb: output database (SQLite)\n";
       exit (0);
    }
    else
    {
        pathName = argv[1];
        dataBaseName = argv[2];
    }

    // open and check DB
    QSqlDatabase myDB = QSqlDatabase::addDatabase("QSQLITE");
    myDB.setDatabaseName (dataBaseName);
    if(!myDB.open())
    {
        qDebug() << "\n-----ERROR-----\n" << myDB.lastError().text() << myDB.databaseName();
        exit(-1);
    }

    // open and check path
    if (pathName.right(1) != "\\") pathName += "\\";
    QDir myDir = QDir(pathName);
    if(!myDir.exists())
    {
        qDebug() << "\n-----ERROR-----\n" << "Directory doesn't exist.";
        myDB.close();
        exit(-1);
    }

    // list csv files
    myDir.setNameFilters(QStringList("*.csv"));
    QStringList fileList = myDir.entryList();

    for (int i=0; i<fileList.count(); i++)
    {
        fn = fileList[i];

        tableName = fn.left(fn.length()-4);
        fileName = pathName + fn;
        //qDebug() << tableName;

        cleanTable(tableName, &myDB);
        insertData(fileName, tableName, &myDB);
    }

    myDB.close();
}


bool cleanTable(QString tableName, QSqlDatabase* myDB)
{
    QString query = "DROP TABLE " + tableName;
    myDB->exec(query);

    query = "CREATE TABLE " + tableName;
    query += "(date char(10), tmin float, tmax float, tavg float, prec float, et0 float, watertable float);";
    myDB->exec(query);

    if (myDB->lastError().type() != QSqlError::NoError)
    {
        qDebug() << "---Error---\n" << myDB->lastError().text();
        return false;
    }
    else
        return true;
}


bool insertData(QString fileName, QString tableName, QSqlDatabase* myDB)
{
    QFile myFile(fileName);
    if(! myFile.open (QIODevice::ReadOnly))
    {
        qDebug() << myFile.errorString();
        return false;
    }

    QString query, valueStr;
    QTextStream myStream (&myFile);
    query = "INSERT INTO " + tableName + " VALUES";
    QStringList line;
    int nrLine = 0;

    while(!myStream.atEnd())
    {
        line = myStream.readLine().split(',');
        // skip header or void lines
        if ((nrLine > 0) && (line.length()>1))
        {
            query.append("(");
            for(int i=0; i<7; ++i)
            {
                if (i > 0) query.append(",");
                if (i == line.length())
                {
                    // et0 and watertable missing -> void
                    if (i == 5)
                    {
                        valueStr = ",";
                    }
                    // watertable missing -> void
                    else if (i == 6)
                    {
                        valueStr = "";
                    }
                    else
                    {
                        qDebug() << "---Error---\n" << "missing values in line nr:" << nrLine+1;
                        myFile.close ();
                        return false;
                    }
                }
                else
                {
                    valueStr = line.at(i);

                    if (valueStr.at(0) == '\"')
                        valueStr = valueStr.mid(1, valueStr.length()-2);

                    if (valueStr == "-9999" || valueStr == "-999.9" || valueStr == " ")
                        valueStr = "";
                }
                query.append("'" + valueStr + "'");
            }
            query.append("),");
        }
        nrLine++;
    }
    query.chop(1); // remove the trailing comma
    myFile.close ();

    myDB->exec(query);

    if (myDB->lastError().type() != QSqlError::NoError)
    {
        qDebug() << "---Error---\n" << myDB->lastError().text();
        return false;
    }

    return true;
}

