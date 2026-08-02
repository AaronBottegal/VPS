#ifndef VPS_H
#define VPS_H

#include <QAbstractSocket>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMainWindow>
#include <QString>
#include <QTimer>
#include <QVector>
#include <QWebSocket>
#include "vps_scripts_base.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class VPS;
}
QT_END_NAMESPACE

class VPS : public QMainWindow
{
    Q_OBJECT

public:
    VPS(QWidget *parent = nullptr);
    ~VPS();

    void OBS_Create_New_Scene();
    void OBS_Fetch_Input_Kind_Defaults(); //Get defaults info for each kind.
    void OBS_verification(QJsonObject dobj); //Internal OBS handling stuff.
    void OBS_Send_Request_Simple(QString msg);
public slots:
    //QT-ish
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError err);
    void msgrecv(const QString &msg);
    void framerecv(const QString &msg, bool fin);

    //Somewhat internal.
    void timed_script_check();
    void process_websock_data(QJsonDocument &doc);

private slots:
    //Internal buttons and whatnot.
    void on_BTN_Connect_clicked();

    void on_BTN_SCENE1_clicked();

    void on_BTN_GET_BG_clicked();

    void on_BTN_OTHER_clicked();

private:
    Ui::VPS *ui;
    //Our internal crap.
    QTimer check_script_timer;
    bool disable_timed_check = false, wait_for_obs_reply = false;
};

extern QWebSocket obs;
extern QVector<VPS_Scripts_Base *> active_scripts;

#endif // VPS_H
