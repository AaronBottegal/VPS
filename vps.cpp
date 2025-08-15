#include "vps.h"
#include <QCryptographicHash>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QWebSocket>
#include <QDebug>
#include "ui_vps.h"

QFile outfile;
QWebSocket obs;
QVector<VPS_ScriptChainBase *> active_scripts;
//TODO: Watcher array for re-activating scripts on events.



VPS::VPS(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::VPS)
{
    ui->setupUi(this);

    //Other stuff.
    outfile.setFileName("websocket.txt");
    outfile.open(QIODevice::Append | QIODevice::Text); //Open as text.

    //qDebug("Clicking button...");
    //on_BTN_Connect_clicked(); //Click button automagically.
}

VPS::~VPS()
{
    //Deconstruct.
    if (outfile.isOpen()) {
        outfile.flush();
        outfile.close(); //Close file.
    }

    delete ui; //Free UI.
}

void VPS::onConnected()
{
    qDebug() << "Connected to OBS instance.";
    return;
}

void VPS::onDisconnected()
{
    qDebug() << "Disconnected from OBS.";
    qDebug() << "Reason:" << obs.closeReason();
    return;
}

void VPS::onError(QAbstractSocket::SocketError err)
{
    qDebug() << "WEBSOCK ERROR:" << err;
}

void VPS::msgrecv(const QString &msg)
{
    //qDebug().noquote() << "MESSAGE NORML: " << msg;                  //Show msg.
    QJsonDocument json_data = QJsonDocument::fromJson(msg.toUtf8()); //Turn it in to data.

    VPS::process_websock_data(json_data);

    QTextStream helper(&outfile); //Set up streamer.
    helper << msg << "\n\n";      //Output data to it.

    return;
}

void VPS::framerecv(const QString &msg, bool fin)
{
    qDebug() << "MESSAGE FRAME: " << msg;
    if (fin) return;

    return;
}

