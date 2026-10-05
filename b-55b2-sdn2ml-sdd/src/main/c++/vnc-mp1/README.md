# VNC MP1: анализ и прототип SDN processing node

Имя **MP1** принято по уточнению пользователя: приложение `vnc-mp1`,
topic `vnc-mp1-topic`, XML `vnc-mp1-topic.xml`. MP2-файл не создаётся.
Это самостоятельный C++11 процесс внутри I&C subsystem, а не новый IOC-драйвер.
SDN transport использует предоставленный API CODAC 7.4.0. На CODAC этот прототип
ещё необходимо собрать и проверить в сети; сборка и end-to-end запуск здесь не подтверждены.

## A. Что найдено в существующем коде

Основной проект — `b-55b2-sdn2ml-sdd`, остальные каталоги workspace — референсы.
Структура до добавления приложения:

```text
b-55b2-sdn2ml-sdd/
  pom.xml                       CODAC subsystem, project.type=iandc, parent 7.4.0
  sdd.xml.gz                    исходная SDD-модель
  .ioc_at_hostname.mapping       размещение IOC
  src/main/
    c++/Makefile                рекурсивная сборка */Makefile
    c++/nds-daq-generator/      NDS3 симулятор восьми каналов
    c++/nds-sdn/                библиотека NDS3 SDN publisher
    epics/D1-I2App/             IOC executable и базы PV
    epics/iocBoot/              CORE/SYSM IOC, в том числе PCF0..PCF6
    resources/                 шесть SDN topic XML и шесть NDS mapping XML
    boy/                       операторские панели
    beast/                     alarm configuration
    beauty/                    archive configuration
```

Отдельного C++ processing executable до изменения не найдено. CMake отсутствовал.
[pom.xml](../../../../pom.xml) использует `maven-iter-plugin`, packaging `codac`;
его packaging перечисляет IOC/OPI/archive/alarm. Реализация родительского Maven
plugin в workspace отсутствует, поэтому автоматическое включение нового бинарника
в CODAC RPM из этих файлов установить нельзя.

Точка входа IOC — [D1-I2Main.cpp](../../epics/D1-I2App/src/D1-I2Main.cpp):
`iocsh(argv[1])`, затем интерактивный IOC shell. Файл
[st.cmd](../../epics/iocBoot/iocD1-I2-PCF0CORE/st.cmd) загружает пользовательские
конфигурации, регистрирует устройства, выполняет `iocInit`, включает `iocLogInit`.

Реальные источники — **симуляторы**, не аппаратный neutron acquisition driver:
[NdsDaqGenerator.cpp](../nds-daq-generator/src/NdsDaqGenerator.cpp),
`NdsDaqGenerator::simulationThreadBody()`. Раз в 100 ms при `running` он последовательно
вызывает `m_countRatePVs[i].push(ts, value)` для восьми `PVVariableIn<double>`.
Значение: `BaseRate + i*250 + 50*sin(tick*0.1+i) + tick%17`.

| IOC | NDS generator | Topic | Payload |
|---|---|---|---|
| PCF0CORE | G-0 | pcf0-nds-sdn-topic | CountRate00..07: double |
| PCF1CORE | G-1 | pcf1-nds-sdn-topic | CountRate00..07: double |
| PCF2CORE | G-2 | pcf2-nds-sdn-topic | CountRate00..07: double |
| PCF3CORE | G-3 | pcf3-nds-sdn-topic | CountRate00..07: double |
| PCF4CORE | G-4 | pcf4-nds-sdn-topic | CountRate00..07: double |
| PCF5CORE | G-5 | pcf5-nds-sdn-topic | CountRate00..07: double |

PCF6 существует в IOC-структуре, но к шести входным publisher здесь не относится.
Каждый payload содержит 64 bytes, единицы `cps`. В payload нет measurement timestamp,
sequence или quality. XML ranks 1..8 — это не номера IOC.

Фактическая цепочка публикации:

* [userPreDriverConf.cmd](../../epics/iocBoot/iocD1-I2-PCF0CORE/userPreDriverConf.cmd)
  загружает `libnds3-sdn-publisher.so`, создаёт `DeviceNDSSDNPUB` с `INTERFACE=lo`.
