#ifndef VPS_SCRIPT_TESTING_H
#define VPS_SCRIPT_TESTING_H

#include <QJsonDocument>
#include "vps_scripts_base.h"

class VPS_Script_Testing : public VPS_Scripts_Base
{
public:
    VPS_Script_Testing();                                  //Constructor
    ~VPS_Script_Testing();                                 //Deconstructor.
    QJsonDocument process_reply(QJsonDocument *obj) override; //Script runner we link to.

protected:
private:
};

#endif // VPS_SCRIPT_TESTING_H
