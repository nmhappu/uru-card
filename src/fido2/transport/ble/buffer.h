#pragma once

#include <Arduino.h>

#include "config.h"

namespace FIDO2
{
    namespace Transport
    {
        namespace BLE
        {
            class CommandBuffer
            {
            public:
                void reset();

                uint16_t init(const uint8_t *data, const uint16_t len);
                uint16_t append(const uint8_t *data, const uint16_t len);

                uint8_t *getBuffer();
                uint16_t getBufferLength();
                size_t getMaxBufferLength();

                bool isComplete();

                uint8_t getCmd();
                uint16_t getPayloadLength();
                void setPayloadLength(uint16_t length);
                uint8_t *getPayload();

            protected:
                uint8_t buffer[FIDO2_MAX_MSG_SIZE];
                uint16_t position;
            };

            extern CommandBuffer commandBuffer;

        } // namespace BLE
    }     // namespace Transport
} // namespace FIDO2