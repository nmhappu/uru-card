#pragma once

#include <exception>
#include <memory>
#include <vector>

#include <YACL.h>

#include "config.h"
#include "crypto/crypto.h"
#include "fido2/uuid.h"
#include "util/be.h"
#include "util/fixedbuffer.h"

namespace FIDO2
{
    namespace CTAP
    {
        enum CommandCode
        {
            authenticatorNoCommand = 0x00,
            authenticatorMakeCredential = 0x01,
            authenticatorGetAssertion = 0x02,
            authenticatorGetInfo = 0x04,
            authenticatorClientPIN = 0x06,
            authenticatorReset = 0x07,
            authenticatorGetNextAssertion = 0x08,
            authenticatorBioEnrollment = 0x09,
            authenticatorCredentialManagement = 0x0A,
            authenticatorPlatformConfig = 0x0c,
            authenticatorVendorFirst = 0x40,
            authenticatorVendorLast = 0xBF,
        };

        enum Status
        {
            CTAP2_OK = 0x00,                        // Indicates successful response.
            CTAP1_ERR_INVALID_COMMAND = 0x01,       // The command is not a valid CTAP command.
            CTAP1_ERR_INVALID_PARAMETER = 0x02,     // The command included an invalid parameter.
            CTAP1_ERR_INVALID_LENGTH = 0x03,        // Invalid message or item length.
            CTAP1_ERR_INVALID_SEQ = 0x04,           // Invalid message sequencing.
            CTAP1_ERR_TIMEOUT = 0x05,               // Message timed out.
            CTAP1_ERR_CHANNEL_BUSY = 0x06,          // Channel busy. Client SHOULD retry the request after a short delay. Note that the client may abort the transaction if the command is no longer relevant.
            CTAP1_ERR_LOCK_REQUIRED = 0x0A,         // Command requires channel lock.
            CTAP1_ERR_INVALID_CHANNEL = 0x0B,       // Command not allowed on this cid.
            CTAP2_ERR_CBOR_UNEXPECTED_TYPE = 0x11,  // Invalid/unexpected CBOR error.
            CTAP2_ERR_INVALID_CBOR = 0x12,          // Error when parsing CBOR.
            CTAP2_ERR_MISSING_PARAMETER = 0x14,     // Missing non-optional parameter.
            CTAP2_ERR_LIMIT_EXCEEDED = 0x15,        // Limit for number of items exceeded.
            CTAP2_ERR_UNSUPPORTED_EXTENSION = 0x16, // Unsupported extension.
            CTAP2_ERR_CREDENTIAL_EXCLUDED = 0x19,   // Valid credential found in the exclude list.
            CTAP2_ERR_PROCESSING = 0x21,            // Processing (Lengthy operation is in progress).
            CTAP2_ERR_INVALID_CREDENTIAL = 0x22,    // Credential not valid for the authenticator.
            CTAP2_ERR_USER_ACTION_PENDING = 0x23,   // Authentication is waiting for user interaction.
            CTAP2_ERR_OPERATION_PENDING = 0x24,     // Processing, lengthy operation is in progress.
            CTAP2_ERR_NO_OPERATIONS = 0x25,         // No request is pending.
            CTAP2_ERR_UNSUPPORTED_ALGORITHM = 0x26, // Authenticator does not support requested algorithm.
            CTAP2_ERR_OPERATION_DENIED = 0x27,      // Not authorized for requested operation.
            CTAP2_ERR_KEY_STORE_FULL = 0x28,        // Internal key storage is full.
            CTAP2_ERR_NO_OPERATION_PENDING = 0x2A,  // No outstanding operations.
            CTAP2_ERR_UNSUPPORTED_OPTION = 0x2B,    // Unsupported option.
            CTAP2_ERR_INVALID_OPTION = 0x2C,        // Not a valid option for current operation.
            CTAP2_ERR_KEEPALIVE_CANCEL = 0x2D,      // Pending keep alive was cancelled.
            CTAP2_ERR_NO_CREDENTIALS = 0x2E,        // No valid credentials provided.
            CTAP2_ERR_USER_ACTION_TIMEOUT = 0x2F,   // Timeout waiting for user interaction.
            CTAP2_ERR_NOT_ALLOWED = 0x30,           // Continuation command, such as, authenticatorGetNextAssertion not allowed.
            CTAP2_ERR_PIN_INVALID = 0x31,           // PIN Invalid.
            CTAP2_ERR_PIN_BLOCKED = 0x32,           // PIN Blocked.
            CTAP2_ERR_PIN_AUTH_INVALID = 0x33,      // PIN authentication,pinUvAuthParam, verification failed.
            CTAP2_ERR_PIN_AUTH_BLOCKED = 0x34,      // PIN authentication,pinUvAuthParam, blocked. Requires power recycle to reset.
            CTAP2_ERR_PIN_NOT_SET = 0x35,           // No PIN has been set.
            CTAP2_ERR_PIN_REQUIRED = 0x36,          // PIN is required for the selected operation.
            CTAP2_ERR_PIN_POLICY_VIOLATION = 0x37,  // PIN policy violation. Currently only enforces minimum length.
            CTAP2_ERR_PIN_TOKEN_EXPIRED = 0x38,     // pinUvAuthToken expired on authenticator.
            CTAP2_ERR_REQUEST_TOO_LARGE = 0x39,     // Authenticator cannot handle this request due to memory constraints.
            CTAP2_ERR_ACTION_TIMEOUT = 0x3A,        // The current operation has timed out.
            CTAP2_ERR_UP_REQUIRED = 0x3B,           // User presence is required for the requested operation.
            CTAP2_ERR_UV_BLOCKED = 0x3C,            // Built in UV is blocked.
            CTAP1_ERR_OTHER = 0x7F,                 // Other unspecified error.
            CTAP2_ERR_SPEC_LAST = 0xDF,             // CTAP 2 spec last error.
            CTAP2_ERR_EXTENSION_FIRST = 0xE0,       // Extension specific error.
            CTAP2_ERR_EXTENSION_LAST = 0xEF,        // Extension specific error.
            CTAP2_ERR_VENDOR_FIRST = 0xF0,          // Vendor specific error.
            CTAP2_ERR_VENDOR_LAST = 0xFF,           // Vendor specific error.
        };

