#include "mapping.h"
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
    { HID_USAGE_AXIS_X,  &ButtonsReport::x  },
    { HID_USAGE_AXIS_Y,  &ButtonsReport::y  },
    { HID_USAGE_AXIS_Z,  &ButtonsReport::z  },
    { HID_USAGE_AXIS_RX, &ButtonsReport::rx },
    { HID_USAGE_AXIS_RY, &ButtonsReport::ry },
    { HID_USAGE_AXIS_RZ, &ButtonsReport::rz },
};

static const HidButtonMapEntry kDefaultButtonMap[] = {
    { 0,  &ButtonsReport::a_button    },
    { 1,  &ButtonsReport::b_button   },
    { 2,  &ButtonsReport::x_button   },
    { 3,  &ButtonsReport::y_button },
    { 4,  &ButtonsReport::l1       },
    { 5,  &ButtonsReport::r1       },
    { 6,  &ButtonsReport::l2       },
    { 7,  &ButtonsReport::r2       },
    { 8,  &ButtonsReport::back     },
    { 9,  &ButtonsReport::start    },
    { 10, &ButtonsReport::l3       },
    { 11, &ButtonsReport::r3       },
    { 12, &ButtonsReport::xbox     },
    { 13, &ButtonsReport::dpad_left },
    { 14, &ButtonsReport::dpad_right },
    { 15, &ButtonsReport::dpad_up },
    { 16, &ButtonsReport::dpad_down },
};

static HidDeviceMapping kStaticDeviceMappings[] = {
    {
        0x0000, 0x0000,
        kDefaultAxisMap,  sizeof(kDefaultAxisMap) / sizeof(kDefaultAxisMap[0]),
        kDefaultButtonMap, sizeof(kDefaultButtonMap) / sizeof(kDefaultButtonMap[0]),
        {false, false, false, false, false, false},
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
