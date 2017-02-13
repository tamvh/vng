#include "commmessage.h"
namespace iot {
namespace comm {
Message::Message()
{

}

Message::~Message()
{

}

void Message::fill(Packet &packet) const
{
    Size size = this->size();
    packet.resize(sizeof(Id) + sizeof(Size) + size);
    uint8_t *payload = packet.data();
    *(Id *) payload = this->id();
    *(Size *) (payload + sizeof(size)) = size;
    fill(payload + sizeof(Id) + sizeof(Size));
}

} // namespace comm
} // namespace iot
