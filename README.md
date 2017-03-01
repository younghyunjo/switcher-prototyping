# switcher-prototype

## 0. 시행착오
### 3/2
  * 문제점 : Ubuntu에 Github markdown format을 지원하는 에디터가 없음
    * 적절한 markdown editor가 없어 개인 PC에서 잘 보이던 Readme.md파일이 github에 올라가면 깨져보인다.
  * 해결책 : https://jbt.github.io/markdown-editor에서 편집
### 3/1
  * 문제점 : DK 보드 부팅안됨
    * 발생 원인
      * Bootloader를 Flash 중 보드가 반응이 없어 전원을 재인가하니깐, 보드가 PC와 연결이 안된다.
      * 타겟보드에서 Bootloader를 Loading하지 못해서 보드 부팅이 안되는 것으로 추측이 돤다.
      * Bootloader loading을 못하는 이유는 Flash Memory에 Bootloader가 비정쌍적으로 Flash 되었거나, 일부만 Flash 된 것으로 추측이 돤다.
  * 증상
  	* DK보드를 PC에 연결하면, 장치관리자에 COM PORT로 인식이 안됨.
  	* 보드의 "BOOT/RESET" 버튼을 누른 후 부팅을 해도 BOOTLOADER 모드로 진입이 안됨.
  * 해결책 : 보드 교체 또는 하드웨어 디버거 장비로 Bootloader reflash.

### 2/28
  * 내용 : Flash Memory의 내용을 삭제가 안되는 버그는 SDK의 버그가 아니고, FILE_ID의 값을 0으로 사용할 경우 발생한다.
    * Flash에 데이터를 저장할 때 FILE_ID와 RECORD_ID를 사용한다.
    * FILE_ID를 0으로 사용하면 Data의 삭제 또는 수정이 안되는 경우가 발생한다.
    * FILE_ID를 0에서 0x5555로 변경하니 해당 기능 동작이 잘 된다.

###  ~~2/26~~
* ~~SDK 버그 발견~~
  * ~~버그 내용 : Flash Memory의 내용을 삭제해도, 삭제가 안되는 경우가 발생한다.~~
  * ~~버그 API : fds_record_delete()~~
  * ~~배경~~
    * ~~예약 스켸줄을 저정하기 위해선 칩의 Flash Memory를 사용해야 한다.~~
    * ~~Flash Memory를 편하게 사용하기 Flash Data Storage(fds) 라이브러리를 사용하였다.~~
  * 문제점
    * ~~BLE를 통해서 삭제 명령을 받았을 경우, fds_record_delete() API를 호출해서 Flash에 저장된 예약 스케쥴을 삭제한다.~~
    * ~~**fds_record_delete를 호출해도, 해당 데이터가 삭제가 안되는 경우가 자주 발생하고 있다.**~~
  * ~~해결책~~
    * ~~SDK 내부 버그로 추청이 되며, SDK 버전 업데이트 또는, 제조사에게 수정 요청을 해야한다.~~

  ~~** KNOWN BUG로 처리하였습니다.**~~

