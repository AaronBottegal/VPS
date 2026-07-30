#include "vps.h"
#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QTimer>
#include <QWebSocket>
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
    if (outfile.open(QIODevice::Append | QIODevice::Text))
        qDebug() << "Failed to open outfile of OBS replies.\n"; //Open as text.

    //qDebug("Clicking button...");
    //on_BTN_Connect_clicked(); //Click button automagically.

    //Script timer for commands.
    connect(&check_script_timer, &QTimer::timeout, this, &VPS::timed_script_check);
}

VPS::~VPS()
{
    //Deconstruct.
    if (outfile.isOpen()) {
        outfile.flush();
        outfile.close(); //Close file when deconstructing.
    }

    delete ui; //Free UI.
}

void VPS::onConnected()
{
    qDebug() << "Connected to OBS instance.";

    //Turn on script timer.
    check_script_timer.start(10);
    return;
}

void VPS::onDisconnected()
{
    //Turn off script timer.
    check_script_timer.stop(); //Stop script timer.

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
    ui->TXT_DEBUG_RECV->append(msg);

    QJsonDocument json_data = QJsonDocument::fromJson(msg.toUtf8()); //Turn it in to data.

    VPS::process_websock_data(json_data);

    QTextStream helper(&outfile); //Set up streamer.
    helper << msg << "\n\n";      //Output data to it.

    return;
}

void VPS::framerecv(const QString &msg, bool fin)
{
    qDebug() << "MESSAGE FRAME: " << msg;
    ui->TXT_DEBUG_RECV->append(msg);

    if (fin)
        return;

    return;
}

void VPS::timed_script_check()
{
    //qDebug("Script tick ran. (10ms)");

    //Check the scripts.
    if (disable_timed_check == true)
        return; //Don't do it.

    //Check scripts here.
    for (auto it = active_scripts.begin(); it != active_scripts.end();) {
        if ((*it)->script_step == -1) {
            delete *it;
            it = active_scripts.erase(it); //Take out of scripts.
            continue;                      //Skip to next.
        }

        if ((*it)->waiting_for_reply == true) //Leave if waiting.
            continue;

        (*it)->process_reply(NULL); //Run step otherwise.

        it++; //Next iterator.
    };

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
        return; //Failed, bad message.

    //Op fetch.
    int data_op;
    if (obj.contains("op"))
        data_op = obj.value("op").toInt();
    else
        return;

    //Tell us.
    qDebug().noquote() << "Data op: " << data_op;

    if (data_op >= 3) {
        qDebug() << "Op GTE 3:";
    }

    //Process data op
    switch (data_op) {
    case 0: {
        OBS_verification(obj);
        break;
    }
    case 1: { //Identify, shouldn't get.
        qDebug("Invalid packet from server data op 1.");
        break;
    }
    case 2: { //Identified.
        OBS_verification(obj);
        break;
    }
    case 3: { //Reidentify.
        OBS_verification(obj);
        break;
    }
    case 4: { //Doesn't exist.
        break;
    }
    case 5: { //Event.
        //Pass to listener scripts.
        break;
    }
    case 6: { //Request.
        //Shouoldn't get.
        break;
    }
    case 7: { //Reply from request.
        //Send reply to script.
        QString recv_request_type_name
            = obj.value("d").toObject().value("requestType").toString(); //Get script name.
        QString recv_request_id = obj.value("d").toObject().value("requestId").toString(); //Get ID.

        //Send it to the script runner.
        for (auto &ptr : std::as_const(active_scripts)) {
            if (ptr->script_name_id == recv_request_type_name)
                ptr->process_reply(&obj);
        }

        //Done.
        break;
    };
    case 8: { //RequestBatch.
        break;
    }
    case 9: { //RequestBatchResponse.
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

void VPS::on_BTN_OTHER_clicked()
{
    //Do information gathering.
}

/*
 *
 * SCRIPT BASE CLASS.
 *
*/


VPS_ScriptChainBase::VPS_ScriptChainBase() {
    //Add to running scripts.
    this->script_step = 0;  //No steps.
    this->waiting_for_reply = false;
    //active_scripts.push_back(this); //Add self to list.
    return;
}


VPS_ScriptChainBase::~VPS_ScriptChainBase() {
    //Remove from running scripts.
    //active_scripts.removeAll(this); //Remove from active scripts.
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

VPS_Script_Testing::VPS_Script_Testing()
{
    //No extras.
    return;
}

VPS_Script_Testing::~VPS_Script_Testing()
{
    //No extras to delete.
    return;
}

void VPS_Script_Testing::process_reply(QJsonObject *json_data) { //Virtual function called for replies to step script.
    if (json_data) add_reply_data(*json_data); //Add data we got.

    //TODO: Process each step for script.
    switch(this->script_step) {
    case 0:
        qDebug("We are running the test script fresh.");
        break;
    default:
        qDebug("We have removed the test script!");
        this->script_step = -1; //Destroying value.
        return;
    }

    this->script_step++; //Next script step always.

    return;
}

#include "vps_obs.cpp"
