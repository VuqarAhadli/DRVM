#include <opcode.hpp>




bool isImplemented(Opcode opcode)
{
    switch (opcode)
    {
        case Opcode::Nop:
        case Opcode::AConstNull:
        case Opcode::IConstM1:
        case Opcode::IConst0:
        case Opcode::IConst1:
        case Opcode::IConst2:
        case Opcode::IConst3:
        case Opcode::IConst4:
        case Opcode::IConst5:
        case Opcode::Ldc:
        case Opcode::LdcW:
        case Opcode::Ldc2W:
        case Opcode::BiPush:
        case Opcode::SiPush:
        case Opcode::ILoad:
        case Opcode::ILoad0:
        case Opcode::ILoad1:
        case Opcode::ILoad2:
        case Opcode::ILoad3:
        case Opcode::IStore:
        case Opcode::IStore0:
        case Opcode::IStore1:
        case Opcode::IStore2:
        case Opcode::IStore3:
        case Opcode::IAdd:
        case Opcode::ISub:
        case Opcode::NewArray:
        case Opcode::ANewArray:
        case Opcode::ArrayLength:
        case Opcode::Goto:
        case Opcode::IReturn:
        case Opcode::Return:
        case Opcode::GetStatic:
        case Opcode::PutStatic:
        case Opcode::GetField:
        case Opcode::PutField:
        case Opcode::InvokeVirtual:
        case Opcode::InvokeSpecial:
        case Opcode::InvokeStatic:
        case Opcode::New:
            return true;

        default:
            return false;
    }
}
