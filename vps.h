#ifndef VPS_H
#define VPS_H

#include <QAbstractSocket>
#include <QMainWindow>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui {
class VPS;
}
QT_END_NAMESPACE

class VPS_ScriptChainBase {
public:
    VPS_ScriptChainBase(); //Init.
    ~VPS_ScriptChainBase(); //Destroy.
    //Variables.
    QString script_name_id; //ID Name given to OBS for reply.
    quint32 script_step; //Step we're at in the function script.
    QVector<QJsonObject> script_past_data; //Past data packets we processed/requested for reference later.
    //Virtual function for processing replies.
    virtual void process_reply(QJsonObject *reply) = 0; //Pure virtual function spec.
protected:
    //Internal functions.
    void add_reply_data(QJsonObject *data);
    //void end_process(void); //We end it.
private:
};

class VPS_Script_Testing : public VPS_ScriptChainBase {
    void process_reply(QJsonObject *reply) override; //Script runner we link to.
};

class VPS : public QMainWindow
{
    Q_OBJECT

public:
    VPS(QWidget *parent = nullptr);
    ~VPS();
    //Our internal crap.
    void OBS_Create_New_Scene();
    void OBS_Fetch_Input_Kind_Defaults(); //Get defaults info for each kind.

public slots:
    void onConnected();
    void onDisconnected();
    void onError(QAbstractSocket::SocketError err);
    void msgrecv(const QString &msg);
    void framerecv(const QString &msg, bool fin);
    void process_websock_data(QJsonDocument &doc);

private slots:
    //Internal buttons and whatnot.
    void on_BTN_Connect_clicked();

    void on_BTN_SCENE1_clicked();

    void on_BTN_GET_BG_clicked();

    void on_BTN_OTHER_clicked();

private:
    Ui::VPS *ui;
};

extern QString NEW_SCENE_ID;

#endif // VPS_H