* [pcf0-nds-sdn-conf.xml](../../resources/pcf0-nds-sdn-conf.xml) сопоставляет
  `G-0-CountRate00` с `CountRate00`, и аналогично остальные каналы.
  Для **каждого** канала стоит `trigger_pub=yes`.
* [DeviceNDSSDNPub.cpp](../nds-sdn/DeviceNDSSDNPub.cpp): конструктор читает mapping,
  получает имя topic из XML, вызывает `TopicHelper::LocateDefinitionFile()` и
  `LoadDefinitionFile()`. Указанный в IOC `TOPICNAME` не определяет topic в этом коде:
  фактически используется имя из mapping XML.
* `stm_SwitchOn()` создаёт `sdn::Publisher(m_topic)`, вызывает `SetInterface()` и
  `Configure()`. `PV_doubleAttribute_writer()` находит атрибут через introspection,
  выполняет `m_topic.SetAttribute<double>()`, затем `m_pub->Publish()`.
* Применяется NDS state machine `off → on → running`; ошибки конфигурации —
  `logic_error`/`nds::StateMachineError`, сообщения — `ndsDebugStream`,
  `ndsWarningStream`, `ndsErrorStream`. Некоторые triggered Publish errors в исходнике
  не сопровождаются отдельным сообщением об ошибке.

Таким образом, здесь используется **SDN core API**, а не raw UDP. В референсе
`SDN_VNC_Publisher_Subscriber` другой интерфейс: `initializeSDN()`,
`SdnSubscriber<metadata, datatype>`, ручной datatype header, `terminateSDN()`.
Новый код следует API IOC: `sdn::Subscriber`/`sdn::Publisher` + XML-loaded `sdn::Topic`.
Отдельных `initializeSDN()`/`terminateSDN()` для этого пути существующий IOC не вызывает.

Предоставленные заголовки в workspace (от корня workspace-репозитория):

| Файл | Что подтверждает |
|---|---|
| `sdn-core/Subscriber.h` | `Subscriber(Topic&)`, `GetSocket`, `Receive(uint64_t timeout)`, `IsPayloadOrdered` |
| `sdn-core/Publisher.h` | `Publisher(Topic&)`, `SetInterface`, `Configure`, `Publish`; native payload byte order |
| `sdn-core/any-type-ext.h` | проверка имени/типа/размера/смещения атрибутов |
| `sdn-core/sdn-topic.h` | named `SetAttribute`, multicast metadata |
| `sdn-core/sdn-topic-xml.h` | XML parser, `LocateDefinitionFile`, `LoadDefinitionFile` |
| `sdn-core/sdn-header.h` | `Header_t.topic_counter`, `send_time`, `recv_time` |

SDN header counter — счётчик пакетов конкретного publisher, не общий номер измерения.
`send_time` — время передачи; timestamp NDS PV в `PV_doubleAttribute_writer()` игнорируется.
При восьми triggers в одном цикле передаются промежуточные состояния массива.
Сумма шести последних пакетов не гарантирует ни согласованность восьми каналов одного IOC,
ни одновременность 48 измерений.

В шести XML нет явных multicast IP/port. SDN предоставляет
`TopicHelper::GenerateMCastAddress`, адреса выбираются библиотекой при конфигурации.
Численные значения в проекте не зафиксированы; новый процесс печатает фактически
полученные `GetMCastGroup()`/`GetMCastPort()` и размер каждого topic после Configure.

XML лежат в `src/main/resources`. Lookup поддерживает `${SDN_TOPIC_PATH}`,
`${CODAC_CONF}/sdn`, `${CODAC_CONF}/default` и относительные пути; для имени topic
добавляется `.xml` или `_typedef.xml`. Поэтому при lookup следует сохранять
`<topic-name>.xml`. `LoadDefinitionFile` принимает явный путь, независимо от имени
файла; прототип дополнительно проверяет совпадение ожидаемого имени и XML `name`.
Он всегда использует явно переданный каталог, чтобы избежать выбора старого XML
из системного каталога. C/C++ wire-структуры не генерируются: описание динамическое.

## B. Архитектура и объединение входов

```text
pcf0 topic ─> SdnInputReceiver[0] ─┐
pcf1 topic ─> SdnInputReceiver[1] ─┤
pcf2 topic ─> SdnInputReceiver[2] ─┤
pcf3 topic ─> SdnInputReceiver[3] ─┼─> LatestInputs → InputData[6][8]
pcf4 topic ─> SdnInputReceiver[4] ─┤                      │
pcf5 topic ─> SdnInputReceiver[5] ─┘                VncMp1Processor
                                                        │
                                                    OutputData
                                                        │
                                                 SdnOutputPublisher
                                                        │
                                                  vnc-mp1-topic
```