### 2/18
  * 초기 계획
    1. UART를 동작시켜, 디버그 메시지 확인
    2. 버튼(HW)를 눌려 모터 동작
    3. 배터리(ADC), 시간동기 기능 구현
    4. UART를 통해 각 기능 제어
    5. BLE을 이용해 각 기능 제어
    6. 예약기능 동작
    7. OTA
    8. 저전력 방안 바면

  * 배경 : 시간 증가의 정확도 향상을 위해 RTC 하드웨어 자원을 시간 관리 모듈(NOW)만 사용하도록 설계함
    * 타겟 보드의 시간을 증가시키기 위해, RTC에서 매 초마다 이벤트를 발생시켜 현재 시간을 증가하도록 설계했다.
    * 설계 도중, RTC 시간 이벤트를 다른 모듈과 공유했을 때, 다른 모듈에서 이벤트를 길게 잡고 있으면, NOW 모듈이 매 초마다 이벤트를 못 받는 경우가 발생할 수 있다.
    * 즉, RTC 이벤트를 여러 모듈과 공유하면, 타겟 보드의 시간이 느리게 흐를 수 있다.
    * 따라서, RTC 이벤트를 NOW 모듈만 독점적으로 쓰게 하고, RTC 이벤트를 사용하는 APP_TIMER LIBRARY, BUTTON LIBRARY는 사용을 안하고, 기능 구현을 하였다.(초기 계획의 1~4까지 구현 완료)
  * 문제점 : BLE를 사용하기 위해선 무조건 APP_TIMER LIBRARY를 사용해야 함.
    * BLE연결을 위해선 BLE LIBRARY를 사용해햐하며, BLE LIBRARY는 APP TIMER를 사용하고 있었다.
    * 즉, BLE를 사용하기 위해선 APP TIMER를 사용해야하며, RTC 이벤트를 NOW 모듈만 독점적으로 사용할 수 없다.
  * 수정해야 될 사항
    1. BLE 연결을 먼저 구현한다.
    2. RTC 이벤트의 Handler는 하나만 등록할 수 있다. 따라서, NOW 모듈에서 등록한 이벤트 핸들러는 삭제하고, APP_TIMER에 기 구현된 이벤트 핸들러를 사용한다.
    2. RTC는 APP_TIMER에 이벤트 핸들러를 등록해서 사용한다.
    3. 버튼 눌림을 확인하기 위해 SIMPLE_TIMER LIBRARY를 이용해 직접 구현하였는데, SDK에서 제공하는 BUTTON_LIBRARY를 이용해 구현한다.

* 변경된 계획
  1. BLE 연결
  2. BLE를 통해 배터리 잔량 읽기
  3. NOW모듈을 RTC를 독점하는 구조에서, APP TIMER를 써서 시간을 증가시키도록 수정
  4. BLE를 통해 시간 동기
  5. 버튼 모듈 수정
  6. 모터 제어
  6. HW SWITCH를 눌러 버튼 동작
  5. BLE를 통해 모터제어
  7. 예약기능 구현
  8. UART를 통해 각 기능 제어
  9. OTA
  10. 저전력 방안 마련

이런 시행착오를 들은 전 펌웨어 개발자는 매우 즐거워했다는.....

## 1. 진행사항

### 10/12 완료

|번호|요구사항|
|---|---|
|1|~~버튼 누르면 모터 동작~~|
|2|~~BLE 통해 모터 동작~~|
|3|~~BLE 통해 배터리 잔량 읽기~~|
|4|~~BLE 통해 시간 동기 및 보드 시간 읽기~~|
|5|~~BLE 통해 예약정보 10개 저장, 수정 및 앱을 통해 해당정보 확인~~|
|6|~~예약기능 동작~~|
|7|OTA|
|8|~~PC를 통해 모터 동작~~|
|9|~~PC를 통해 베터리 잔량 읽기~~|
|10|~~PC를 통해 보드의 실제 시간 확인~~|
|11|~~PC를 통해 예약정보 확인~~|
|12|저전력 방안 마련|

