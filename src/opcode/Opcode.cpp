/*
 * DRVM (drvm)
 * Copyright (C) 2026 Vugar Ahadli
 * Contact: vuqarahadli17@gmail.com | vuqar@div.edu.az
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <https://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include <Opcode.hpp>


bool isImplemented(Opcode opcode)
{
    if (static_cast<U1>(opcode) <= 152 )
    {
        return true;
    }
    
    return false;
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
        case Opcode::LConst0:
            return "LConst0";
        case Opcode::LConst1:
            return "LConst1";
        case Opcode::FConst0:
            return "FConst0";
        case Opcode::FConst1:
            return "FConst1";
        case Opcode::FConst2:
            return "FConst2";
        case Opcode::DConst0:
            return "DConst0";
        case Opcode::DConst1:
            return "DConst1";
        case Opcode::BiPush:
            return "BiPush";
        case Opcode::SiPush:
            return "SiPush";
        case Opcode::Ldc:
            return "Ldc";
        case Opcode::LdcW:
            return "LdcW";
        case Opcode::Ldc2W:
            return "Ldc2W";
        case Opcode::ILoad:
            return "ILoad";
        case Opcode::LLoad:
            return "LLoad";
        case Opcode::FLoad:
            return "FLoad";
        case Opcode::DLoad:
            return "DLoad";
        case Opcode::ALoad:
            return "ALoad";
        case Opcode::ILoad0:
            return "ILoad0";
        case Opcode::ILoad1:
            return "ILoad1";
        case Opcode::ILoad2:
            return "ILoad2";
        case Opcode::ILoad3:
            return "ILoad3";
        case Opcode::LLoad0:
            return "LLoad0";
        case Opcode::LLoad1:
            return "LLoad1";
        case Opcode::LLoad2:
            return "LLoad2";
        case Opcode::LLoad3:
            return "LLoad3";
        case Opcode::FLoad0:
            return "FLoad0";
        case Opcode::FLoad1:
            return "FLoad1";
        case Opcode::FLoad2:
            return "FLoad2";
        case Opcode::FLoad3:
            return "FLoad3";
        case Opcode::DLoad0:
            return "DLoad0";
        case Opcode::DLoad1:
            return "DLoad1";
        case Opcode::DLoad2:
            return "DLoad2";
        case Opcode::DLoad3:
            return "DLoad3";
        case Opcode::ALoad0:
            return "ALoad0";
        case Opcode::ALoad1:
            return "ALoad1";
        case Opcode::ALoad2:
            return "ALoad2";
        case Opcode::ALoad3:
            return "ALoad3";
        case Opcode::IALoad:
            return "IALoad";
        case Opcode::LALoad:
            return "LALoad";
        case Opcode::FALoad:
            return "FALoad";
        case Opcode::DALoad:
            return "DALoad";
        case Opcode::AALoad:
            return "AALoad";
        case Opcode::BALoad:
            return "BALoad";
        case Opcode::CALoad:
            return "CALoad";
        case Opcode::SALoad:
            return "SALoad";
        case Opcode::IStore:
            return "IStore";
        case Opcode::LStore:
            return "LStore";
        case Opcode::FStore:
            return "FStore";
        case Opcode::DStore:
            return "DStore";
        case Opcode::AStore:
            return "AStore";
        case Opcode::IStore0:
            return "IStore0";
        case Opcode::IStore1:
            return "IStore1";
        case Opcode::IStore2:
            return "IStore2";
        case Opcode::IStore3:
            return "IStore3";
        case Opcode::LStore0:
            return "LStore0";
        case Opcode::LStore1:
            return "LStore1";
        case Opcode::LStore2:
            return "LStore2";
        case Opcode::LStore3:
            return "LStore3";
        case Opcode::FStore0:
            return "FStore0";
        case Opcode::FStore1:
            return "FStore1";
        case Opcode::FStore2:
            return "FStore2";
        case Opcode::FStore3:
            return "FStore3";
        case Opcode::DStore0:
            return "DStore0";
        case Opcode::DStore1:
            return "DStore1";
        case Opcode::DStore2:
            return "DStore2";
        case Opcode::DStore3:
            return "DStore3";
        case Opcode::AStore0:
            return "AStore0";
        case Opcode::AStore1:
            return "AStore1";
        case Opcode::AStore2:
            return "AStore2";
        case Opcode::AStore3:
            return "AStore3";
        case Opcode::IAStore:
            return "IAStore";
        case Opcode::LAStore:
            return "LAStore";
        case Opcode::FAStore:
            return "FAStore";
        case Opcode::DAStore:
            return "DAStore";
        case Opcode::AAStore:
            return "AAStore";
        case Opcode::BAStore:
            return "BAStore";
        case Opcode::CAStore:
            return "CAStore";
        case Opcode::SAStore:
            return "SAStore";
        case Opcode::Pop:
            return "Pop";
        case Opcode::Pop2:
            return "Pop2";
        case Opcode::Dup:
            return "Dup";
        case Opcode::DupX1:
            return "DupX1";
        case Opcode::DupX2:
            return "DupX2";
        case Opcode::Dup2:
            return "Dup2";
        case Opcode::Dup2X1:
            return "Dup2X1";
        case Opcode::Dup2X2:
            return "Dup2X2";
        case Opcode::Swap:
            return "Swap";
        case Opcode::IAdd:
            return "IAdd";
        case Opcode::LAdd:
            return "LAdd";
        case Opcode::FAdd:
            return "FAdd";
        case Opcode::DAdd:
            return "DAdd";
        case Opcode::ISub:
            return "ISub";
        case Opcode::LSub:
            return "LSub";
        case Opcode::FSub:
            return "FSub";
        case Opcode::DSub:
            return "DSub";
        case Opcode::IMul:
            return "IMul";
        case Opcode::LMul:
            return "LMul";
        case Opcode::FMul:
            return "FMul";
        case Opcode::DMul:
            return "DMul";
        case Opcode::IDiv:
            return "IDiv";
        case Opcode::LDiv:
            return "LDiv";
        case Opcode::FDiv:
            return "FDiv";
        case Opcode::DDiv:
            return "DDiv";
        case Opcode::IRem:
            return "IRem";
        case Opcode::LRem:
            return "LRem";
        case Opcode::FRem:
            return "FRem";
        case Opcode::DRem:
            return "DRem";
        case Opcode::INeg:
            return "INeg";
        case Opcode::LNeg:
            return "LNeg";
        case Opcode::FNeg:
            return "FNeg";
        case Opcode::DNeg:
            return "DNeg";
        case Opcode::IShl:
            return "IShl";
        case Opcode::LShl:
            return "LShl";
        case Opcode::IShr:
            return "IShr";
        case Opcode::LShr:
            return "LShr";
        case Opcode::IUShr:
            return "IUShr";
        case Opcode::LUShr:
            return "LUShr";
        case Opcode::IAnd:
            return "IAnd";
        case Opcode::LAnd:
            return "LAnd";
        case Opcode::IOr:
            return "IOr";
        case Opcode::LOr:
            return "LOr";
        case Opcode::IXor:
            return "IXor";
        case Opcode::LXor:
            return "LXor";
        case Opcode::IInc:
            return "IInc";
        case Opcode::I2L:
            return "I2L";
        case Opcode::I2F:
            return "I2F";
        case Opcode::I2D:
            return "I2D";
        case Opcode::L2I:
            return "L2I";
        case Opcode::L2F:
            return "L2F";
        case Opcode::L2D:
            return "L2D";
        case Opcode::F2I:
            return "F2I";
        case Opcode::F2L:
            return "F2L";
        case Opcode::F2D:
            return "F2D";
        case Opcode::D2I:
            return "D2I";
        case Opcode::D2L:
            return "D2L";
        case Opcode::D2F:
            return "D2F";
        case Opcode::I2B:
            return "I2B";
        case Opcode::I2C:
            return "I2C";
        case Opcode::I2S:
            return "I2S";
        case Opcode::LCmp:
            return "LCmp";
        case Opcode::FCmpL:
            return "FCmpL";
        case Opcode::FCmpG:
            return "FCmpG";
        case Opcode::DCmpL:
            return "DCmpL";
        case Opcode::DCmpG:
            return "DCmpG";
        case Opcode::IfEq:
            return "IfEq";
        case Opcode::IfNe:
            return "IfNe";
        case Opcode::IfLt:
            return "IfLt";
        case Opcode::IfGe:
            return "IfGe";
        case Opcode::IfGt:
            return "IfGt";
        case Opcode::IfLe:
            return "IfLe";
        case Opcode::IfICmpEq:
            return "IfICmpEq";
        case Opcode::IfICmpNe:
            return "IfICmpNe";
        case Opcode::IfICmpLt:
            return "IfICmpLt";
        case Opcode::IfICmpGe:
            return "IfICmpGe";
        case Opcode::IfICmpGt:
            return "IfICmpGt";
        case Opcode::IfICmpLe:
            return "IfICmpLe";
        case Opcode::IfACmpEq:
            return "IfACmpEq";
        case Opcode::IfACmpNe:
            return "IfACmpNe";
        case Opcode::Goto:
            return "Goto";
        case Opcode::Jsr:
            return "Jsr";
        case Opcode::Ret:
            return "Ret";
        case Opcode::TableSwitch:
            return "TableSwitch";
        case Opcode::LookupSwitch:
            return "LookupSwitch";
        case Opcode::IReturn:
            return "IReturn";
        case Opcode::LReturn:
            return "LReturn";
        case Opcode::FReturn:
            return "FReturn";
        case Opcode::DReturn:
            return "DReturn";
        case Opcode::AReturn:
            return "AReturn";
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
        case Opcode::InvokeInterface: 
            return "InvokeInterface";
        case Opcode::InvokeDynamic:   
            return "InvokeDynamic";
        case Opcode::New:
            return "New";
        case Opcode::NewArray:
            return "NewArray";
        case Opcode::ANewArray:
            return "ANewArray";
        case Opcode::ArrayLength:
            return "ArrayLength";
        case Opcode::AThrow:
            return "AThrow";
        case Opcode::CheckCast:
            return "CheckCast";
        case Opcode::InstanceOf:
            return "InstanceOf";
        case Opcode::MonitorEnter:
            return "MonitorEnter";
        case Opcode::MonitorExit:
            return "MonitorExit";
        case Opcode::Wide:
            return "Wide";
        case Opcode::MultiANewArray:    
            return "MultiANewArray";
        case Opcode::IfNull:
            return "IfNull";
        case Opcode::IfNonNull:
            return "IfNonNull";
        case Opcode::GotoW:
            return "GotoW";
        case Opcode::JsrW:
            return "JsrW";
        case Opcode::Breakpoint:
            return "Breakpoint";

        case Opcode::Unused_0xCB:
            return "Unused_0xCB";
        case Opcode::Unused_0xCC:
            return "Unused_0xCC";
        case Opcode::Unused_0xCD:
            return "Unused_0xCD";
        case Opcode::Unused_0xCE:
            return "Unused_0xCE";
        case Opcode::Unused_0xCF:
            return "Unused_0xCF";
        case Opcode::Unused_0xD0:
            return "Unused_0xD0";
        case Opcode::Unused_0xD1:
            return "Unused_0xD1";
        case Opcode::Unused_0xD2:
            return "Unused_0xD2";
        case Opcode::Unused_0xD3:
            return "Unused_0xD3";
        case Opcode::Unused_0xD4:
            return "Unused_0xD4";
        case Opcode::Unused_0xD5:
            return "Unused_0xD5";
        case Opcode::Unused_0xD6:
            return "Unused_0xD6";
        case Opcode::Unused_0xD7:
            return "Unused_0xD7";
        case Opcode::Unused_0xD8:
            return "Unused_0xD8";
        case Opcode::Unused_0xD9:
            return "Unused_0xD9";
        case Opcode::Unused_0xDA:
            return "Unused_0xDA";
        case Opcode::Unused_0xDB:
            return "Unused_0xDB";
        case Opcode::Unused_0xDC:
            return "Unused_0xDC";
        case Opcode::Unused_0xDD:
            return "Unused_0xDD";
        case Opcode::Unused_0xDE:
            return "Unused_0xDE";
        case Opcode::Unused_0xDF:
            return "Unused_0xDF";
        case Opcode::Unused_0xE0:
            return "Unused_0xE0";
        case Opcode::Unused_0xE1:
            return "Unused_0xE1";
        case Opcode::Unused_0xE2:
            return "Unused_0xE2";
        case Opcode::Unused_0xE3:
            return "Unused_0xE3";
        case Opcode::Unused_0xE4:
            return "Unused_0xE4";
        case Opcode::Unused_0xE5:
            return "Unused_0xE5";
        case Opcode::Unused_0xE6:
            return "Unused_0xE6";
        case Opcode::Unused_0xE7:
            return "Unused_0xE7";
        case Opcode::Unused_0xE8:
            return "Unused_0xE8";
        case Opcode::Unused_0xE9:
            return "Unused_0xE9";
        case Opcode::Unused_0xEA:
            return "Unused_0xEA";
        case Opcode::Unused_0xEB:
            return "Unused_0xEB";
        case Opcode::Unused_0xEC:
            return "Unused_0xEC";
        case Opcode::Unused_0xED:
            return "Unused_0xED";
        case Opcode::Unused_0xEE:
            return "Unused_0xEE";
        case Opcode::Unused_0xEF:
            return "Unused_0xEF";
        case Opcode::Unused_0xF0:
            return "Unused_0xF0";
        case Opcode::Unused_0xF1:
            return "Unused_0xF1";
        case Opcode::Unused_0xF2:
            return "Unused_0xF2";
        case Opcode::Unused_0xF3:
            return "Unused_0xF3";
        case Opcode::Unused_0xF4:
            return "Unused_0xF4";
        case Opcode::Unused_0xF5:
            return "Unused_0xF5";
        case Opcode::Unused_0xF6:
            return "Unused_0xF6";
        case Opcode::Unused_0xF7:
            return "Unused_0xF7";
        case Opcode::Unused_0xF8:
            return "Unused_0xF8";
        case Opcode::Unused_0xF9:
            return "Unused_0xF9";
        case Opcode::Unused_0xFA:
            return "Unused_0xFA";
        case Opcode::Unused_0xFB:
            return "Unused_0xFB";
        case Opcode::Unused_0xFC:
            return "Unused_0xFC";
        case Opcode::Unused_0xFD:
            return "Unused_0xFD";

        case Opcode::ImpDep1:
            return "ImpDep1";
        case Opcode::ImpDep2:
            return "ImpDep2";
    }

    /**
     * We wont ever see this case being returned but for the sake of compiler compliance we will keep it.
     */
    return "Not implemented/Unknown";
}

