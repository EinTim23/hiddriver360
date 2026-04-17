#include "mapping.h"
#include <xtl.h>
#include <xkelib.h>
#include <string.h>
#include <vector>
#include <memory>
#include <sstream>
#include <fstream>
#include <rapidjson/document.h>
#include <rapidjson/writer.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/filereadstream.h>
#include <rapidjson/filewritestream.h>
#include <cstdio>

static const HidAxisMapEntry kDefaultAxisMap[] = {
    { HID_USAGE_AXIS_X,  0, &ButtonsReport::x  },
    { HID_USAGE_AXIS_Y,  0, &ButtonsReport::y  },
    { HID_USAGE_AXIS_Z,  0, &ButtonsReport::z  },
    { HID_USAGE_AXIS_RX, 0, &ButtonsReport::rx },
    { HID_USAGE_AXIS_RY, 0, &ButtonsReport::ry },
    { HID_USAGE_AXIS_RZ, 0, &ButtonsReport::rz },
};

static const HidAxisMapEntry kPs4GuitarAxisMap[] = {
    { HID_USAGE_AXIS_X, 0, &ButtonsReport::x  },
    { HID_USAGE_AXIS_Y,  0, &ButtonsReport::y  },
    { 0, PS_RAW_EXTENDED_DATA_PS4 + PS_EXTENDED_OFFSET_WHAMMY, &ButtonsReport::z  },
    { HID_USAGE_AXIS_RX, 0, &ButtonsReport::rx },
    { HID_USAGE_AXIS_RY, 0, &ButtonsReport::ry },
    { 0, PS_RAW_EXTENDED_DATA_PS4 + PS_EXTENDED_OFFSET_TILT, &ButtonsReport::rz },
};
static const HidAxisMapEntry kPs5GuitarAxisMap[] = {
    { HID_USAGE_AXIS_X, 0, &ButtonsReport::x  },
    { HID_USAGE_AXIS_Y,  0, &ButtonsReport::y  },
    { 0, PS_RAW_EXTENDED_DATA_PS5 + PS_EXTENDED_OFFSET_WHAMMY, &ButtonsReport::z  },
    { HID_USAGE_AXIS_RX, 0, &ButtonsReport::rx },
    { HID_USAGE_AXIS_RY, 0, &ButtonsReport::ry },
    { 0, PS_RAW_EXTENDED_DATA_PS5 + PS_EXTENDED_OFFSET_TILT, &ButtonsReport::rz },
};

static const HidButtonMapEntry kPlayStationButtonMapping[] = {
    { 1,  &ButtonsReport::a_button    },
    { 2,  &ButtonsReport::b_button   },
    { 0,  &ButtonsReport::x_button   },
    { 3,  &ButtonsReport::y_button },
    { 4,  &ButtonsReport::l1       },
    { 5,  &ButtonsReport::r1       },
    { 8,  &ButtonsReport::back     },
    { 9,  &ButtonsReport::start    },
    { 10, &ButtonsReport::l3       },
    { 11, &ButtonsReport::r3       },
    { 12, &ButtonsReport::xbox     },
};

// PS3 uses L3 (10) as pad flag, 360 uses R3
// PS3 uses R3 (11) as cymbal flag, 360 uses r1
// PS3 uses R1 (5) as kick 2, 360 uses L3
static const HidButtonMapEntry kPlayStationRBDrumButtonMapping[] = {
    { 1,  &ButtonsReport::a_button    },
    { 2,  &ButtonsReport::b_button   },
    { 0,  &ButtonsReport::x_button   },
    { 3,  &ButtonsReport::y_button },
    { 4,  &ButtonsReport::l1       },
    { 11,  &ButtonsReport::r1       },
    { 8,  &ButtonsReport::back     },
    { 9,  &ButtonsReport::start    },
    { 5, &ButtonsReport::l3       },
    { 10, &ButtonsReport::r3       },
    { 12, &ButtonsReport::xbox     },
};

// GH PS3 guitars swapped x and y
static const HidButtonMapEntry kPlayStationGHGuitarButtonMapping[] = {
    { 1,  &ButtonsReport::a_button    },
    { 2,  &ButtonsReport::b_button   },
    { 3,  &ButtonsReport::x_button   },
    { 0,  &ButtonsReport::y_button },
    { 4,  &ButtonsReport::l1       },
    { 5,  &ButtonsReport::r1       },
    { 8,  &ButtonsReport::back     },
    { 9,  &ButtonsReport::start    },
    { 10, &ButtonsReport::l3       },
    { 11, &ButtonsReport::r3       },
    { 12, &ButtonsReport::xbox     },
};

