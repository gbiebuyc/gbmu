/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Emulator.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nathan <unkown@noaddress.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/11 03:22:40 by nathan            #+#    #+#             */
/*   Updated: 2021/02/19 08:42:33 by nathan           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Emulator.hpp"
#include <iostream> //to remove, debug
#include "Operations.tpp"
#include <iomanip>

unsigned char Emulator::memory[64] = {};
unsigned short Emulator::PC = 0;
unsigned short Emulator::SP = 0;
unsigned char Emulator::registers[8] = {};
unsigned long Emulator::clock = 0;

unsigned char& Emulator::A = registers[7];
unsigned char& Emulator::B = registers[6];
unsigned char& Emulator::C = registers[5];
unsigned char& Emulator::D = registers[4];
unsigned char& Emulator::E = registers[3];
unsigned char& Emulator::F = registers[2];
unsigned char& Emulator::H = registers[1];
unsigned char& Emulator::L = registers[0];

unsigned short& Emulator::BC = *reinterpret_cast<unsigned short*>(&C);
unsigned short& Emulator::DE = *reinterpret_cast<unsigned short*>(&E);
unsigned short& Emulator::HL = *reinterpret_cast<unsigned short*>(&L);

void	Emulator::increment(unsigned char& reg)
{
	unsigned char tmp = reg & 0xF0;

	reg--;
	setFlag0(reg == 0);
	setFlagN(false);
	setFlagH(tmp != (reg & 0xF0));
	clock += 1;
}

void	Emulator::decrement(unsigned char& reg)
{
	unsigned char tmp = reg & 0xF0;

	reg--;
	setFlag0(reg == 0);
	setFlagN(true);
	setFlagH(tmp != (reg & 0xF0));
	clock += 1;
}

void	Emulator::loadOperation(unsigned short opcode)
{
	unsigned char *dst;
	unsigned char *src;
	unsigned char iterator = 0x48;

	if (opcode < iterator)
		dst = &B;
	else if ((iterator += 8) && opcode < iterator)
		dst = &C;
	else if ((iterator += 8) && opcode < iterator)
		dst = &D;
	else if ((iterator += 8) && opcode < iterator)
		dst = &E;
	else if ((iterator += 8) && opcode < iterator)
		dst = &H;
	else if ((iterator += 8) && opcode < iterator)
		dst = &L;
	else if ((iterator += 8) && opcode < iterator)
		dst = &memory[HL];
	else if ((iterator += 8) && opcode < iterator)
		dst = &A;
	if (opcode & 0x7 == 0x7)
		src = &A;
	if (opcode & 0x7 == 0x0)
		src = &B;
	if (opcode & 0x7 == 0x1)
		src = &C;
	if (opcode & 0x7 == 0x2)
		src = &D;
	if (opcode & 0x7 == 0x3)
		src = &E;
	if (opcode & 0x7 == 0x4)
		src = &H;
	if (opcode & 0x7 == 0x5)
		src = &L;
	if (opcode & 0x7 == 0x6)
	{
		src = &memory[HL];
		clock += 1;
	}
	clock += 1;
	*dst = *src;
}

bool	Emulator::getFlag0()
{
	return F & (1 << 7);
}

bool	Emulator::getFlagN()
{
	return F & (1 << 6);
}

bool	Emulator::getFlagH()
{
	return F & (1 << 5);
}

bool	Emulator::getFlagC()
{
	return F & (1 << 4);
}

void	Emulator::setFlag0(unsigned char option)
{
	if (option < 2)
		F = F & ((unsigned char)option << 7);
}

void	Emulator::setFlagN(unsigned char option)
{
	if (option < 2)
		F = F & ((unsigned char)option << 6);
}

void	Emulator::setFlagH(unsigned char option)
{
	if (option < 2)
		F = F & ((unsigned char)option << 5);
}

void	Emulator::setFlagC(unsigned char option)
{
	if (option < 2)
		F = F & ((unsigned char)option << 4);
}

unsigned char Emulator::readByte(unsigned short addr)
{
	(void)addr;
	return (0);//read in memory
}

unsigned short Emulator::readData(unsigned char nb)
{
	unsigned short value;

	value = 0;
	value = readByte();
	if (nb == 2)
		value = value | (readByte() << 8);
	PC += nb;
	std::cout << "i read" << std::endl;
	if (nb == 1)
		return ((unsigned char)value);
	return (value);
}

void Emulator::executeOpcode(unsigned short opcode)
{
	B = 0xFF;
	C = 0x23;// litle endian ;)
	printRegisters();
	unsigned short tmp;
	if (opcode >> 8 == 0)
	{
		std::cout << std::setfill('0') << "Doing opcode: 0x" << std::setw(2) << std::hex << +(char)opcode << std::endl;
		switch (opcode)
		{
			case 0: // NOP
				clock += 1;
				break;
			case 0x01: // LD BC, d16
				BC = readData(2);
				clock += 3;
				break;
			case 0x02: // LD BC, a
				BC = A;
				clock += 3;
				break;
			case 0x03: // INC BC
				BC++;
				clock += 2;
				break;
			case 0x04: // INC B
				increment(B);
				break;
			case 0x05: // DEC B
				decrement(B):
				break;
			case 0x06: // LD B, d8
				B = readData(1);
				clock += 2;
				break;
			case 0x07: // RLCA
				setFlagC(A >> 7);
				A <<= 1;
				setFlag0(false);
				setFlagH(false);
				setFlagN(false);
				A |= getFlagC();
				clock += 1;
				break;
			case 0x08: // LD (a16), SP
				addr = readData(2);
				memory[addr] = SP & 0xFF;
				memory[addr + 1] = SP >> 8;
				clock += 5;
				break;
			case 0x09: // ADD HL, BC
				tmp = HL;
				HL += BC;
				setFlagN(false);
				setFlagH((tmp & 0xFE00) != (HL & 0xFE00));//TODO to check which bit half carry
				setFlagC(tmp > HL);
				clock += 2;
				break;
			case 0x0A: // LD A, (BC)
				A = memory[BC];
				clock += 2;
				break;
			case 0x0B: // DEC BC
				BC--;
				clock += 2;
				break;
			case 0x0C: // INC C
				increment(C);
				break;
			case 0x0D: // DEC C
				decrement(C);
				break;
			case 0x0E: // LD C, d8
				C = readData(1);
				clock += 2;
				break;
			case 0x0F: //RRCA
				tmp = A & 1;
				A >>= 1;
				A |= (tmp << 7);
				setFlag0(0);
				setFlagN(0);
				setFlagH(0);
				setFlagC(tmp);
				clock += 1;
				break;
			case 0x10: // STOP
				//TODO
				clock += 1;
				break;

			case 0x11: // LD DE, d16
				DE = readData(2);
				clock += 3;
				break;
			case 0x12: // LD DE, A
				DE = A;
				clock += 3;
				break;
			case 0x13: // INC DE
				DE++;
				clock += 2;
				break;
			case 0x14: // INC D
				increment(D);
				break;
			case 0x15: // DEC D
				decrement(D);
				break;
			case 0x16: // LD D, d8
				D = readData(1);
				clock += 2;
				break;
			case 0x17: // RLA
				A <<= 1;
				setFlag0(0);
				setFlagN(0);
				setFlagH(0);
				A |= getFlagC();
				setFlagC(0);
				clock += 1;
				break;
			case 0x18: // JR s8
				PC += (char)readData(1);
				//TODO TO CHECK
				clock += 3;
				break;
			case 0x19: // ADD HL, DE
				tmp = HL;
				HL += DE;
				setFlagN(false);
				setFlagH((tmp & 0xFE00) != (HL & 0xFE00));//TODO to check which bit half carry
				setFlagC(tmp > HL);
				clock += 2;
				break;
			case 0x1A: // LD A, (DE)
				A = memory[DE];
				clock += 2;
				break;
			case 0x1B: // DEC DE
				DE--;
				clock += 2;
				break;
			case 0x1C: // INC E
				increment(E);
				break;
			case 0x1D: // DEC E
				decrement(E);
				break;
			case 0x1E: // LD E, d8
				E = readData(1);
				clock += 2;
				break;
			case 0x1F: // RRA
				tmp = A & (1 << 7);
				A >>= 1;
				setFlag0(false);
				setFlagN(false);
				setFlagH(false);
				A |= (getFlagC() << 7);
				clock += 1;
				break;
			case 0x20: // JR NZ, s8
				if (!getFlag0() && clock++)//TODO check if clock
					PC += (char)readData(1);
				clock += 2;
				break;
			case 0x21: // LD HL, d16
				HL = readData(2);
				clock += 3;
				break;
			case 0x22: // LD (HL+), A
				memory[HL++] = A;
				clock += 2;
				break;
			case 0x23: // INC HL
				HL++;
				clock += 2;
				break;
			case 0x24: // INC H
				increment(H);
				break;
			case 0x25: // DEC H
				decrement(H);
				break;
			case 0x26: // LD H, d8
				H = readData(1);
				clock += 2;
				break;
			case 0x27: // DAA
				if (getFlagH() || (!getFlagN() && (value & 0xf) > 9))// TODO to verify
					tmp |= 0x6;
				if (getFlagC() || (getFlagN() && value > 0x99))
				{
					tmp |= 0x60;
					setFlagC(1);
				}
				A = A + (getFlagN() ? -tmp : tmp);
				setFlag0(A == 0);
				setFlagN(0);
				clock += 1;
				break;
			case 0x28: // JR Z, s8
				if (getFlag0() && clock++)
					PC += (char)readData(1);
				clock += 2;
				break;
			case 0x29: // ADD HL, HL
				tmp = HL;
				HL += HL;
				setFlagN(false);
				setFlagH((tmp & 0xFE00) != (HL & 0xFE00));//TODO to check which bit half carry
				setFlagC(tmp > HL);
				clock += 2;
				break;
			case 0x2A: //LD A, (HL-)
				A = memory[HL++];
				clock += 2;
				break;
			case 0x2B: // DEC HL
				HL--;
				clock += 2;
				break;
			case 0x2C: // INC L
				increment(L);
				break;
			case 0x2D: // DEC L
				decrement(L);
				break;
			case 0x2E: // LD L, d8
				L = readData(1);
				clock += 2;
				break;
			case 0x2F: // CCF
				A ^= A;
				setFlagH(1);
				setFlagN(1);
				clock += 1;
				break;
			case 0x30: // JR NC, s8
				if (!getFlagC() && clock++)//TODO check if clock
					PC += (char)readData(1);
				clock += 2;
				break;
			case 0x31: // LD SP, d16
				SP = readData(2);
				clock += 3;
				break;
			case 0x32: // LD (HL-), A
				memory[HL--] = A;
				clock += 2;
				break;
			case 0x33: // INC SP
				SP++;
				clock += 2;
				break;
			case 0x34: // INC (HL)
				increment(memory[HL]);
				clock += 2;
				break;
			case 0x35: // DEC (HL)
				DEC(memory[HL]);
				clock += 2;
				break;
			case 0x36: // LD (HL), d8
				memory[HL] = readData(1);
				clock += 3;
				break;
			case 0x37: // SCF
				setFlagH(0);
				setFlagN(0);
				setFlagC(1);
				clock += 1;
				break;
			case 0x38: //JR C, s8
				if (getFlagC() && clock++)
					PC += (char)readData(1);
				clock += 2;
				break;
			case 0x39: // ADD HL, SP
				tmp = HL;
				HL += SP;
				setFlagN(false);
				setFlagH((tmp & 0xFE00) != (HL & 0xFE00));//TODO to check which bit half carry
				setFlagC(tmp > HL);
				clock += 2;
				break;
			case 0x3A: // LD A, (HL+)
				A = memory[HL--];
				clock += 2;
				break;
			case 0x3B: // DEC SP
				SP--;
				clock += 2;
				clock += ;
				break;
			case 0x3C: // INC A
				increment(A);
				break;
			case 0x3D: // DECREMENT A
				decrement(A);
				break;
			case 0x3E: // LD A, d8
				A = readData(1);
				clock += 2;
				break;
			case 0x3F: // CCF
				setFlagC(!getFlagC());
				setFlagH(0);
				setFlagN(0);
				clock += 1;
				break;
			case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:
			case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4E: case 0x4F:
			case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
			case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5E: case 0x5F:
			case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
			case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6E: case 0x6F:
			case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77:
			case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7E: case 0x7F:
				loadOperation(opcode);
				break;
			case 0x80: //
				clock += ;
				break;
			case 0x81: //
				clock += ;
				break;
			case 0x82: //
				clock += ;
				break;
			case 0x83: //
				clock += ;
				break;
			case 0x84: //
				clock += ;
				break;
			case 0x85: //
				clock += ;
				break;
			case 0x86: //
				clock += ;
				break;
			case 0x87: //
				clock += ;
				break;
			case 0x88: //
				clock += ;
				break;
			case 0x89: //
				clock += ;
				break;
			case 0x8A: //
				clock += ;
				break;
			case 0x8B: //
				clock += ;
				break;
			case 0x8C: //
				clock += ;
				break;
			case 0x8D: //
				clock += ;
				break;
			case 0x8E: //
				clock += ;
				break;
			case 0x8F: //
				clock += ;
				break;
			case 0x90: //
				clock += ;
				break;
			case 0x91: //
				clock += ;
				break;
			case 0x92: //
				clock += ;
				break;
			case 0x93: //
				clock += ;
				break;
			case 0x94: //
				clock += ;
				break;
			case 0x95: //
				clock += ;
				break;
			case 0x96: //
				clock += ;
				break;
			case 0x97: //
				clock += ;
				break;
			case 0x98: //
				clock += ;
				break;
			case 0x99: //
				clock += ;
				break;
			case 0x9A: //
				clock += ;
				break;
			case 0x9B: //
				clock += ;
				break;
			case 0x9C: //
				clock += ;
				break;
			case 0x9D: //
				clock += ;
				break;
			case 0x9E: //
				clock += ;
				break;
			case 0x9F: //
				clock += ;
				break;
			case 0xA0: //
				clock += ;
				break;
			case 0xA1: //
				clock += ;
				break;
			case 0xA2: //
				clock += ;
				break;
			case 0xA3: //
				clock += ;
				break;
			case 0xA4: //
				clock += ;
				break;
			case 0xA5: //
				clock += ;
				break;
			case 0xA6: //
				clock += ;
				break;
			case 0xA7: //
				clock += ;
				break;
			case 0xA8: //
				clock += ;
				break;
			case 0xA9: //
				clock += ;
				break;
			case 0xAA: //
				clock += ;
				break;
			case 0xAB: //
				clock += ;
				break;
			case 0xAC: //
				clock += ;
				break;
			case 0xAD: //
				clock += ;
				break;
			case 0xAE: //
				clock += ;
				break;
			case 0xAF: //
				clock += ;
				break;
			case 0xB0: //
				clock += ;
				break;
			case 0xB1: //
				clock += ;
				break;
			case 0xB2: //
				clock += ;
				break;
			case 0xB3: //
				clock += ;
				break;
			case 0xB4: //
				clock += ;
				break;
			case 0xB5: //
				clock += ;
				break;
			case 0xB6: //
				clock += ;
				break;
			case 0xB7: //
				clock += ;
				break;
			case 0xB8: //
				clock += ;
				break;
			case 0xB9: //
				clock += ;
				break;
			case 0xBA: //
				clock += ;
				break;
			case 0xBB: //
				clock += ;
				break;
			case 0xBC: //
				clock += ;
				break;
			case 0xBD: //
				clock += ;
				break;
			case 0xBE: //
				clock += ;
				break;
			case 0xBF: //
				clock += ;
				break;
			case 0xC0: //
				clock += ;
				break;
			case 0xC1: //
				clock += ;
				break;
			case 0xC2: //
				clock += ;
				break;
			case 0xC3: //
				clock += ;
				break;
			case 0xC4: //
				clock += ;
				break;
			case 0xC5: //
				clock += ;
				break;
			case 0xC6: //
				clock += ;
				break;
			case 0xC7: //
				clock += ;
				break;
			case 0xC8: //
				clock += ;
				break;
			case 0xC9: //
				clock += ;
				break;
			case 0xCA: //
				clock += ;
				break;
			case 0xCB: //
				clock += ;
				break;
			case 0xCC: //
				clock += ;
				break;
			case 0xCD: //
				clock += ;
				break;
			case 0xCE: //
				clock += ;
				break;
			case 0xCF: //
				clock += ;
				break;
			case 0xD0: //
				clock += ;
				break;
			case 0xD1: //
				clock += ;
				break;
			case 0xD2: //
				clock += ;
				break;
			case 0xD3: //
				clock += ;
				break;
			case 0xD4: //
				clock += ;
				break;
			case 0xD5: //
				clock += ;
				break;
			case 0xD6: //
				clock += ;
				break;
			case 0xD7: //
				clock += ;
				break;
			case 0xD8: //
				clock += ;
				break;
			case 0xD9: //
				clock += ;
				break;
			case 0xDA: //
				clock += ;
				break;
			case 0xDB: //
				clock += ;
				break;
			case 0xDC: //
				clock += ;
				break;
			case 0xDD: //
				clock += ;
				break;
			case 0xDE: //
				clock += ;
				break;
			case 0xDF: //
				clock += ;
				break;
			case 0xE0: //
				clock += ;
				break;
			case 0xE1: //
				clock += ;
				break;
			case 0xE2: //
				clock += ;
				break;
			case 0xE3: //
				clock += ;
				break;
			case 0xE4: //
				clock += ;
				break;
			case 0xE5: //
				clock += ;
				break;
			case 0xE6: //
				clock += ;
				break;
			case 0xE7: //
				clock += ;
				break;
			case 0xE8: //
				clock += ;
				break;
			case 0xE9: //
				clock += ;
				break;
			case 0xEA: //
				clock += ;
				break;
			case 0xEB: //
				clock += ;
				break;
			case 0xEC: //
				clock += ;
				break;
			case 0xED: //
				clock += ;
				break;
			case 0xEE: //
				clock += ;
				break;
			case 0xEF: //
				clock += ;
				break;
			case 0xF0: //
				clock += ;
				break;
			case 0xF1: //
				clock += ;
				break;
			case 0xF2: //
				clock += ;
				break;
			case 0xF3: //
				clock += ;
				break;
			case 0xF4: //
				clock += ;
				break;
			case 0xF5: //
				clock += ;
				break;
			case 0xF6: //
				clock += ;
				break;
			case 0xF7: //
				clock += ;
				break;
			case 0xF8: //
				clock += ;
				break;
			case 0xF9: //
				clock += ;
				break;
			case 0xFA: //
				clock += ;
				break;
			case 0xFB: //
				clock += ;
				break;
			case 0xFC: //
				clock += ;
				break;
			case 0xFD: //
				clock += ;
				break;
			case 0xFE: //
				clock += ;
				break;
			case 0xFF: //
				clock += ;
	break;
			default:
				break;
		}
	}
	else if ((opcode & 0xCB00) == 0xCB)
	{
		std::cout << std::setfill('0') << "Doing opcode: 0x" << std::setw(4) << std::hex << +opcode << std::endl;
		opcode >>= 8;
	}
	printRegisters();
	PC++;
}

void Emulator::printRegisters()
{
	std::cout << std::hex << "A: " << +A << " B: " << +B << std::endl;
	std::cout << "C: " << +C << " D: " << +D << std::endl;
	std::cout << "E: " << +E << " F: " << +F << std::endl;
	std::cout << "H: " << +H << " L: " << +L << std::endl;
	std::cout << "BC: " << +BC << "  DE: " << +DE << std::endl;
	std::cout << "HL: " << +HL << "  A: " << +A << std::endl;
	std::cout << "F: " << +F << std::endl << std::endl;
}
