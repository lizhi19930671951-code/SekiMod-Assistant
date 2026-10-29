#pragma once
#include "services.hpp"
namespace sma {
inline uint32_t testCrc(const std::string& data){uint32_t c=~0u;for(unsigned char b:data){c^=b;for(int i=0;i<8;++i)c=(c>>1)^(0xedb88320u&uint32_t(-int(c&1)));}return ~c;}
inline void testLe(std::string& data,uint64_t value,int bytes){for(int i=0;i<bytes;++i)data+=char(value>>(i*8));}
inline void testZip(const fs::path& path,const std::string& name,bool link=false){
    std::string data="fixture",out,central;auto crc=testCrc(data);
    testLe(out,0x04034b50,4);testLe(out,20,2);testLe(out,0,2);testLe(out,0,2);testLe(out,0,4);testLe(out,crc,4);testLe(out,data.size(),4);testLe(out,data.size(),4);testLe(out,name.size(),2);testLe(out,0,2);out+=name;out+=data;auto offset=out.size();
    testLe(central,0x02014b50,4);testLe(central,link?0x0314:20,2);testLe(central,20,2);testLe(central,0,2);testLe(central,0,2);testLe(central,0,4);testLe(central,crc,4);testLe(central,data.size(),4);testLe(central,data.size(),4);testLe(central,name.size(),2);testLe(central,0,2);testLe(central,0,2);testLe(central,0,2);testLe(central,0,2);testLe(central,link?(0120777u<<16):0,4);testLe(central,0,4);central+=name;out+=central;testLe(out,0x06054b50,4);testLe(out,0,4);testLe(out,1,2);testLe(out,1,2);testLe(out,central.size(),4);testLe(out,offset,4);testLe(out,0,2);write(path,out);
}
inline void testRar(const fs::path& path){
    std::string out("Rar!\x1a\x07\x00",7),header,data="fixture",name="chr/test.bin";auto block=[&](std::string h){testLe(out,testCrc(h)&0xffff,2);out+=h;};
    header+=char(0x73);testLe(header,0,2);testLe(header,13,2);testLe(header,0,6);block(header);header.clear();header+=char(0x74);testLe(header,0x8000,2);testLe(header,32+name.size(),2);testLe(header,data.size(),4);testLe(header,data.size(),4);header+=char(2);testLe(header,testCrc(data),4);testLe(header,0,4);header+=char(20);header+=char(0x30);testLe(header,name.size(),2);testLe(header,0x20,4);header+=name;block(header);out+=data;header.clear();header+=char(0x7b);testLe(header,0,2);testLe(header,7,2);block(header);write(path,out);
}
}
