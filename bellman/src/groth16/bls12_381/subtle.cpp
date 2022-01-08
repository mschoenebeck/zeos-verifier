
#include "groth16/bls12_381/subtle.hpp"

Choice::Choice() : data(0)
{
}

Choice::Choice(uint8_t d) : data(d)
{
}

Choice Choice::ct_eq(uint64_t a, uint64_t b)
{
    return a == b ? Choice(1) : Choice(0);
}

Choice Choice::conditional_select(Choice a, Choice b, Choice choice)
{
    return Choice((uint8_t)(a.unwrap_u8() ^ (-((int8_t)choice.unwrap_u8()) & (a.unwrap_u8() ^ b.unwrap_u8()))));
}

Choice Choice::operator & (const Choice& rhs) const
{
    return Choice(this->data & rhs.data);
}

Choice Choice::operator | (const Choice& rhs) const
{
    return Choice(this->data | rhs.data);
}

Choice Choice::operator!()  const
{
    return Choice(1 & ~this->data);
}

uint8_t Choice::unwrap_u8() const
{
    return data;
}
