#pragma once
#include <stdint.h>
#include <string>
#include <vector>
#include <memory>
#include "usb.h"
#define RAPIDJSON_ENDIAN RAPIDJSON_BIGENDIAN

struct HidAxisMapEntry {
    uint16_t usage;
    uint16_t raw;
    int16_t ButtonsReport::* field;
};

struct HidButtonMapEntry {
    uint8_t idx;
    uint8_t ButtonsReport::* field;
};

struct HidAxisInvertFlags {
    bool invertX;
    bool invertY;
    bool invertZ;
    bool invertRX;
    bool invertRY;
    bool invertRZ;
};

struct HidDeviceMapping {
    uint16_t vendorId;
    uint16_t productId;

    uint8_t subType;
    uint16_t flags;

    const HidAxisMapEntry* axisMap;
    uint8_t axisMapCount;

    const HidButtonMapEntry* buttonMap;
    uint8_t buttonMapCount;

    HidAxisInvertFlags invert;
};

struct DynamicMappingData {
    std::vector<HidAxisMapEntry> axisEntries;
    std::vector<HidButtonMapEntry> buttonEntries;
};

extern std::vector<HidDeviceMapping> g_dynamicMappings;
extern std::vector<std::unique_ptr<DynamicMappingData>> g_dynamicData;

HidDeviceMapping* FindStaticMapping(uint16_t vid, uint16_t pid);
HidDeviceMapping* FindStaticSonyMapping(uint16_t usage, uint8_t subType);

HidDeviceMapping* FindMapping(uint16_t vid, uint16_t pid);

bool LoadMappingsFromJson(const std::string& jsonString);
bool LoadMappingsFromFile(const std::string& filepath);
std::string SaveMappingsToJson();
bool SaveMappingsToFile(const std::string& filepath);

void ClearDynamicMappings();