//Protocol: https://github.com/obsproject/obs-websocket/blob/master/docs/generated/protocol.md
void VPS::process_websock_data(QJsonDocument &doc)
{
    //Object test.
    QJsonObject obj;
    if (doc.isObject())
        obj = doc.object();
    else
        return; //Failed.

    //Op fetch.
    int data_op;
    if (obj.contains("op"))
        data_op = obj.value("op").toInt();
    else
        return;

    //Tell us.
    qDebug().noquote() << "Data op: " << data_op;

    if (data_op >= 3) {
        qDebug() << doc;
    }

    //Process data op
    switch (data_op) {
    case 0: {
        QString salt;
        QString challenge;
        QByteArray salt_utf8;
        QByteArray challenge_utf8;
        //QByteArray salt_hex;
        //QByteArray challenge_hex;
        QCryptographicHash hash(QCryptographicHash::Sha256);
        QByteArray hash_data;
        QByteArray secret_base64;
        QByteArray secret_hex;
        QByteArray auth_send;

        //Gather from incoming.
        salt = obj["d"].toObject()["authentication"].toObject()["salt"].toString();
        challenge = obj["d"].toObject()["authentication"].toObject()["challenge"].toString();
        //Straight text to data byte conversions.
        salt_utf8 = salt.toUtf8();
        challenge_utf8 = challenge.toUtf8();

        qDebug() << "Hashing data points: " << ui->obs_pw->text().toUtf8()
                 << "Salt: " << salt.toUtf8() << "Challenge: " << challenge.toUtf8();

        hash_data.append(ui->obs_pw->text().toUtf8());
        hash_data.append(salt_utf8);
        hash.addData(hash_data); //Add password and salt first first.

        //Get result as base64.
        secret_hex = hash.result();
        secret_base64 = hash.result().toBase64(); //Copy base64 secret.

        //Reset hash data.
        hash.reset(); //Reset data.

        //Now add the challenge after the base64 secret.
        hash_data.clear();               //Clear combiner.
        hash_data.append(secret_base64); //Add Base64 and challenge.
        hash_data.append(challenge_utf8);
        hash.addData(hash_data); //Hash it.

        auth_send = hash.result().toBase64(
            QByteArray::Base64Encoding); //Final challenge hash to base64 as auth code.
        QString auth_stringed(auth_send);

        //Create auth packet here.
        QJsonDocument auth_doc;
        QJsonObject obj_root;
        QJsonObject obj_data;

        //Add data for op d key.
        obj_data["authentication"] = auth_stringed; //Auth string.
        obj_data["eventSubscriptions"] = 255;       //No events subscribed to.
        obj_data["rpcVersion"] = 1;                 //RPC Version.

        //Now combine to form the document.
        obj_root["d"] = obj_data; //D key as data object for op packet.
        obj_root["op"] = 1;       //Set op on root.
        auth_doc.setObject(obj_root);

        //Make packet from doc.
        QString packet = auth_doc.toJson(QJsonDocument::Compact);

        //Send it!
        obs.sendTextMessage(packet); //Send the data.
        break;
    }
    case 1: {
        qDebug("Invalid packet from server data op 1.");
        break;
    }
    case 2: {
        qDebug("AUTH OK");
        //Add script runner for testing.
        VPS_Script_Testing *script_adding = new VPS_Script_Testing; //Make the script we want ran.
        script_adding->add_name("AARON_TEST"); //Name it.
        //Run the script runner.
        script_adding->process_reply(nullptr); //Process it, no data because first run.





        //Shoot off packets for data back.

        //Create packet here.
        /*
        QJsonDocument auth_doc;
        QJsonObject obj_root;
        QJsonObject obj_data;
        QJsonObject request_data;
        */


        /*
        //Add data for op d key.
        obj_data["requestType"] = "GetInputList";
        obj_data["requestId"] = "N/A";
        if (!request_data.isEmpty())
            obj_data["requestData"] = request_data; //Add if not empty.

        //Now combine to form the document.
        obj_root["d"] = obj_data; //D key as data object for op packet.
        obj_root["op"] = 6;       //Set op for request.
        auth_doc.setObject(obj_root);

        //Make packet from doc.
        QString packet = auth_doc.toJson(QJsonDocument::Compact);

        //Send it!
        obs.sendTextMessage(packet); //Send the data.

        //Create packet here.
        obj_root.empty();
        obj_data.empty();
        request_data.empty();

        //Add data for op d key.
        obj_data["requestType"] = "GetInputKindList";
        obj_data["requestId"] = "N/A";
        if (!request_data.isEmpty())
            obj_data["requestData"] = request_data; //Add if not empty.

        //Now combine to form the document.
        obj_root["d"] = obj_data; //D key as data object for op packet.
        obj_root["op"] = 6;       //Set op for request.
        auth_doc.setObject(obj_root);

        //Make packet from doc.
        packet = auth_doc.toJson(QJsonDocument::Compact);

        //Send it!
        obs.sendTextMessage(packet); //Send the data.

        //Create packet here.
        obj_root.empty();
        obj_data.empty();
        request_data.empty();

        //Add data for op d key.
        obj_data["requestType"] = "GetInputSettings";
        obj_data["requestId"] = "VCAP";
        request_data["inputName"] = "Video Capture Device";
        if (!request_data.isEmpty())
            obj_data["requestData"] = request_data; //Add if not empty.

        //Now combine to form the document.
        obj_root["d"] = obj_data; //D key as data object for op packet.
        obj_root["op"] = 6;       //Set op for request.
        auth_doc.setObject(obj_root);

        //Make packet from doc.
        packet = auth_doc.toJson(QJsonDocument::Compact);

        //Send it!
        obs.sendTextMessage(packet); //Send the data
        */
        break;
    }
    case 3: {
        break;
    }
    case 4: {
        break;
    }
    case 5: {
        break;
    }
    case 6: {
        break;
    }
    case 7: { //Reply from request.
        QString recv_request_name = obj["d"].toObject()["requestId"].toString(); //Get string.
        //Find virtual function to pass to.

        //Send it to the script runner.
        for (const auto &ptr : active_scripts) {
            if (ptr->script_name_id == recv_request_name) ptr->process_reply(&obj);
        }

        //Done.
        break;
    };
    case 8: {
        break;
    }
    case 9: {
        break;
    }
    default: {
        break;
    }
    }

    return;
}

void VPS::on_BTN_Connect_clicked()
{
    if (obs.isValid())
        return; //Don't re-connect.

    QUrl url;

    //url.setUrl("ws://10.0.40.132:4455");
    url.setHost(ui->obs_ip->text()); //Grab IP/Pass/Port from UI.
    url.setPort(ui->obs_port->text().toInt());
    url.setPassword(ui->obs_pw->text());
    url.setScheme("ws"); //We're websocket.

    qDebug("Connect signal for connect/disconnect...");
    connect(&obs, SIGNAL(connected()), this, SLOT(onConnected()));
    connect(&obs, SIGNAL(disconnected()), this, SLOT(onDisconnected()));
    //connect(&obs, SIGNAL(errorOccured(err)), this, SLOT(onError(err)));
    qDebug("Connect signal for text receive from obs server...");
    connect(&obs, SIGNAL(textMessageReceived(QString)), this, SLOT(msgrecv(QString)));
    //connect(&obs, SIGNAL(textFrameReceived(QString, bool)), this, SLOT(framerecv(QString, bool)));

    qDebug() << obs.error();
    obs.open(url); //Open URL.
    qDebug() << obs.error();

    qDebug() << "Trying to connect...";

    return;
}

