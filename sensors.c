
#include "in4073.h"

Q13_20 toFixedPoint(uint16_t value) {
    Q13_20 val = value;
    return val << 20;
}