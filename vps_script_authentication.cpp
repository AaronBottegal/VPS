#include "vps_script_authentication.h"

VPS_Script_Authentication::VPS_Script_Authentication() {

};

VPS_Script_Authentication::~VPS_Script_Authentication() {

};

void VPS_Script_Authentication::process_reply(QJsonObject *json_object)
{
    switch (this->script_step) {
    default:
        this->script_step = -1; //Done.
    }

    return;
}