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
 * accepted NODATA value: void, single space, -9999, -999.9
  ---------------------------------------------------------------------------*/

#include <QCoreApplication>
#include <QDir>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QDate>

//#define TEST


void cleanAllDataTable(QSqlDatabase &myDB);

bool cleanTable(QString tableName, QSqlDatabase& myDB);
bool insertData(QString fileName, QString tableName, QSqlDatabase& myDB);

bool cleanTableTPrec(QString tableName, QSqlDatabase &myDB);
bool cleanTableTPrecWaterTable(QString tableName, QSqlDatabase &myDB);
bool insertDataTPrecWaterTable(QString fileName, QString tableName, int nrColumns, QSqlDatabase& myDB);

int getNrColumns(QString fileName);


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QString pathName, dataBaseName, tableName, fn, fileName;

#ifdef TEST
    pathName = "//icolt-smr/ICOLT/INPUT/C1/2025/sfOutput/JJA";
    dataBaseName = "//icolt-smr/CRITERIA1D/PROJECTS/icolt2025_JJA/seasonal_data/seasonal_C1.db";
#else
    if (argc < 3)
    {
       qDebug() << "USAGE: cvsToMeteoDb [inputPath] [outputName.db]";
       qDebug() << "\ninputPath: input directory of csv data files";
       qDebug() << "outputName.db: output database (SQLite)\n";
       exit (0);

    }
    else
    {
        pathName = argv[1];
        dataBaseName = argv[2];
    }
#endif

    // open and check DB
    QSqlDatabase myDB = QSqlDatabase::addDatabase("QSQLITE");
    myDB.setDatabaseName (dataBaseName);
    if(! myDB.open())
    {
        qDebug() << "\n-----ERROR-----\n" << myDB.lastError().text() << myDB.databaseName();
        exit(-1);
    }

    // open and check path
    if (pathName.right(1) != "\\") pathName += "\\";
    QDir myDir = QDir(pathName);
    if(!myDir.exists())
    {
        qDebug() << "\n-----ERROR-----\n" << "Directory doesn't exist: " << pathName;
        myDB.close();
        exit(-1);
    }

    // list files (csv)
    myDir.setNameFilters(QStringList("*.csv"));
    QList<QString> fileList = myDir.entryList();
    if (fileList.size() == 0)
    {
        qDebug() << "\n-----ERROR-----\n" << "Missing csv files.";
        myDB.close();
        exit(-1);
    }

    int nrColumns = getNrColumns(pathName + fileList[0]);
    if (nrColumns == 0)
    {
        qDebug() << "\n-----ERROR-----\n";
        qDebug() << "Missing data.";
        myDB.close();
        exit(-1);
    }
    else if (nrColumns == 4)
    {
        qDebug() << "File format: Date, Tmin, Tmax, Prec";
    }
    else if (nrColumns == 5)
    {
        qDebug() << "File format: Date, Tmin, Tmax, Prec, WaterTable";
    }
    else if (nrColumns == 7)
    {
        qDebug() << "File format: Date, Tmin, Tmax, Tavg, Prec, ET0, WaterTable";
    }
    else
    {
        qDebug() << "\n-----ERROR-----\n";
        qDebug() << "wrong nr of column: " << nrColumns;
        myDB.close();
        exit(-1);
    }

    qDebug() << "Clean previous data on DB...";
    cleanAllDataTable(myDB);

    qDebug() << "Save to DB...";
    for (int i = 0; i < fileList.count(); i++)
    {
        fn = fileList[i];

        tableName = fn.left(fn.length()-4);
        fileName = pathName + fn;
        qDebug() << fileName;

        if (nrColumns == 4)
        {
            cleanTableTPrec(tableName, myDB);
            insertDataTPrecWaterTable(fileName, tableName, nrColumns, myDB);
        }
        else if (nrColumns == 5)
        {
            cleanTableTPrecWaterTable(tableName, myDB);
            insertDataTPrecWaterTable(fileName, tableName, nrColumns, myDB);
        }
        else
        {
            cleanTable(tableName, myDB);
            insertData(fileName, tableName, myDB);
        }
    }

    qDebug() << "END.";

    myDB.close();
}


void cleanAllDataTable(QSqlDatabase &myDB)
{
    QList<QString> tablesList = myDB.tables();
    int indexLastTable = tablesList.size() -1;
    for (int i = indexLastTable; i >= 0; i--)
    {
        if (tablesList[i].toUpper() != "POINT_PROPERTIES" && tablesList[i].toUpper() != "METEO_LOCATIONS")
        {
            QString query = "DROP TABLE '" + tablesList[i] + "'";
            myDB.exec(query);
        }
    }
}


