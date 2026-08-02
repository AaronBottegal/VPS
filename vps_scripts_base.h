#ifndef VPS_SCRIPTS_BASE_H
#define VPS_SCRIPTS_BASE_H

#include <QJsonObject>
#include <QJsonValue>
#include <QObject>
#include <QUuid>
#include <QVector>

extern QJsonDocument VPS_Script_Document_Blank; //Special use for bailing, always .isempty()/size==0.
extern QJsonObject VPS_Script_Object_Blank; //Special use for bailing, always .isempty()/size==0.

class VPS_Scripts_Base
{
public:
    VPS_Scripts_Base();              //Init.
    virtual ~VPS_Scripts_Base() = 0; //Destroy.

    //Virtual function for processing replies.
    virtual QJsonDocument process_reply(QJsonDocument *doc) = 0; //Pure virtual function spec.

    //
    //Variables.
    //
    bool waiting_for_reply; //Wait for OBS data back.
    QUuid script_id;        //UUID for identity.

    qint32 script_step = 0;            //Step we're at in the script process runner.
    QVector<QJsonDocument> obs_replies;  //Past data packets we processed back.
    QVector<QJsonDocument> obs_messages; //Past messages sent.

    //For making JSON Objects.
    QJsonDocument request_doc; //Document maker to return.

    //JSON Structure for the core node: {op:int,d:obj_data}
    //JSON Structure for obj_data: {requestType:str,requestId:str,requestData:req_data}
    //JSON Structure for req_data: {per-command}
    quint8 message_op; //Op int value.
    QJsonObject
        message_data; //Under "d" in core of packet, contains "requestType", "requestId", and add'l request data under "requestData"
    QString message_request_type;     //Reuqest type. Name, E.g. SetCurrentProgramScene
    QJsonObject message_request_data; //Request data under "requestData" that changes per-command.
    //Getters/Setters for those.
    void set_message_op(const quint8 &set) { message_op = set; };
    void set_message_data(const QJsonObject &set) { message_data = set; };
    void set_message_type(const QString &set) { message_request_type = set; };
    void set_message_request_data(const QJsonObject &set) { message_request_data = set; };

protected:
    //Message data stashing helpers we use.
    void stash_obs_msg_reply(QJsonDocument &json_object);
    void stash_obs_msg(QJsonDocument &json_object);
    //Make the request from data above.
    void ScriptErrorReport(void);
    void CreateOBSRequest(void);

private:
};

#endif // VPS_SCRIPTS_BASE_H
