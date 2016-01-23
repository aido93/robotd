#include "periphery.hpp"

boost::shared_ptr<TDeviceConfig> TPeriphery::usb;
boost::shared_ptr<TDeviceConfig> TPeriphery::i2c;

void TPeriphery::LoadConfig(const libconfig::Setting& settings)
{
    unsigned int usb_count;
    usb_count=settings[ "usb" ].getLength();
    usb=new TDeviceConfig[usb_count];
    const libconfig::Setting& usbconf = settings["usb"];
    for(short int i=0;i<usb_count;++i)
    {
        usbconf[i].lookupValue("name" , usb.Name );
        libconfig::Setting& commands=usbconf[i]["commands"];
        for(short int j=0;j<;++j)
        {
            commands[j].
        }
    }
}
