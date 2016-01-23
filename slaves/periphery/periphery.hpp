#ifndef __PERIPHERY_HPP
#define __PERIPHERY_HPP

#include <stdint.h>
#include <map>
#include <boost/shared_ptr.hpp>

#pragma pack (push,1)
template<uint8_t funcBits=8, dataBits=6, sizeBits=6, retBits=2>struct TCommand
{
    uint32_t FunctionName:funcBits;//имя функции/команды, которую умеет выполнять устройство
    uint32_t pData:dataBits;//указатель на кусок данных, передаваемых в функцию
    uint32_t size:sizeBits;//размер данных. ограничен размерами пакета(64 байта для USB)
    uint32_t ret:retBits;//адрес, куда запишется ответ этой функции(3 варианта на выбор, 0 - никуда)
};
#pragma pack(pop)

struct TDeviceConfig
{
    char* Name;//нужно для загрузки внутренней конфигурации устройства
               //для объявления всех используемых устройств
    //задаем, как выглядят функции в нашем сообщении устройству
    
    std::map<char[16], TCommand> commands; //список команд, которые умеет
                                    //выполнять устройство в виде списка
    //"название команды на человеческом языке" - "битовое представление"
};

#include <libconfig.h++>

class TPeriphery:
public boost::enable_shared_from_this<TPeriphery>
{
    private:
        static boost::shared_ptr<TDeviceConfig> usb;
        static boost::shared_ptr<TDeviceConfig> i2c;
    protected:
        ~TPeriphery()
        {
            delete [] usb;
            delete [] i2c;
        };
    public:
        static void LoadConfig(const libconfig::Setting&);
        TPeriphery()
        {
        };
};

#endif
