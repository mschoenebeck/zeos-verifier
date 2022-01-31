#pragma once

#include <stdint.h>
#include <json_struct.h>

using namespace std;

class Choice
{
    public:
        uint8_t data;
        JS_OBJ(data);

        Choice();
        Choice(uint8_t d);

        static Choice ct_eq(uint64_t a, uint64_t b);

        static Choice conditional_select(Choice a, Choice b, Choice choice);

        Choice operator & (const Choice& rhs) const;

        Choice operator | (const Choice& rhs) const;

        Choice operator!()  const;

        uint8_t unwrap_u8() const;
};
