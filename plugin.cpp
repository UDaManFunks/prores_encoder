#include "plugin.h"
#include "uisettings_controller.h"
#include "prores_encoder.h"

#include <assert.h>
#include <cstring>

static const uint8_t pMyUUID[] = { 0x5c, 0x42, 0xce, 0x60, 0x45, 0x11, 0x4f, 0x58, 0x87, 0xde, 0xf3, 0x02, 0x80, 0x1e, 0x7b, 0xbd };

using namespace IOPlugin;

StatusCode g_HandleGetInfo(HostPropertyCollectionRef* p_pProps)
{
    StatusCode err = p_pProps->SetProperty(pIOPropUUID, propTypeUInt8, pMyUUID, 16);
    if (err == errNone)
    {
        err = p_pProps->SetProperty(pIOPropName, propTypeString, "Sample Plugin", strlen("Sample Plugin"));
    }

    return err;
}

StatusCode g_HandleCreateObj(unsigned char* p_pUUID, ObjectRef* p_ppObj)
{
    if (memcmp(p_pUUID, ProResEncoder::s_UUID, 16) == 0)
    {
        *p_ppObj = new ProResEncoder();
        return errNone;
    }

    return errUnsupported;
}

StatusCode g_HandlePluginStart()
{
    // perform libs initialization if needed
    return errNone;
}

StatusCode g_HandlePluginTerminate()
{
    return errNone;
}

StatusCode g_ListCodecs(HostListRef* p_pList)
{
    StatusCode err = ProResEncoder::s_RegisterCodecs(p_pList);
    if (err != errNone)
    {
        return err;
    }

    return errNone;
}

StatusCode g_ListContainers(HostListRef* p_pList)
{
    return errNone;

}

StatusCode g_GetEncoderSettings(unsigned char* p_pUUID, HostPropertyCollectionRef* p_pValues, HostListRef* p_pSettingsList)
{
    if (memcmp(p_pUUID, ProResEncoder::s_UUID, 16) == 0)
    {
        return ProResEncoder::s_GetEncoderSettings(p_pValues, p_pSettingsList);
    }

    return errNoCodec;
}
