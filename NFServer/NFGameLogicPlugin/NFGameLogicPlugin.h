// -------------------------------------------------------------------------
//    @FileName      :    NFZoneLogicPlugin.h
//    @Author           :    LvSheng.Huang
//    @Date             :    2012-07-14 08:51
//    @Module           :   NFZoneLogicPlugin
//
// -------------------------------------------------------------------------

#ifndef NFC_GAMELOGICLUGIN_H
#define NFC_GAMELOGICLUGIN_H

#include "NFComm/NFPluginModule/NFIPlugin.h"
#include "NFComm/NFPluginModule/NFIPluginManager.h"

class NFGameLogicPlugin : public NFIPlugin
{
public:
    NFGameLogicPlugin( NFIPluginManager* p )
    {
        pPluginManager = p;
    }
    virtual const int GetPluginVersion();

    virtual const std::string GetPluginName();

    virtual void Install();

    virtual void Uninstall();
};

#endif
