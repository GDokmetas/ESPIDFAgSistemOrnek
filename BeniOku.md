# ESP32 Network Örnekleri 

Bu depoda ESP-IDF ile yapılmış (VSCode Ext Kullanılarak) Network ile ilgili basit ve açıklamalı örnekler yer almaktadır. 

Örneklerin tamamı ESP-IDF kullanılarak harici kütüphane kullanılmadan VSCode eklentisi ile gerçekleştirilmiştir ve ESP-Prog debugger uyumludur. 

## Örnekler

- Wifi Ağına Bağlanma
- Wifi Ağından kopunca tarama moduna geçme (periyodik)
- Çoklu ağ bilgilerini kaydetme ve tarama modunda bu ağlar arasında geçiş yapma
- Ping atma
- Wifi bağlantı durumunu güncelleyebilme ve state, event özellikleri (durum geçişlerinde)
- AP/STA modları arasında geçiş yapabilme
- AP modunda bağlanan istemciye bir sayfa/bilgi gönderme (sunucu özelliği)
- AP modunda istemciye gönderdiği formdaki bilgileri okuma 
- AP modundayken kullanıcıya captive portal açma
- Kalıcı hafızaya parametreleri kaydetme (NVS)
- Networkle ilgili (IP, MAC vb.) bilgileri izleme
- Wifi'dan RSSI bilgisini almalıdır
- DNS özelliği ile captive portal isim ile açılabilecek (IP şart değil)

## Seçimlik Özellikler

- Değişken boyutlu verileri kalıcı hafızada saklamak için (örn çoklu ağ parametreleri) SPIFFS kullanılabilir ve RAM'de bunlar dinamik olarak saklanabilir. 


## Kurallar

- Wifi örnekleri modüler olmalıdır.
- Wifi örnekleri daha sonra bir kütüphaneye aktarılacağı için belli kurallara uygun yazılmalıdır.
- Wifi örneklerini yazarken driver yazımına uygun bir usül izlenmelidir.
- Örneklerde kullanmak üzere 2 düğme ve 3 adet LED için wrapper yazılacaktır.
- Sistemle ilgili olan uygulamaların başında _SYS_ ağ ile ilgili olan uygulamaların başında _NW_ ön eki bulunmalıdır.
- Örnekler 1, 2, 3, 4 diye sıra ile gitmelidir. 