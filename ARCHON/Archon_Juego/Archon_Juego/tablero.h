#pragma once
#include <array>
#include <vector>
#include <memory>
#include <SFML/Graphics.hpp>
#include "Pieza.h"

extern sf::RenderWindow* gVentana;

enum class ResultadoVictoria { GanaAzul, GanaRojo, Ninguno };

struct Casilla {
    bool esPuntoPoder = false;
    Pieza* pieza = nullptr;
};

class Tablero {
public:
    static constexpr int TAM = 9;

    Tablero();
    virtual ~Tablero();
    void inicializa();
    void dibuja() const;
    void mueve(double dt);
    void tecla(unsigned char key);

    bool moverPieza(int xOrig, int yOrig, int xDest, int yDest);
    void finalizarTurno();
    ResultadoVictoria comprobarVictoria() const;

    bool ejecutarHechizo(int idHechizo, int x, int y, int x2 = -1, int y2 = -1);
    void resolverCombate(Pieza* perdedor);
    Pieza* getAtacante() const;
    Pieza* getDefensor() const;
    bool hayCombatePendiente() const;

private:
    Casilla matriz[TAM][TAM];
    Equipo turnoActual;
    int contadorTurnos;

    bool esMovimientoLegal(Pieza* p, int xO, int yO, int xD, int yD) const;
    void aplicarCuracion();

    int atacanteX = -1, atacanteY = -1;
    int defensorX = -1, defensorY = -1;

    int contarPiezas(Equipo equipo) const;
    int contarPuntosPoder(Equipo equipo) const;
    bool soloQuedaEncarcelada(Equipo equipo) const;
};