#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QtDebug>
#include <QMessageBox>
#include <QSqlQuery>
#include <QDate>
#include <QDateTime>
#include <QFile>
#include <QSqlError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QDir>
#include <QSqlDriver>
#include <QSettings>
#include <QStandardItemModel>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    qDebug() << QSqlDatabase::drivers();

    QSettings settings("settings.ini", QSettings::IniFormat);
    _server = settings.value("server", ".").toString();
    qDebug() << "server = " + _server;
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_btnQuit_clicked()
{
    QApplication::quit();
}

void MainWindow::on_btnFind_clicked()
{
    ui->btnFind->setEnabled(false);

    QString firstName = ui->edtFirstName->text();
    QString lastName = ui->edtLastName->text();
    QString medicareNo = ui->edtMedicareNo->text();
    QDate dob = QDate::fromString(ui->dateEditDOB->text(), "dd/MM/yyyy");

    bool something_to_search = firstName.length() > 1 || lastName.length() > 1 || medicareNo.length() > 1;

    if (!something_to_search)
    {
        QMessageBox::warning(NULL, "Please enter some criteria to search for", "Not enough critiera to search for.", QMessageBox::Ok);
        ui->btnFind->setEnabled(true);
        return;
    }


    if (connectToDatabase())
    {
        obstRecord a = getBasicDetails(firstName, lastName, ui->chkDobUnknown->isChecked(), dob, medicareNo);
        // savePage1(a);
    }
    else
    {
        QMessageBox::critical(NULL, "Unable to connect to the BP database", "Could not connect to the BP database.", QMessageBox::Ok);
    }

    ui->btnFind->setEnabled(true);
}

bool MainWindow::connectToDatabase()
{
    // TO DO : escape server name and re-use existing connection if already open
    db = new QSqlDatabase(QSqlDatabase::addDatabase("QODBC"));
    db->setDatabaseName("Driver={SQL Server};Server=" + _server + "\\BPSINSTANCE;Database=BPSSamples;Uid=BPSClinical;Pwd=BPS_c!lnLc41;");

    return db->open();
}

obstRecord MainWindow::getBasicDetails(QString firstName, QString lastName, bool dobUnknown, QDate dob, QString medicareNo)
{
    _obst_list.clear();
    obstRecord failed_obst;

    QSqlQuery q;
    q.setForwardOnly(true);

    QString sql = "SELECT [INTERNALID] "
            " ,[RECORDSTATUS] "
            " ,[TITLECODE] "
            " ,[FIRSTNAME] "
            " ,[MIDDLENAME] "
            " ,[SURNAME] "
            " ,[ADDRESS1] "
            " ,[ADDRESS2] "
            " ,[CITY] "
            " ,[POSTCODE] "
            " ,[DOB] "
            " ,[HOMEPHONE] "
            " ,[WORKPHONE] "
            " ,[MOBILEPHONE] "
            " ,[MEDICARENO] "
            " ,[MEDICARELINENO] "
            " ,[MEDICAREEXPIRY] "
            " ,[OTHERNOTES] "
            " FROM [PATIENTS] "
            " WHERE 1 = 1 ";

    if (firstName.length() > 1)
    {
        sql += " AND FIRSTNAME LIKE :firstname ";
    }

    if (lastName.length() > 1)
    {
        sql += " AND SURNAME LIKE :lastname ";
    }

    if (!dobUnknown)
    {
        sql += " AND DOB = :dob ";
    }

    if (medicareNo.length() > 0)
    {
        sql += " AND MEDICARENO = :medicare_no ";
    }

    if (!q.prepare(sql))
    {
        qDebug() << "Couldn't prepare create patient query:" << q.lastError().text();

        return failed_obst;
    }

    q.bindValue(":firstname", "%" + firstName + "%");
    q.bindValue(":lastname", "%" + lastName + "%");
    q.bindValue(":dob", dob.toString("yyyy-MM-dd"));
    q.bindValue(":medicare_no", medicareNo);

    if(q.exec())
    {
        while (q.next())
        {
            obstRecord obst;

            obst.patient_id = q.value("INTERNALID").toInt();
            obst.firstName = q.value("FIRSTNAME").toString().trimmed();
            obst.lastName = q.value("SURNAME").toString().trimmed();
            obst.dob = q.value("DOB").toDate();
            obst.phone_home = q.value("HOMEPHONE").toString().trimmed();
            obst.phone_mobile = q.value("MOBILEPHONE").toString().trimmed();
            obst.phone_work = q.value("WORKPHONE").toString().trimmed();
            obst.medicareNo = q.value("MEDICARENO").toString().trimmed();
            obst.medicareLineNo = q.value("MEDICARELINENO").toString().trimmed();
            obst.medicareExpiry = q.value("MEDICAREEXPIRY").toString().trimmed();
            obst.postcode = q.value("POSTCODE").toString().trimmed();
            obst.address1 = q.value("ADDRESS1").toString().trimmed();
            obst.address2 = q.value("ADDRESS2").toString().trimmed();
            obst.suburb = q.value("CITY").toString().trimmed();

            // can't find language / country of birth / interpreter needed from BP
            // but that could be cause I'm developing against BP v1.8.4

            _obst_list.append(obst);
        }

        if (_obst_list.count() == 0)
        {
            qDebug() << "patient details not found";
            return failed_obst;
        }

        populateSearchResults(_obst_list);

        // obstRecord chosen_obst = searchResults->selected_obst;

        // return chosen_obst;
    }

    return failed_obst;
}