U1 operandSize(Opcode opcode)
{
    switch (opcode)
    {
        /**
         * 1 - Byte operands
         */
        case Opcode::BiPush:
        case Opcode::Ldc:
        case Opcode::NewArray:
        case Opcode::ILoad:
        case Opcode::LLoad:
        case Opcode::FLoad:
        case Opcode::DLoad:
        case Opcode::ALoad:
        case Opcode::IStore:
        case Opcode::LStore:
        case Opcode::FStore:
        case Opcode::DStore:
        case Opcode::AStore:
        case Opcode::Ret:
       return 1;

        /**
         * 2 - Byte operands
         */
        case Opcode::SiPush:
        case Opcode::LdcW:
        case Opcode::Ldc2W:
        case Opcode::IInc:
        case Opcode::IfEq:
        case Opcode::IfNe:
        case Opcode::IfLt:
        case Opcode::IfGe:
        case Opcode::IfGt:
        case Opcode::IfLe:
        case Opcode::IfICmpEq:
        case Opcode::IfICmpNe:
        case Opcode::IfICmpLt:
        case Opcode::IfICmpGe:
        case Opcode::IfICmpGt:
        case Opcode::IfICmpLe:
        case Opcode::IfACmpEq:
        case Opcode::IfACmpNe:
        case Opcode::Goto:
        case Opcode::Jsr:
        case Opcode::GetStatic:
        case Opcode::PutStatic:
        case Opcode::GetField:
        case Opcode::PutField:
        case Opcode::InvokeVirtual:
        case Opcode::InvokeSpecial:
        case Opcode::InvokeStatic:
        case Opcode::New:
        case Opcode::ANewArray:
        case Opcode::CheckCast:
        case Opcode::InstanceOf:
        case Opcode::IfNull:
        case Opcode::IfNonNull:
       return 2;

        /**
         * 3 - Byte operands
         */
        case Opcode::MultiANewArray:  // index (2) + dimensions (1)
       return 3;

        /**
         * 4 - Byte operands
         */
        case Opcode::InvokeInterface: // index (2) + count (1) + 0 (1)
        case Opcode::InvokeDynamic:   // index (2) + 0 (1) + 0 (1)
        case Opcode::GotoW:
        case Opcode::JsrW:
       return 4;

        /**
         * Variable length operands
         */
        case Opcode::TableSwitch:
        case Opcode::LookupSwitch:
        case Opcode::Wide:
       return 0;

        /**
         * 0 - Byte operands
         */
        case Opcode::Nop:
        case Opcode::AConstNull:
        case Opcode::IConstM1:
        case Opcode::IConst0:
        case Opcode::IConst1:
        case Opcode::IConst2:
        case Opcode::IConst3:
        case Opcode::IConst4:
        case Opcode::IConst5:
        case Opcode::LConst0:
        case Opcode::LConst1:
        case Opcode::FConst0:
        case Opcode::FConst1:
        case Opcode::FConst2:
        case Opcode::DConst0:
        case Opcode::DConst1:
        case Opcode::ILoad0:
        case Opcode::ILoad1:
        case Opcode::ILoad2:
        case Opcode::ILoad3:
        case Opcode::LLoad0:
        case Opcode::LLoad1:
        case Opcode::LLoad2:
        case Opcode::LLoad3:
        case Opcode::FLoad0:
        case Opcode::FLoad1:
        case Opcode::FLoad2:
        case Opcode::FLoad3:
        case Opcode::DLoad0:
        case Opcode::DLoad1:
        case Opcode::DLoad2:
        case Opcode::DLoad3:
        case Opcode::ALoad0:
        case Opcode::ALoad1:
        case Opcode::ALoad2:
        case Opcode::ALoad3:
        case Opcode::IALoad:
        case Opcode::LALoad:
        case Opcode::FALoad:
        case Opcode::DALoad:
        case Opcode::AALoad:
        case Opcode::BALoad:
        case Opcode::CALoad:
        case Opcode::SALoad:
        case Opcode::IStore0:
        case Opcode::IStore1:
        case Opcode::IStore2:
        case Opcode::IStore3:
        case Opcode::LStore0:
        case Opcode::LStore1:
        case Opcode::LStore2:
        case Opcode::LStore3:
        case Opcode::FStore0:
        case Opcode::FStore1:
        case Opcode::FStore2:
        case Opcode::FStore3:
        case Opcode::DStore0:
        case Opcode::DStore1:
        case Opcode::DStore2:
        case Opcode::DStore3:
        case Opcode::AStore0:
        case Opcode::AStore1:
        case Opcode::AStore2:
        case Opcode::AStore3:
        case Opcode::IAStore:
        case Opcode::LAStore:
        case Opcode::FAStore:
        case Opcode::DAStore:
        case Opcode::AAStore:
        case Opcode::BAStore:
        case Opcode::CAStore:
        case Opcode::SAStore:
        case Opcode::Pop:
        case Opcode::Pop2:
        case Opcode::Dup:
        case Opcode::DupX1:
        case Opcode::DupX2:
        case Opcode::Dup2:
        case Opcode::Dup2X1:
        case Opcode::Dup2X2:
        case Opcode::Swap:
        case Opcode::IAdd:
        case Opcode::LAdd:
        case Opcode::FAdd:
        case Opcode::DAdd:
        case Opcode::ISub:
        case Opcode::LSub:
        case Opcode::FSub:
        case Opcode::DSub:
        case Opcode::IMul:
        case Opcode::LMul:
        case Opcode::FMul:
        case Opcode::DMul:
        case Opcode::IDiv:
        case Opcode::LDiv:
        case Opcode::FDiv:
        case Opcode::DDiv:
        case Opcode::IRem:
        case Opcode::LRem:
        case Opcode::FRem:
        case Opcode::DRem:
        case Opcode::INeg:
        case Opcode::LNeg:
        case Opcode::FNeg:
        case Opcode::DNeg:
        case Opcode::IShl:
        case Opcode::LShl:
        case Opcode::IShr:
        case Opcode::LShr:
        case Opcode::IUShr:
        case Opcode::LUShr:
        case Opcode::IAnd:
        case Opcode::LAnd:
        case Opcode::IOr:
        case Opcode::LOr:
        case Opcode::IXor:
        case Opcode::LXor:
        case Opcode::I2L:
        case Opcode::I2F:
        case Opcode::I2D:
        case Opcode::L2I:
        case Opcode::L2F:
        case Opcode::L2D:
        case Opcode::F2I:
        case Opcode::F2L:
        case Opcode::F2D:
        case Opcode::D2I:
        case Opcode::D2L:
        case Opcode::D2F:
        case Opcode::I2B:
        case Opcode::I2C:
        case Opcode::I2S:
        case Opcode::LCmp:
        case Opcode::FCmpL:
        case Opcode::FCmpG:
        case Opcode::DCmpL:
        case Opcode::DCmpG:
        case Opcode::IReturn:
        case Opcode::LReturn:
        case Opcode::FReturn:
        case Opcode::DReturn:
        case Opcode::AReturn:
        case Opcode::Return:
        case Opcode::ArrayLength:
        case Opcode::AThrow:
        case Opcode::MonitorEnter:
        case Opcode::MonitorExit:

        /**
         * Reserved opcodes no operation and opreand
         */
        case Opcode::Breakpoint:
        case Opcode::Unused_0xCB:
        case Opcode::Unused_0xCC:
        case Opcode::Unused_0xCD:
        case Opcode::Unused_0xCE:
        case Opcode::Unused_0xCF:
        case Opcode::Unused_0xD0:
        case Opcode::Unused_0xD1:
        case Opcode::Unused_0xD2:
        case Opcode::Unused_0xD3:
        case Opcode::Unused_0xD4:
        case Opcode::Unused_0xD5:
        case Opcode::Unused_0xD6:
        case Opcode::Unused_0xD7:
        case Opcode::Unused_0xD8:
        case Opcode::Unused_0xD9:
        case Opcode::Unused_0xDA:
        case Opcode::Unused_0xDB:
        case Opcode::Unused_0xDC:
        case Opcode::Unused_0xDD:
        case Opcode::Unused_0xDE:
        case Opcode::Unused_0xDF:
        case Opcode::Unused_0xE0:
        case Opcode::Unused_0xE1:
        case Opcode::Unused_0xE2:
        case Opcode::Unused_0xE3:
        case Opcode::Unused_0xE4:
        case Opcode::Unused_0xE5:
        case Opcode::Unused_0xE6:
        case Opcode::Unused_0xE7:
        case Opcode::Unused_0xE8:
        case Opcode::Unused_0xE9:
        case Opcode::Unused_0xEA:
        case Opcode::Unused_0xEB:
        case Opcode::Unused_0xEC:
        case Opcode::Unused_0xED:
        case Opcode::Unused_0xEE:
        case Opcode::Unused_0xEF:
        case Opcode::Unused_0xF0:
        case Opcode::Unused_0xF1:
        case Opcode::Unused_0xF2:
        case Opcode::Unused_0xF3:
        case Opcode::Unused_0xF4:
        case Opcode::Unused_0xF5:
        case Opcode::Unused_0xF6:
        case Opcode::Unused_0xF7:
        case Opcode::Unused_0xF8:
        case Opcode::Unused_0xF9:
        case Opcode::Unused_0xFA:
        case Opcode::Unused_0xFB:
        case Opcode::Unused_0xFC:
        case Opcode::Unused_0xFD:
        case Opcode::ImpDep1:
        case Opcode::ImpDep2:
       return 0;
    }

    return 0;
}