static const HidButtonMapEntry kDualShock3ButtonMapping[] = {
    { 14,  &ButtonsReport::a_button    },
    { 13,  &ButtonsReport::b_button   },
    { 15,  &ButtonsReport::x_button   },
    { 12,  &ButtonsReport::y_button },
    { 10,  &ButtonsReport::l1       },
    { 11,  &ButtonsReport::r1       },
    { 8,  &ButtonsReport::l2       },
    { 9,  &ButtonsReport::r2       },
    { 0,  &ButtonsReport::back     },
    { 3,  &ButtonsReport::start    },
    { 1, &ButtonsReport::l3       },
    { 2, &ButtonsReport::r3       },
    { 16, &ButtonsReport::xbox     },
    { 7, &ButtonsReport::dpad_left },
    { 5, &ButtonsReport::dpad_right },
    { 4, &ButtonsReport::dpad_up },
    { 6, &ButtonsReport::dpad_down },
};

static HidDeviceMapping kStaticSonyMapping = {
    1356, 2508,
    XINPUT_DEVSUBTYPE_GAMEPAD, 0,
    kDefaultAxisMap,  sizeof(kDefaultAxisMap) / sizeof(kDefaultAxisMap[0]),
    kPlayStationButtonMapping, sizeof(kPlayStationButtonMapping) / sizeof(kPlayStationButtonMapping[0]),
    {false, true, false, false, false, true},
};


static HidDeviceMapping kStaticPs4Mappings[] = {
    {   
        0, 0,
        XINPUT_DEVSUBTYPE_GUITAR, 0,
        kPs4GuitarAxisMap,  sizeof(kPs4GuitarAxisMap) / sizeof(kPs4GuitarAxisMap[0]),
        kPlayStationButtonMapping, sizeof(kPlayStationButtonMapping) / sizeof(kPlayStationButtonMapping[0]),
        {false, true, false, false, false, true},
    },
    {   
        0, 0,
        XINPUT_DEVSUBTYPE_DRUM_KIT, 0x000D,
        kDefaultAxisMap,  sizeof(kDefaultAxisMap) / sizeof(kDefaultAxisMap[0]),
        kPlayStationButtonMapping, sizeof(kPlayStationButtonMapping) / sizeof(kPlayStationButtonMapping[0]),
        {false, true, false, false, false, true},
    },
};


static HidDeviceMapping kStaticPs5Mappings[] = {
    {   
        0, 0,
        XINPUT_DEVSUBTYPE_GUITAR, 0,
        kPs5GuitarAxisMap,  sizeof(kPs5GuitarAxisMap) / sizeof(kPs5GuitarAxisMap[0]),
        kPlayStationButtonMapping, sizeof(kPlayStationButtonMapping) / sizeof(kPlayStationButtonMapping[0]),
        {false, true, false, false, false, true},
    },
    {   
        0, 0,
        XINPUT_DEVSUBTYPE_DRUM_KIT, 0x000D,
        kDefaultAxisMap,  sizeof(kDefaultAxisMap) / sizeof(kDefaultAxisMap[0]),
        kPlayStationButtonMapping, sizeof(kPlayStationButtonMapping) / sizeof(kPlayStationButtonMapping[0]),
        {false, true, false, false, false, true},
    },
};

