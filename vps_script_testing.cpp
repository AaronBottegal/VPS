#include "vps_script_testing.h"
#include "vps_helpers.h"

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

QJsonArray kinds_arr;

QJsonDocument VPS_Script_Testing::process_reply(
    QJsonDocument *json_doc) //Virtual function called for replies to step script.
{
    QJsonObject Creating_Reply; //Used to make reply.
    QJsonValue jsval;
    bool send_to_obs = false; //Init it.

    if (json_doc)
        stash_obs_msg_reply(*json_doc); //Add message data we got, if any.

    //TODO: Process each step for script.
    switch (this->script_step) {
    case 0:
        qDebug("First command sending: ");
        set_message_op(6);                             //Request.
        set_message_data(VPS_Script_Object_Blank);     //No data here.
        set_message_type("GetInputKindList");          //Message request type.
        Creating_Reply["unversioned"] = false;         //Set parameter on request.
        set_message_request_data(Creating_Reply);      //Copy from this.
        CreateOBSRequest();                            //Create the request inside us.
        //qDebug() << request_doc.toJson(QJsonDocument::Compact); //Show JSON.
        waiting_for_reply = true; //We wait for a reply, no wake until we have one.
        send_to_obs = true;       //Send it.
        break;
    case 1:
        jsval = getJsonValueByPath(obs_replies[0], {"d", "responseData", "inputKinds"});
        if (jsval.isArray() != true) {
            this->script_step = 3; //Not array, skippy to next things.
            break;
        }
        kinds_arr = jsval.toArray(); //Make it an array.
        qDebug() << "Kinds arr: " << kinds_arr;
        Q_FALLTHROUGH(); //Fall through to next case.
    case 2:              //Fall-through first run.
        qDebug() << "Run case 2...";
        if (kinds_arr.count() == 0) {
            waiting_for_reply = false; //Not waiting, run again soon.
            this->script_step++;
            break;
        } else
            this->script_step = 1;                                //Re-do step 2 later.
        Creating_Reply["inputKind"] = kinds_arr.at(0).toString(); //Set parameter on request.
        set_message_request_data(Creating_Reply);                 //Copy from this.
        set_message_op(6);                                        //Request.
        set_message_data(VPS_Script_Object_Blank);                //No data here.
        set_message_type("GetInputDefaultSettings");              //Message request type.
        CreateOBSRequest();                                       //Create the request inside us.
        //qDebug() << request_doc.toJson(QJsonDocument::Compact);   //Show JSON.
        waiting_for_reply = true; //We wait for a reply, no wake until we have one.
        send_to_obs = true;       //Send it.
        kinds_arr.removeFirst();  //Take it.
        break;
    default:
        qDebug("We have removed the test script!");
        this->script_step = -1;           //Destroy value.
        return VPS_Script_Document_Blank; //No sending anything.
    }

    if (send_to_obs == true) { //If true, next step and send it.
        this->script_step++;   //Next script step if we send something.
        stash_obs_msg(request_doc); //Add it to history.
        return request_doc;         //Return the full doc to actually send from this script.
    }

    waiting_for_reply = false;        //Not waiting, error probably.
    return VPS_Script_Document_Blank; //Reply nothing, for whatever reason.
}