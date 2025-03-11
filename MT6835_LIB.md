# อธิบายโค้ด

## ส่วนของ Header
```c
#include "mt6835.h"
#include <math.h>
#include <string.h>

static char *TAG = "MT6835";
```

- `#include "mt6835.h"` → เป็นไฟล์เฮดเดอร์ของเซ็นเซอร์ MT6835 ซึ่งอาจมีฟังก์ชันอื่น ๆ ที่เกี่ยวข้องกับเซ็นเซอร์
- `#include <math.h>` และ `#include <string.h>` → ใช้สำหรับการคำนวณทางคณิตศาสตร์และการจัดการข้อมูลสตริง
- `static char *TAG = "MT6835";` → ใช้สำหรับแสดงข้อความ debug (ในระบบที่รองรับ logging)

## ตาราง Lookup Table สำหรับ CRC-8

```c
#if MT6835_USE_CRC == 1
static uint8_t crc8_table[256] = {
    0x00, 0x07, 0x0e, 0x09, 0x1c, 0x1b, 0x12, 0x15, 0x38, 0x3f, 0x36, 0x31, 0x24, 0x23, 0x2a, 0x2d,
    0x70, 0x77, 0x7e, 0x79, 0x6c, 0x6b, 0x62, 0x65, 0x48, 0x4f, 0x46, 0x41, 0x54, 0x53, 0x5a, 0x5d,
    ...
};
```
- ตาราง `crc8_table[256]` → เป็น Lookup Table ที่ใช้สำหรับคำนวณค่า CRC-8 โดยใช้โพลิโนเมียล X⁸ + X² + X + 1
- `#if MT6835_USE_CRC == 1` → กำหนดให้ใช้ CRC-8 ก็ต่อเมื่อมีการเปิดใช้งาน `MT6835_USE_CRC`
  
```c
static uint8_t crc_table(const uint8_t *data, uint8_t len) {
    uint8_t crc = 0x00; // ค่าเริ่มต้นของ CRC

    for (uint8_t i = 0; i < len; i++) {
        crc ^= data[i]; //ทำ XOR ระหว่างค่า CRC ปัจจุบันกับค่าข้อมูล
        crc = crc8_table[crc]; // ใช้ตาราง lookup table เพื่อคำนวณค่าใหม่
    }

    return crc;
}
```
+ อินพุต:
    + `data` → ตัวชี้ไปยังข้อมูลที่ต้องการคำนวณ CRC
    + `len` → ขนาดของข้อมูล (จำนวนไบต์)
+   การทำงาน:
    1. กำหนดค่าเริ่มต้นของ CRC เป็น 0x00
    2. วนลูปทีละไบต์ของข้อมูล → นำมาทำ XOR กับค่า CRC ปัจจุบัน
    3. ใช้ค่า XOR ที่ได้ไปค้นหาค่าจาก crc8_table เพื่อนำมาปรับปรุงค่า CRC ใหม่
    4. ทำซ้ำไปเรื่อย ๆ จนครบทุกไบต์
    5. ส่งคืนค่า CRC-8 ที่คำนวณได้

## ฟังก์ชันเปิด/ปิดการตรวจสอบ CRC
```c
/**
 * @brief enable crc check
 * @param mt6835 mt6835 object
 */
void mt6835_enable_crc_check(mt6835_t *mt6835) {
    mt6835->crc_check = true;
}

/**
 * @brief disable crc check
 * @param mt6835 mt6835 object
 */
void mt6835_disable_crc_check(mt6835_t *mt6835) {
    mt6835->crc_check = false;
}
```

### การทำงาน
+ `mt6835_enable_crc_check(mt6835_t *mt6835)`
→ เปิดใช้งานการตรวจสอบ CRC โดยตั้งค่า `mt6835->crc_check = true;`
+ `mt6835_disable_crc_check(mt6835_t *mt6835)`
→ ปิดการตรวจสอบ CRC โดยตั้งค่า `mt6835->crc_check = false;`
### ใช้ทำอะไร?
ใช้ควบคุมว่าเซ็นเซอร์ MT6835 จะตรวจสอบค่าความถูกต้องของข้อมูลที่รับมาหรือไม่

## ฟังก์ชันควบคุม SPI Chip Select (CS)
```c
/**
 * @brief spi cs control, this function is weak, you can override it
 * @param state MT6835_CS_HIGH or MT6835_CS_LOW
 */
__attribute__((weak)) void mt6835_cs_control(mt6835_cs_state_enum_t state) {
    (void)state;
}
```
### การทำงาน
+ ฟังก์ชันนี้ใช้สำหรับ ควบคุมสัญญาณ CS (Chip Select) ของ SPI
+ ใช้ค่า `MT6835_CS_HIGH` หรือ `MT6835_CS_LOW` เพื่อกำหนดสถานะของ CS
+ ฟังก์ชันถูกกำหนดให้เป็น `__attribute__((weak))`ซึ่งหมายความว่าสามารถเขียนฟังก์ชันของตัวเองมาทดแทนได้โดยไม่ต้องแก้ไขโค้ดหลัก
### ใช้ทำอะไร?
กำหนดสถานะของขา CS ของ SPI เพื่อเลือกอุปกรณ์ MT6835 ก่อนรับ-ส่งข้อมูล