static HidDeviceMapping kStaticDeviceMappings[] = {
    // ds3
    {   
        1356, 616,
        XINPUT_DEVSUBTYPE_GAMEPAD, 0,
        kDefaultAxisMap,  sizeof(kDefaultAxisMap) / sizeof(kDefaultAxisMap[0]),
        kDualShock3ButtonMapping, sizeof(kDualShock3ButtonMapping) / sizeof(kDualShock3ButtonMapping[0]),
        {false, true, false, false, false, true},
    },
    // ds4 v2
    {
        1356, 2508,
        XINPUT_DEVSUBTYPE_GAMEPAD, 0,
        kDefaultAxisMap,  sizeof(kDefaultAxisMap) / sizeof(kDefaultAxisMap[0]),
        kPlayStationButtonMapping, sizeof(kPlayStationButtonMapping) / sizeof(kPlayStationButtonMapping[0]),
        {false, true, false, false, false, true},
    },

    // ds4 v1
    {
        1356, 1476,
        XINPUT_DEVSUBTYPE_GAMEPAD, 0,
        kDefaultAxisMap,  sizeof(kDefaultAxisMap) / sizeof(kDefaultAxisMap[0]),
        kPlayStationButtonMapping, sizeof(kPlayStationButtonMapping) / sizeof(kPlayStationButtonMapping[0]),
        {false, true, false, false, false, true},
    },

    // ds4 wireless adapter
   {
       1356, 0x0BA0,
       XINPUT_DEVSUBTYPE_GAMEPAD, 0,
       kDefaultAxisMap,  sizeof(kDefaultAxisMap) / sizeof(kDefaultAxisMap[0]),
       kPlayStationButtonMapping, sizeof(kPlayStationButtonMapping) / sizeof(kPlayStationButtonMapping[0]),
       {false, true, false, false, false, true},
   },

    // dualsense
    {
        1356, 3302,
        XINPUT_DEVSUBTYPE_GAMEPAD, 0,
        kDefaultAxisMap,  sizeof(kDefaultAxisMap) / sizeof(kDefaultAxisMap[0]),
        kPlayStationButtonMapping, sizeof(kPlayStationButtonMapping) / sizeof(kPlayStationButtonMapping[0]),
        {false, true, false, false, false, true},
    },

    // dualsense edge
    {
        1356, 0x0DF2,
        XINPUT_DEVSUBTYPE_GAMEPAD, 0,
        kDefaultAxisMap,  sizeof(kDefaultAxisMap) / sizeof(kDefaultAxisMap[0]),
        kPlayStationButtonMapping, sizeof(kPlayStationButtonMapping) / sizeof(kPlayStationButtonMapping[0]),
        {false, true, false, false, false, true},
    },

    // switch pro controller(This is a dummy mapping as their HID descriptor is broken)
    {
        0x057E, 0x2009,
        XINPUT_DEVSUBTYPE_GAMEPAD, 0,
        kDefaultAxisMap,  sizeof(kDefaultAxisMap) / sizeof(kDefaultAxisMap[0]),
        kPlayStationButtonMapping, sizeof(kPlayStationButtonMapping) / sizeof(kPlayStationButtonMapping[0]),
        {false, true, false, false, false, true},
    },

    // PS3 Guitar Hero Guitar
    {
        0x12ba, 0x0100,
        XINPUT_DEVSUBTYPE_GUITAR_ALTERNATE, 0,
        kDefaultAxisMap,  sizeof(kDefaultAxisMap) / sizeof(kDefaultAxisMap[0]),
        kPlayStationGHGuitarButtonMapping, sizeof(kPlayStationGHGuitarButtonMapping) / sizeof(kPlayStationGHGuitarButtonMapping[0]),
        {false, true, false, false, false, true},
    },

    // PC Guitar Hero WT Guitar
    {
        0x1430, 0x474C,
        XINPUT_DEVSUBTYPE_GUITAR_ALTERNATE, 0,
        kDefaultAxisMap,  sizeof(kDefaultAxisMap) / sizeof(kDefaultAxisMap[0]),
        kPlayStationGHGuitarButtonMapping, sizeof(kPlayStationGHGuitarButtonMapping) / sizeof(kPlayStationGHGuitarButtonMapping[0]),
        {false, true, false, false, false, true},
    },

    // PS3 Rock Band Guitar
    {
        0x12ba, 0x0200,
        XINPUT_DEVSUBTYPE_GUITAR, 0,
        kDefaultAxisMap,  sizeof(kDefaultAxisMap) / sizeof(kDefaultAxisMap[0]),
        kPlayStationButtonMapping, sizeof(kPlayStationButtonMapping) / sizeof(kPlayStationButtonMapping[0]),
        {false, true, false, false, false, true},
    },

    // Wii Rock Band 1 Guitar
    {
        0x1BAD, 0x0004,
        XINPUT_DEVSUBTYPE_GUITAR, 0,
        kDefaultAxisMap,  sizeof(kDefaultAxisMap) / sizeof(kDefaultAxisMap[0]),
        kPlayStationButtonMapping, sizeof(kPlayStationButtonMapping) / sizeof(kPlayStationButtonMapping[0]),
        {false, true, false, false, false, true},
    },

    // Wii Rock Band 2 Guitar
    {
        0x1BAD, 0x3010,
        XINPUT_DEVSUBTYPE_GUITAR, 0,
        kDefaultAxisMap,  sizeof(kDefaultAxisMap) / sizeof(kDefaultAxisMap[0]),
        kPlayStationButtonMapping, sizeof(kPlayStationButtonMapping) / sizeof(kPlayStationButtonMapping[0]),
        {false, true, false, false, false, true},
    },

    // PS3 Guitar Hero Drums
    {
        0x12BA, 0x0120,
        XINPUT_DEVSUBTYPE_DRUM_KIT, 0,
        kDefaultAxisMap,  sizeof(kDefaultAxisMap) / sizeof(kDefaultAxisMap[0]),
        kPlayStationButtonMapping, sizeof(kPlayStationButtonMapping) / sizeof(kPlayStationButtonMapping[0]),
        {false, true, false, false, false, true},
    },

    // PS3 Rock Band Drums
    {
        0x12BA, 0x0120,
        XINPUT_DEVSUBTYPE_DRUM_KIT, 0x000D,
        kDefaultAxisMap,  sizeof(kDefaultAxisMap) / sizeof(kDefaultAxisMap[0]),
        kPlayStationRBDrumButtonMapping, sizeof(kPlayStationRBDrumButtonMapping) / sizeof(kPlayStationRBDrumButtonMapping[0]),
        {false, true, false, false, false, true},
    },

    // Wii Rock Band 1 Drums
    {
        0x12BA, 0x0005,
        XINPUT_DEVSUBTYPE_DRUM_KIT, 0x000D,
        kDefaultAxisMap,  sizeof(kDefaultAxisMap) / sizeof(kDefaultAxisMap[0]),
        kPlayStationRBDrumButtonMapping, sizeof(kPlayStationRBDrumButtonMapping) / sizeof(kPlayStationRBDrumButtonMapping[0]),
        {false, true, false, false, false, true},
    },

    // Wii Rock Band 2 Drums
    {
        0x12BA, 0x3110,
        XINPUT_DEVSUBTYPE_DRUM_KIT, 0x000D,
        kDefaultAxisMap,  sizeof(kDefaultAxisMap) / sizeof(kDefaultAxisMap[0]),
        kPlayStationRBDrumButtonMapping, sizeof(kPlayStationRBDrumButtonMapping) / sizeof(kPlayStationRBDrumButtonMapping[0]),
        {false, true, false, false, false, true},
    },

    // PS3 Midi Pro Adapter Drums
    {
        0x12BA, 0x0218,
        XINPUT_DEVSUBTYPE_DRUM_KIT, 0x000D,
        kDefaultAxisMap,  sizeof(kDefaultAxisMap) / sizeof(kDefaultAxisMap[0]),
        kPlayStationRBDrumButtonMapping, sizeof(kPlayStationRBDrumButtonMapping) / sizeof(kPlayStationRBDrumButtonMapping[0]),
        {false, true, false, false, false, true},
    },

    // Wii Midi Pro Adapter Drums
    {
        0x12BA, 0x3138,
        XINPUT_DEVSUBTYPE_DRUM_KIT, 0x000D,
        kDefaultAxisMap,  sizeof(kDefaultAxisMap) / sizeof(kDefaultAxisMap[0]),
        kPlayStationRBDrumButtonMapping, sizeof(kPlayStationRBDrumButtonMapping) / sizeof(kPlayStationRBDrumButtonMapping[0]),
        {false, true, false, false, false, true},
    },
};