bool cleanTable(QString tableName, QSqlDatabase &myDB)
{
    QString query = "DROP TABLE '" + tableName + "'";
    myDB.exec(query);

    query = "CREATE TABLE '" + tableName + "'";
    query += "(date char(10), tmin float, tmax float, tavg float, prec float, et0 float, watertable float);";
    myDB.exec(query);

    if (myDB.lastError().type() != QSqlError::NoError)
    {
        qDebug() << "---Error---\n" << myDB.lastError().text();
        return false;
    }

    return true;
}


bool cleanTableTPrec(QString tableName, QSqlDatabase &myDB)
{
    QString query = "DROP TABLE '" + tableName + "'";
    myDB.exec(query);

    query = "CREATE TABLE '" + tableName + "'";
    query += "(date char(10), tmin float, tmax float, prec float);";
    myDB.exec(query);

    if (myDB.lastError().type() != QSqlError::NoError)
    {
        qDebug() << "---Error---\n" << myDB.lastError().text();
        return false;
    }

    return true;
}


bool cleanTableTPrecWaterTable(QString tableName, QSqlDatabase &myDB)
{
    QString query = "DROP TABLE '" + tableName + "'";
    myDB.exec(query);

    query = "CREATE TABLE '" + tableName + "'";
    query += "(date char(10), tmin float, tmax float, prec float, watertable float);";
    myDB.exec(query);

    if (myDB.lastError().type() != QSqlError::NoError)
    {
        qDebug() << "---Error---\n" << myDB.lastError().text();
        return false;
    }

    return true;
}


// date (yyyy-mm-dd), tmin, tmax, tavg, prec, et0, watertable
bool insertData(QString fileName, QString tableName, QSqlDatabase &myDB)
{
    QFile myFile(fileName);
    if(! myFile.open (QIODevice::ReadOnly))
    {
        qDebug() << myFile.errorString();
        return false;
    }

    QString valueStr;
    QTextStream myStream (&myFile);

    QString query = "INSERT INTO '" + tableName + "' VALUES";
    QList<QString> line;
    int nrLine = 0;

    while(! myStream.atEnd())
    {
        line = myStream.readLine().split(',');
        // skip header or void lines
        if ((nrLine > 0) && (line.length()>1))
        {
            query.append("(");
            for(int i=0; i <7; ++i)
            {
                if (i > 0) query.append(",");
                if (i >= line.length())
                {
                    // missing data -> void
                    valueStr = "";
                }
                else
                {
                    valueStr = line.at(i);
                    if (valueStr != "")
                    {
                        if (valueStr.at(0) == '\"')
                            valueStr = valueStr.mid(1, valueStr.length()-2);

                        if (valueStr == "-9999" || valueStr == "-999.9" || valueStr == " ")
                            valueStr = "";
                    }
                }
                query.append("'" + valueStr + "'");
            }
            query.append("),");
        }
        nrLine++;
    }
    query.chop(1); // remove the trailing comma
    myFile.close ();

    myDB.exec(query);

    if (myDB.lastError().type() != QSqlError::NoError)
    {
        qDebug() << "Error in table: " << tableName << "\n" << myDB.lastError().text();
        return false;
    }

    return true;
}


// format: date (yyyy-mm-dd), tmin, tmax, prec, watertable
bool insertDataTPrecWaterTable(QString fileName, QString tableName, int nrColumns, QSqlDatabase& myDB)
{
    QFile myFile(fileName);
    if(! myFile.open (QIODevice::ReadOnly))
    {
        qDebug() << myFile.errorString();
        return false;
    }

    int nrLine = 0;
    QString query = "INSERT INTO '" + tableName + "' VALUES";

    QTextStream myStream (&myFile);
    QList<QString> line;
    while(! myStream.atEnd())
    {
        line = myStream.readLine().split(',');
        // check date
        QDate myDate = QVariant(line.at(0)).toDate();

        // skip header, void lines and invalid dates
        if ( nrLine > 0 && line.length() > 1 && myDate.isValid() )
        {
            query.append("(");
            for(int i=0; i < nrColumns; ++i)
            {
                if (i > 0) query.append(",");

                QString valueStr = line.at(i);
                if (valueStr != "")
                {
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

    myDB.exec(query);

    if (myDB.lastError().type() != QSqlError::NoError)
    {
        qDebug() << "Error in table: " << tableName << "\n" << myDB.lastError().text();
        return false;
    }

    return true;
}


int getNrColumns(QString fileName)
{
    QFile myFile(fileName);
    if(! myFile.open (QIODevice::ReadOnly))
    {
        qDebug() <<"Error in opening: " << fileName <<  myFile.errorString();
        return 0;
    }

    QTextStream myStream (&myFile);
    QList<QString> line = myStream.readLine().split(',');
    myFile.close();

    return line.size();
}

