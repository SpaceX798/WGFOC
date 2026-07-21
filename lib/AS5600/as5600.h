/**
 * @file    as5600.h
 * @brief   AS5600 磁编码器 I2C 驱动库
 * @author  Q3_SimulinkFOC
 * @date    2026-07-06
 *
 * 基于 HAL 库的 AS5600 12-bit 磁编码器驱动，通过 I2C 接口读取角度和磁场强度。
 * 该驱动独立于 CubeMX 代码生成，位于 lib/AS5600/ 目录下。
 *
 * @note   I2C 地址: 0x36 (7-bit), 400kHz Fast Mode
 * @note   角度分辨率: 12-bit (0 ~ 4095), 对应 0° ~ 360°
 */

#ifndef __AS5600_H__
#define __AS5600_H__

#include "main.h"

/** @defgroup AS5600_I2C_Address I2C 地址
 *  @{
 */
#define AS5600_I2C_ADDR       (0x36 << 1)  /**< 7-bit 地址 0x36 左移 1 位 */
/** @} */

/** @defgroup AS5600_Registers 寄存器地址
 *  @{
 */
#define AS5600_REG_ZMCO       0x00  /**< 零位烧写次数 (OTP)                    */
#define AS5600_REG_ZPOS_H     0x01  /**< 零位角度 高字节 [11:8]                */
#define AS5600_REG_ZPOS_L     0x02  /**< 零位角度 低字节 [7:0]                 */
#define AS5600_REG_MPOS_H     0x03  /**< 最大角度 高字节 [11:8]                */
#define AS5600_REG_MPOS_L     0x04  /**< 最大角度 低字节 [7:0]                 */
#define AS5600_REG_MANG_H     0x05  /**< 最大角度限制 高字节 [11:8]            */
#define AS5600_REG_MANG_L     0x06  /**< 最大角度限制 低字节 [7:0]             */
#define AS5600_REG_CONF_H     0x07  /**< 配置寄存器 高字节                    */
#define AS5600_REG_CONF_L     0x08  /**< 配置寄存器 低字节                    */
#define AS5600_REG_STATUS     0x0B  /**< 状态寄存器 (含磁场强度高 3 位)       */
#define AS5600_REG_RAW_ANGLE_H 0x0C /**< 原始角度 高字节 [11:4]                */
#define AS5600_REG_RAW_ANGLE_L 0x0D /**< 原始角度 低字节 [3:0] (高半字节)      */
#define AS5600_REG_ANGLE_H    0x0E  /**< 滤波角度 高字节 [11:4]                */
#define AS5600_REG_ANGLE_L    0x0F  /**< 滤波角度 低字节 [3:0] (高半字节)      */
/** @} */

/** @defgroup AS5600_Constants 常量定义
 *  @{
 */
#define AS5600_RESOLUTION     4096.0f  /**< 12-bit 分辨率，用于角度转换计算 */
/** @} */

/** @defgroup AS5600_API 公共 API
 *  @{
 */

/**
 * @brief  初始化 AS5600，检测设备是否在线
 * @note   通过 I2C 发送设备就绪检测，确认编码器连接正常
 * @retval HAL_OK    设备检测成功
 * @retval HAL_ERROR 设备无应答，检查 I2C 接线和供电
 */
HAL_StatusTypeDef AS5600_Init(void);

/**
 * @brief  读取经滤波处理后的角度值
 * @note   读取 ANGLE 寄存器 (0x0E, 0x0F)，该值经过 AS5600 内部低通滤波，
 *         响应较平滑，适合用于 FOC 控制的角度反馈
 * @param[out] angle 指向角度变量的指针，读取结果存入 (范围 0 ~ 4095)
 * @retval HAL_OK    读取成功
 * @retval HAL_ERROR I2C 通信失败
 */
HAL_StatusTypeDef AS5600_ReadAngle(uint16_t *angle);

/**
 * @brief  读取未经滤波的原始角度值
 * @note   读取 RAW_ANGLE 寄存器 (0x0C, 0x0D)，该值为原始采样值，
 *         响应速度更快但噪声较大，适合需要快速响应的场景
 * @param[out] raw_angle 指向原始角度变量的指针 (范围 0 ~ 4095)
 * @retval HAL_OK    读取成功
 * @retval HAL_ERROR I2C 通信失败
 */
HAL_StatusTypeDef AS5600_ReadRawAngle(uint16_t *raw_angle);

/**
 * @brief  读取当前磁场强度 (CORDIC 幅值)
 * @note   磁场强度可用于诊断磁铁安装是否偏轴或距离过远。
 *         正常工作时该值应在一定范围内波动，过高提示磁铁过近，过低提示磁铁过远或偏心。
 * @param[out] magnitude 指向磁场强度变量的指针 (范围 0 ~ 4095)
 * @retval HAL_OK    读取成功
 * @retval HAL_ERROR I2C 通信失败
 */
HAL_StatusTypeDef AS5600_ReadMagnitude(uint16_t *magnitude);

/**
 * @brief  将角度原始值转换为弧度
 * @note   转换公式: angle / 4096 * 2π
 * @param[in] angle 角度原始值 (0 ~ 4095)
 * @return 弧度值 (0.0f ~ 2π)
 */
float AS5600_AngleToRadians(uint16_t angle);

/**
 * @brief  将角度原始值转换为归一化值
 * @note   转换公式: angle / 4096，结果范围 0.0 ~ 1.0
 * @param[in] angle 角度原始值 (0 ~ 4095)
 * @return 归一化角度值 (0.0f ~ 1.0f)
 */
float AS5600_AngleToNormalized(uint16_t angle);

/** @} */

#endif /* __AS5600_H__ */