std::vector<HidDeviceMapping> g_dynamicMappings;
std::vector<std::unique_ptr<DynamicMappingData>> g_dynamicData;

HidDeviceMapping* FindStaticMapping(uint16_t vid, uint16_t pid) {
    for (size_t i = 0; i < sizeof(kStaticDeviceMappings) / sizeof(kStaticDeviceMappings[0]); i++) {
        auto& m = kStaticDeviceMappings[i];
        if (m.vendorId == vid && m.productId == pid)
            return &m;
    }
    return nullptr;
}


HidDeviceMapping* FindStaticSonyMapping(uint16_t usage, uint8_t psType) {
    uint8_t subType;
    // parse the PS4/PS5 capabilities request and figure out the subtype
    switch (psType) {
        case 0x00:
            subType = XINPUT_DEVSUBTYPE_GAMEPAD;
            break;
        case 0x01:
            subType = XINPUT_DEVSUBTYPE_GUITAR;
            break;
        case 0x02:
            subType = XINPUT_DEVSUBTYPE_DRUM_KIT;
            break;
        case 0x04:
            subType = XINPUT_DEVSUBTYPE_DANCEPAD;
            break;
        case 0x06:
            subType = XINPUT_DEVSUBTYPE_WHEEL;
            break;
        case 0x07:
            subType = XINPUT_DEVSUBTYPE_ARCADE_STICK;
            break;
        case 0x08:
            subType = XINPUT_DEVSUBTYPE_FLIGHT_STICK;
            break;
        default:
            subType = XINPUT_DEVSUBTYPE_GAMEPAD;
            break;
    }
    if (usage == HID_USAGE_PS5_CAPABILITIES) {
        for (size_t i = 0; i < sizeof(kStaticPs5Mappings) / sizeof(kStaticPs5Mappings[0]); i++) {
            auto& m = kStaticPs5Mappings[i];
            if (m.subType == subType)
                return &m;
        }
    }
    if (usage == HID_USAGE_PS4_CAPABILITIES) {
        for (size_t i = 0; i < sizeof(kStaticPs4Mappings) / sizeof(kStaticPs4Mappings[0]); i++) {
            auto& m = kStaticPs4Mappings[i];
            if (m.subType == subType)
                return &m;
        }
    }
    if (usage) {
        return &kStaticSonyMapping;
    }
    return nullptr;
}