QString NEW_SCENE_ID;
quint32 scene_id = 0;

void VPS::OBS_Create_New_Scene()
{
    scene_id++;
    //QString name = QString("VPS Scene %1").arg(scene_id); //Do name manually.
    QString name = "VPS WIP SCENE DYNAMIC";

    //Create packet here.
    QJsonDocument auth_doc;
    QJsonObject obj_root;
    QJsonObject obj_data;
    QJsonObject request_data;

    //Delete old scene.
    //Add data for op d key.
    obj_data["requestType"] = "CreateScene";
    obj_data["requestId"] = "CREATENEXTSCENE";
    request_data["sceneName"] = name;

    if (!request_data.isEmpty())
        obj_data["requestData"] = request_data; //Add if not empty.

    //Now combine to form the document.
    obj_root["d"] = obj_data; //D key as data object for op packet.
    obj_root["op"] = 6;       //Set op for request.
    auth_doc.setObject(obj_root);

    //Make packet from doc.
    QString packet = auth_doc.toJson(QJsonDocument::Compact);

    //Send it!
    obs.sendTextMessage(packet); //Send the data.
    obs.flush();

    //Create packet here.
    obj_root.empty();
    obj_data.empty();
    request_data.empty();

    //Add data for op d key.
    obj_data["requestType"] = "CreateScene";
    obj_data["requestId"] = "CREATENEXTSCENE";
    request_data["sceneName"] = name;

    if (!request_data.isEmpty())
        obj_data["requestData"] = request_data; //Add if not empty.

    //Now combine to form the document.
    obj_root["d"] = obj_data; //D key as data object for op packet.
    obj_root["op"] = 6;       //Set op for request.
    auth_doc.setObject(obj_root);

    //Make packet from doc.
    packet = auth_doc.toJson(QJsonDocument::Compact);

    //Send it!
    obs.sendTextMessage(packet); //Send the data.
    obs.flush();

    //Create packet here.
    obj_root.empty();
    obj_data.empty();
    request_data.empty();

    //Add data for op d key.
    obj_data["requestType"] = "CreateSceneItem";
    obj_data["requestId"] = "ADDCAMBG";
    request_data["sourceName"] = "Blue";
    request_data["sceneName"] = name; //Adding to next (creating)

    if (!request_data.isEmpty())
        obj_data["requestData"] = request_data; //Add if not empty.

    //Now combine to form the document.
    obj_root["d"] = obj_data; //D key as data object for op packet.
    obj_root["op"] = 6;       //Set op for request.
    auth_doc.setObject(obj_root);

    //Make packet from doc.
    packet = auth_doc.toJson(QJsonDocument::Compact);

    //Send it!
    obs.sendTextMessage(packet); //Send the data.
    obs.flush();

    //Create packet here.
    obj_root.empty();
    obj_data.empty();
    request_data.empty();

    //Add data for op d key.
    obj_data["requestType"] = "CreateSceneItem";
    obj_data["requestId"] = "ADDCAMERA";
    request_data["sourceName"] = "Camera A";
    request_data["sceneName"] = name; //Adding to next (creating)

    if (!request_data.isEmpty())
        obj_data["requestData"] = request_data; //Add if not empty.

    //Now combine to form the document.
    obj_root["d"] = obj_data; //D key as data object for op packet.
    obj_root["op"] = 6;       //Set op for request.
    auth_doc.setObject(obj_root);

    //Make packet from doc.
    packet = auth_doc.toJson(QJsonDocument::Compact);

    //Send it!
    obs.sendTextMessage(packet); //Send the data.
    obs.flush();

    //Create packet here.
    obj_root.empty();
    obj_data.empty();
    request_data.empty();

    //Add data for op d key.
    obj_data["requestType"] = "CreateSceneItem";
    obj_data["requestId"] = "ADDSCENEOVERLAY";
    request_data["sourceName"] = "VPS Overlay";
    request_data["sceneName"] = name; //Adding to next (creating)

    if (!request_data.isEmpty())
        obj_data["requestData"] = request_data; //Add if not empty.

    //Now combine to form the document.
    obj_root["d"] = obj_data; //D key as data object for op packet.
    obj_root["op"] = 6;       //Set op for request.
    auth_doc.setObject(obj_root);

    //Make packet from doc.
    packet = auth_doc.toJson(QJsonDocument::Compact);

    //Send it!
    obs.sendTextMessage(packet); //Send the data.
    obs.flush();

    //Add data for op d key.
    obj_data["requestType"] = "SetCurrentPreviewScene";
    obj_data["requestId"] = "COMMITSCENE";
    request_data["sceneName"] = name;

    if (!request_data.isEmpty())
        obj_data["requestData"] = request_data; //Add if not empty.

    //Now combine to form the document.
    obj_root["d"] = obj_data; //D key as data object for op packet.
    obj_root["op"] = 6;       //Set op for request.
    auth_doc.setObject(obj_root);

    //Make packet from doc.
    packet = auth_doc.toJson(QJsonDocument::Compact);

    //Send it!
    obs.sendTextMessage(packet); //Send the data.
    obs.flush();                 //Flush it.

    //Create packet here.
    obj_root.empty();
    obj_data.empty();
    request_data.empty();

    //Add data for op d key.
    obj_data["requestType"] = "TriggerStudioModeTransition";
    obj_data["requestId"] = "DOTRANSITION";

    if (!request_data.isEmpty())
        obj_data["requestData"] = request_data; //Add if not empty.

    //Now combine to form the document.
    obj_root["d"] = obj_data; //D key as data object for op packet.
    obj_root["op"] = 6;       //Set op for request.
    auth_doc.setObject(obj_root);

    //Make packet from doc.
    packet = auth_doc.toJson(QJsonDocument::Compact);

    //Send it!
    obs.sendTextMessage(packet); //Send the data.

    return;
}