        class Exception : public std::exception
        {
        public:
            Exception()
            {
                this->status = CTAP1_ERR_OTHER;
            }

            Exception(const Status status)
            {
                this->status = status;
            }

            const Status getStatus() const
            {
                return this->status;
            }

        private:
            Status status;
        };

#pragma pack(push, 1)
        union AuthenticatorDataFlags
        {
            struct flag_bitfields
            {
                bool userPresent : 1;
                uint8_t rfu1 : 1;
                bool userVerified : 1;
                uint8_t rfu2 : 3;
                bool attestationData : 1;
                bool extensions : 1;
            } f;
            uint8_t val;
        };

        struct AttestedCredentialData
        {
            uint8_t aaguid[16];
            be_uint16_t credentialIdLen;
            uint8_t credentialId[CREDENTIAL_ID_LENGTH];
            uint8_t publicKey[77];
        };

        struct AuthenticatorData
        {
            uint8_t rpIdHash[32];
            AuthenticatorDataFlags flags;
            uint32_t signCount;
            AttestedCredentialData attestedCredentialData;
        };
#pragma pack(pop)

        struct PublicKeyCredentialRpEntity
        {
            String id;
            String name;
            String icon;
        };

        struct PublicKeyCredentialUserEntity
        {
            FixedBuffer64 id;
            String name;
            String displayName;
            String icon;
        };

        struct PublicKeyCredentialDescriptor
        {
            String type;
            FixedBuffer<CREDENTIAL_ID_LENGTH> credentialId;
            std::vector<String> transports;
        };

        class Command
        {
        public:
            virtual CommandCode getCommandCode() const = 0;
        };

        namespace Request
        {
            class GetInfo : public Command
            {
            public:
                virtual CommandCode getCommandCode() const;
            };

            class GetAssertion : public Command
            {
            public:
                enum MapKeys
                {
                    keyRpId = 0x01,
                    keyClientDataHash = 0x02,
                    keyAllowList = 0x03,
                    keyExtensions = 0x04,
                    keyOptions = 0x05,
                    keyPinUvAuthParam = 0x06,
                    keyPinUvAuthProtocol = 0x07,
                };

            public:
                virtual CommandCode getCommandCode() const;

            public:
                String rpId;
                uint8_t clientDataHash[32];
                std::vector<std::unique_ptr<PublicKeyCredentialDescriptor>> allowList;
            };

            class MakeCredential : public Command
            {
            public:
                enum MapKeys
                {
                    keyclientDataHash = 0x01,
                    keyRp = 0x02,
                    keyUser = 0x03,
                    keyPubKeyCredParams = 0x04,
                    keyExcludeList = 0x05,
                    keyExtensions = 0x06,
                    keyOptions = 0x07,
                    keyPinUvAuthParam = 0x08,
                    keyPinUvAuthProtocol = 0x09,
                };

                struct Options
                {
                    bool rk : 1;
                    bool uv : 1;
                    bool up : 1;
                };

            public:
                MakeCredential()
                {
                    options.rk = false;
                    options.uv = false;
                    options.up = false;
                }

                virtual CommandCode getCommandCode() const;

            public:
                uint8_t clientDataHash[32];
                PublicKeyCredentialRpEntity rp;
                PublicKeyCredentialUserEntity user;
                std::vector<int8_t> algorithms;
                std::unique_ptr<FixedBuffer16> pinUvAuthParam;
                uint8_t pinUvAuthProtocol;
                std::vector<PublicKeyCredentialDescriptor> excludeList;
                Options options;
            };