HidDeviceMapping* FindMapping(uint16_t vid, uint16_t pid) {
    for (int i = 0; i < g_dynamicMappings.size(); i++) {
        auto& m = g_dynamicMappings[i];
        if (m.vendorId == vid && m.productId == pid)
            return &m;
    }

    // Fall back to static mappings
    return FindStaticMapping(vid, pid);
}

static const char* kAxisFieldNames[] = {
    "x", "y", "z", "rx", "ry", "rz"
};

static const char* kButtonFieldNames[] = {
    "a_button", "b_button", "x_button", "y_button",
    "l1", "r1", "l2", "r2",
    "back", "start", "l3", "r3",
    "xbox", "dpad_left", "dpad_right", "dpad_up", "dpad_down"
};

static int16_t ButtonsReport::* GetAxisFieldPtr(const char* name) {
    if (strcmp(name, "x") == 0)  return &ButtonsReport::x;
    if (strcmp(name, "y") == 0)  return &ButtonsReport::y;
    if (strcmp(name, "z") == 0)  return &ButtonsReport::z;
    if (strcmp(name, "rx") == 0) return &ButtonsReport::rx;
    if (strcmp(name, "ry") == 0) return &ButtonsReport::ry;
    if (strcmp(name, "rz") == 0) return &ButtonsReport::rz;
    return nullptr;
}

static uint8_t ButtonsReport::* GetButtonFieldPtr(const char* name) {
    if (strcmp(name, "a_button") == 0)     return &ButtonsReport::a_button;
    if (strcmp(name, "b_button") == 0)    return &ButtonsReport::b_button;
    if (strcmp(name, "x_button") == 0)    return &ButtonsReport::x_button;
    if (strcmp(name, "y_button") == 0)    return &ButtonsReport::y_button;
    if (strcmp(name, "l1") == 0)          return &ButtonsReport::l1;
    if (strcmp(name, "r1") == 0)          return &ButtonsReport::r1;
    if (strcmp(name, "l2") == 0)          return &ButtonsReport::l2;
    if (strcmp(name, "r2") == 0)          return &ButtonsReport::r2;
    if (strcmp(name, "back") == 0)        return &ButtonsReport::back;
    if (strcmp(name, "start") == 0)       return &ButtonsReport::start;
    if (strcmp(name, "l3") == 0)          return &ButtonsReport::l3;
    if (strcmp(name, "r3") == 0)          return &ButtonsReport::r3;
    if (strcmp(name, "xbox") == 0)        return &ButtonsReport::xbox;
    if (strcmp(name, "dpad_left") == 0)   return &ButtonsReport::dpad_left;
    if (strcmp(name, "dpad_right") == 0)  return &ButtonsReport::dpad_right;
    if (strcmp(name, "dpad_up") == 0)     return &ButtonsReport::dpad_up;
    if (strcmp(name, "dpad_down") == 0)   return &ButtonsReport::dpad_down;
    return nullptr;
}