* `SdnInputReceiver` владеет одним `Topic` и одним `Subscriber`. XML определяет
  смещения; декодирование использует `memcpy`, без предположений о padding/alignment.
  Тип/размер проверяются при запуске, endian conversion — по `IsPayloadOrdered()`.
  Основание: IOC publisher и приведённые SDN core headers.
* `LatestInputs` хранит только последние восемь значений каждого IOC в `InputData`.
  `update()` заменяет один набор, `data()` возвращает const-ссылку на массив 6 × 8.
  Начальные значения — нули. Время приёма и флаги достоверности не хранятся.
* `VncMp1Processor` только суммирует входы, не знает о topic, сокетах и NDS.
  Его можно заменить ML/VNC алгоритмом, сохранив transport.
* `SdnOutputPublisher` владеет выходным `Topic` и `Publisher`, записывает атрибуты
  по именам тем же механизмом, что NDS publisher. `OutputData` не копируется как struct.
* `main` владеет всеми компонентами. Participant уничтожается раньше своего Topic,
  как требует lifetime contract `Subscriber.h`/`Publisher.h`; копирования объектов нет.

Один поток и `poll()` на шести `GetSocket()` исключают блокировку на неработающем IOC
и гонки при обновлении и чтении кэша. `Receive(0)` — документированный nonblocking режим.
За проход читается не более 32 пакетов каждого готового входа; выход публикуется
по расписанию. После задержки процесса нет burst-публикаций для наверстывания.
Это прототип для текущего симулятора (~80 packets/s/IOC), не доказанный hard real-time pipeline.

По умолчанию каждые 100 ms процессор суммирует текущие 48 значений прямо из кэша,
без промежуточной копии. До первого пакета вклад соответствующего IOC равен нулю.
После остановки IOC его последние значения сохраняются и продолжают участвовать
в сумме. Приложение не ожидает первого пакета от всех шести IOC.

Snapshot, контроль давности, маска актуальности и проверки значений на этой стадии
отсутствуют. Отрицательные числа, NaN и Inf участвуют в обычном сложении double.
Синхронизация и алгоритмы проверки будут добавлены отдельно.
API возвращает общий `STATUS_ERROR` для пустой очереди и ошибки Receive;
ошибки сокета `poll` фатальны.

Для следующего этапа следует передавать атомарный массив восьми каналов вместе с
measurement timestamp/frameId и quality, затем собирать окна по общей временной шкале.
Равенство независимых packet counters не обеспечивает синхронизацию.
IOC-код в этой работе не менялся.

Lifecycle прототипа: parse config → load/validate XML → configure 6 subscribers и
publisher → receive/process/publish → SIGINT/SIGTERM → RAII cleanup.
Ожидание poll ограничено 50 ms. Ошибка запуска или публикации даёт stderr и exit 1,
ошибка аргументов — exit 2, нормальная остановка — exit 0.
Логируются конфигурация topics, ошибки и завершение процесса.
Для эксплуатации можно обернуть configure/start/stop в NDS state machine по образцу
`DeviceNDSSDNPUB`; в текущем самостоятельном executable такой control-plane integration нет.

## C. Путь одного значения

`G-2-CountRate03.push(ts, value)` → mapping `pcf2-nds-sdn-conf.xml` →
`DeviceNDSSDNPUB::PV_doubleAttribute_writer()` → `SetAttribute<double>(rank, value)` →
`Publish()` всего `pcf2-nds-sdn-topic` → `SdnInputReceiver[2].receive()` → проверка
byte order, чтение double по XML offset → `cache.update(2, rates)` →
`VncMp1Processor::process(cache.data())` складывает 48 чисел →
`OutputData.totalCountRate` → named `SetAttribute` → `Publish()` в `vnc-mp1-topic`.
Исходный `ts` до SDN payload не доходит. Выходной timestamp — Unix/POSIX ns времени
агрегирования; он не заявлен как measurement time или CODAC/TAI timestamp.

## D. Добавленные файлы

