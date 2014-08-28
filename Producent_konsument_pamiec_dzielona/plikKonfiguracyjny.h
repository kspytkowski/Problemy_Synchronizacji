/*
 * plikKonfiguracyjny.h
 *
 *  Created on: 24 maj 2014
 *      Author: krzysztof
 */

#ifndef PLIKKONFIGURACYJNY_H_
#define PLIKKONFIGURACYJNY_H_

#define SENDERS 2 // liczba nadawcow
#define S_TIMES {200000,300000} // czasy miedzy nadaniem kolejnych liter przez kolejnych nadawcow (MIKROSEKUNDY)
#define RECEIVERS 6 // liczba odbiorcow
#define R_TIMES {100,700000,800,40000,500000,600000} // czasy miedzy odbiorem kolejnych liter przez kolejnych odbiorcow (MIKROSEKUNDY)
#define FILE_TO_READ "doOdczytu" // plik z ktorego nadawcy beda czytac
#define BUFFER 10 // wielkosc bufora cyklicznego (ilosc znakow)

#endif /* PLIKKONFIGURACYJNY_H_ */