QString MainWindow::capitalizeString(QString str)
{
    if (str.length() > 0)
    {
        str = str.toLower();
        str[0] = str[0].toUpper();
    }

    return str;
}

void MainWindow::savePage1(obstRecord obst)
{
    QJsonObject json;
    json["surname"] = capitalizeString(obst.lastName);
    json["middle_name"] = capitalizeString(obst.middleName);
    json["first_name"] = capitalizeString(obst.firstName);
    json["country_of_birth"] = "";
    json["dob"] = obst.dob.toString("dd/MM/yyyy");
    json["postcode"] = obst.postcode;
    json["address1"] = obst.address1;
    json["address2"] = obst.address2;
    json["medicare_no"] = obst.medicareNo;
    json["medicare_line_no"] = obst.medicareLineNo;
    json["medicare_expiry"] = obst.medicareExpiry;
    json["interpreter"] = "";
    json["language"] = "";
    json["mobile"] = obst.phone_mobile;

    QJsonDocument doc(json);
    QByteArray page1 = doc.toJson(QJsonDocument::Indented);

    saveFile(page1, "page1.txt");
}

void MainWindow::savePage2(obstRecord obst)
{
    QJsonObject json;
    json["height"] = obst.patient_height;
    json["weight"] = obst.patient_weight;
    json["known_conditions"] = obst.known_conditions;

    QJsonDocument doc(json);
    QByteArray page2 = doc.toJson(QJsonDocument::Indented);

    saveFile(page2, "page2.txt");
}

void MainWindow::saveFile(QByteArray data, QString fileName)
{
    if (!QDir("C:/OBST").exists())
    {
        QDir().mkdir("C:/OBST");
    }

    QFile file("C:/OBST/" + fileName);
    file.open(QIODevice::WriteOnly);
    file.write(data);
    file.close();
}

void MainWindow::on_chkDobUnknown_toggled(bool checked)
{
    if (checked)
    {
        ui->dateEditDOB->setEnabled(false);
    }
    else
    {
        ui->dateEditDOB->setEnabled(true);
    }
}

