/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Emulator.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nathan <unkown@noaddress.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/11 03:22:43 by nathan            #+#    #+#             */
/*   Updated: 2021/03/10 19:20:07 by nathan           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#ifndef EMULATOR_CLASS_H
# define EMULATOR_CLASS_H

class Emulator {
public:
	static void executeOpcode(unsigned char opcode);
	static void executeOpcode2(unsigned char opcode);
	static unsigned short readData(unsigned char nb);
	static void writeByte(unsigned short addr, unsigned char data);
	static unsigned char readByte();
	static unsigned char& readAddr(unsigned short addr);
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
	static void loadOperation(unsigned char opcode);
	static unsigned char* getSource(unsigned char opcode);
	static unsigned char* getSource2(unsigned char opcode);



	static void updateClockReg();
	static void launch();
	static unsigned char* getMemory(){return memory;};
	static unsigned short pop();
	static void push(unsigned short reg);

private:
	static unsigned char memory[16 * 1024 * 1024];//Rom will be loaded here, testing size
	static unsigned char stack[64]; // TODO  whats the max stack?
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
	static bool IME;
	static unsigned int clockCount;
};

#endif