            class ClientPIN : public Command
            {
            public:
                enum MapKeys
                {
                    keyPinUvAuthProtocol = 0x01,
                    keySubCommand = 0x02,
                    keyKeyAgreement = 0x03,
                    keyPinUvAuthParam = 0x04,
                    keyNewPinEnc = 0x05,
                    keyPinHashEnc = 0x06,
                };

                enum SubCommand
                {
                    cmdGetPINRetries = 0x01,
                    cmdGetKeyAgreement = 0x02,
                    cmdSetPIN = 0x03,
                    cmdChangePIN = 0x04,
                    cmdGetPinUvAuthTokenUsingPin = 0x05,
                    cmdGetPinUvAuthTokenUsingUv = 0x06,
                    cmdGetUVRetries = 0x07
                };

            public:
                virtual CommandCode getCommandCode() const;

            public:
                uint8_t protocol;
                SubCommand subCommand;
                Crypto::ECDSA::PublicKey publicKey;
                uint8_t pinUvAuthParam[16];
                uint8_t newPinEnc[64];
                uint8_t pinHashEnc[16];
            };

            class Reset : public Command
            {
            public:
                virtual CommandCode getCommandCode() const;
            };

            Status parse(const uint8_t *data, const size_t length, std::unique_ptr<Command> &request);

            Status parseGetInfo(const CBOR &cbor, std::unique_ptr<Command> &request);
            Status parseGetAssertion(const CBOR &cbor, std::unique_ptr<Command> &request);
            Status parseMakeCredential(const CBOR &cbor, std::unique_ptr<Command> &request);
            Status parseClientPIN(const CBOR &cbor, std::unique_ptr<Command> &request);
            Status parseReset(const CBOR &cbor, std::unique_ptr<Command> &request);

            // parse data structures
            Status parseRpEntity(const CBOR &cbor, PublicKeyCredentialRpEntity *rp);
            Status parseUserEntity(const CBOR &cbor, PublicKeyCredentialUserEntity *user);
            Status parsePublicKey(const CBOR &cbor, Crypto::ECDSA::PublicKey *publicKey);

        }; // namespace Request

        namespace Response
        {
            class GetInfo : public Command
            {
            public:
                struct Options
                {
                    bool plat : 1;
                    bool rk : 1;
                    bool clientPinSupported : 1;
                    bool clientPin : 1;
                    bool up : 1;
                    bool uvSupported : 1;
                    bool uv : 1;
                    bool uvToken : 1;
                    bool config : 1;
                };

            public:
                virtual CommandCode getCommandCode() const;

            public:
                std::vector<String> versions;
                std::vector<String> extensions;
                FIDO2::UUID aaguid;
                Options options;
                std::unique_ptr<uint16_t> maxMsgSize;
                std::unique_ptr<std::vector<uint8_t>> pinUvAuthProtocols;
                std::unique_ptr<uint8_t> maxCredentialCountInList;
                std::unique_ptr<uint8_t> maxCredentialIdLength;
                std::unique_ptr<std::vector<String>> transports;
                // std::unique_ptr<std::vector<> algorithms;
                std::unique_ptr<uint8_t> maxAuthenticatorConfigLength;
                std::unique_ptr<uint8_t> defaultCredProtect;
            };

            class GetAssertion : public Command
            {
            public:
                virtual CommandCode getCommandCode() const;

            public:
                PublicKeyCredentialDescriptor credential;
                AuthenticatorData authenticatorData;
                uint8_t signature[72];
                size_t signatureSize;
                PublicKeyCredentialUserEntity user;
                int numberOfCredentials;
                bool userSelected;
            };

            class MakeCredential : public Command
            {
            public:
                virtual CommandCode getCommandCode() const;

            public:
                AuthenticatorData authenticatorData;
                uint8_t signature[72];
                size_t signatureSize;
            };

            class ClientPIN : public Command
            {
            public:
                virtual CommandCode getCommandCode() const;

            public:
                std::unique_ptr<Crypto::ECDSA::PublicKey> publicKey;
                std::unique_ptr<FixedBuffer16> pinUvAuthToken;
                std::unique_ptr<uint8_t> pinRetries;
                std::unique_ptr<bool> powerCycleState;
                std::unique_ptr<uint8_t> uvRetries;
            };

            class Reset : public Command
            {
            public:
                virtual CommandCode getCommandCode() const;
            };

            // encode the response
            Status encode(const Command *response, std::unique_ptr<CBOR> &cbor);

            Status encode(const Response::GetInfo *response, std::unique_ptr<CBOR> &cbor);
            Status encode(const Response::GetAssertion *response, std::unique_ptr<CBOR> &cbor);
            Status encode(const Response::MakeCredential *response, std::unique_ptr<CBOR> &cbor);
            Status encode(const Response::ClientPIN *response, std::unique_ptr<CBOR> &cbor);
            Status encode(const Response::Reset *response, std::unique_ptr<CBOR> &cbor);

            void encodePublicKey(Crypto::ECDSA::PublicKey *publicKey, uint8_t *encodedKey);

        } // namespace Response

    } // namespace CTAP
} // namespace FIDO2