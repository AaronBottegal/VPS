#include "vps_scripts_base.h"
#include <QDebug>

/*
 *
 * SCRIPTS BASE STUFF.
 *
*/

QJsonDocument VPS_Script_Document_Blank; //Special use for bailing, always .isempty()/size==0.
QJsonObject VPS_Script_Object_Blank;     //Special use for bailing, always .isempty()/size==0.

VPS_Scripts_Base::VPS_Scripts_Base()
{
    //Add to running scripts.
    this->script_step = 0; //No steps.
    this->waiting_for_reply = false;
    this->script_id = QUuid::createUuid(); //Create UUID V4 for script.

    return;
}

VPS_Scripts_Base::~VPS_Scripts_Base()
{
    return;
}

//Generic base class features.

void VPS_Scripts_Base::stash_obs_msg_reply(QJsonDocument &doc)
{
    //TODO: Add data to self.
    this->obs_replies.push_back(doc); //Add data to it.
    //TODO: Get rid of header reply value portion.
    return;
};

void VPS_Scripts_Base::stash_obs_msg(QJsonDocument &doc)
{
    //TODO: Add data to self.
    this->obs_messages.push_back(doc); //Add data to it.
    //TODO: Get rid of header reply value portion.
    return;
};

void VPS_Scripts_Base::CreateOBSRequest(void)
{
    //Clear the data held.
    request_doc = VPS_Script_Document_Blank;       //Blank it.
    QJsonObject manip_root = request_doc.object(); //Get root obj.

    //Add the op to it.
    manip_root["op"] = message_op; //Add message ID.

    if (message_op == 6) { //Add
        QJsonObject data_ex;
        data_ex["requestType"] = message_request_type;                   //Add type.
        data_ex["requestId"] = script_id.toString(QUuid::WithoutBraces); //Add UUID as ID.
        data_ex["requestData"] = message_request_data; //Add data object for anything else.
        manip_root["d"] = data_ex;                     //Add extra data.
    }

    request_doc.setObject(manip_root); //Add back to.

    qDebug().noquote() << "Put together message to send: "
                       << request_doc.toJson(QJsonDocument::Compact); //Show data we made.

    return;
}

void VPS_Scripts_Base::ScriptErrorReport(void)
{
    qDebug() << "SCRIPT ERROR: ";
    qDebug() << "SCRIPT STEP: " << this->script_step;
    //REQUEST
    if (this->obs_messages.count() > 1)
        qDebug().noquote() << "SCRIPT 2ND LAST REQUESTS SENT: "
                           << this->obs_messages[this->obs_messages.count() - 2].toJson(
                                  QJsonDocument::Compact); //Output last packet.
    //REPLY
    if (this->obs_replies.count() > 0)
        qDebug().noquote() << "OBS REPLY TO THAT PREVIOUS MESSAGE: "
                           << this->obs_replies[this->obs_replies.count() - 1].toJson(
                                  QJsonDocument::Compact); //Output last.
    //REQUESTS
    if (this->obs_messages.count() > 0) {
        qDebug().noquote() << "SCRIPT LAST REQUEST SENT: "
                           << this->obs_messages[this->obs_messages.count() - 1].toJson(
                                  QJsonDocument::Compact); //Output last packet.
    } else
        qDebug() << "NO SENT STUFF BEFORE ERROR! T.T";

    return;
}