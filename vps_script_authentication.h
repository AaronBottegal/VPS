#ifndef VPS_SCRIPT_AUTHENTICATION_H
#define VPS_SCRIPT_AUTHENTICATION_H

#include <QJsonObject>
#include "vps.h"

class VPS_Script_Authentication : public VPS_ScriptChainBase
{
public:
    VPS_Script_Authentication();
    ~VPS_Script_Authentication();
    void process_reply(QJsonObject *json_object) override;

protected:
private:
};

#endif // VPS_SCRIPT_AUTHENTICATION_H
