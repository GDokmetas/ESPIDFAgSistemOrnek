# API Guide Maddeler Listesi

- [ ] Application Level Tracing Library
- [ ] Application Startup Flow
- [ ] BluFi
- [ ] Bluetooth Overview
- [ ] Bootloader
- [ ] Build System
- [ ] RF Coexistence
- [ ] Core Dump
- [ ] C++ Support
- [ ] Current Consumption Measurement
- [ ] Deep Sleep Wake Stubs
- [ ] Error Handling
- [ ] ESP-BLE-MESH
- [ ] ESP-WIFI-MESH
- [ ] External RAM Support
- [ ] Fatal Errors
- [ ] Hardware Abstraction
- [ ] JTAG Debugging
- [ ] Linked Script Generation
- [ ] LwIP
- [ ] Memory Types
- [ ] OpenThread
- [ ] Partition Tables
- [ ] Performance
- [ ] Reproducible Builds
- [ ] RF Calibration
- [ ] Thread Local Storage
- [ ] Tools
- [ ] Unit Testing in ESP32
- [ ] Running ESP-IDF Apps on Host
- [ ] Wi-Fi Driver
- [ ] Wi-Fi Security



## Application Level Tracing Library

Link: [Application Level Tracing Library](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/tracing.html)

### Application Level Tracing Açıklama



## Application Startup Flow

Link: [Application Startup Flow](https://docs.espressif.com/projects/esp-idf/en/latest/api-guides/application-startup-flow.html)

### Application Startup Flow Açıklama

ESP-IDF'de app_main fonksiyonu ana fonksiyon olarak (main) ilk çalıştırılan fonksiyondur. Bu fonksiyon çalıştırılmadan önce gerçekleşen işlemler şu şekildedir. 

1. İlk aşama ön yükleyici çalıştırılır. Bu ön yükleyici ROM bellek içerisinde yer almakta ve ikinci aşama ön yükleyiciyi RAM belleğe (DRAM/IRAM) yüklemek için kullanılmaktadır. İkinci aşama ön yükleyici Flash bellekte 0x1000'dan itibaren yer almaktadır.
2. İkinci aşama ön yükleyici bölüntü çizelgesini ve ana imajı flash bellekten yüklemektedir.
3. Uygulama başlatımı gerçekleştirilmektedir. Bu noktada ikinci işlemci çekirdeği ve RTOS görev planlayıcısı başlamaktadır.