## 2. Architecture
![Alt text](https://docs.google.com/drawings/d/1HWsrJ0OCyh-hFPSqX2w1N-bl5Fi8vEi3wBuQTSjsZSE/pub?w=946&h=194)
각 컴포넌트간에 Decoupling을 최소화하기 위하여 Callback함수를 적극적으로 이용하여 설계하였다.
예를 들어 battery_service에서 배터리 레벨을 얻기 위해 battery.h에 있는 API를 직접 호출하지 않고, 베터리 레벨을 얻어도는 함수 포인터를 받아서 해당 함수를 호출하는 방법으로 설계했다.
motor_service, current_time_service도 이와 비슷하게 설계하였다.

## 3. 주요 컴포넌트 설명
* battery.h : ADC를 이용하여 배터리 잔량을 확인한다.
  * Battery Level을 얻어오는 동작은 비동기로 동작하니 사용에 주의해야 한다.
![Alt text](https://www.websequencediagrams.com/cgi-bin/cdraw?lz=dGl0bGUgYmF0dGVyeV9sZXZlbF9nZXQoYXN5bmMpCgpNYWluLT4rQgAbBjoAGBIKABQHLT4rQURDKEhXKTogbnJmX2Rydl9hZGNfYnVmZmVyX2NvbnZlcnQKAB0HLS0-LQBMCAoAIyBzYW1wbGUKbm90ZSByaWdodCBvZiAAZQhBREMgU1RBUlQAPQotPi1NYWluOiAKABkbRklOSVNIAIEHCQCBBQogQURDIERvbmUgQ2FsbGJhY2sAgRgJIC0tPiAAVQYAggsGIExldmVsAB8K&s=napkin)
* now.h : 시간 동기를 하며, 하드웨어자원을 이용하여 보드에 저장된 시간을 흐르게 한다.
  * now는 RTC 하드웨어를 이용하여, 매 초마다 이벤트를 받아서 시간을 증가시킨다.
  * 모든 시간은 Unix Time(Epoch), UTC + 0 이다.</br>
![Alt text](https://www.websequencediagrams.com/cgi-bin/cdraw?lz=dGl0bGUgIFRpbWUgSW5jcmVhc2luZwoKTWFpbi0-K05vdzogbm93X2luaXQKCk5vdy0-K1JUQyhIVyk6ADEFciBTdGFydAoADgctLT4tADAFACcGLT4tTWFpbjoKCgpsb29wIEV2ZXJ5IGEgc2Vjb25kCiAgICAKICAgIAA2CD4AbAUAUgZFdmVudAAeBQByBQCBBgUAgRwHZSBDdXJyZW50AIE0BQBBBmVuZAoKCg&s=napkin)
* motor.h : PWM을 이용하여 모터를 구동한다.
  * motor_move() 함수는 **Blocking**으로 동작한다.
  * 따라서, 인터럽트 내에서 호출 시 ISR(Interrupt Service Routine)을 빠져나가는 시간이 길어, 우선순위가 낮은 인터럽트가 밀린다.
  * 나은 성능을 위해서는 timer를 써서 Non-Blocking으로 동작하게 수정을 해야 한다.
![Alt text](https://www.websequencediagrams.com/cgi-bin/cdraw?lz=dGl0bGUgTW90b3IgTW92ZQoKTWFpbiAtPiArABAFOiBtb3Rvcl9tABYGACMFLT4gR1BJTyhUUikgOiBncGlvIHNldAATC1BXTShIVykgOiBQV00gOgBUBSArOTAANgoAVQdXYWl0IHVudGlsAGEGIHN0b3AAKR8tAB8pAIEVGmNsZWFyAIFIBy0tPiAtTWFpbjoKCg&s=napkin)
* toggle_switch.h : 타켓보드에 연결된 버튼의 눌러짐을 확인한다.
  * app_button library를 이용하여 구현했으며, 스위치가 눌러졌을 때 호출할 콜백을 등록하여, 이벤트를 받는다.
* UART 관련 모듈
  * uart_queue.h : UART로 들어온 명령어를 Circular Queue에 저장한다.
    * UART를 통해서 받는 한개 명령어는 4Byte(Null 포함)으로 고정하고, 각 명령어를 Queuing하고 있다.
![Alt text](https://docs.google.com/drawings/d/1LiOG0qBIr-ZDl0M1lI3EmqdE67v0CMmAC8M0H53JQA8/pub?w=339&h=552)
  * io_uart.h : UART을 통해 들어온 문자를 받고, UART를 통해 문자열을 출력한다.
    * UART(HW)에서 문자를 수신 이벤트가 발생하면, 해당 이벤트를 처리한다.
    * UART(HW)로 문자열을 전송한다.<br>
![Alt text](https://www.websequencediagrams.com/cgi-bin/cdraw?lz=VGl0bGU6IFJlY2VpdmUgQ01EIGZyb20gUEMKClBDIC0-IFVBUlQoSFcpOiBDSEFSIHNlbmQKAAwIIC0-IGlvX3VhcnQAGAdyAEMGZAoKCmFsdAAtBmlzIE5PVCBORVdMSU5FIAogICAALgggLT4ANwlidWZmZXJpbmcAZAUKZWxzZQogIGFsdAATB2VkAH0FcyBhcmUgNABBBQA8DiB1YXJ0X3F1ZXVlIDogZW4ABQUAaQUARQcAJA0AbwtDbGVhcgBXDwAyBm5kCmVuZAoKCg&s=napkin)
  * uart_service.h
    * UART로 수신해야 할 명령어와 호출해야 할 함수 포인터를 가지고 있다.
    * PC에서 해당 명령어를 수신하면 명령어 수행을 위한 콜백을 호출한다.
![Alt text](https://www.websequencediagrams.com/cgi-bin/cdraw?lz=dGl0bGUgVUFSVAoKCk1haW4tPiArdWFydF9zZXJ2aWNlOiAAAgxfZG8KABIMLT4gK2lvX3VhcnQ6IAACB19jbWRfZ2V0CgASBwBJCXF1ZXVlOiBkZQAEBQBCBgAPBSAtLT4tAD4JY21kADIJLT4tAIB_DmNtZABzDi0-LU1haW46IGNhbGxiYWNrAAQFCgoKCgo&s=napkin)

## 4. BLE Unknown Service Spec
### 4.1 모터 구동
- Service UUID : 0000f000-0000-1000-8000-008050f9b34fb
- Characteristic UUID : 0000f000-0000-1000-8000-008050f9b34fb
  - Write
    - Type : unsigned char
    - Value : 1

### 4.2 시간동기
- Service UUID : 0x1805
- Characteristic UUID : 0x2A2B
  - Write
    - Type : uint8_t time[4]
    - Value : Unix Time(Epoch Time), UTC + 0
  - Read
    - Type : uint8_t time[4]
    - Value : Board Time
    - Format : Unix Time(Epoch Time), UTC + 0

### 4.3 예약 스케쥴
- 기본 규칙
  - **day값은 'OR' 연산을 해서 중복으로 표시할 수 있다.**
  - 모든 시간은 UTC+0를 따른다.
- Service UUID : 0000fa01-0000-1000-8000-00805f9b34fb
   - Characteristic  UUID : 0000fa01-0000-1000-8000-00805f9b34fb
    - 예약 List 보기
    - Read
```
struct schedule{
    uint8_t id;   //0xff, if empty schedule
    uint8_t day;  //0x1:Sun, 0x2:Mon, 0x4:Tue, 0x8:Wen, 0x10:Thu, 0x20:Fri, 0x40:Sat
    uint8_t hour; //0~23
    uint8_t minute; //0~59
}
struct schedule schedules[10];  // 예약 리스트
```
  
  - Characteristic UUID : 0000fa02-0000-1000-8000-00805f9b34fb
    - 예약 기능 추가, 수정, 삭제
    - Write
    - 추가
```
struct schedule{
    uint8_t id;   //Always 0xff
    uint8_t day;  //0x1:Sun, 0x2:Mon, 0x4:Tue, 0x8:Wen, 0x10:Thu, 0x20:Fri, 0x40:Sat
    uint8_t hour; //0~23
    uint8_t minute; //0~59
};
```
-   - 수정
```
struct schedule{
    uint8_t id;   //Updating schedule ID
    uint8_t day;  //0x1:Sun, 0x2:Mon, 0x4:Tue, 0x8:Wen, 0x10:Thu, 0x20:Fri, 0x40:Sat
    uint8_t hour; //0~23
    uint8_t minute; //0~59
};
```
-   - 삭제
```
struct schedule{
    uint8_t id;   //Deleting schedule ID
    uint8_t day;  //Always 0xff
    uint8_t hour; //Always 0xff
    uint8_t minute; //Always 0xff
};
```

## 5. PC가 전송하는 UART 명령어 정의
|명령어|설명|
|---|---|
|BAT|배터리 잔량을 출력한다.|
|NOW|보드에 저장된 시간을 출력한다.|
|SCH|예약정보를 출력한다.|
|SWT|모터를 동작시킨다.|

## 6. 배터리 잔량 계산
##### 배터리 최대, 최소 전압 정의
* 최대전압 : 4.09V
* 최소전압 : 3.41V

##### 이론적인 배터리 레벨 계산
* MAX_ADC_VALUE = 4.09 / 2 * 1024 / 3.6
* MIN_ADC_VALUE = 3.41 / 2 * 1024 / 3.6
* LEVEL = (ADC_VALUE - MIN_ADC_VALUE) / MAX_ADC_VALUE * 100

##### 코드상 베터리 레벨 계산
* MAX_ADC_VALUE = 4.09 / 2 \* **1000** / 3.6
* MIN_ADC_VALUE = 3.41 / 2 \* **1000** / 3.6
* LEVEL = (ADC_VALUE - MIN_ADC_VALUE) / MAX_ADC_VALUE * 100

battery.h에 배터리 래벨의 최대, 최소 전압을 받도록 인터페이스를 정의했다.
배터리 전압은 소수점으로 표시되기 때문에 Floating Point 계산을 하지 않기 위해 최대, 최소 전압 x1000한 값을 받는다.
이미 x1000을 한 값을 받기 때문에, 계산상에 x1024한 값 대신에 x1000한 값을 그대로 쓰도록 구현하였다.


## 7. Trouble Shooting
- nRF51 보드들 PC와 연결했을 때 USB 인식이 안됨
    - 원인 : 부트로더를 잘못 구워서 인식이 안됨
    - 해결 : 부트로더 Reflashing
        - 부트로더 다운로드
            - https://www.nordicsemi.com/eng/nordic/Products/nRF52-DK/nRF5x-OB-JLink-IF/52275
        - 부트로더 모드 진입
            - nRF51 보드의 전원을 끈다.
            - BOOT/RESET 버튼을 계속 누른다
            - nRF51보드 전원을 킨다.
            - BOOT/RESET 버튼을 땐다.
            - "BOOTLOADER" USB 디바이스 생성 된다.
        - 부트로더 Flashing
            - 다운로드 받은 부트로더를 "BOOTLOADER" USB 디바이스로 복사(DRAG & DROP)
            - 정상적으로 되면 LD6 LED가 깜빡인다.

-  UART로 printf 출력이 안 나올경우
    - 해결 : sdk_config.h에 아래 코드 추가
```
#ifndef RETARGET_ENABLE
#definen RETARGET_ENABLE 1
#endif
```

- APP_BUTTON에서 Button Event가 발생 안함
    - 원인 : SOFTDEVICE에 CLOCK 설정을 하지 않아서 Timer Tick이 동작하지 않았다.
    - 해결 :  아래코드 호출하도록 수정
```
nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;
SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);
```

- nrf_log_XXX 출력
    - 해결 : sdk_config.h에 아래 코드 추가
```
#ifndef NRF_LOG_ENABLE
#definen NRF_LOG_ENABLE 1
#endif
```

- bootloader Build 시 uEEC.h 해더 파일이 없어서 에러 발생
  - 원인 : external/ 에 있는 라이브러리들은 수동으로 설치를 해야 한다.
  - 해결 : http://infocenter.nordicsemi.com/index.jsp?topic=%2Fcom.nordic.infocenter.sdk5.v12.0.0%2Flib_crypto.html&cp=4_0_0_3_3_1_2&anchor=lib_crypto_installing

## Appendix
### A. BLE 용어설명
#### GAP(Generic Access Profile)
* Advertising, Connection 제어
* Device가 다른 장치에 어떻게 보여질껀지, 어떻게 연결할 것인가를 정의한다.

##### Advertising Mode
* Broadcaster : Trasmit advertising packet to others.
* Observer : Listen advertising packet.

##### Connection Mode
* Perioperial : advertises to central devices. After connecting, no longer broadcast data and stay connted to the device that accepted connection request.
* Central : initiates a connection with a peripheral device by first listening to the advertising packets.

#### GATT(Generic Attribute profile)
* client : Requesting read/write attributes to the GATT server.
* server : Store attribues. the servier must response the attribute request from client.

### B. ARMGCC 빌드
* toolchain 설정 : components/toolchain/gcc/Makefile.posix
* Makefile 위치 : pca10028/s130/armgcc/Makefile
* Build 명령어
```
make
make flash
make flash_softdevice
```

### C. 참고사이트
칩 데이터시트 : https://lancaster-university.github.io/microbit-docs/resources/datasheets/nRF51822.pdf<br>
S130 Softdevice Specification : http://infocenter.nordicsemi.com/pdf/S130_SDS_v2.0.pdf