```text
src/main/c++/vnc-mp1/
  main.cpp                  конфигурация, poll loop, lifecycle
  Processing.h/.cpp         модель, latest cache, алгоритм
  SdnTransport.h/.cpp       SDN receivers и publisher
  Makefile                  включается существующим ../Makefile
  CMakeLists.txt            самостоятельная альтернативная сборка
  tests/ProcessingTest.cpp  начальные нули, сумма, замена данных одного IOC
  README.md                 этот анализ и инструкция
  .gitignore                build artifacts
src/main/resources/vnc-mp1-topic.xml
```

Существующие файлы не изменялись; новые файлы находятся только внутри основного проекта.

## E. Минимальный C++ prototype

Полная реализация: [main.cpp](main.cpp), [SdnTransport.cpp](SdnTransport.cpp),
[Processing.cpp](Processing.cpp). Её основа:

```cpp
// configure, как в IOC:
sdn::TopicHelper::LoadDefinitionFile(&topic, path);
sdn::Subscriber subscriber(topic);
subscriber.SetInterface(interface);
subscriber.Configure();
// После готовности socket:
subscriber.Receive(uint64_t(0));
// XML-validated offsets -> rates -> cache.data() -> processor.process()
// Выход: named SetAttribute() и sdn::Publisher::Publish().
```

Один `SdnInputReceiver` уже является вертикальным срезом для одного topic; `main`
создаёт шесть одинаковых экземпляров. При запуске только PCF0 сумма содержит
его восемь значений; остальные входы пока равны нулю. Такой запуск
проверяет путь одного subscriber до publisher без отдельной несовместимой схемы topic.

## F. Выходной XML

[vnc-mp1-topic.xml](../../resources/vnc-mp1-topic.xml) повторяет плоскую схему исходных XML:

| Атрибут | SDN datatype | Значение |
|---|---|---|
| timestamp | uint64 | время агрегации, Unix/POSIX ns |
| sequenceNumber | uint64 | свой счётчик результатов, сбрасывается при рестарте |
| totalCountRate | double | сумма последних 48 значений, cps |

Ожидаемый размер payload — 24 bytes. Не используйте `sizeof(OutputData)` для
передачи: C++ может добавить padding. Новый consumer должен загрузить тот же XML.
SDN transport header со своими counter/timestamps остаётся отдельным от payload.

## G. Сборка, установка и runtime

В CODAC Linux из корня **основного проекта**:

```sh
make -C src/main/c++/vnc-mp1 CODAC_ROOT=/opt/codac
make -C src/main/c++/vnc-mp1 test
target/bin/vnc-mp1 lo "$PWD/src/main/resources" 100
```

`lo` подходит только для локального стенда: текущие IOC также используют `INTERFACE=lo`.
На разных машинах у каждого IOC и приложения должен быть выбран доступный SDN NIC.
CLI `-` вместо интерфейса читает `SDN_INTERFACE_NAME`; это не меняет настройки IOC.

```sh
export SDN_INTERFACE_NAME=<SDN_NIC>
target/bin/vnc-mp1 - "$PWD/src/main/resources"
```

Новый Makefile подключается существующим wildcard `src/main/c++/Makefile`.
Цель `all` родительского Makefile явно вызывает сборку подкаталогов.
При прямой сборке из `src/main/c++/vnc-mp1` и при Maven-сборке из
`target/main/c++/vnc-mp1` результаты размещаются относительно корня unit:

* `target/bin/vnc-mp1` — программа;
* `target/obj/vnc-mp1/` — объектные файлы и зависимости заголовков;
* `target/test/bin/vnc-mp1-processing-test` — тест алгоритма.

Корень вывода можно переопределить абсолютным путём `BUILD_ROOT=/path/to/build`.
`clean` удаляет только перечисленные результаты MP1. XML остаются в каталоге
ресурсов; цель `install` копирует программу и семь XML в `PREFIX/bin` и `TOPIC_DIR`
с поддержкой `DESTDIR`. Размещение бинарника в `target/bin` соответствует §B.4.12
CODAC Application Development, но само по себе не настраивает аргументы `mvn run`.

Зависимости: CODAC `sdn-core`, `ccs-core`, `log`, libxml2, pthread, rt.
Основание — [nds-sdn/Makefile](../nds-sdn/Makefile); NDS3 и legacy `sdn` facade
не используются новым executable. libxml2 указана явно для inline XML parser.
Headers берутся из установленного CODAC; референсные копии workspace не составляют
полный SDK: отсутствуют CCS common headers, реализации и библиотеки.

