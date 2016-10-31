#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QHash>
#include <QDate>
#include <random>

namespace Ui {
class MainWindow;
}

struct obstRecord
{
    int patient_id;

    QString firstName;
    QString middleName;
    QString lastName;
    QDate dob;

    QString medicareNo;
    QString medicareLineNo;
    QString medicareExpiry;

    QString phone_mobile;
    QString phone_home;
    QString phone_work;

    QString address1;
    QString address2;
    QString suburb;
    QString postcode;

    QString patient_weight;
    QString patient_height;

    QString known_conditions;
};

typedef std::mt19937 MyRNG;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void saveFile(QByteArray data, QString fileName);
    obstRecord getBasicDetails(QString firstName, QString lastName, bool dobUnknown, QDate dob, QString medicareNo);

    void savePage1(obstRecord obst);
    void populateSearchResults(QList<obstRecord> obst_list);
    int _rowSelected;

    void loadPage2Details(obstRecord &currentRecord);
    void getObservations(obstRecord &currentRecord);
    void getPastHistory(obstRecord &currentRecord);
    void savePage2(obstRecord obst);
public slots:
    void resultSelected(const QModelIndex &current, const QModelIndex &);

private slots:
    void on_btnQuit_clicked();
    void on_btnFind_clicked();

    void on_chkDobUnknown_toggled(bool checked);

    void on_btnGenerateForms_clicked();

private:
    QSqlDatabase *db;
    Ui::MainWindow *ui;
    MyRNG rng;
    QString _server;
    QList<obstRecord> _obst_list;

    bool connectToDatabase();
    QString capitalizeString(QString str);
};

#endif // MAINWINDOW_H
