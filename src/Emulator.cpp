/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Emulator.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nathan <unkown@noaddress.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/11 03:22:40 by nathan            #+#    #+#             */
/*   Updated: 2021/04/04 23:58:26 by nathan           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Emulator.hpp"
#include <iostream> //to remove, debug
#include "Operations.tpp"
#include <iomanip>
#include <vector>
#include <algorithm>
#include <unistd.h>

unsigned char Emulator::memory[16 * 1024 * 1024] = {};
unsigned char Emulator::stack[64] = {};
unsigned short Emulator::PC = 0x100;
unsigned short Emulator::SP = 0;
unsigned char Emulator::registers[8] = {};
unsigned long Emulator::clock = 0;
unsigned int Emulator::clockCount = 0;

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

bool Emulator::IME = false;

void	Emulator::increment(unsigned char& reg)
{
	unsigned char tmp = reg & 0xF0;

	reg++;
	setFlag0(reg == 0);
	setFlagN(false);
	setFlagH(tmp != (reg & 0xF0));
	clock += 1;
}

void	Emulator::decrement(unsigned char& reg)
{
	setFlagH((reg & 0xF) - 1 < 0);
	reg--;
	setFlag0(reg == 0);
	setFlagN(true);
	clock += 1;
}

unsigned short Emulator::pop()
{
	unsigned short ret;

	ret = memory[SP++];
	ret |= ((unsigned short)memory[SP++] << 8);
	clock += 3;
	return (ret);
}

void Emulator::push(unsigned short reg)
{
	SP--;
	memory[SP] = (unsigned char)(reg >> 8);
	SP--;
	memory[SP] = (unsigned char)(reg & 0xFF);
	clock += 3;
}

void Emulator::writeByte(unsigned short src, unsigned char data)
{
	memory[src] = data;
	if (src == 0xFF44)// tmp debug
	{
		std::cout << "data in FF44: " << data << std::endl;
		memory[0xFF44] = 0;
	}
	if (src == 0xFF41)// tmp debug
		std::cout << "data in FF41: " << data << std::endl;
	if (src == 0xFF04)
	{
		std::cout << "reset 0xFF04 clock" << std::endl;
		memory[0xFF04] = 0;
	}
	if (src == 0xFF02 && data == 0x81)
		std::cout << (char)memory[0xFF01];//<< std::endl;
}

unsigned char*	Emulator::getSource(unsigned char opcode)
{
	unsigned char*	src;
	if ((opcode & 0x7) == 0x7)
		src = &A;
	if ((opcode & 0x7) == 0x0)
		src = &B;
	if ((opcode & 0x7) == 0x1)
		src = &C;
	if ((opcode & 0x7) == 0x2)
		src = &D;
	if ((opcode & 0x7) == 0x3)
		src = &E;
	if ((opcode & 0x7) == 0x4)
		src = &H;
	if ((opcode & 0x7) == 0x5)
		src = &L;
	if ((opcode & 0x7) == 0x6)
	{
		src = &readAddr(HL);
		clock += 1;
	}
	return (src);
}

unsigned char* Emulator::getSource2(unsigned char opcode)
{
	unsigned char*	src;
	if ((opcode & 0x7) == 0x7)
		src = &A;
	if ((opcode & 0x7) == 0x0)
		src = &B;
	if ((opcode & 0x7) == 0x1)
		src = &C;
	if ((opcode & 0x7) == 0x2)
		src = &D;
	if ((opcode & 0x7) == 0x3)
		src = &E;
	if ((opcode & 0x7) == 0x4)
		src = &H;
	if ((opcode & 0x7) == 0x5)
		src = &L;
	if ((opcode & 0x7) == 0x6)
	{
		src = &memory[HL];
		clock += 2;
	}
	return (src);
}

void	Emulator::loadOperation(unsigned char opcode)
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
	{
		dst = &memory[HL];
		clock += 1;
	}
	else if ((iterator += 8) && opcode < iterator)
		dst = &A;
	src = getSource(opcode);
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
	if (option == 0 && (F & (1 << 7)))
		F = F - ((unsigned char)1 << 7);
	else if (option == 1)
		F = F | ((unsigned char)option << 7);
}

void	Emulator::setFlagN(unsigned char option)
{
	if (option == 0 && (F & (1 << 6)))
		F = F - ((unsigned char)1 << 6);
	else if (option == 1)
		F = F | ((unsigned char)option << 6);
}

void	Emulator::setFlagH(unsigned char option)
{
	if (option == 0 && (F & (1 << 5)))
		F = F - ((unsigned char)1 << 5);
	else if (option == 1)
		F = F | ((unsigned char)option << 5);
}

void	Emulator::setFlagC(unsigned char option)
{
	if (option == 0 && (F & (1 << 4)))
		F = F - ((unsigned char)1 << 4);
	else if (option == 1)
		F = F | ((unsigned char)option << 4);
}

unsigned char& Emulator::readAddr(unsigned short addr)
{
	return (memory[addr]);
}

unsigned char Emulator::readByte()
{
	return (memory[PC++]);
}

unsigned short Emulator::readData(unsigned char nb)
{
	unsigned short value;

	value = 0;
	value = readByte();
	if (nb == 2)
		value = value | (readByte() << 8);
	if (nb == 1)
		return ((unsigned char)value);
	return (value);
}

void Emulator::executeOpcode2(unsigned char opcode)
{
	unsigned short tmp;
	unsigned char* src;

	switch (opcode)
	{
		case 0x00: case 0x01: case 0x02: case 0x03: case 0x04: case 0x05: case 0x06: case 0x07: //RLC X
			src = getSource2(opcode);
			tmp = *src;
			setFlagC(tmp >> 7);
			*src <<= 1;
			*src |= getFlagC();
			setFlag0(*src == 0);
			setFlagN(0);
			setFlagH(0);
			clock += 2;
			break;
		case 0x8: case 0x9: case 0xA: case 0xB: case 0xC: case 0xD: case 0xE: case 0xF: // RRC X
			src = getSource2(opcode);
			tmp = *src;
			setFlagC(tmp & 0x1);
			*src >>= 1;
			*src |= (getFlagC() << 7);
			setFlag0(*src == 0);
			setFlagN(0);
			setFlagH(0);
			clock += 2;
			break;
		case 0x10: case 0x11: case 0x12: case 0x13: case 0x14: case 0x15: case 0x16: case 0x17: // RL X
			src = getSource2(opcode);
			tmp = *src;
			*src <<= 1;
			*src |= getFlagC();
			setFlag0(*src == 0);
			setFlagN(0);
			setFlagH(0);
			setFlagC(tmp >> 7);
			clock += 2;
			break;
		case 0x18: case 0x19: case 0x1A: case 0x1B: case 0x1C: case 0x1D: case 0x1E: case 0x1F: //RR X
			src = getSource2(opcode);
			tmp = *src;
			*src >>= 1;
			*src |= (getFlagC() << 7);
			setFlagC(tmp & 0x1);
			setFlag0(*src == 0);
			setFlagN(0);
			setFlagH(0);
			clock += 2;
			break;
		case 0x20: case 0x21: case 0x22: case 0x23: case 0x24: case 0x25: case 0x26: case 0x27: // SLA X
			src = getSource2(opcode);
			tmp = *src;
			setFlagC(tmp >> 7);
			*src <<= 1;
			setFlag0(*src == 0);
			setFlagN(0);
			setFlagH(0);
			clock += 2;
			break;
		case 0x28: case 0x29: case 0x2A: case 0x2B: case 0x2C: case 0x2D: case 0x2E: case 0x2F: // SRA X
			src = getSource2(opcode);
			tmp = *src;
			*src >>= 1;
			*src |= (tmp & (1 << 7));
			setFlagC(tmp & 0x1);
			setFlag0(*src == 0);
			setFlagN(0);
			setFlagH(0);
			clock += 2;
			break;
		case 0x30: case 0x31: case 0x32: case 0x33: case 0x34: case 0x35: case 0x36: case 0x37: // SWAP X
			src = getSource2(opcode);
			tmp = *src;
			*src <<= 4;
			*src |= (tmp >> 4);
			setFlag0(*src == 0);
			setFlagN(0);
			setFlagH(0);
			setFlagC(0);
			clock += 2;
			break;
		case 0x38: case 0x39: case 0x3A: case 0x3B: case 0x3C: case 0x3D: case 0x3E: case 0x3F: //SRL X
			src = getSource2(opcode);
			tmp = *src;
			*src >>= 1;
			setFlagC(tmp & 0x1);
			setFlag0(*src == 0);
			setFlagN(0);
			setFlagH(0);
			clock += 2;
			break;
		case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47: //BIT 0, X
			src = getSource2(opcode);
			setFlag0((*src & 1) == 0);
			setFlagN(0);
			setFlagH(1);
			clock += 2;
			break;
		case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4E: case 0x4F: //BIT 1, X
			src = getSource2(opcode);
			setFlag0((*src & (1 << 1)) == 0);
			setFlagN(0);
			setFlagH(1);
			clock += 2;
			break;
		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57: //BIT 2, X
			src = getSource2(opcode);
			setFlag0((*src & (1 << 2)) == 0);
			setFlagN(0);
			setFlagH(1);
			clock += 2;
			break;
		case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5E: case 0x5F: //BIT 3, X
			src = getSource2(opcode);
			setFlag0((*src & (1 << 3)) == 0);
			setFlagN(0);
			setFlagH(1);
			clock += 2;
			break;
		case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67: //BIT 4, X
			src = getSource2(opcode);
			setFlag0((*src & (1 << 4)) == 0);
			setFlagN(0);
			setFlagH(1);
			clock += 2;
			break;
		case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6E: case 0x6F: //BIT 5, X
			src = getSource2(opcode);
			setFlag0((*src & (1 << 5)) == 0);
			setFlagN(0);
			setFlagH(1);
			clock += 2;
			break;
		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x76: case 0x77: //BIT 6, X
			src = getSource2(opcode);
			setFlag0((*src & (1 << 6)) == 0);
			setFlagN(0);
			setFlagH(1);
			clock += 2;
			break;
		case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7E: case 0x7F: //BIT 7, X
			src = getSource2(opcode);
			setFlag0((*src & (1 << 7)) == 0);
			setFlagN(0);
			setFlagH(1);
			clock += 2;
			break;
		case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87: //RES 0, X
			src = getSource2(opcode);
			*src = *src & 0b11111110;
			clock += 2;
			break;
		case 0x88: case 0x89: case 0x8A: case 0x8B: case 0x8C: case 0x8D: case 0x8E: case 0x8F: //RES 1, X
			src = getSource2(opcode);
			*src = *src & 0b11111101;
			clock += 2;
			break;
		case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97: // RES 2, X
			src = getSource2(opcode);
			*src = *src & 0b11111011;
			clock += 2;
			break;
		case 0x98: case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D: case 0x9E: case 0x9F: // RES 3, X
			src = getSource2(opcode);
			*src = *src & 0b11110111;
			clock += 2;
			break;
		case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4: case 0xA5: case 0xA6: case 0xA7: // RES 4, X
			src = getSource2(opcode);
			*src = *src & 0b11101111;
			clock += 2;
			break;
		case 0xA8: case 0xA9: case 0xAA: case 0xAB: case 0xAC: case 0xAD: case 0xAE: case 0xAF: // RES 5, X
			src = getSource2(opcode);
			*src = *src & 0b11011111;
			clock += 2;
			break;
		case 0xB0: case 0xB1: case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB6: case 0xB7: // RES 6, X
			src = getSource2(opcode);
			*src = *src & 0b10111111;
			clock += 2;
			break;
		case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC: case 0xBD: case 0xBE: case 0xBF: // RES 7, X
			src = getSource2(opcode);
			*src = *src & 0b01111111;
			clock += 2;
			break;
		case 0xC0: case 0xC1: case 0xC2: case 0xC3: case 0xC4: case 0xC5: case 0xC6: case 0xC7: // SET 0, X
			src = getSource2(opcode);
			*src = *src | 0b1;
			clock += 2;
			break;
		case 0xC8: case 0xC9: case 0xCA: case 0xCB: case 0xCC: case 0xCD: case 0xCE: case 0xCF: // SET 1, X
			src = getSource2(opcode);
			*src = *src | 0b10;
			clock += 2;
			break;
		case 0xD0: case 0xD1: case 0xD2: case 0xD3: case 0xD4: case 0xD5: case 0xD6: case 0xD7: // SET 2, X
			src = getSource2(opcode);
			*src = *src | 0b100;
			clock += 2;
			break;
		case 0xD8: case 0xD9: case 0xDA: case 0xDB: case 0xDC: case 0xDD: case 0xDE: case 0xDF: // SET 3, X
			src = getSource2(opcode);
			*src = *src | 0b1000;
			clock += 2;
			break;
		case 0xE0: case 0xE1: case 0xE2: case 0xE3: case 0xE4: case 0xE5: case 0xE6: case 0xE7: // SET 4, X
			src = getSource2(opcode);
			*src = *src | 0b10000;
			clock += 2;
			break;
		case 0xE8: case 0xE9: case 0xEA: case 0xEB: case 0xEC: case 0xED: case 0xEE: case 0xEF: // SET 5, X
			src = getSource2(opcode);
			*src = *src | 0b100000;
			clock += 2;
			break;
		case 0xF0: case 0xF1: case 0xF2: case 0xF3: case 0xF4: case 0xF5: case 0xF6: case 0xF7: // SET 6, X
			src = getSource2(opcode);
			*src = *src | 0b1000000;
			clock += 2;
			break;
		case 0xF8: case 0xF9: case 0xFA: case 0xFB: case 0xFC: case 0xFD: case 0xFE: case 0xFF: // SET 7, X
			src = getSource2(opcode);
			*src = *src | 0b10000000;
			clock += 2;
			break;
	}
}

void Emulator::executeOpcode(unsigned char opcode)
{
	unsigned char* src;
	unsigned short tmp = 0;
	switch (opcode)
	{
		case 0x00: // NOP
			clock += 1;
			break;
		case 0x01: // LD BC, d16
			BC = readData(2);
			clock += 3;
			break;
		case 0x02: // LD (BC), A
			writeByte(BC, A);
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
			decrement(B);
			break;
		case 0x06: // LD B, d8
			B = readData(1);
			clock += 2;
			break;
		case 0x07: // RLCA
			setFlagC(A >> 7);
			A <<= 1;
			setFlag0(0);
			setFlagH(0);
			setFlagN(0);
			A |= (getFlagC() << 7);
			clock += 1;
			break;
		case 0x08: // LD (a16), SP
			tmp = readData(2);
			writeByte(tmp, SP & 0xFF);
			writeByte(tmp + 1, SP >> 8);
			clock += 5;
			break;
		case 0x09: // ADD HL, BC
			tmp = HL;
			HL += BC;
			setFlagN(0);
			setFlagH((tmp & 0xF000) != (HL & 0xF000));
			setFlagC(tmp > HL);
			clock += 2;
			break;
		case 0x0A: // LD A, (BC)
			A = readAddr(BC);
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
			std::cout << "STOP INSTRUCTION NOT MADE" << std::endl;
			exit(0);
			//TODO
			clock += 1;
			break;

		case 0x11: // LD DE, d16
			DE = readData(2);
			clock += 3;
			break;
		case 0x12: // LD (DE), A
			writeByte(DE, A);
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
			tmp = A;
			A <<= 1;
			setFlag0(0);
			setFlagN(0);
			setFlagH(0);
			A |= getFlagC();
			setFlagC(tmp >> 7);
			clock += 1;
			break;
		case 0x18: // JR s8
			PC = (char)readData(1) + PC;
			clock += 3;
			break;
		case 0x19: // ADD HL, DE
			tmp = HL;
			HL += DE;
			setFlagN(false);
			setFlagH((tmp & 0xF000) != (HL & 0xF000));
			setFlagC(tmp > HL);
			clock += 2;
			break;
		case 0x1A: // LD A, (DE)
			A = readAddr(DE);
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
			tmp = A;
			A >>= 1;
			A |= (getFlagC() << 7);
			setFlag0(0);
			setFlagN(0);
			setFlagH(0);
			setFlagC(tmp & 0x1);
			clock += 1;
			break;
		case 0x20: // JR NZ, s8
			if (!getFlag0())
				PC = (char)readData(1) + PC;
			else
				PC++;
			clock += 2;
			break;
		case 0x21: // LD HL, d16
			HL = readData(2);
			clock += 3;
			break;
		case 0x22: // LD (HL+), A
			writeByte(HL++, A);
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
			if (getFlagH() || (!getFlagN() && (A & 0xf) > 9))
				tmp |= 0x6;
			if (getFlagC() || (!getFlagN() && A > 0x99))
			{
				tmp |= 0x60;
				setFlagC(1);
			}
			A = A + (getFlagN() ? -tmp : tmp);
			setFlag0(A == 0);
			setFlagH(0);
			clock += 1;
			break;
		case 0x28: // JR Z, s8
			if (getFlag0())
				PC = (char)readData(1) + PC;
			else
				PC++;
			clock += 2;
			break;
		case 0x29: // ADD HL, HL
			tmp = HL;
			HL += HL;
			setFlagN(0);
			setFlagH((tmp & 0xF000) != (HL & 0xF000));
			setFlagC(tmp > HL);
			clock += 2;
			break;
		case 0x2A: //LD A, (HL+)
			A = readAddr(HL);
			HL++;
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
		case 0x2F: // CPL
			A ^= A;
			setFlagH(1);
			setFlagN(1);
			clock += 1;
			break;
		case 0x30: // JR NC, s8
			if (!getFlagC())
				PC = (char)readData(1) + PC;
			else
				PC++;
			clock += 2;
			break;
		case 0x31: // LD SP, d16
			SP = readData(2);
			clock += 3;
			break;
		case 0x32: // LD (HL-), A
			writeByte(HL--, A);
			clock += 2;
			break;
		case 0x33: // INC SP
			SP++;
			clock += 2;
			break;
		case 0x34: // INC (HL)
			increment(readAddr(HL));
			clock += 2;
			break;
		case 0x35: // DEC (HL)
			decrement(readAddr(HL));
			clock += 2;
			break;
		case 0x36: // LD (HL), d8
			writeByte(HL, readData(1));
			clock += 3;
			break;
		case 0x37: // SCF
			setFlagH(0);
			setFlagN(0);
			setFlagC(1);
			clock += 1;
			break;
		case 0x38: //JR C, s8
			if (getFlagC())
				PC = (char)readData(1) + PC;
			else
				PC++;
			clock += 2;
			break;
		case 0x39: // ADD HL, SP
			tmp = HL;
			HL += SP;
			setFlagN(false);
			setFlagH((tmp & 0xF000) != (HL & 0xF000));
			setFlagC(tmp > HL);
			clock += 2;
			break;
		case 0x3A: // LD A, (HL-)
			A = readAddr(HL);
			HL--;
			clock += 2;
			break;
		case 0x3B: // DEC SP
			SP--;
			clock += 2;
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
			setFlagN(0);
			setFlagH(0);
			setFlagC(!getFlagC());
			clock += 1;
			break;
		case 0x40: case 0x41: case 0x42: case 0x43: case 0x44: case 0x45: case 0x46: case 0x47:// loads
		case 0x48: case 0x49: case 0x4A: case 0x4B: case 0x4C: case 0x4D: case 0x4E: case 0x4F:
		case 0x50: case 0x51: case 0x52: case 0x53: case 0x54: case 0x55: case 0x56: case 0x57:
		case 0x58: case 0x59: case 0x5A: case 0x5B: case 0x5C: case 0x5D: case 0x5E: case 0x5F:
		case 0x60: case 0x61: case 0x62: case 0x63: case 0x64: case 0x65: case 0x66: case 0x67:
		case 0x68: case 0x69: case 0x6A: case 0x6B: case 0x6C: case 0x6D: case 0x6E: case 0x6F:
		case 0x70: case 0x71: case 0x72: case 0x73: case 0x74: case 0x75: case 0x77:
		case 0x78: case 0x79: case 0x7A: case 0x7B: case 0x7C: case 0x7D: case 0x7E: case 0x7F:
			loadOperation(opcode);
			break;
		case 0x76: // HALT
			clock += 1;//TODO
			break;
		case 0x80: case 0x81: case 0x82: case 0x83: case 0x84: case 0x85: case 0x86: case 0x87: // ADD A, X
			tmp = A;
			A = *getSource(opcode) + A;
			setFlag0(A == 0);
			setFlagN(0);
			setFlagH((tmp & 0xF0) != (A & 0xF0));
			setFlagC(tmp > A);
			clock += 1;
			break;
		case 0x88: case 0x89: case 0x8A: case 0x8B: case 0x8C: case 0x8D: case 0x8E: case 0x8F: // ADC A, X
			tmp = A;
			A = *getSource(opcode) + A + getFlagC();
			setFlag0(A == 0);
			setFlagN(0);
			setFlagH((tmp & 0xF0) != (A & 0xF0));
			setFlagC(tmp > A);
			clock += 1;
			break;
		case 0x90: case 0x91: case 0x92: case 0x93: case 0x94: case 0x95: case 0x96: case 0x97: // SUB A, X
			tmp = A;
			src = getSource(opcode);
			setFlagC(*src > A);
			A = A - *src;
			setFlag0(A == 0);
			setFlagN(1);
			setFlagH((tmp & 0xF) < (*src & 0xF));
			clock += 1;
			break;
		case 0x98: case 0x99: case 0x9A: case 0x9B: case 0x9C: case 0x9D: case 0x9E: case 0x9F: // SBC A, X
			tmp = A;
			src = getSource(opcode);
			setFlagC((*src + getFlagC()) > A);
			A = A - *src - getFlagC();
			setFlag0(A == 0);
			setFlagN(1);
			setFlagH((tmp & 0xF) < (*src & 0xF) + getFlagC());
			setFlagC(tmp > A);
			clock += 1;
			break;
		case 0xA0: case 0xA1: case 0xA2: case 0xA3: case 0xA4: case 0xA5: case 0xA6: case 0xA7: //AND X
			A &= *getSource(opcode);
			setFlag0(A == 0);
			setFlagN(0);
			setFlagH(1);
			setFlagC(0);
			break;
		case 0xA8: case 0xA9: case 0xAA: case 0xAB: case 0xAC: case 0xAD: case 0xAE: case 0xAF: // XOR X
			A ^= *getSource(opcode);
			setFlag0(A == 0);
			setFlagN(0);
			setFlagH(0);
			setFlagC(0);
			break;
		case 0xB0: case 0xB1: case 0xB2: case 0xB3: case 0xB4: case 0xB5: case 0xB6: case 0xB7: // OR X
			A |= *getSource(opcode);
			setFlag0(A == 0);
			setFlagN(0);
			setFlagH(0);
			setFlagC(0);
			break;
		case 0xB8: case 0xB9: case 0xBA: case 0xBB: case 0xBC: case 0xBD: case 0xBE: case 0xBF: // CP X
			src = getSource(opcode);
			setFlag0(*src == A);
			setFlagN(1);
			setFlagH((A & 0xF) < (*src & 0xF));
			setFlagC(*src > A);
			clock += 1;
			break;
		case 0xC0: // RET NZ
			if (!getFlag0())
				PC = pop();
			clock += 2;
			break;
		case 0xC1: // POP BC
			BC = pop();
			break;
		case 0xC2: // JP NZ, a16
			tmp = readData(2);
			if (!getFlag0())
				PC = tmp;
			clock += 3;
			break;
		case 0xC3: // JP a16
			PC = readData(2);
			clock += 4;
			break;
		case 0xC4: // CALL NZ, a16
			tmp = readData(2);
			if (!getFlag0())
			{
				push(PC);
				PC = tmp;
				clock += 3;
			}
			clock += 3;
			break;
		case 0xC5: // PUSH BC
			push(BC);
			clock += 4;
			break;
		case 0xC6: // ADD A, d8
			tmp = A;
			A += readData(1);
			setFlagN(0);
			setFlag0(A == 0);
			setFlagH((tmp & 0xF0) != (A & 0xF0));
			setFlagC(tmp > A);
			clock += 2;
			break;
		case 0xC7: // RST 0 //TODO check
			push(PC);
			PC = (unsigned short)(readAddr(0x00) << 8);
			PC |= (unsigned short)(readAddr(0x00) & 0xff);
			clock += 4;
			break;
		case 0xC8: // RET Z
			if (getFlag0())	
				PC = pop();
			clock += 2;
			break;
		case 0xC9: // RET 
			PC = pop();
			clock += 1; // pop adds 3 to clock already
			break;
		case 0xCA: // JP Z, a16
			if (getFlag0())
				PC = readData(2);
			clock += 3;
			break;
		case 0xCB: // 16 bit opCode
			executeOpcode2(readData(1));
			break;
		case 0xCC: // CALL Z, a16
			tmp = readData(2);
			if (getFlag0())
			{
				push(PC);
				PC = tmp;
				clock += 3;
			}
			clock += 3;
			break;
		case 0xCD: // CALL a16
			tmp = readData(2);
			push(PC);
			PC = tmp;
			clock += 6;
			break;
		case 0xCE: // ADC A, d8
			tmp = A;
			A = A + readData(1) + getFlagC();
			setFlag0(A == 0);
			setFlagN(0);
			setFlagH((tmp & 0xF0) != (A & 0xF0));
			setFlagC(tmp > A);
			clock += 2;
			break;
		case 0xCF: // RST 1
			push(PC);
			PC = (unsigned short)(readAddr(0x00) << 8);
			PC |= (unsigned short)(readAddr(0x08) & 0xff);
			clock += 4;
			break;
		case 0xD0: // RET NC
			if (!getFlagC())
				PC = pop();
			clock += 2;
			break;
		case 0xD1: // POP DE
			DE = pop();
			break;
		case 0xD2: // JP NC, a16
			tmp = readData(2);
			if (!getFlag0())
				PC = tmp;
			clock += 3;
			break;
		case 0xD4: // CALL NC, a16
			tmp = readData(2);
			if (!getFlagC())
			{
				push(PC);
				PC = tmp;
				clock += 3;
			}
			clock += 3;
			break;
		case 0xD5: // PUSH DE
			push(DE);
			clock += 4;
			break;
		case 0xD6: // SUB d8
			tmp = A;
			A -= readData(1);
			setFlagN(1);
			setFlag0(A == 0);
			setFlagH((tmp & 0xF) < (readAddr(PC - 1) & 0xF));
			setFlagC(A > tmp);
			clock += 2;
			break;
		case 0xD7: // RST 2
			push(PC);
			PC = (unsigned short)(readAddr(0x00) << 8);
			PC |= (unsigned short)(readAddr(0x10) & 0xff);
			clock += 4;
			break;
		case 0xD8: // RET C
			if (getFlagC())	
				PC = pop();
			clock += 2;
			break;
		case 0xD9: // RET I
			IME = true;
			PC = pop();
			clock += 4;
			break;
		case 0xDA: // JP C, a16
			if (getFlagC())
				PC = readData(2);
			clock += 3;
			break;
		case 0xDC: // CALL C, a16
			tmp = readData(2);
			if (getFlagC())
			{
				push(PC);
				PC = tmp;
				clock += 3;
			}
			clock += 3;
			break;
		case 0xDE: // SBC A, d8
			tmp = A;
			A = A - readData(1) - getFlagC();
			setFlagN(1);
			setFlag0(A == 0);
			setFlagH((tmp & 0xF) < (readAddr(PC - 1) + getFlagC()));
			setFlagC(A > tmp);
			clock += 2;
			break;
		case 0xDF: // RST 3
			push(PC);
			PC = (unsigned short)(readAddr(0x00) << 8);
			PC |= (unsigned short)(readAddr(0x18) & 0xff);
			clock += 4;
			break;
		case 0xE0: // LD (a8), A
			writeByte(0xFF00 | readData(1), A);
			clock += 3;
			break;
		case 0xE1: // POP HL
			HL = pop();
			break;
		case 0xE2: // LD C, A
			writeByte(0xFF00 | (unsigned short)C, A);
			clock += 2;
			break;
		case 0xE5: // PUSH HL
			push(HL);
			clock += 4;
			break;
		case 0xE6: // AND d8
			tmp = readData(1);
			A &= tmp;
			setFlag0(A == 0);
			setFlagN(0);
			setFlagH(1);
			setFlagC(0);
			clock += 2;
			break;
		case 0xE7: // RST 4
			push(PC);
			PC = (unsigned short)(readAddr(0x00) << 8);
			PC |= (unsigned short)(readAddr(0x20) & 0xff);
			clock += 4;
			break;
		case 0xE8: // ADD SP, s8
			tmp = SP;
			SP += readData(1);
			setFlag0(0);
			setFlagN(0);
			setFlagH((tmp & 0xF0) != (SP & 0xF0));
			setFlagC(tmp < SP);
			clock += 4;
			break;
		case 0xE9: // JP HL
			PC = HL;
			clock += 1;
			break;
		case 0xEA: // LD (a16), A
			writeByte(readData(2), A);
			clock += 4;
			break;
		case 0xEE: // XOR d8
			A ^= readData(1);
			setFlag0(A == 0);
			setFlagN(0);
			setFlagH(0);
			setFlagC(0);
			clock += 2;
			break;
		case 0xEF: // RST 5
			push(PC);
			PC = (unsigned short)(readAddr(0x00) << 8);
			PC |= (unsigned short)(readAddr(0x28) & 0xff);
			clock += 4;
			break;
		case 0xF0: // LD A, (a8) 
			A = readAddr(0xFF00 | (unsigned short)readData(1));// TODO bug PC: C411 routine, i: 16501 : registers ff44 increments in bgb but stays at 0 in my emulator
			clock += 3;
			break;
		case 0xF1: // POP AF
			tmp = pop();
			A = (tmp >> 8);
			F = tmp & 0xF0;
			break;
		case 0xF2: // LD A, C
			A = readAddr(0xFF00 | (unsigned short)C);
			clock += 2;
			break;
		case 0xF3: //DI
			IME = false;
			break;
		case 0xF5: // PUSH AF
			tmp = ((unsigned short)A << 8) | ((unsigned short)F & 0xFF);
			push(tmp);
			clock += 4;
			break;
		case 0xF6: // OR D8
			A |= readData(1);
			setFlag0(A == 0);
			setFlagN(0);
			setFlagH(0);
			setFlagC(0);
			clock += 2;
			break;
		case 0xF7: // RST 6
			push(PC);
			PC = (unsigned short)(readAddr(0x00) << 8);
			PC |= (unsigned short)(readAddr(0x30) & 0xff);
			clock += 4;
			break;
		case 0xF8: // LD HL, SP+s8 //TODO recheck
			short signedTmp;
			tmp = HL;
			signedTmp = (char)readData(1);
			HL = SP + signedTmp;
			setFlag0(0);
			setFlagN(0);
			setFlagH((tmp & 0xF000) != (HL & 0xF000));
			setFlagC(((int)signedTmp + (int)tmp > 0xFFFF || (int)signedTmp + (int)tmp < 0));
			clock += 3;
			break;
		case 0xF9: // LD SP, HL
			SP = HL;
			clock += 2;
			break;
		case 0xFA: // LD A, (a16)
			A = readAddr(readData(2));
			clock += 4;
			break;
		case 0xFB: // EI
			IME = true;
			clock += 1;
			break;
		case 0xFE: // CP d8
			tmp = A - readData(1);
			setFlag0(tmp == 0);
			setFlagN(1);
			setFlagH((A & 0xF) < (readAddr(PC - 1) & 0xF));
			setFlagC(tmp > A);
			clock += 2;
			break;
		case 0xFF: // RST 7
			push(PC);
			PC = (unsigned short)(readAddr(0x00) << 8);
			PC |= (unsigned short)(readAddr(0x38) & 0xff);
			clock += 4;
			break;
		default:
			break;
	}
}

void Emulator::printRegisters()
{
	std::cout << std::setfill('0') << "Current opcode: 0x";
	if (+readAddr(PC) == 0xCB)
		std::cout << std::setw(4) << std::hex << +((readAddr(PC) << 8) + readAddr(PC + 1)) << " with PC: 0x" << std::setw(4) << PC << std::endl;
	else
		std::cout << std::setw(2) << std::hex << +readAddr(PC) << " with PC: 0x" << std::setw(4) << PC << std::endl;

	std::cout << std::setfill('0') << std::uppercase;
	std::cout << "AF 0x" << std::setw(2) << +A << std::setw(2) << +F << std::endl;
	std::cout << "BC: 0x" << std::setw(4) << +BC << std::endl;
	std::cout << "DE: 0x" << std::setw(4) << +DE << std::endl;
	std::cout << "HL: 0x" << std::setw(4) << +HL << std::endl;
	std::cout << "SP: 0x" << std::setw(4) << +SP << std::endl;
	std::cout << "clock: " << std::dec << clock * 2 << std::endl;
	std::cout << "0xff44: " << std::hex << +memory[0xFF44] << std::endl;
	std::cout << std::endl << std::endl;
}

void Emulator::updateClockReg()
{
	unsigned int tmp = clock / 114;

	if (tmp > clockCount)
	{
		clockCount += 1;
		memory[0xFF44]++;
		if (memory[0xFF44] > 153)
			memory[0xFF44] = 0;
	}
	if (clock >= 70224 / 4)
	{
		clock = 0;
		clockCount = 0;
	}
	//memory[0xFF41] |= 
}

void Emulator::launch()
{
	std::vector<unsigned int> debugOpcode;
	std::vector<unsigned int> addresses;

	std::string line;
	BC = 0x0000;
	DE = 0xFF56;
	HL = 0x000D;
	SP = 0xFFFE;
	A = 0x11;
	F = 0x80;
	memory[0xFF44] = 0x90;
	clock = 32916 / 2;
	clockCount = clock / 114;

	//while (std::getline(std::cin, line))
	int i = 0;
	unsigned int opcode;
	while(PC != 0xC7D2)
	{
		i++;
		//if (i >= 16488)
		{
			//std::cout << std::dec << i << ": ";
			//printRegisters();
		}
		opcode = readByte();
		if (opcode == 0xCB)
		{
			opcode = readByte();
			if (std::find(debugOpcode.begin(), debugOpcode.end(), 0xCB00 | opcode) == debugOpcode.end())
			{
				debugOpcode.push_back(0xCB00 | opcode);
				addresses.push_back(PC - 2);
			}
			executeOpcode2(opcode);
		}
		else
		{
			if (std::find(debugOpcode.begin(), debugOpcode.end(), opcode) == debugOpcode.end())
			{
				debugOpcode.push_back(opcode);
				addresses.push_back(PC - 1);
			}
			executeOpcode(opcode);
		}
		updateClockReg();
	}
	for (size_t size = 0; size < debugOpcode.size(); size++)
	{
		//std::cout << std::hex << debugOpcode[size] << " at address: 0x" << addresses[size] << std::endl;
	}
}
