/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Emulator.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nathan <unkown@noaddress.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/11 03:22:40 by nathan            #+#    #+#             */
/*   Updated: 2021/02/11 05:30:26 by nathan           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Emulator.hpp"
#include <iostream> //to remove, debug
#include "Operations.tpp"

unsigned char Emulator::memory[64] = {};
unsigned short Emulator::PC = 0;
unsigned short Emulator::SP = 0;
unsigned char Emulator::registers[8] = {};
unsigned long Emulator::clock = 0;

unsigned char& Emulator::A = registers[7];
unsigned char& Emulator::B = registers[0];
unsigned char& Emulator::C = registers[1];
unsigned char& Emulator::D = registers[2];
unsigned char& Emulator::E = registers[3];
unsigned char& Emulator::F = registers[4];
unsigned char& Emulator::H = registers[5];
unsigned char& Emulator::L = registers[6];

unsigned short& Emulator::BC = *reinterpret_cast<unsigned short*>(&B);
unsigned short& Emulator::DE = *reinterpret_cast<unsigned short*>(&D);
unsigned short& Emulator::HL = *reinterpret_cast<unsigned short*>(&H);

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
	C = 0x22;// litle endian ;)
	if (opcode >> 8 == 0)
	{
		switch (opcode)
		{
			case 0: // NOP
				clock += 1;
				break;
			case 0x01: // LD BC, d16
				BC = readData(2);
				clock += 3;
				break;
			case 0x02:
				BC = A;
				clock += 3;
				break;
			case 0x03:
				load(DE, BC);
			default:
				break;
		}
	}
	else if ((opcode & 0xCB00) == 0xCB)
	{
		opcode >>= 8;
	}
	std::cout << std::hex << BC << std::endl;
	std::cout << std::hex << DE << std::endl;
	PC++;
}
