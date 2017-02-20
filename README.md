# switcher-prototype

##0. 시행착오

* 초기 계획
  1. UART를 동작시켜, 디버그 메시지 확인
  2. 버튼(HW)를 눌려 모터 동작
  3. 배터리(ADC), 시간동기 기능 구현
  4. UART를 통해 각 기능 제어
  5. BLE을 이용해 각 기능 제어
  6. 예약기능 동작
  7. OTA
  8. 저전력 방안 바면

* 시행착오(2/18)
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

현재 4번까지 구현완료 했으며, 5~7은 이전에 작성한 소스를 약간만 수정하면 금방 할 수 있을 것으로 예샹된다.
8번 기능도, BLE 서비스만 추가하면 되기 때문에 큰 문제가 없을 것으 예상된다.

이런 시행착오를 들은 전 펌웨어 개발자는 매우 즐거워했다는.....

##1. 진행사항

### 17%(2/12) 완료 중

|번호|요구사항|
|---|---|
|1|버튼 누르면 모터 동작|
|2|BLE 통해 모터 동작|
|3|~~BLE 통해 배터리 잔량 읽기~~|
|4|~~BLE 통해 시간 동기 및 보드 시간 읽기~~|
|5|BLE 통해 예약정보 10개 저장, 수정 및 앱을 통해 해당정보 확인|
|6|예약기능 동작|
|7|OTA|
|8|PC를 통해 모터 동작|
|9|PC를 통해 베터리 잔량 읽기|
|10|PC를 통해 보드의 실제 시간 확인|
|11|PC를 통해 예약정보 확인|
|12|저전력 방안 마련|

##2. 소스 파일 설명
* battery.h : ADC를 이용하여 배터리 잔량을 확인한다.
* * Battery Level을 얻어오는 동작은 비동기로 동작하니 사용에 주의해야 한다.

![Alt text](https://www.websequencediagrams.com/cgi-bin/cdraw?lz=dGl0bGUgYmF0dGVyeV9sZXZlbF9nZXQoYXN5bmMpCgpNYWluLT4rQgAbBjoAGBIKABQHLT4rQURDKEhXKTogbnJmX2Rydl9hZGNfYnVmZmVyX2NvbnZlcnQKAB0HLS0-LQBMCAoAIyBzYW1wbGUKbm90ZSByaWdodCBvZiAAZQhBREMgU1RBUlQAPQotPi1NYWluOiAKABkbRklOSVNIAIEHCQCBBQogQURDIERvbmUgQ2FsbGJhY2sAgRgJIC0tPiAAVQYAggsGIExldmVsAB8K&s=napkin)

* now.h : 시간 동기를 하며, 하드웨어자원을 이용하여 보드에 저장된 시간을 흐르게 한다.
* * now는 RTC 하드웨어를 이용하여, 매 초마다 이벤트를 받아서 시간을 증가시킨다.

![Alt text](https://www.websequencediagrams.com/cgi-bin/cdraw?lz=dGl0bGUgIFRpbWUgSW5jcmVhc2luZwoKTWFpbi0-K05vdzogbm93X2luaXQKCk5vdy0-K1JUQyhIVyk6ADEFciBTdGFydAoADgctLT4tADAFACcGLT4tTWFpbjoKCgpsb29wIEV2ZXJ5IGEgc2Vjb25kCiAgICAKICAgIAA2CD4AbAUAUgZFdmVudAAeBQByBQCBBgUAgRwHZSBDdXJyZW50AIE0BQBBBmVuZAoKCg&s=napkin)
* io_button.h : 타켓보드에 연결된 버튼의 눌러짐을 확인한다.
* io_motor.h : PWM을 이용하여 모터를 구동한다.
* io_uart.h : UART을 통해 들어온 문자를 받고, UART를 통해 문자열을 출력한다.
* uart_queue.h : UART로 들어온 문자열을 Circular Queue에 저장한다.
* uart_service.h : UART로 수신하는 명령어들을 관리하고, 해당 명령어를 수신하면 명령어 수행을 위한 콜백을 호출한다.

##3. PC가 전송하는 UART 명령어 정의

|명령어|설명|
|---|---|
|BAT|배터리 잔량을 출력한다.|
|NOW|보드에 저장된 시간을 출력한다.|
|SCH|예약정보를 출력한다.|
|SWT|모터를 동작시킨다.|

##4. 배터리 잔량 계산

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

##5. Trouble Shooting
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
- UART로 printf 출력이 안 나올경우
    - 해결 : sdk_config.h에 아래 코드 추가
    - ```
        #ifndef RETARGET_ENABLE
        #definen RETARGET_ENABLE 1
        #endif
```
- APP_BUTTON에서 Button Event가 발생 안함
    - 원인 : SOFTDEVICE에 CLOCK 설정을 하지 않아서 Timer Tick이 동작하지 않았다.
    - 해결 :  아래코드 호출하도록 수정
    - ```
nrf_clock_lf_cfg_t clock_lf_cfg = NRF_CLOCK_LFCLKSRC;
SOFTDEVICE_HANDLER_INIT(&clock_lf_cfg, NULL);
```
    - 원인 : sdk_config.h에 Timer가 비활성화 되어 있었다.
- nrf_log_XXX 출력
    - 해결 : sdk_config.h에 아래 코드 추가
    - ```
        #ifndef NRF_LOG_ENABLE
        #definen NRF_LOG_ENABLE 1
        #endif
```

##Appendix
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

###B. ARMGCC 빌드 
* Makefile 위치 : pca10028/s130/armgcc/Makefile

###C. 참고사이트
칩 데이터시트 : https://lancaster-university.github.io/microbit-docs/resources/datasheets/nRF51822.pdf
S130 Softdevice Specification : http://infocenter.nordicsemi.com/pdf/S130_SDS_v2.0.pdf

