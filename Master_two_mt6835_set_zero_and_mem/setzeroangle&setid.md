# อธิบายการทำงานของ setid และ set_zero_angle
## 1. การทำงานของ `setid`
ใช้เพื่อกำหนด id ของ MT6835 ใช้ใน main loop ในการกำหนด id
```c
mt6835_set_id(mt6835_1, 0x00);
HAL_Delay(1);
uint8_t id = mt6835_get_id(mt6835_1);
printf("id: 0x%x\r\n", id);
```
+ `mt6835_set_id`กำหนด id เป็นเลขฐาน 16
+ `HAL_Delay(1);` ให้ delay เล็กน้อยเพือเขียนข้อมูลลงใน MT6835
+ `uint8_t id = mt6835_get_id(mt6835_1);` เก็บค่าไว้ใน `id` ด้วยคำสั่ง `mt6835_get_id`
+ แสดงค่า id `printf("id: 0x%x\r\n", id);`

### 💡 สรุป
ฟังก์ชัน setid มีหน้าที่กำหนดหมายเลขประจำตัว (ID) ให้กับ MT6835 และสามารถอ่านค่า ID ที่กำหนดแล้วมาตรวจสอบความถูกต้องได้

## 2.การทำงานของ `set_zero_angle`
ในโค้ดมีการคอมเมนต์ส่วนที่เกี่ยวข้องกับ set_zero_angle ไว้:
```c
mt6835_set_zero_angle(mt6835_1, 2.0943951024f); //Set by RAD
HAL_Delay(1);
float zero_angle = mt6835_get_zero_angle(mt6835_1)
printf("Zero_angle: %f rad\r\n",zero_angle);
bool respond = mt6835_write_eeprom(mt6835);
if (!respond) 
{
    printf("write eeprom failed\r\n");
} else 
{
    printf("write eeprom success\r\n");
}
HAL_Delay(6000);    // จำเป็นต้อง Delay 6 วินาทีตาม datasheet
```
ฟังก์ชัน set_zero_angle ใช้ในการกำหนด "จุดอ้างอิงมุมศูนย์ (Zero Angle)" ให้กับ MT6835

### ขั้นตอนการทำงาน
#### 1.ตั้งค่ามุมศูนย์ใหม่ให้เซ็นเซอร์
+ `mt6835_set_zero_angle(mt6835_1, 2.0943951024f);`
    + ส่งคำสั่งไปยังเซ็นเซอร์ให้กำหนดค่า `Zero Angle` เป็น `2.0943951024 `เรเดียน (ประมาณ 120 องศา)
    + ค่า `Zero Angle` ให้เอาค่าจาก `mt6835_get_angle` ค่า วงกลม 1 หน่วยเรเดียน 0.0 - 6.0 
  
#### 2.หน่วงเวลาเพื่อรอให้ค่าถูกบันทึก

+ HAL_Delay(1);
    + รอ 1 มิลลิวินาทีให้ค่าถูกอัปเดต

#### 3.อ่านค่ามุมศูนย์ที่ตั้งไว้

+ float zero_angle = mt6835_get_zero_angle(mt6835_1);
    + อ่านค่ามุมศูนย์ที่ถูกตั้งไว้จากเซ็นเซอร์

#### 4.แสดงค่ามุมศูนย์ออกทาง Serial Monitor

+ printf("Zero_angle: %f rad\r\n", zero_angle);
    + แสดงค่ามุมศูนย์ที่อ่านได้ออกมา

#### 5.บันทึกค่าลง EEPROM ของเซ็นเซอร์

+ bool respond = mt6835_write_eeprom(mt6835);
    + ส่งคำสั่งให้บันทึกค่ามุมศูนย์ลง EEPROM เพื่อให้ค่าคงอยู่แม้ปิดเครื่อง

#### 6.ตรวจสอบว่าการเขียน EEPROM สำเร็จหรือไม่

+ ถ้าสำเร็จ → printf("write eeprom success\r\n");
+ ถ้าล้มเหลว → printf("write eeprom failed\r\n");

#### 7.รอ 6 วินาทีก่อนปิดไฟเลี้ยง

+ HAL_Delay(6000);
    + ต้องรออย่างน้อย 6 วินาทีเพื่อให้ EEPROM เขียนค่าเสร็จสมบูรณ์

#### 💡 สรุป
ฟังก์ชัน set_zero_angle มีหน้าที่กำหนดค่ามุมศูนย์ให้กับเซ็นเซอร์ MT6835 และบันทึกค่าลง EEPROM เพื่อให้ค่าคงอยู่แม้ปิดเครื่อง