void MainWindow::populateSearchResults(QList<obstRecord> obst_list)
{
    QStandardItemModel *model = new QStandardItemModel(obst_list.count(), 3, this);
    model->setHorizontalHeaderItem(0, new QStandardItem(QString("Last Name")));
    model->setHorizontalHeaderItem(1, new QStandardItem(QString("First Name")));
    model->setHorizontalHeaderItem(2, new QStandardItem(QString("DOB")));

    ui->tblviewSearchResults->setModel(model);

    for (int i = 0; i < obst_list.count(); i++)
    {
        model->setItem(i, 0, new QStandardItem(obst_list.at(i).lastName));
        model->setItem(i, 1, new QStandardItem(obst_list.at(i).firstName));
        model->setItem(i, 2, new QStandardItem(obst_list.at(i).dob.toString("dd/MM/yyyy")));
    }

    connect(ui->tblviewSearchResults->selectionModel(), SIGNAL(currentChanged(const QModelIndex& , const QModelIndex&)), this, SLOT(resultSelected(const QModelIndex& , const QModelIndex&)));
}

void MainWindow::loadPage2Details(obstRecord &currentRecord)
{
    getObservations(currentRecord);
    getPastHistory(currentRecord);


}

void MainWindow::resultSelected(const QModelIndex& current, const QModelIndex& /* previous */)
{
    _rowSelected = current.row();

    if (_rowSelected > -1)
    {
        ui->btnGenerateForms->setEnabled(true);
    }
    else
    {
        ui->btnGenerateForms->setEnabled(false);
    }


}

void MainWindow::on_btnGenerateForms_clicked()
{
    obstRecord currentRecord = _obst_list.at(_rowSelected);
    savePage1(currentRecord);

    loadPage2Details(currentRecord);
    savePage2(currentRecord);
}

void MainWindow::getObservations(obstRecord &currentRecord)
{
    QSqlQuery q;
    q.setForwardOnly(true);

    QString sql = "SELECT TOP 1 DATAVALUE AS patientWeight FROM OBSERVATIONS WHERE INTERNALID = :patient_id AND RECORDSTATUS = 1 AND DATANAME = 'Weight' "
            " ORDER BY OBSDATE DESC ";

    QString sql2 = "SELECT TOP 1 DATAVALUE AS patientHeight FROM OBSERVATIONS WHERE INTERNALID = :patient_id AND RECORDSTATUS = 1 AND DATANAME = 'Height' "
            " ORDER BY OBSDATE DESC ";


    if (!q.prepare(sql))
    {
        qDebug() << "Couldn't prepare create patient query:" << q.lastError().text();
        return;
    }

    q.bindValue(":patient_id", currentRecord.patient_id);

    if(q.exec())
    {
        if (q.next())
        {
            currentRecord.patient_weight = q.value("patientWeight").toString().trimmed();
        }
    }

    if (!q.prepare(sql2))
    {
        qDebug() << "Couldn't prepare create patient2 query:" << q.lastError().text();
        return;
    }

    q.bindValue(":patient_id", currentRecord.patient_id);

    if(q.exec())
    {
        if (q.next())
        {
            currentRecord.patient_height = q.value("patientHeight").toString().trimmed();
        }
    }
}

void MainWindow::getPastHistory(obstRecord &currentRecord)
{
    QString sql = "SELECT [ITEMTEXT], [STATUSCODE] FROM [PASTHISTORY] WHERE INTERNALID = :patient_id AND STATUSCODE = 1 AND RECORDSTATUS = 1";
    QSqlQuery q;
    q.setForwardOnly(true);

    if (!q.prepare(sql))
    {
        qDebug() << "Couldn't prepare create patient query:" << q.lastError().text();
        return;
    }

    q.bindValue(":patient_id", currentRecord.patient_id);

    QStringList pastHistoryList;

    if(q.exec())
    {
        while (q.next())
        {
            pastHistoryList.append(q.value("ITEMTEXT").toString().trimmed());
        }
    }

    currentRecord.known_conditions = pastHistoryList.join(", ");
}