## ฟังก์ชันรับ-ส่งข้อมูลผ่าน SPI
```c
/**
 * @brief spi send, this function is weak, you can override it
 * @param tx_buf tx buffer
 * @param len length
 */
__attribute__((weak)) void mt6835_spi_send(uint8_t *tx_buf, uint8_t len) {
    (void)tx_buf;
    (void)len;
}
```
### การทำงาน
+ ฟังก์ชันนี้ใช้ส่งข้อมูลไปยังเซ็นเซอร์ MT6835 ผ่าน SPI
+ รับค่า `tx_buf` (บัฟเฟอร์ข้อมูล) และ `len` (ความยาวข้อมูล)
+ ฟังก์ชันเป็น `weak` ทำให้สามารถกำหนดฟังก์ชันที่ใช้งานจริงทับได้
### ใช้ทำอะไร?
ใช้ส่งข้อมูลไปยังเซ็นเซอร์ MT6835 ผ่าน SPI

```c
/**
 * @brief spi receive, this function is weak, you can override it
 * @param rx_buf rx buffer
 * @param len length
 * @return uint8_t rx data
 */
__attribute__((weak)) void mt6835_spi_recv(uint8_t *rx_buf, uint8_t len) {
    (void)rx_buf;
    (void)len;
}
```
### การทำงาน
  + ฟังก์ชันนี้ใช้รับข้อมูลจากเซ็นเซอร์ MT6835 ผ่าน SPI
  + รับค่า `rx_buf` (บัฟเฟอร์รับข้อมูล) และ `len` (จำนวนไบต์ที่จะรับ)
  + ฟังก์ชันนี้เป็น `weak` ทำให้สามารถ override ได้
### ใช้ทำอะไร?
ใช้รับข้อมูลจากเซ็นเซอร์ MT6835 ผ่าน SPI

```c
/**
 * @brief spi send and receive, this function is weak, you can override it
 * @param tx_buf tx buffer
 * @param rx_buf rx buffer
 * @param len length
 */
__attribute__((weak)) void mt6835_spi_send_recv(uint8_t *tx_buf, uint8_t *rx_buf, uint8_t len) {
    (void)tx_buf;
    (void)rx_buf;
    (void)len;
}
```
### การทำงาน
+ ฟังก์ชันนี้ใช้สำหรับ ส่งและรับข้อมูลพร้อมกัน ผ่าน SPI (Full Duplex)
+ ใช้ `tx_buf` (ข้อมูลที่จะส่ง) และ `rx_buf` (บัฟเฟอร์เก็บค่าที่รับมา)
+ ฟังก์ชันเป็น `weak` ทำให้สามารถ override ได้
### ใช้ทำอะไร?
ใช้ส่งและรับข้อมูลกับเซ็นเซอร์ MT6835 พร้อมกันผ่าน SPI

---
---

##  ฟังก์ชันสร้างอ็อบเจ็กต์ mt6835_create()
```c
/**
 * @brief create a mt6835 object
 * @return mt6835 object
 */
mt6835_t *mt6835_create() {
    mt6835_t *mt6835 = (mt6835_t *)MT6835_MALLOC(sizeof(mt6835_t));
    if (mt6835 == NULL) {
        MT6835_DEBUG("%s malloc failed", TAG);
        return NULL;
    }
    memset(mt6835, 0, sizeof(mt6835_t));
    mt6835->crc_check = false;
    return mt6835;
}
```
### รายละเอียด
#### 1.ขอหน่วยความจำสำหรับอ็อบเจ็กต์ `mt6835_t`
```c
mt6835_t *mt6835 = (mt6835_t *)MT6835_MALLOC(sizeof(mt6835_t));
```
+ ใช้ `MT6835_MALLOC(sizeof(mt6835_t))` เพื่อจองหน่วยความจำแบบ dynamic
+ `mt6835_t *` คือพอยน์เตอร์ไปยังอ็อบเจ็กต์ที่สร้างขึ้น
+ `MT6835_MALLOC` น่าจะเป็น macro ที่กำหนดเป็น `malloc()`หรือฟังก์ชันที่คล้ายกัน
#### 2.ตรวจสอบว่า `malloc` สำเร็จหรือไม่
```c
if (mt6835 == NULL) {
    MT6835_DEBUG("%s malloc failed", TAG);
    return NULL;
}
```
+ หาก `malloc()` ล้มเหลว (หน่วยความจำไม่พอ) ให้พิมพ์ข้อความแจ้งเตือนและคืนค่า `NULL`

#### 3.เคลียร์หน่วยความจำของ `mt6835`
```c
memset(mt6835, 0, sizeof(mt6835_t));
```
ใช้ `memset()` เพื่อกำหนดค่าเริ่มต้นของทุกไบต์เป็น `0`
ป้องกันค่าขยะจากหน่วยความจำที่ไม่ได้กำหนดค่า

#### 4.ตั้งค่าเริ่มต้นให้ `crc_check` เป็น `false`
```c
mt6835->crc_check = false;
```
+ ใช้ค่าเริ่มต้นของตัวแปร `crc_check` เป็น `false` (ปิดการตรวจสอบ CRC)

#### 5.คืนค่าอ็อบเจ็กต์ `mt6835_t *` ที่สร้างขึ้น

```c
return mt6835;
```

##  ฟังก์ชันทำลายอ็อบเจ็กต์ mt6835_destroy()
```c
/**
 * @brief destroy a mt6835 object
 * @param mt6835 mt6835 object
 */
void mt6835_destroy(mt6835_t *mt6835) {
    if (mt6835 != NULL) {
        MT6835_FREE(mt6835);
    }
}
```

