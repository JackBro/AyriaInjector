/*
    Initial author: (https://github.com/)Convery for Ayria.se
    License: GPL 2.0
    Started: 2016-10-9
    Notes:
        Fast runtime and compiletime hashing.
        Not cryptographically secure.
*/

#pragma once
#include <cstdint>
#include <string>

// FNV1 constants.
constexpr uint32_t FNV1_Prime_32 = 16777619u;
constexpr uint64_t FNV1_Prime_64 = 1099511628211u;
constexpr uint32_t FNV1_Offset_32 = 2166136261u;
constexpr uint64_t FNV1_Offset_64 = 14695981039346656037u;

namespace COAL
{
    namespace FNV1
    {
        namespace Compiletime
        {
            constexpr uint32_t FNV1_32(const char *String, uint32_t LastValue = FNV1_Offset_32)
            {
                return *String ? FNV1_32(String + 1, (LastValue * FNV1_Prime_32) ^ *String) : LastValue;
            };
            constexpr uint64_t FNV1_64(const char *String, uint64_t LastValue = FNV1_Offset_64)
            {
                return *String ? FNV1_64(String + 1, (LastValue * FNV1_Prime_64) ^ *String) : LastValue;
            };
            constexpr uint32_t FNV1a_32(const char *String, uint32_t LastValue = FNV1_Offset_32)
            {
                return *String ? FNV1a_32(String + 1, (*String ^ LastValue) * FNV1_Prime_32) : LastValue;
            };
            constexpr uint64_t FNV1a_64(const char *String, uint64_t LastValue = FNV1_Offset_64)
            {
                return *String ? FNV1a_64(String + 1, (*String ^ LastValue) * FNV1_Prime_64) : LastValue;
            };
        }

        namespace Runtime
        {
            inline uint32_t FNV1_32(const void *Data, size_t Length)
            {
                uint32_t Hash = FNV1_Offset_32;

                for (size_t i = 0; i < Length; ++i)
                {
                    Hash *= FNV1_Prime_32;
                    Hash ^= ((uint8_t *)Data)[i];
                }

                return Hash;
            };
            inline uint64_t FNV1_64(const void *Data, size_t Length)
            {
                uint64_t Hash = FNV1_Offset_64;

                for (size_t i = 0; i < Length; ++i)
                {
                    Hash *= FNV1_Prime_64;
                    Hash ^= ((uint8_t *)Data)[i];
                }

                return Hash;
            };
            inline uint32_t FNV1a_32(const void *Data, size_t Length)
            {
                uint32_t Hash = FNV1_Offset_32;

                for (size_t i = 0; i < Length; ++i)
                {
                    Hash ^= ((uint8_t *)Data)[i];
                    Hash *= FNV1_Prime_32;
                }

                return Hash;
            };
            inline uint64_t FNV1a_64(const void *Data, size_t Length)
            {
                uint64_t Hash = FNV1_Offset_64;

                for (size_t i = 0; i < Length; ++i)
                {
                    Hash ^= ((uint8_t *)Data)[i];
                    Hash *= FNV1_Prime_64;
                }

                return Hash;
            };

            inline uint32_t FNV1_32(std::string Data)
            {
                return FNV1_32(Data.data(), Data.size());
            }
            inline uint64_t FNV1_64(std::string Data)
            {
                return FNV1_64(Data.data(), Data.size());
            }
            inline uint32_t FNV1a_32(std::string Data)
            {
                return FNV1a_32(Data.data(), Data.size());
            }
            inline uint64_t FNV1a_64(std::string Data)
            {
                return FNV1a_64(Data.data(), Data.size());
            }
        }
    }
}
