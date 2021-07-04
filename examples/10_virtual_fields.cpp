/// @example 10_virtual_fields.cpp
/// @brief This example demonstrates how to define virtual fields, and pure
/// virtual (mandatory) fields, that can be overridden in child classes.

#include "../include/serdes.h"
#include <stdio.h>
#include "../test/notify_when_dynamic_allocation_used.h"

struct packet_format : serdes::packet_base
{
    uint8_t id = 0;
    serdes::formatter payload = {serdes::pure_virtual_formatter};
    uint16_t checksum = 0;
    serdes::formatter optional_trailing_data = {serdes::virtual_formatter};

    void format(serdes::packet &p) override
    {
        p + id + payload + checksum + optional_trailing_data;
    }
};

struct amperage_command : packet_format
{
    uint64_t amperage = 0;

    amperage_command()
    {
        packet_format::payload = {serdes::init_formatter(amperage)};
    }
};

struct voltage_command : packet_format
{
    struct payload_t : serdes::packet_base
    {
        uint32_t voltage = 0;
        void format(serdes::packet &p) override
        {
            p + voltage;
        }
    } payload = {};

    voltage_command()
    {
        packet_format::payload = {serdes::init_formatter(payload)};
    }
};

int main()
{
    uint8_t serial_data[] = {0xAB, 0x01, 0x02, 0x03, 0x04, 0xCD, 0xEF, 0x05, 0x06, 0x07, 0x08, 0x09};

    voltage_command obj;
    auto result = obj.load(serial_data);

    printf("voltage_command (%s) = {\n", serdes::status2str(result.status));
    printf("    id = 0x%02X\n", obj.id);
    printf("    payload.voltage = 0x%08X\n", obj.payload.voltage);
    printf("    checksum = 0x%04X\n}\n", obj.checksum);
}