### รายละเอียด
#### 1. ตรวจสอบว่าพอยน์เตอร์ `mt6835` เป็น `NULL` หรือไม่
```c
if (mt6835 != NULL) {
```
+ เพื่อป้อกันข้อผิดพลาด หากพยายามคืนหน่วยความจำของ `NULL` pointer`
+ ถ้า `mt6835` เป็น `NULL` ฟังก์ชันจะไม่ทำอะไรเลย

#### 2.คืนหน่วยความจำ
```c
MT6835_FREE(mt6835);
```
+ `MT6835_FREE()` น่าจะเป็น macro ของ `free()`
+ คืนหน่วยความจำที่เคยจองไว้ตอน `mt6835_create()`
+ ป้องกัน memory leak

## ฟังก์ชันเชื่อมโยงการควบคุม SPI Chip Select (CS)
```c
void mt6835_link_spi_cs_control(mt6835_t *mt6835, void (*spi_cs_control)(mt6835_cs_state_enum_t state)) {
    if (mt6835 == NULL) {
        MT6835_DEBUG("%s mt6835 object is null", TAG);
        return;
    }
    if (spi_cs_control == NULL) {
        MT6835_DEBUG("%s mt6835 object use default spi_cs_control(null)", TAG);
        mt6835->func.spi_cs_control = mt6835_cs_control;
    } else {
        mt6835->func.spi_cs_control = spi_cs_control;
    }
}
```

### การทำงาน
#### 1.ตรวจสอบว่าอ็อบเจ็กต์ `mt6835` เป็น `NULL` หรือไม่
```c
if (mt6835 == NULL) {
    MT6835_DEBUG("%s mt6835 object is null", TAG);
    return;
}
```
+ ถ้า `mt6835` เป็น `NULL` แสดงว่าอ็อบเจ็กต์ไม่ถูกสร้างขึ้น -> แสดง debug log แล้วออกจากฟังก์ชัน

#### 2.ตรวจสอบว่า `spi_cs_control` เป็น `NULL` หรือไม่
```c
if (spi_cs_control == NULL) {
    MT6835_DEBUG("%s mt6835 object use default spi_cs_control(null)", TAG);
    mt6835->func.spi_cs_control = mt6835_cs_control;
} else {
    mt6835->func.spi_cs_control = spi_cs_control;
}
```
+ ถ้าพารามิเตอร์ `spi_cs_control` เป็น `NULL` ให้ใช้ค่าเริ่มต้น `mt6835_cs_control`
+ ถ้าผู้ใช้กำหนดฟังก์ชันมาเอง (`spi_cs_control` ไม่เป็น `NULL`) ให้ใช้ฟังก์ชันที่กำหนด

## ฟังก์ชันเชื่อมโยงการส่งข้อมูล SPI
```c
void mt6835_link_spi_send(mt6835_t *mt6835, void (*spi_send)(uint8_t *tx_buf, uint8_t len)) {
    if (mt6835 == NULL) {
        MT6835_DEBUG("%s mt6835 object is null", TAG);
        return;
    }
    if (spi_send == NULL) {
        MT6835_DEBUG("%s mt6835 object use default spi_send(null)", TAG);
        mt6835->func.spi_send = mt6835_spi_send;
    }
    mt6835->func.spi_send = spi_send;
}
```

### การทำงาน
1. เช็คว่า `mt6835` เป็น `NULL` -> ถ้าเป็น `NULL` ให้แสดง debug log แล้วออกจากฟังก์ชัน
2. เช็คว่า `spi_send` เป็น `NULL`
   + ถ้า `NULL` -> ใช้ค่าเริ่มต้น `mt6835_spi_send`

## ฟังก์ชันเชื่อมโยงการรับข้อมูล SPI
```c
void mt6835_link_spi_recv(mt6835_t *mt6835, void (*spi_recv)(uint8_t *rx_buf, uint8_t len)) {
    if (mt6835 == NULL) {
        MT6835_DEBUG("%s mt6835 object is null", TAG);
        return;
    }
    if (spi_recv == NULL) {
        MT6835_DEBUG("%s mt6835 object use default spi_recv(null)", TAG);
        mt6835->func.spi_recv = mt6835_spi_recv;
    }
    mt6835->func.spi_recv = spi_recv;
}
```
### การทำงาน
+ คล้ายกับ `mt6835_link_spi_send()`

## ฟังก์ชันเชื่อมโยงการส่งและรับข้อมูล SPI พร้อมกัน
```c
void mt6835_link_spi_send_recv(mt6835_t *mt6835, void (*spi_send_recv)(uint8_t *tx_buf, uint8_t *rx_buf, uint8_t len)) {
    if (mt6835 == NULL) {
        MT6835_DEBUG("%s mt6835 object is null", TAG);
        return;
    }
    if (spi_send_recv == NULL) {
        MT6835_DEBUG("%s mt6835 object use default spi_send_recv(null)", TAG);
        mt6835->func.spi_send_recv = mt6835_spi_send_recv;
    }
    mt6835->func.spi_send_recv = spi_send_recv;
}
```
### การทำงาน
+ คล้ายกับฟังก์ชัน `mt6835_link_spi_send()` และ `mt6835_link_spi_recv()`

##  ฟังก์ชันอ่านรีจิสเตอร์: mt6835_read_reg()
```c
uint8_t mt6835_read_reg(mt6835_t *mt6835, mt6835_reg_enum_t reg) {
    uint8_t result[3] = {0, 0, 0};

    /* data frame */
    mt6835->func.spi_cs_control(MT6835_CS_HIGH);
    mt6835->data_frame.cmd = MT6835_CMD_RD; // byte read command
    mt6835->data_frame.reg = reg;

    mt6835->func.spi_cs_control(MT6835_CS_LOW);
    if (mt6835->func.spi_send_recv == NULL ? false : true) {
        mt6835->func.spi_send_recv((uint8_t *)&mt6835->data_frame.pack, (uint8_t *)&result, 3);
    } else {
        mt6835->func.spi_send((uint8_t *)&mt6835->data_frame.pack, 3);
        mt6835->func.spi_recv((uint8_t *)&result, 3);
    }
    mt6835->func.spi_cs_control(MT6835_CS_HIGH);

    return result[2];
}
```
### การทำงานของฟังก์ชัน
#### 1.สร้างตัวแปร `result[3]`
```c
uint8_t result[3] = {0, 0, 0};
```
+ ใช้เก็บค่าที่อ่านจาก SPI (3 ไบต์)

#### 2.กำหนดคำสั่งอ่าน (`MT6835_CMD_RD`) และรีจิสเตอร์ที่ต้องการอ่าน
```c
mt6835->data_frame.cmd = MT6835_CMD_RD; // byte read command
mt6835->data_frame.reg = reg;
```
+ `cmd = MT6835_CMD_RD` → กำหนดให้เป็นคำสั่ง "อ่านข้อมูล"
+ `reg = reg` → กำหนดรีจิสเตอร์ที่ต้องการอ่าน

#### 3.เริ่มต้น SPI Communication
```c
mt6835->func.spi_cs_control(MT6835_CS_LOW);
```
ดึง **Chip Select (CS)** ต่ำ เพื่อเริ่มการส่งข้อมูล SPI

#### 4.เช็คว่ามีฟังก์ชัน `spi_send_recv()` หรือไม่
```c
if (mt6835->func.spi_send_recv == NULL ? false : true) {
    mt6835->func.spi_send_recv((uint8_t *)&mt6835->data_frame.pack, (uint8_t *)&result, 3);
} else {
    mt6835->func.spi_send((uint8_t *)&mt6835->data_frame.pack, 3);
    mt6835->func.spi_recv((uint8_t *)&result, 3);
}
```
+ ถ้ามีฟังก์ชัน `spi_send_recv()` → ใช้งานทันที (ส่งและรับข้อมูลพร้อมกัน)
+ ถ้าไม่มี → ใช้ `spi_send()` และ `spi_recv()` แยกกัน

#### 5.ปิดการสื่อสาร SPI
```c
mt6835->func.spi_cs_control(MT6835_CS_HIGH);
```
+ ปรับ Chip Select (CS) สูง เพื่อจบการสื่อสาร

#### 6.คืนค่าข้อมูลที่อ่านจากรีจิสเตอร์
```c
return result[2];
```
+ ข้อมูลที่อ่านได้อยู่ที่ ไบต์ที่ 2 (result[2])
+ ค่าไบต์แรก (result[0]) และไบต์ที่สอง (result[1]) อาจใช้สำหรับการตรวจสอบ CRC หรือสถานะ

## ฟังก์ชันเขียนรีจิสเตอร์: mt6835_write_reg()
```c
void mt6835_write_reg(mt6835_t *mt6835, mt6835_reg_enum_t reg, uint8_t data) {
    uint8_t result[3] = {0, 0, 0};

    mt6835->func.spi_cs_control(MT6835_CS_HIGH);
    mt6835->data_frame.cmd = MT6835_CMD_WR; // byte write command
    mt6835->data_frame.reg = reg;
    mt6835->data_frame.normal_byte = data;

    mt6835->func.spi_cs_control(MT6835_CS_LOW);
    mt6835->func.spi_send_recv((uint8_t *)&mt6835->data_frame.pack, (uint8_t *)&result, 3);
    mt6835->func.spi_cs_control(MT6835_CS_HIGH);
}
```

### การทำงานของฟังก์ชัน
#### 1.สร้างตัวแปร `result[3]`
```c
uint8_t result[3] = {0, 0, 0};
```
+ ใช้เก็บค่าตอบกลับจาก MT6835
#### 2.กำหนดคำสั่งเขียน (`MT6835_CMD_WR`) และข้อมูลที่ต้องการเขียน
```c
mt6835->data_frame.cmd = MT6835_CMD_WR; // byte write command
mt6835->data_frame.reg = reg;
mt6835->data_frame.normal_byte = data;
```
+ `cmd = MT6835_CMD_WR` → กำหนดให้เป็นคำสั่ง "เขียนข้อมูล"
+ `reg = reg` → กำหนดรีจิสเตอร์ที่ต้องการเขียน
+ `normal_byte = data` → กำหนดค่าที่ต้องการเขียนลงรีจิสเตอร์

#### 3.เริ่มต้น SPI Communication
```c
mt6835->func.spi_cs_control(MT6835_CS_LOW);
```
+ ดึง Chip Select (CS) ต่ำ เพื่อเริ่มการส่งข้อมูล SPI

#### 4.ส่งข้อมูลไปยังรีจิสเตอร์ของ MT6835
```c
mt6835->func.spi_send_recv((uint8_t *)&mt6835->data_frame.pack, (uint8_t *)&result, 3);
```
+ ใช้ `spi_send_recv()` ส่งคำสั่งไปยังเซ็นเซอร์
+ `result[]` ใช้เก็บค่าตอบกลับจาก MT6835

#### 5.ปิดการสื่อสาร SPI
```c
mt6835->func.spi_cs_control(MT6835_CS_HIGH);
```
+ ปรับ Chip Select (CS) สูง เพื่อจบการสื่อสาร

## ฟังก์ชัน `mt6835_write_eeprom()`
```c
/**
 * @brief write mt6835 eeprom, you must to write corresponding register first, and then call this function.
 * you need wait 6 seconds after write eeprom
 * @param mt6835 mt6835 object
 * @return true if success, false otherwise
 */
