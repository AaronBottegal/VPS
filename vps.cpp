#include "vps.h"
#include <QCryptographicHash>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QString>
#include <QTimer>
#include <QWebSocket>
#include "ui_vps.h"
#include "vps.h"
#include "vps_script_testing.h" //We make this one, need info.

QFile outfile;
QWebSocket obs;
QVector<VPS_Scripts_Base *> active_scripts;
//TODO: Watcher array for re-activating scripts on events.

VPS::VPS(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::VPS)
{
    ui->setupUi(this);

    //Other stuff.

    //Websocket log of send.
    outfile.setFileName("websocket_log.txt");
    if (outfile.open(QIODevice::Append | QIODevice::Text) == false) {
        qDebug() << "Failed to open outfile of OBS replies.\n";
    }

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

void VPS::msgrecv(const QString &msg) //Whole message, not using, works fine.
{
    qDebug().noquote() << "OBS MESSAGE: " << msg; //Show msg.
    ui->TXT_DEBUG_RECV->append(msg);

    QJsonDocument json_data = QJsonDocument::fromJson(msg.toUtf8()); //Turn it in to data.

    VPS::process_websock_data(json_data);

    QTextStream helper(&outfile); //Set up streamer.
    helper << "JSON Recv: \n" << msg << "\n\n"; //Output data to it.

    return;
}

void VPS::framerecv(const QString &msg, bool fin)
{
    static QString frame_recv_buf;

    qDebug().noquote() << "OBS MESSAGE FRAME:" << msg;

    frame_recv_buf.append(msg);

    if (fin) {
        ui->TXT_DEBUG_RECV->append(frame_recv_buf); //Append to buffer.
        QJsonDocument json_data = QJsonDocument::fromJson(
            frame_recv_buf.toUtf8()); //Turn it in to json data type.

        VPS::process_websock_data(json_data); //Process it.

        outfile.write(frame_recv_buf.toUtf8()); //Write data out.
        outfile.write(QString("\n").toUtf8());  //Add new line.

        frame_recv_buf.clear(); //No more text.
    }

    return;
}

void VPS::OBS_Send_Request_Simple(QString msg)
{
    //Add to UI.
    ui->TXT_DEBUG_SEND->append(msg);
    ui->TXT_DEBUG_SEND->append("\n"); //Spaces between.

    //Send to socket.
    obs.sendTextMessage(msg); //Send over socket.
    obs.flush();              //Make sure it is sent. Optional, test without too randomly.

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
            qDebug() << "Script completed!";
            delete *it;
            it = active_scripts.erase(it); //Take out of scripts.
            continue;                      //Skip to next.
        }

        if ((*it)->waiting_for_reply == true) { //Leave if waiting, will come via socket.
            it++;                               //To next script iterator.
            continue;
        }

        QJsonDocument ret_doc = (*it)->process_reply(NULL); //Run step manually otherwise.

        if (!ret_doc.isEmpty()) //Send the document returned to OBS if given one.
            OBS_Send_Request_Simple(ret_doc.toJson(QJsonDocument::Compact));

        it++; //Next script iterator.
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
        QString recv_request_id
            = obj.value("d").toObject().value("requestId").toString(); //Get ID of script.

        //Send it to the script runner.
        for (auto &ptr : std::as_const(active_scripts)) {
            QJsonDocument reply_data;
            if (ptr->waiting_for_reply == false)
                continue;                                         //Not waiting for reply, bail.
            if (ptr->script_id.toString(QUuid::WithoutBraces)
                == recv_request_id) {                             //This script sent it.
                ptr->waiting_for_reply = false;                   //Not anymore.
                reply_data = ptr->process_reply(&doc);            //Process the reply in it.
                if (!reply_data.isEmpty())
                    OBS_Send_Request_Simple(
                        reply_data.toJson(QJsonDocument::Compact)); //Send the reply to OBS.
            }
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
    connect(&obs, &QWebSocket::connected, this, &VPS::onConnected);
    connect(&obs, &QWebSocket::disconnected, this, &VPS::onDisconnected);
    //connect(&obs, SIGNAL(errorOccured(err)), this, SLOT(onError(err)));

    qDebug("Connect signal for text receive from obs server...");
    //connect(&obs, SIGNAL(textMessageReceived(QString)), this, SLOT(msgrecv(QString)));
    connect(&obs, &QWebSocket::textFrameReceived, this, &VPS::framerecv);

    qDebug() << obs.error();
    obs.open(url); //Open URL.
    qDebug() << obs.error();

    qDebug() << "Trying to connect...";

    return;
}


void VPS::on_BTN_SCENE1_clicked()
{ //Scene 1
    //Create next scene.
    OBS_Create_New_Scene();
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
    OBS_Send_Request_Simple(packet); //Send the data.

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
    OBS_Send_Request_Simple(packet); //Send the data.

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
    OBS_Send_Request_Simple(packet); //Send the data.

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
    OBS_Send_Request_Simple(packet); //Send the data.

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
    OBS_Send_Request_Simple(packet); //Send the data.

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
    OBS_Send_Request_Simple(packet); //Send the data.

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
    OBS_Send_Request_Simple(packet); //Send the data.

    return;
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
    OBS_Send_Request_Simple(packet); //Send the data.

    //Create packet here.
    obj_root.empty();
    obj_data.empty();
    request_data.empty();
}

void VPS::on_BTN_OTHER_clicked()
{
    //Do information gathering.
}

void VPS::OBS_verification(QJsonObject dobj)
{ //Internal OBS handling stuff.
    //Op fetch.
    int data_op;
    if (dobj.contains("op"))
        data_op = dobj.value("op").toInt();
    else
        return; //Failure, need type.

    //Tell us.
    qDebug().noquote() << "Data op in OBS Verify: " << data_op;

    if (data_op >= 3) {
        qDebug() << "OBS Verify: Verify GTE 3";
    }

    switch (data_op) {
    case 0: {
        //Packet 0, auth to.
        //Auth packet.
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
        salt = dobj.value("d").toObject().value("authentication").toObject().value("salt").toString();
        challenge = dobj.value("d")
                        .toObject()
                        .value("authentication")
                        .toObject()
                        .value("challenge")
                        .toString();
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
        OBS_Send_Request_Simple(packet); //Send the data.
    } break;
    case 2: {
        //Auth okay.
        qDebug("OBS Verify: AUTH OK");
        //Add script runner for testing.
        VPS_Script_Testing *script_adding = new VPS_Script_Testing; //Make the script we want ran.
        //Add it to active scripts.
        active_scripts.push_back(script_adding); //Push as active.

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
        OBS_Send_Request_Simple(packet); //Send the data.

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
        OBS_Send_Request_Simple(packet); //Send the data.

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
        OBS_Send_Request_Simple(packet); //Send the data
        */
    } break;
    }
}