#ifndef VPS_H
#define VPS_H

#include <QAbstractSocket>
#include <QMainWindow>
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>

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
    void add_name(QString str);
    quint32 script_step; //Step we're at in the script process runner.
    QVector<QJsonObject> script_past_data; //Past data packets we processed/requested for reference later in the script.
    //For making JSON Objects.
    QJsonDocument json_doc; //Document maker.
    //JSON Structure for the core node: {op:int,d:obj_data}
    //JSON Structure for obj_data: {requestType:str,requestId:str,requestData:req_data}
    //JSON Structure for req_data: {per-command}
    quint8 json_core_op; //Op int value.
    QString json_core_request_id;
    QString json_core_request_type;
    QJsonObject json_request_attrs; //Under "d" in core of packet, contains "requestType", "requestId", and optional request data.
    QJsonObject json_request_data; //Request data under "requestData" that changes per-command.
    //JSON Functions
    void set_json_op_id(quint8 val);
    void set_json_request_type(QString &str);
    void set_json_request_id(QString &str);
    //Virtual function for processing replies.
    virtual void process_reply(QJsonObject *json_object) = 0; //Pure virtual function spec.
protected:
    void add_reply_data(QJsonObject &json_object);    //Internal function for adding JSON Data for this script invokation.
private:
};



class VPS_Script_Testing : public VPS_ScriptChainBase {
public:
    void process_reply(QJsonObject *json_object) override; //Script runner we link to.
protected:
private:
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