static const char* GetAxisFieldName(int16_t ButtonsReport::* ptr) {
    if (ptr == &ButtonsReport::x)  return "x";
    if (ptr == &ButtonsReport::y)  return "y";
    if (ptr == &ButtonsReport::z)  return "z";
    if (ptr == &ButtonsReport::rx) return "rx";
    if (ptr == &ButtonsReport::ry) return "ry";
    if (ptr == &ButtonsReport::rz) return "rz";
    return nullptr;
}

static const char* GetButtonFieldName(uint8_t ButtonsReport::* ptr) {
    if (ptr == &ButtonsReport::a_button)     return "a_button";
    if (ptr == &ButtonsReport::b_button)    return "b_button";
    if (ptr == &ButtonsReport::x_button)    return "x_button";
    if (ptr == &ButtonsReport::y_button)    return "y_button";
    if (ptr == &ButtonsReport::l1)          return "l1";
    if (ptr == &ButtonsReport::r1)          return "r1";
    if (ptr == &ButtonsReport::l2)          return "l2";
    if (ptr == &ButtonsReport::r2)          return "r2";
    if (ptr == &ButtonsReport::back)        return "back";
    if (ptr == &ButtonsReport::start)       return "start";
    if (ptr == &ButtonsReport::l3)          return "l3";
    if (ptr == &ButtonsReport::r3)          return "r3";
    if (ptr == &ButtonsReport::xbox)        return "xbox";
    if (ptr == &ButtonsReport::dpad_left)   return "dpad_left";
    if (ptr == &ButtonsReport::dpad_right)  return "dpad_right";
    if (ptr == &ButtonsReport::dpad_up)     return "dpad_up";
    if (ptr == &ButtonsReport::dpad_down)   return "dpad_down";
    return nullptr;
}


bool LoadMappingsFromJson(const std::string& jsonString) {
    using namespace rapidjson;

    Document doc;
    doc.Parse(jsonString.c_str());
    if (doc.HasParseError()) {
        return false;
    }

    if (!doc.IsArray()) {
        return false;
    }

    // Clear existing dynamic mappings
    ClearDynamicMappings();

    for (SizeType i = 0; i < doc.Size(); i++) {
        const Value& entry = doc[i];
        if (!entry.IsObject()) continue;

        // Required fields
        if (!entry.HasMember("vid") || !entry.HasMember("pid")) continue;
        if (!entry["vid"].IsUint() || !entry["pid"].IsUint()) continue;

        std::unique_ptr<DynamicMappingData> data(new DynamicMappingData());
        HidDeviceMapping mapping = {};

        mapping.vendorId = static_cast<uint16_t>(entry["vid"].GetUint());
        mapping.productId = static_cast<uint16_t>(entry["pid"].GetUint());

        // Parse axis map
        if (entry.HasMember("axes") && entry["axes"].IsArray()) {
            const Value& axes = entry["axes"];
            for (SizeType j = 0; j < axes.Size(); j++) {
                const Value& axis = axes[j];
                if (!axis.IsObject()) continue;
                if (!axis.HasMember("usage") || !axis.HasMember("field")) continue;

                HidAxisMapEntry ae = {};
                ae.usage = static_cast<uint16_t>(axis["usage"].GetUint());

                const char* fieldName = axis["field"].GetString();
                ae.field = GetAxisFieldPtr(fieldName);

                if (ae.field != nullptr) {
                    data->axisEntries.push_back(ae);
                }
            }
        }

        // Parse button map
        if (entry.HasMember("buttons") && entry["buttons"].IsArray()) {
            const Value& buttons = entry["buttons"];
            for (SizeType j = 0; j < buttons.Size(); j++) {
                const Value& btn = buttons[j];
                if (!btn.IsObject()) continue;
                if (!btn.HasMember("idx") || !btn.HasMember("field")) continue;

                HidButtonMapEntry be = {};
                be.idx = static_cast<uint8_t>(btn["idx"].GetUint());

                const char* fieldName = btn["field"].GetString();
                be.field = GetButtonFieldPtr(fieldName);

                if (be.field != nullptr) {
                    data->buttonEntries.push_back(be);
                }
            }
        }

        // Parse invert flags
        if (entry.HasMember("invert") && entry["invert"].IsObject()) {
            const Value& inv = entry["invert"];
            if (inv.HasMember("x"))  mapping.invert.invertX = inv["x"].GetBool();
            if (inv.HasMember("y"))  mapping.invert.invertY = inv["y"].GetBool();
            if (inv.HasMember("z"))  mapping.invert.invertZ = inv["z"].GetBool();
            if (inv.HasMember("rx")) mapping.invert.invertRX = inv["rx"].GetBool();
            if (inv.HasMember("ry")) mapping.invert.invertRY = inv["ry"].GetBool();
            if (inv.HasMember("rz")) mapping.invert.invertRZ = inv["rz"].GetBool();
        }

        // Finalize mapping pointers
        if (!data->axisEntries.empty()) {
            mapping.axisMap = data->axisEntries.data();
            mapping.axisMapCount = static_cast<uint8_t>(data->axisEntries.size());
        }
        if (!data->buttonEntries.empty()) {
            mapping.buttonMap = data->buttonEntries.data();
            mapping.buttonMapCount = static_cast<uint8_t>(data->buttonEntries.size());
        }

        g_dynamicData.push_back(std::move(data));
        g_dynamicMappings.push_back(mapping);
    }

    return true;
}