bool mt6835_write_eeprom(mt6835_t *mt6835) {
    uint8_t result[3] = {0, 0, 0xFF};

    mt6835->func.spi_cs_control(MT6835_CS_HIGH);
    mt6835->data_frame.cmd = MT6835_CMD_EEPROM; // byte read command
    mt6835->data_frame.reg = 0;
    mt6835->data_frame.normal_byte = 0;

    mt6835->func.spi_cs_control(MT6835_CS_LOW);
    mt6835->func.spi_send_recv((uint8_t *)&mt6835->data_frame.pack, (uint8_t *)&result, 3);
    mt6835->func.spi_cs_control(MT6835_CS_HIGH);

    if (result[2] == 0xFF || result[2] != 0x55) {
        return false;
    }
    return true;
}
```
#### 1.ประกาศตัวแปร `uint8_t result[3]`
```c
bool mt6835_write_eeprom(mt6835_t *mt6835) {
    uint8_t result[3] = {0, 0, 0xFF};
```
+  ประกาศตัวแปร `result[3]` เพื่อเก็บค่าตอบกลับจาก SPI
    + `result[0] = 0` → ไบต์แรกของค่าตอบกลับ
    + `result[1] = 0` → ไบต์ที่สองของค่าตอบกลับ
    + `result[2] = 0xFF` → ตั้งค่าดีฟอลต์ให้เป็น `0xFF` ซึ่งหมายถึง ยังไม่มีข้อมูลที่อ่านมา

---
```c
 mt6835->func.spi_cs_control(MT6835_CS_HIGH);
```
+ ตั้งค่า Chip Select (CS) เป็น HIGH เพื่อให้แน่ใจว่า SPI ไม่ได้ทำงานก่อนเริ่มส่งข้อมูล
```c
 mt6835->data_frame.cmd = MT6835_CMD_EEPROM; // byte read command
    mt6835->data_frame.reg = 0;
    mt6835->data_frame.normal_byte = 0;
```
+ กำหนดค่าคำสั่ง (`cmd`)
    + `MT6835_CMD_EEPROM` → เป็นคำสั่งเขียน EEPROM
+ ตั้งค่าข้อมูลอื่นเป็น 0
    + `reg = 0` → ไม่ได้ระบุรีจิสเตอร์
    + `normal_byte = 0` → ไม่ได้ส่งข้อมูลเพิ่มเติม

```c
mt6835->func.spi_cs_control(MT6835_CS_LOW);
```
+ ดึง CS ลงต่ำ (CS_LOW) เพื่อเริ่มต้นการส่งข้อมูลผ่าน SPI

```c
    mt6835->func.spi_send_recv((uint8_t *)&mt6835->data_frame.pack, (uint8_t *)&result, 3);
```
+ ใช้ SPI ส่งคำสั่ง ไปยังเซ็นเซอร์ และรับค่าตอบกลับ
+ ส่งข้อมูลเป็น 3 ไบต์
+ ผลลัพธ์ที่อ่านได้จะถูกเก็บไว้ใน `result[]`

```c
    mt6835->func.spi_cs_control(MT6835_CS_HIGH);
```
+ ปิดการสื่อสาร SPI โดยดึง CS ขึ้นสูง (`CS_HIGH`)

```c
    if (result[2] == 0xFF || result[2] != 0x55) {
        return false;
    }
```

+ ตรวจสอบผลลัพธ์ที่ได้รับ (`result[2]`)
    + ถ้า result[2] == 0xFF → แสดงว่าการอ่านค่าล้มเหลว (ผิดพลาด)
    + ถ้า result[2] != 0x55 → EEPROM ไม่ยืนยันว่าการเขียนสำเร็จ (ผิดพลาด)
    + ถ้าผิดพลาด ให้คืนค่า false
  
```c
     return true;
}
```
+ ถ้าค่าที่อ่านได้คือ `0x55` → แสดงว่าการเขียน EEPROM สำเร็จ → คืนค่า `true`


## ฟังก์ชัน `mt6835_get_id()`
```c
uint8_t mt6835_get_id(mt6835_t *mt6835) {
    return mt6835_read_reg(mt6835, MT6835_REG_ID);
}
```

1. เรียกใช้ `mt6835_read_reg(mt6835, MT6835_REG_ID)`
+ ฟังก์ชัน `mt6835_read_reg()` จะใช้ SPI อ่านค่าจากรีจิสเตอร์ `MT6835_REG_ID`
2. คืนค่าผลลัพธ์ที่อ่านได้ (ค่า ID ของเซ็นเซอร์) ออกไปเป็น `uint8_t`

##  ฟังก์ชัน `mt6835_set_id()`
```c
void mt6835_set_id(mt6835_t *mt6835, uint8_t custom_id) {
    mt6835_write_reg(mt6835, MT6835_REG_ID, custom_id);
}
```
1. รับค่า `custom_id` เป็นพารามิเตอร์ (ค่า ID ที่ต้องการตั้ง)
2. เรียกใช้ `mt6835_write_reg(mt6835, MT6835_REG_ID, custom_id)`
+ ฟังก์ชัน `mt6835_write_reg()` จะใช้ SPI ส่งค่าที่กำหนดไปยังรีจิสเตอร์ `MT6835_REG_ID`
3. ค่าที่กำหนดจะถูกบันทึกเป็น ID ของเซ็นเซอร์

##  ฟังก์ชัน `mt6835_get_raw_zero_angle()`
```c
uint16_t mt6835_get_raw_zero_angle(mt6835_t *mt6835) {
    uint8_t rx_buf[2] = {0};
    rx_buf[1] = mt6835_read_reg(mt6835, MT6835_REG_ZERO2);
    rx_buf[0] = mt6835_read_reg(mt6835, MT6835_REG_ZERO1);
    uint16_t res = (rx_buf[1] << 4) | (rx_buf[0] >> 4);
    return res;
}
```
### การทำงาน
1. ประกาศ `rx_buf[2]` เพื่อเก็บค่าที่อ่านจากรีจิสเตอร์ของ MT6835
2. อ่านค่าจากรีจิสเตอร์ `MT6835_REG_ZERO2` และเก็บที่ `rx_buf[1]`
3. อ่านค่าจากรีจิสเตอร์ `MT6835_REG_ZERO1` และเก็บที่ `rx_buf[0]`
4. นำค่าจาก `rx_buf[1]` และ `rx_buf[0]` มารวมกันเป็นค่า Zero Angle (raw)
+ ใช้บิตชิฟต์ `(rx_buf[1] << 4) | (rx_buf[0] >> 4)` เพื่อรวมค่าจากรีจิสเตอร์ทั้งสองให้เป็นค่า 12-bit
5. คืนค่าผลลัพธ์เป็น `uint16_t` ซึ่งเป็นค่า Zero Angle ในรูปแบบดิบ (Raw Data)
###  วิธีการรวมค่ารีจิสเตอร์ (12-bit Zero Angle)
+ `MT6835_REG_ZERO1` เก็บ 8-bit ต่ำสุด
+ `MT6835_REG_ZERO2` เก็บ 4-bit สูงสุด
ค่าดิบจะต้องรวมกันเป็น 12-bit โดยใช้

```c
(rx_buf[1] << 4) | (rx_buf[0] >> 4)
```
+ `rx_buf[1] << 4` → ขยับค่า `ZERO2` ขึ้นไป 4 บิต
+ `rx_buf[0] >> 4` → ดึง 4 บิตที่ต้องการจาก `ZERO1`
+ จากนั้นรวมกันเป็นค่า 12-bit

## ฟังก์ชัน `mt6835_get_zero_angle()`
```c
float mt6835_get_zero_angle(mt6835_t *mt6835) {
    return (float)mt6835_get_raw_zero_angle(mt6835) * MT6835_ZERO_REG_STEP;
}
```

### การทำงาน
1. เรียกใช้ `mt6835_get_raw_zero_angle()` เพื่อรับค่า Zero Angle (raw)
2. คูณค่าดิบด้วยค่าคงที่ `MT6835_ZERO_REG_STEP` เพื่อแปลงเป็นหน่วย องศา (degree)
3. คืนค่าเป็น `float` ซึ่งเป็นค่ามุม Zero Angle ในหน่วยองศา
### `MT6835_ZERO_REG_STEP` คืออะไร?
+ ค่าคงที่ที่ใช้แปลงค่า Raw Data (12-bit) ไปเป็นหน่วย **องศา**
+ ปกติแล้ว Zero Angle จะมีขนาด 12-bit (0 ถึง 4095) ซึ่งแทนมุม 0° ถึง 360°
ค่าที่ใช้แปลงจะเป็น
```c
MT6835_ZERO_REG_STEP = 360.0 / 4096.0 = 0.087890625
```
+ ซึ่งหมายความว่า 1 unit ของ Raw Data = 0.08789°

## ฟังก์ชัน `mt6835_set_zero_angle()`

```c
bool mt6835_set_zero_angle(mt6835_t *mt6835, float rad) {
    uint16_t angle = (uint16_t)roundf(rad * 57.295779513f / MT6835_ZERO_REG_STEP);
    if (angle > 0xFFF) {
        return false;
    }

    uint8_t tx_buf[2] = {0};

    tx_buf[1] = angle >> 4;
    tx_buf[0] = (angle & 0x0F) << 4;
    tx_buf[0] |= mt6835_read_reg(mt6835, MT6835_REG_ZERO1) & 0x0F;

    mt6835_write_reg(mt6835, MT6835_REG_ZERO2, tx_buf[1]);
    mt6835_write_reg(mt6835, MT6835_REG_ZERO1, tx_buf[0]);

    return true;
}
```

####  1.แปลงค่าเรเดียนเป็นค่าที่เซ็นเซอร์รองรับ (12-bit)
```c
uint16_t angle = (uint16_t)roundf(rad * 57.295779513f / MT6835_ZERO_REG_STEP);
```

+ `rad * 57.295779513f` → แปลงค่าจากเรเดียนเป็นองศา
+ หารด้วย `MT6835_ZERO_REG_STEP` → คำนวณค่าที่เหมาะสมสำหรับรีจิสเตอร์
+ `roundf(...)` → ปัดค่าทศนิยมให้เป็นจำนวนเต็ม
+ แปลงค่าเป็น `uint16_t` (12-bit)

####  2.ตรวจสอบค่าที่คำนวณว่าอยู่ในช่วงที่รองรับ (0 - 4095)
```c
if (angle > 0xFFF) {
    return false;
}
```
+  ค่า Zero Angle ที่เซ็นเซอร์รองรับเป็น 12-bit (0 - 0xFFF หรือ 0 - 4095)
+   ถ้าค่าที่คำนวณได้ เกิน 4095 → ฟังก์ชัน ล้มเหลว (return false)

#### 3.จัดการค่าที่จะเขียนลงรีจิสเตอร์
```c
uint8_t tx_buf[2] = {0};
tx_buf[1] = angle >> 4;
tx_buf[0] = (angle & 0x0F) << 4;
tx_buf[0] |= mt6835_read_reg(mt6835, MT6835_REG_ZERO1) & 0x0F;
```
#### 📌 อธิบายการจัดการค่า Zero Angle (12-bit)
+ Zero Angle เก็บเป็น 12-bit (2 รีจิสเตอร์)
    + MT6835_REG_ZERO1 → เก็บ 8-bit ล่างสุด (เฉพาะ 4-bit สูงสุด)
    + MT6835_REG_ZERO2 → เก็บ 4-bit บนสุด
+ ใช้ Bitwise Operations แยกค่าจาก `angle` เป็น 2-byte
    + `tx_buf[1] = angle >> 4;` → ดึง 4-bit สูงสุดไปไว้ที่ `tx_buf[1]`
    + `tx_buf[0] = (angle & 0x0F) << 4;` → ดึง 4-bit ต่ำสุดไปไว้ที่ `tx_buf[0]`
    + `tx_buf[0] |= mt6835_read_reg(mt6835, MT6835_REG_ZERO1) & 0x0F;` → รักษา 4-bit ต่ำสุดของรีจิสเตอร์ `ZERO1 `ที่มีอยู่เดิม

#### 4. เขียนค่าลงรีจิสเตอร์
```c
mt6835_write_reg(mt6835, MT6835_REG_ZERO2, tx_buf[1]);
mt6835_write_reg(mt6835, MT6835_REG_ZERO1, tx_buf[0]);
```

+ การเขียนค่าไปยังเซ็นเซอร์
  + `MT6835_REG_ZERO2` → เขียน 4-bit บน
  + `MT6835_REG_ZERO1` → เขียน 4-bit ล่าง + 4-bit เดิมที่อ่านมา

#### 5.ส่งค่ากลับเพื่อบอกว่าตั้งค่าสำเร็จ
```c
return true;
```
+  ถ้าตั้งค่าได้สำเร็จ ฟังก์ชันจะคืนค่า `true`


---
```c

