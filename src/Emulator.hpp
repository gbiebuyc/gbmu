/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Emulator.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nathan <unkown@noaddress.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/11 03:22:43 by nathan            #+#    #+#             */
/*   Updated: 2021/02/19 08:42:51 by nathan           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef EMULATOR_CLASS_H
# define EMULATOR_CLASS_H

class Emulator {
public:
	static void executeOpcode(unsigned short opcode);
	static unsigned short readData(unsigned char nb);
	static unsigned char readByte(unsigned short addr = PC);
	static void printRegisters();
	static void setFlag0(unsigned char option = 2);
	static void setFlagN(unsigned char option = 2);
	static void setFlagH(unsigned char option = 2);
	static void setFlagC(unsigned char option = 2);
	static bool getFlag0();
	static bool getFlagN();
	static bool getFlagH();
	static bool getFlagC();
	static void increment(unsigned char& reg);
	static void decrement(unsigned char& reg);
	static void loadOperation(unsigned short opcode);

private:
	static unsigned char memory[64];//Rom will be loaded here, testing size
	static unsigned short PC;
	static unsigned short SP;
	static unsigned char registers[8];
	static unsigned long clock;
	static unsigned char& A;// reference to registers char
	static unsigned char& B;
	static unsigned char& C;
	static unsigned char& D;
	static unsigned char& E;
	static unsigned char& F;
	static unsigned char& H;
	static unsigned char& L;

	static unsigned short& BC; //reference to registers chars
	static unsigned short& DE;
	static unsigned short& HL;
};

#endif
