#include "display.h"
#include "board_config.h"

typedef struct {
    GPIO_TypeDef *port;
    uint16_t pin;
} SegmentPin;

static const SegmentPin kSegments[8] = {
    {SEG_A_GPIO_Port, SEG_A_Pin},
    {SEG_B_GPIO_Port, SEG_B_Pin},
    {SEG_C_GPIO_Port, SEG_C_Pin},
    {SEG_D_GPIO_Port, SEG_D_Pin},
    {SEG_E_GPIO_Port, SEG_E_Pin},
    {SEG_F_GPIO_Port, SEG_F_Pin},
    {SEG_G_GPIO_Port, SEG_G_Pin},
    {SEG_DP_GPIO_Port, SEG_DP_Pin},
};

static const uint8_t kDigitMask[10] = {
    0x3F, /* 0: a b c d e f */
    0x06, /* 1: b c */
    0x5B, /* 2 */
    0x4F, /* 3 */
    0x66, /* 4 */
    0x6D, /* 5 */
    0x7D, /* 6 */
    0x07, /* 7 */
    0x7F, /* 8 */
    0x6F, /* 9 */
};

void Display_ShowDigit(uint8_t digit)
{
    if (digit > 9) {
        Display_Blank();
        return;
    }

    uint8_t mask = kDigitMask[digit];
    for (uint8_t i = 0; i < 8; ++i) {
        GPIO_PinState state = (mask & (1u << i)) ? GPIO_PIN_RESET : GPIO_PIN_SET;
        HAL_GPIO_WritePin(kSegments[i].port, kSegments[i].pin, state);
    }
}

void Display_Blank(void)
{
    for (uint8_t i = 0; i < 8; ++i) {
        HAL_GPIO_WritePin(kSegments[i].port, kSegments[i].pin, GPIO_PIN_SET);
    }
}