/**
 * @brief get mt6835 raw angle
 * @param mt6835 mt6835 object
 * @param method read angle method, MT6835_READ_ANGLE_METHOD_NORMAL or MT6835_READ_ANGLE_METHOD_BURST
 * @return raw angle data in raw
 */
uint32_t mt6835_get_raw_angle(mt6835_t *mt6835, mt6835_read_angle_method_enum_t method) {
    uint8_t rx_buf[6] = {0};
    uint8_t tx_buf[6] = {0};

    switch (method) {
        case MT6835_READ_ANGLE_METHOD_NORMAL: {
            rx_buf[0] = mt6835_read_reg(mt6835, MT6835_REG_ANGLE3);
            rx_buf[1] = mt6835_read_reg(mt6835, MT6835_REG_ANGLE2);
            rx_buf[2] = mt6835_read_reg(mt6835, MT6835_REG_ANGLE1);
            if (mt6835->crc_check) {
                rx_buf[3] = mt6835_read_reg(mt6835, MT6835_REG_CRC);
            }
            break;
        }
        case MT6835_READ_ANGLE_METHOD_BURST: {
            const uint8_t rd = mt6835->crc_check ? 6 : 5;

            mt6835->func.spi_cs_control(MT6835_CS_HIGH);
            mt6835->data_frame.cmd = MT6835_CMD_BURST;
            mt6835->data_frame.reg = MT6835_REG_ANGLE3;
            tx_buf[0] = mt6835->data_frame.pack & 0xFF;
            tx_buf[1] = (mt6835->data_frame.pack >> 8) & 0xFF;

            mt6835->func.spi_cs_control(MT6835_CS_LOW);
            if (mt6835->func.spi_send_recv) {
                mt6835->func.spi_send_recv(tx_buf, rx_buf, rd);
            } else {
                mt6835->func.spi_send(tx_buf, rd);
                mt6835->func.spi_recv(rx_buf, rd);
            }
            mt6835->func.spi_cs_control(MT6835_CS_HIGH);

            memmove(rx_buf, &rx_buf[2], 3);
            if (mt6835->crc_check) {
                rx_buf[3] = rx_buf[5];
            }
            break;
        }
    }

    if (mt6835->crc_check) {
        mt6835->crc = rx_buf[3];
#if MT6835_USE_CRC == 1
        if (crc_table(rx_buf, 3) != rx_buf[3]) {
            MT6835_DEBUG("%s crc check failed\r\n", TAG);
            mt6835->crc_res = false;
            return 0;
        }
        mt6835->crc_res = true;
#endif
    }

    mt6835->state = rx_buf[2] & 0x07;
    return (rx_buf[0] << 13) | (rx_buf[1] << 5) | (rx_buf[2] >> 3);
}