bool usesConstantPoolOperand(Opcode op)
{
    switch (op)
    {
        case Opcode::Ldc:
        case Opcode::LdcW:
        case Opcode::Ldc2W:
        case Opcode::GetStatic:
        case Opcode::PutStatic:
        case Opcode::GetField:
        case Opcode::PutField:
        case Opcode::InvokeVirtual:
        case Opcode::InvokeSpecial:
        case Opcode::InvokeStatic:
        case Opcode::InvokeInterface:
        case Opcode::InvokeDynamic:
        case Opcode::New:
        case Opcode::ANewArray:
        case Opcode::CheckCast:
        case Opcode::InstanceOf:
        case Opcode::MultiANewArray:
            return true;
        default:
            return false;
    }
}

bool usesBranchOperand(Opcode op)
{
    switch (op)
    {
        case Opcode::IfEq:
        case Opcode::IfNe:
        case Opcode::IfLt:
        case Opcode::IfGe:
        case Opcode::IfGt:
        case Opcode::IfLe:
        case Opcode::IfICmpEq:
        case Opcode::IfICmpNe:
        case Opcode::IfICmpLt:
        case Opcode::IfICmpGe:
        case Opcode::IfICmpGt:  
        case Opcode::IfICmpLe:
        case Opcode::IfACmpEq:  
        case Opcode::IfACmpNe:
        case Opcode::Goto:      
        case Opcode::Jsr:
        case Opcode::IfNull:    
        case Opcode::IfNonNull:
            return true;   
        default:
            return false;
    }
}
