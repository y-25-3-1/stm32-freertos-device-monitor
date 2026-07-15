#include "dht11.h"

/*
 * DWT计数器初始化。
 * STM32F103是Cortex-M3，可以使用CPU周期计数器实现微秒延时。
 */
static void DHT11_DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;

    DWT->CYCCNT = 0;

    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/* 微秒级延时 */
static void DHT11_DelayUs(uint32_t us)
{
    uint32_t start;
    uint32_t ticks;

    start = DWT->CYCCNT;

    ticks = us * (SystemCoreClock / 1000000U);

    while ((uint32_t)(DWT->CYCCNT - start) < ticks)
    {
    }
}

/* 把DATA引脚配置成开漏输出 */
static void DHT11_SetPinOutput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT11_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;

    HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);
}

/* 把DATA引脚配置成输入 */
static void DHT11_SetPinInput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    GPIO_InitStruct.Pin = DHT11_GPIO_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;

    HAL_GPIO_Init(DHT11_GPIO_PORT, &GPIO_InitStruct);
}

/*
 * 等待DATA引脚变成指定电平。
 *
 * level：
 * GPIO_PIN_SET   等待高电平
 * GPIO_PIN_RESET 等待低电平
 *
 * timeout_us：
 * 最多等待多少微秒
 */
static uint8_t DHT11_WaitForLevel(GPIO_PinState level,
                                  uint32_t timeout_us)
{
    uint32_t start;
    uint32_t timeout_ticks;

    start = DWT->CYCCNT;

    timeout_ticks =
        timeout_us * (SystemCoreClock / 1000000U);

    while (HAL_GPIO_ReadPin(DHT11_GPIO_PORT,
                            DHT11_GPIO_PIN) != level)
    {
        if ((uint32_t)(DWT->CYCCNT - start) >
            timeout_ticks)
        {
            return 1;
        }
    }

    return 0;
}

void DHT11_Init(void)
{
    DHT11_DWT_Init();

    DHT11_SetPinOutput();

    /*
     * 开漏输出高电平实际上是释放总线，
     * DATA线会被上拉电阻拉高。
     */
    HAL_GPIO_WritePin(DHT11_GPIO_PORT,
                      DHT11_GPIO_PIN,
                      GPIO_PIN_SET);
}

uint8_t DHT11_Read(DHT11_Data_t *data)
{
    uint8_t raw_data[5] = {0};
    uint8_t i;
    uint32_t interrupt_state;

    /*
     * 第一步：STM32发送开始信号。
     * DATA拉低至少18ms。
     */
    DHT11_SetPinOutput();

    HAL_GPIO_WritePin(DHT11_GPIO_PORT,
                      DHT11_GPIO_PIN,
                      GPIO_PIN_RESET);

    HAL_Delay(20);

    /*
     * 从这里开始需要微秒级时序。
     * 暂时关闭中断，避免任务切换或中断打断读取。
     */
    interrupt_state = __get_PRIMASK();
    __disable_irq();

    /*
     * STM32释放DATA线，等待20～40us，
     * 然后改成输入，等待DHT11响应。
     */
    HAL_GPIO_WritePin(DHT11_GPIO_PORT,
                      DHT11_GPIO_PIN,
                      GPIO_PIN_SET);

    DHT11_DelayUs(30);

    DHT11_SetPinInput();

    /*
     * DHT11响应过程：
     * 约80us低电平
     * 约80us高电平
     * 然后重新拉低，准备发送40位数据
     */
    if (DHT11_WaitForLevel(GPIO_PIN_RESET, 120) != 0)
    {
        goto timeout_error;
    }

    if (DHT11_WaitForLevel(GPIO_PIN_SET, 120) != 0)
    {
        goto timeout_error;
    }

    if (DHT11_WaitForLevel(GPIO_PIN_RESET, 120) != 0)
    {
        goto timeout_error;
    }

    /*
     * DHT11一共发送40位，也就是5个字节：
     *
     * raw_data[0] 湿度整数
     * raw_data[1] 湿度小数
     * raw_data[2] 温度整数
     * raw_data[3] 温度小数
     * raw_data[4] 校验和
     */
    for (i = 0; i < 40; i++)
    {
        /*
         * 每一位开始前，DHT11先保持约50us低电平。
         * 等待它变成高电平。
         */
        if (DHT11_WaitForLevel(GPIO_PIN_SET, 100) != 0)
        {
            goto timeout_error;
        }

        /*
         * 高电平持续时间：
         * 约26～28us表示0
         * 约70us表示1
         *
         * 所以在高电平开始40us后读取：
         * 已经变低就是0
         * 仍然是高就是1
         */
        DHT11_DelayUs(40);

        raw_data[i / 8] <<= 1;

        if (HAL_GPIO_ReadPin(DHT11_GPIO_PORT,
                             DHT11_GPIO_PIN) ==
            GPIO_PIN_SET)
        {
            raw_data[i / 8] |= 0x01;
        }

        /*
         * 等待这一位的高电平结束，
         * 即DATA重新变成低电平。
         */
        if (DHT11_WaitForLevel(GPIO_PIN_RESET, 100) != 0)
        {
            goto timeout_error;
        }
    }

    /* 恢复进入函数前的中断状态 */
    if (interrupt_state == 0U)
    {
        __enable_irq();
    }

    /*
     * 校验：
     * 前4个字节相加后的低8位，
     * 应该等于第5个字节。
     */
    if ((uint8_t)(raw_data[0]
                + raw_data[1]
                + raw_data[2]
                + raw_data[3])
        != raw_data[4])
    {
        return DHT11_ERROR_CHECKSUM;
    }

    data->humidity = raw_data[0];
    data->temperature = raw_data[2];

    return DHT11_OK;

timeout_error:

    if (interrupt_state == 0U)
    {
        __enable_irq();
    }

    return DHT11_ERROR_TIMEOUT;
}