/**
 * @brief get mt6835 angle in rad
 * @param mt6835 mt6835 object
 * @param method read angle method, MT6835_READ_ANGLE_METHOD_NORMAL or MT6835_READ_ANGLE_METHOD_BURST
 * @return angle data in rad
 */
float mt6835_get_angle(mt6835_t *mt6835, mt6835_read_angle_method_enum_t method) {
    uint32_t raw_angle = mt6835_get_raw_angle(mt6835, method);
    return (float)(raw_angle * 2.996056226329803e-6);
}

```

## ฟังก์ชัน `mt6835_get_raw_angle`
ฟังก์ชันนี้ใช้เพื่อดึงข้อมูลมุมดิบจากเซ็นเซอร์ MT6835 โดยสามารถเลือกโหมดการอ่านได้สองโหมดคือ ปกติ และ แบบบัสต์ นอกจากนี้ยังมีการตรวจสอบ CRC เพื่อยืนยันความถูกต้องของข้อมูลที่ได้รับจากเซ็นเซอร์
ฟังก์ชันนี้ทำหน้าที่ในการอ่านค่ามุม (raw angle) จากเซนเซอร์ MT6835 โดยมี 2 วิธีในการอ่านมุม คือ:

+ **MT6835_READ_ANGLE_METHOD_NORMAL**: การอ่านมุมโดยการอ่านค่าแยกเป็นระยะๆ
+ **MT6835_READ_ANGLE_METHOD_BURST**: การอ่านมุมในรูปแบบ burst (การอ่านหลายค่าพร้อมกันในครั้งเดียว)

## ฟังก์ชัน mt6835_get_raw_angle()
### รายละเอียดการทำงาน
#### 1.พารามิเตอร์:
+ `mt6835`: ตัวแปรที่เก็บข้อมูลของเซ็นเซอร์ MT6835
+ `method`: วิธีการอ่านมุมจากเซ็นเซอร์ (สามารถเลือกได้ระหว่าง `MT6835_READ_ANGLE_METHOD_NORMAL` หรือ `MT6835_READ_ANGLE_METHOD_BURST`)

#### 2.ขั้นตอนการทำงาน:
+ **การอ่านแบบ Normal:**
    + ฟังก์ชันจะอ่านค่า raw angle จากรีจิสเตอร์ `MT6835_REG_ANGLE3`, `MT6835_REG_ANGLE2`, และ `MT6835_REG_ANGLE1` โดยใช้ฟังก์ชัน `mt6835_read_reg()`
    + ถ้าตรวจสอบ CRC ถูกเปิดใช้งาน (โดยการตั้งค่า `mt6835->crc_check`), ฟังก์ชันจะอ่านค่า CRC จากรีจิสเตอร์ `MT6835_REG_CRC`
+ **การอ่านแบบ Burst:**
  + หากเลือกวิธีการ Burst, เซ็นเซอร์จะส่งคำสั่ง Burst เพื่ออ่านข้อมูลหลายรีจิสเตอร์ในครั้งเดียว
  + คำสั่ง `MT6835_CMD_BURST` ถูกตั้งค่าใน `mt6835->data_frame.cmd`
  + ฟังก์ชันจะส่งคำสั่งไปที่เซ็นเซอร์โดยใช้ SPI และรับข้อมูลใน `rx_buf`
  + ข้อมูลที่ได้รับจะถูกจัดเรียงใหม่โดยการย้ายข้อมูล (memmove) เพื่อให้ได้ข้อมูลมุมที่ถูกต้อง
#### 3.การตรวจสอบ CRC:

+ ถ้าเปิดการตรวจสอบ CRC (`mt6835->crc_check`), ฟังก์ชันจะตรวจสอบ CRC จากข้อมูลที่ได้รับ
+ หาก CRC ไม่ถูกต้อง, ฟังก์ชันจะส่งค่าผลลัพธ์เป็น 0 และแสดงข้อความว่า crc check failed
+ ถ้า CRC ถูกต้อง, จะบันทึกค่า CRC ที่ได้รับใน `mt6835->crc` และตั้งค่า `mt6835->crc_res` เป็น `true`
#### 4.การคำนวณมุม:

+ ค่ามุม raw จะถูกคำนวณจากข้อมูลที่อ่านได้จากรีจิสเตอร์และจัดเก็บใน `rx_buf`
+ ข้อมูลจาก `rx_buf[0]`, `rx_buf[1]`, และ `rx_buf[2]` จะถูกนำมาคำนวณและรวมกันเพื่อให้ได้ raw angle ที่เป็น 32-bit โดยใช้การ shift บิต:
```c
return (rx_buf[0] << 13) | (rx_buf[1] << 5) | (rx_buf[2] >> 3);
```
+ สถานะ (state) ของเซ็นเซอร์จะถูกบันทึกใน `mt6835->state` โดยเก็บ 3 บิตสุดท้ายจาก` rx_buf[2]`

## ฟังก์ชัน `mt6835_get_angle()`
ฟังก์ชันนี้จะใช้ข้อมูล raw angle ที่ได้รับจาก mt6835_get_raw_angle() แล้วแปลงค่ามุมเป็นหน่วย radian.

### รายละเอียดการทำงาน
#### 1.พารามิเตอร์:
+ `mt6835`: ตัวแปรที่เก็บข้อมูลของเซ็นเซอร์ MT6835
+ `method`: วิธีการอ่านมุมจากเซ็นเซอร์ (`MT6835_READ_ANGLE_METHOD_NORMAL` หรือ `MT6835_READ_ANGLE_METHOD_BURST`)
#### 2.การแปลงค่า raw angle:

+ ฟังก์ชันจะเรียก `mt6835_get_raw_angle()` เพื่อดึงข้อมูล raw angle จากเซ็นเซอร์
ค่า raw angle ที่ได้จะถูกแปลงเป็นมุมในหน่วย radian โดยใช้ตัวคูณ:
```c
(float)(raw_angle * 2.996056226329803e-6)
```
การแปลงนี้จะเปลี่ยนค่ามุมจาก raw data (ที่เซ็นเซอร์เก็บในหน่วยพิเศษ) เป็นมุมในหน่วย radian โดยตัวคูณ 2.`996056226329803e-6 `ใช้เพื่อแปลงค่า raw angle เป็นมุมในหน่วย radian