bool LoadMappingsFromFile(const std::string& filepath) {
    using namespace rapidjson;

    std::ifstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    std::ostringstream ss;
    ss << file.rdbuf();
    std::string jsonContent = ss.str();

    file.close();

    return LoadMappingsFromJson(jsonContent);
}

std::string SaveMappingsToJson() {
    using namespace rapidjson;

    Document doc(kArrayType);
    Document::AllocatorType& alloc = doc.GetAllocator();

    // Save dynamic mappings
    for (int j = 0; j < g_dynamicMappings.size(); j++) {
        const auto& m = g_dynamicMappings[j];

        Value entry(kObjectType);
        entry.AddMember("vid", m.vendorId, alloc);
        entry.AddMember("pid", m.productId, alloc);

        // Axes
        Value axes(kArrayType);
        for (uint8_t i = 0; i < m.axisMapCount; i++) {
            const auto& ae = m.axisMap[i];
            Value axis(kObjectType);
            axis.AddMember("usage", ae.usage, alloc);

            const char* name = GetAxisFieldName(ae.field);
            if (name) {
                axis.AddMember("field", StringRef(name), alloc);
                axes.PushBack(axis, alloc);
            }
        }
        if (!axes.Empty()) {
            entry.AddMember("axes", axes, alloc);
        }

        // Buttons
        Value buttons(kArrayType);
        for (uint8_t i = 0; i < m.buttonMapCount; i++) {
            const auto& be = m.buttonMap[i];
            Value btn(kObjectType);
            btn.AddMember("idx", be.idx, alloc);

            const char* name = GetButtonFieldName(be.field);
            if (name) {
                btn.AddMember("field", StringRef(name), alloc);
                buttons.PushBack(btn, alloc);
            }
        }
        if (!buttons.Empty()) {
            entry.AddMember("buttons", buttons, alloc);
        }

        // Invert flags
        Value inv(kObjectType);
        inv.AddMember("x", m.invert.invertX, alloc);
        inv.AddMember("y", m.invert.invertY, alloc);
        inv.AddMember("z", m.invert.invertZ, alloc);
        inv.AddMember("rx", m.invert.invertRX, alloc);
        inv.AddMember("ry", m.invert.invertRY, alloc);
        inv.AddMember("rz", m.invert.invertRZ, alloc);
        entry.AddMember("invert", inv, alloc);

        doc.PushBack(entry, alloc);
    }

    StringBuffer buffer;
    PrettyWriter<StringBuffer> writer(buffer);
    writer.SetIndent(' ', 2);
    doc.Accept(writer);

    return buffer.GetString();
}

bool SaveMappingsToFile(const std::string& filepath) {
    std::ofstream file(filepath, std::ios::binary);
    if (!file.is_open()) {
        return false;
    }

    std::string json = SaveMappingsToJson();
    if (json.empty()) {
        return false;
    }

    file.write(json.data(), static_cast<std::streamsize>(json.size()));
    file.flush();
    file.close();
    return true;
}
void ClearDynamicMappings() {
    g_dynamicMappings.clear();
    g_dynamicData.clear();
}
