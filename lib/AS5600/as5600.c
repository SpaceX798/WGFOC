/**
 * @file    as5600.c
 * @brief   AS5600 磁编码器 I2C 驱动实现
 * @author  Q3_SimulinkFOC
 * @date    2026-07-06
 *
 * 基于 HAL_I2C 的 AS5600 寄存器读写实现。
 * 内部使用 HAL_I2C_Mem_Read() 进行 I2C 存储器读取操作。
 */

#include "as5600.h"
#include "i2c.h"

extern I2C_HandleTypeDef hi2c1;

/**
 * @brief  通过 I2C 读取 AS5600 指定寄存器
 * @param[in]  reg  寄存器地址
 * @param[out] data 读取数据缓冲区
 * @param[in]  size 读取字节数
 * @retval HAL_OK    读取成功
 * @retval 其他      参考 HAL_I2C_Mem_Read 返回值
 */
static HAL_StatusTypeDef AS5600_ReadReg(uint8_t reg, uint8_t *data, uint8_t size)
{
    return HAL_I2C_Mem_Read(&hi2c1, AS5600_I2C_ADDR, reg,
                            I2C_MEMADD_SIZE_8BIT, data, size, HAL_MAX_DELAY);
}

/**
 * @brief  读取 AS5600 12-bit 字寄存器 (双字节，高字节在前)
 * @note   从指定寄存器地址连续读取 2 字节，拼合为 16-bit 后取低 12 位有效数据
 * @param[in]  reg_high 寄存器高字节地址
 * @param[out] value    指向 16-bit 变量的指针，存储 12-bit 有效值
 * @retval HAL_OK    读取成功
 * @retval 其他      参考 HAL_I2C_Mem_Read 返回值
 */
static HAL_StatusTypeDef AS5600_ReadWord(uint8_t reg_high, uint16_t *value)
{
    uint8_t data[2];
    HAL_StatusTypeDef status;

    status = AS5600_ReadReg(reg_high, data, 2);
    if (status != HAL_OK) {
        return status;
    }

    *value = ((uint16_t)data[0] << 8) | data[1];
    *value &= 0x0FFF;

    return HAL_OK;
}

/**
 * @brief  初始化 AS5600，检测设备是否在线
 * @note   通过 HAL_I2C_IsDeviceReady() 发送设备地址检测，
 *         若设备无应答则返回 HAL_ERROR
 * @retval HAL_OK    设备在线，初始化成功
 * @retval HAL_ERROR 设备无应答
 */
HAL_StatusTypeDef AS5600_Init(void)
{
    if (HAL_I2C_IsDeviceReady(&hi2c1, AS5600_I2C_ADDR, 3, HAL_MAX_DELAY) != HAL_OK) {
        return HAL_ERROR;
    }

    return HAL_OK;
}

/**
 * @brief  读取经滤波处理后的角度值
 * @note   读取 ANGLE 寄存器 (0x0E, 0x0F)，适合 FOC 控制使用
 * @param[out] angle 指向角度变量的指针，读取结果存入 (范围 0 ~ 4095)
 * @retval HAL_OK    读取成功
 * @retval HAL_ERROR I2C 通信失败
 */
HAL_StatusTypeDef AS5600_ReadAngle(uint16_t *angle)
{
    return AS5600_ReadWord(AS5600_REG_ANGLE_H, angle);
}

/**
 * @brief  读取未经滤波的原始角度值
 * @note   读取 RAW_ANGLE 寄存器 (0x0C, 0x0D)，响应更快但噪声较大
 * @param[out] raw_angle 指向原始角度变量的指针 (范围 0 ~ 4095)
 * @retval HAL_OK    读取成功
 * @retval HAL_ERROR I2C 通信失败
 */
HAL_StatusTypeDef AS5600_ReadRawAngle(uint16_t *raw_angle)
{
    return AS5600_ReadWord(AS5600_REG_RAW_ANGLE_H, raw_angle);
}

/**
 * @brief  读取当前磁场强度 (CORDIC 幅值)
 * @note   状态寄存器低 8 位与高 3 位拼合为 12-bit 磁场强度值，
 *         可用于诊断磁铁安装状态
 * @param[out] magnitude 指向磁场强度变量的指针 (范围 0 ~ 4095)
 * @retval HAL_OK    读取成功
 * @retval HAL_ERROR I2C 通信失败
 */
HAL_StatusTypeDef AS5600_ReadMagnitude(uint16_t *magnitude)
{
    uint8_t data[2];

    if (AS5600_ReadReg(AS5600_REG_STATUS, data, 2) != HAL_OK) {
        return HAL_ERROR;
    }

    *magnitude = ((uint16_t)(data[0] & 0x38) << 6) | data[1];
    *magnitude &= 0x0FFF;

    return HAL_OK;
}

/**
 * @brief  将角度原始值转换为弧度值
 * @note   转换公式: angle / 4096 * 2π
 * @param[in] angle 角度原始值 (0 ~ 4095)
 * @return 弧度值，范围 [0.0f, 2π)
 */
float AS5600_AngleToRadians(uint16_t angle)
{
    return (float)angle * 6.283185307f / AS5600_RESOLUTION;
}

/**
 * @brief  将角度原始值转换为归一化值
 * @note   转换公式: angle / 4096
 * @param[in] angle 角度原始值 (0 ~ 4095)
 * @return 归一化角度值，范围 [0.0f, 1.0f)
 */
float AS5600_AngleToNormalized(uint16_t angle)
{
    return (float)angle / AS5600_RESOLUTION;
}