void VPS::on_BTN_SCENE1_clicked()
{ //Scene 1
    //Create next scene.
    OBS_Create_New_Scene();
}

void VPS::on_BTN_GET_BG_clicked()
{
    //Create packet here.
    QJsonDocument auth_doc;
    QJsonObject obj_root;
    QJsonObject obj_data;
    QJsonObject request_data;
    //JSON Structure for the root node: {op:int,d:obj_data}
    //JSON Structure for obj_data: {requestType:str,requestId:str,requestData:req_data}
    //JSON Structure for req_data: {per-command}

    //Delete old scene.
    //Add data for op d key.
    obj_data["requestType"] = "GetSceneItemList";
    obj_data["requestId"] = "GET_SCENE_ITEMS_INFO";
    request_data["sceneName"] = "VPS Backgrounds";

    if (!request_data.isEmpty())
        obj_data["requestData"] = request_data; //Add if not empty.

    //Now combine to form the document.
    obj_root["d"] = obj_data; //D key as data object for op packet.
    obj_root["op"] = 6;       //Set op for request.
    auth_doc.setObject(obj_root);

    //Make packet from doc.
    QString packet = auth_doc.toJson(QJsonDocument::Compact);

    //Send it!
    obs.sendTextMessage(packet); //Send the data.
    obs.flush();

    //Create packet here.
    obj_root.empty();
    obj_data.empty();
    request_data.empty();
}

void VPS::on_BTN_OTHER_clicked() {}


/*
 *
 * SCRIPT BASE CLASS.
 *
*/


VPS_ScriptChainBase::VPS_ScriptChainBase() {
    //Add to running scripts.
    this->script_step = 0;  //No steps.
    active_scripts.push_back(this); //Add self to list.
    return;
}


VPS_ScriptChainBase::~VPS_ScriptChainBase() {
    //Remove from running scripts.
    active_scripts.removeAll(this); //Remove from active scripts.
    return;
}

//Generic base class features.

void VPS_ScriptChainBase::add_name(QString str) {
    this->script_name_id = str; //Copy ref.
}

void VPS_ScriptChainBase::add_reply_data(QJsonObject &json_data) {
    //TODO: Add data to self.
    this->script_past_data.push_back(json_data); //Add data to it.
    //TODO: Get rid of header reply value portion.
    return;
};

void VPS_ScriptChainBase::set_json_op_id(quint8 val) {
    this->json_core_op = val;
    return;
}

void VPS_ScriptChainBase::set_json_request_type(QString &str) {
    this->json_core_request_type = str;
}


void VPS_ScriptChainBase::set_json_request_id(QString &str) {
    this->json_core_request_id = str;
}


/*
 *
 * SCRIPT RUNNERS / DERIVED CLASSES
 *
*/


void VPS_Script_Testing::process_reply(QJsonObject *json_data) { //Virtual function called for replies to step script.
    if (json_data) add_reply_data(*json_data); //Add data we got.

    //TODO: Process each step for script.
    switch(this->script_step) {
    default:
        break;
    case 0:
        qDebug("We are running the test script fresh.");
        break;
    }

    return;
}