Альтернативный [CMakeLists.txt](CMakeLists.txt) не заменяет Maven/Make:

```sh
cmake -S src/main/c++/vnc-mp1 -B build/vnc-mp1 \
  -DCODAC_ROOT=/opt/codac -DCMAKE_INSTALL_PREFIX=/opt/codac \
  -DVNC_TOPIC_INSTALL_DIR=/etc/opt/codac/sdn
cmake --build build/vnc-mp1
ctest --test-dir build/vnc-mp1 --output-on-failure
```

Staging без системной установки:

```sh
make -C src/main/c++/vnc-mp1 install DESTDIR="$PWD/stage" \
  PREFIX=/opt/codac TOPIC_DIR=/etc/opt/codac/sdn
# Или после CMake build:
DESTDIR="$PWD/stage" cmake --install build/vnc-mp1
```

Устанавливаются executable и семь XML. Для реальной установки убрать `DESTDIR`
и использовать принятый на стенде механизм установки. Запуск установленного бинарника:
`/opt/codac/bin/vnc-mp1 - /etc/opt/codac/sdn`.
Для lookup исходного IOC можно выставить `SDN_TOPIC_PATH` на каталог XML до запуска IOC.

Проверки алгоритма без CODAC SDK (нужен C++ compiler):

```sh
cmake -S src/main/c++/vnc-mp1 -B build/vnc-processing -DVNC_BUILD_SDN=OFF
cmake --build build/vnc-processing
ctest --test-dir build/vnc-processing --output-on-failure
```

Новые правила дают сборку и staging, но не регистрируют процесс в SDD deployment,
CODAC supervisor или RPM package. Это требует проверки схемы установленного
`maven-iter-plugin` и выбора целевого хоста; выдуманные packaging tags не добавлялись.

## H. Проверка на CODAC машине

1. Собрать с реальными CODAC 7.4 headers/libraries; проверить разрешение libxml2,
   `log`, `ccs-core`, `sdn-core` через `ldd`. Выполнить `processing-test`.
2. Проверить XML parsing: payload входов 64 bytes, выхода 24; scalar uint64
   и double, имена, ranks, version; сравнить определения на publisher и consumer.
3. Проверить напечатанные multicast IP/ports, отсутствие конфликтов, multicast routes,
   выбранный NIC, доступность пакетов и настройки loopback для локального стенда.
4. Запустить consumer нового XML и MP1. Без IOC: `totalCountRate=0`;
   счётчик растёт, выход имеет cadence около 100 ms.
5. Запустить только PCF0, перевести NDS SDN и generator в `running`:
   результат равен сумме его восьми значений. Так проверяется первый вертикальный срез.
6. Включить PCF1..PCF5. На постоянных входах восемь единиц с каждого IOC дают 48;
   значения IOC 1,2,3,4,5,6 по восемь каналов дают 168. Для синусоидального
   симулятора сравнивать принятые значения, а не PV, прочитанные позднее.
7. Остановить PCF5: его вклад остаётся последним полученным.
   Возобновить PCF5: новые пакеты обновляют его вклад. Аналогично проверить каждый IOC.
8. Проверить приём пакетов, рестарт publisher, насыщение очередей и endian conversion.
   Контроль давности, качества данных и синхронизация отложены на следующий этап.
9. Проверить Ctrl+C/SIGTERM, неверный интерфейс, отсутствующий/несовместимый XML,
   отказ публикации. Consumer должен также иметь timeout исчезновения самого MP1.
10. Перед эксплуатацией согласовать epoch с CODAC timing/TAI, cadence, freshness budget,
    measurement coherence, control-plane states и deployment/package integration.

Локальная проверка в Windows: CMake configuration с `VNC_BUILD_SDN=OFF` была запущена,
но остановилась из-за отсутствия `nmake`/C++ compiler. C++ tests не выполнялись.
PowerShell XML parser успешно проверил имена, типы и ranks всех шести входных topics,
совпадение topic names в NDS mapping. После упрощения выходной XML содержит три поля.
SDN сеть и CODAC SDK в этом окружении не проверены; тестовый успешный запуск не заявляется.
