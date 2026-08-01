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

std::string toString(Opcode opcode)
{
    switch (opcode)
    {
        case Opcode::Nop:
        return "Nop";

        case Opcode::AConstNull:
        return "AConstNull";

        case Opcode::IConstM1:
        return "IConstM1";

        case Opcode::IConst0:
        return "IConst0";

        case Opcode::IConst1:
        return "IConst1";

        case Opcode::IConst2:
        return "IConst2";

        case Opcode::IConst3:
        return "IConst3";

        case Opcode::IConst4:
        return "IConst4";

        case Opcode::IConst5:
        return "IConst5";

        case Opcode::Ldc:
        return "Ldc";

        case Opcode::LdcW:
        return "LdcW";

        case Opcode::Ldc2W:
        return "Ldc2W";

        case Opcode::BiPush:       
        return "BiPush";
        
        case Opcode::SiPush:
        return "SiPush";

        case Opcode::ILoad:
        return "ILoad";

        case Opcode::ILoad0:
        return "ILoad0";

        case Opcode::ILoad1:
        return "ILoad1";
        case Opcode::ILoad2:
        return "ILoad2";
        case Opcode::ILoad3:
        return "ILoad3";

        case Opcode::IStore:    
        return "IStore";

        case Opcode::IStore0:
        return "IStore0";

        case Opcode::IStore1:
        return "IStore1";

        case Opcode::IStore2:
        return "IStore2";

        case Opcode::IStore3:
        return "IStore3";

        case Opcode::IAdd:
        return "IAdd";

        case Opcode::ISub:
        return "ISub";

        case Opcode::NewArray:
        return "NewArray";

        case Opcode::ANewArray:
        return "ANewArray";

        case Opcode::ArrayLength:
        return "ArrayLength";

        case Opcode::Goto:
        return "Goto";

        case Opcode::IReturn:
        return "IReturn";

        case Opcode::Return:
        return "Return";

        case Opcode::GetStatic:
        return "GetStatic";

        case Opcode::PutStatic:
        return "PutStatic";

        case Opcode::GetField:
        return "GetField";

        case Opcode::PutField:
        return "PutField";

        case Opcode::InvokeVirtual:
        return "InvokeVirtual";

        case Opcode::InvokeSpecial:
        return "InvokeSpecial";

        case Opcode::InvokeStatic:
        return "InvokeStatic";

        case Opcode::New:
        return "New";

        default:
        return "Not implemented/Unknown";
    }
}

U1 operandSize(Opcode opcode)
{
    switch (opcode)
    {
        case Opcode::BiPush:
        case Opcode::Ldc:
        case Opcode::NewArray:
        case Opcode::ILoad:
        case Opcode::IStore:
            return 1;

        case Opcode::SiPush:
        case Opcode::LdcW:
        case Opcode::Ldc2W:
        case Opcode::ANewArray:
        case Opcode::New:
        case Opcode::Goto:
        case Opcode::GetStatic:
        case Opcode::PutStatic:
        case Opcode::GetField:
        case Opcode::PutField:
        case Opcode::InvokeVirtual:
        case Opcode::InvokeSpecial:
        case Opcode::InvokeStatic:
            return 2;

        default:
            return 0;
    }
}