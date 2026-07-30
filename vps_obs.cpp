#include <QJsonObject>
#include <QString>
#include "ui_vps.h"
#include "vps.h"

void VPS::OBS_verification(QJsonObject dobj)
{ //Internal OBS handling stuff.
    //Op fetch.
    int data_op;
    if (dobj.contains("op"))
        data_op = dobj.value("op").toInt();
    else
        return; //Failure.

    //Tell us.
    qDebug().noquote() << "Data op in verify: " << data_op;

    if (data_op >= 3) {
        qDebug() << "Verify GTE 3";
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
        obs.sendTextMessage(packet); //Send the data.
    } break;
    case 2: {
        //Auth okay.
        qDebug("AUTH OK");
        //Add script runner for testing.
        VPS_Script_Testing *script_adding = new VPS_Script_Testing; //Make the script we want ran.
        script_adding->add_name("AARON_TEST");                      //Name it.
        //Run the script runner.
        //script_adding->process_reply(nullptr); //Process it, no data because first run.
        active_scripts.push_back(script_adding);

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
    } break;
    }
}
