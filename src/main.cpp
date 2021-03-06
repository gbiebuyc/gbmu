/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: nathan <unkown@noaddress.com>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/10 23:59:48 by nathan            #+#    #+#             */
/*   Updated: 2021/03/04 14:11:21 by nathan           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "Emulator.hpp"
#include <iostream>
#include <fstream>

#define ROM_TEST "cpu_instrs/individual/01-special.gb"

int		main(int argc, char* argv[])
{
	std::ifstream file;
	if (argc < 2)
		file = std::ifstream(ROM_TEST, std::ios::binary);
	else
		file = std::ifstream(argv[1], std::ios::binary);
	if (!file.is_open())
	{
		std::cerr << "file not found" << std::endl;
		return 1;
	}
	char*	memory = (char*)Emulator::getMemory();
	file.read(memory, 10 * 1024 * 1024);
	Emulator::launch();
	return 0;
}
